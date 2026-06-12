#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KL_SQLITE_MAX 1024

static sqlite3* kl_sqlite_handles[KL_SQLITE_MAX];
static char kl_sqlite_last_error[1024] = "";

static void kl_sqlite_set_error(const char* message) {
    if (!message) message = "";
    strncpy(kl_sqlite_last_error, message, sizeof(kl_sqlite_last_error) - 1);
    kl_sqlite_last_error[sizeof(kl_sqlite_last_error) - 1] = '\0';
}

static sqlite3* kl_sqlite_get(int db) {
    if (db <= 0 || db >= KL_SQLITE_MAX) return NULL;
    return kl_sqlite_handles[db];
}

int kl_sqlite_open(const char* path) {
    sqlite3* conn = NULL;
    int rc = sqlite3_open(path, &conn);
    if (rc != SQLITE_OK) {
        kl_sqlite_set_error(conn ? sqlite3_errmsg(conn) : "falha ao abrir sqlite");
        if (conn) sqlite3_close(conn);
        return -1;
    }

    for (int i = 1; i < KL_SQLITE_MAX; i++) {
        if (!kl_sqlite_handles[i]) {
            kl_sqlite_handles[i] = conn;
            kl_sqlite_set_error("");
            return i;
        }
    }

    kl_sqlite_set_error("limite de conexoes sqlite atingido");
    sqlite3_close(conn);
    return -1;
}

int kl_sqlite_close(int db) {
    sqlite3* conn = kl_sqlite_get(db);
    if (!conn) {
        kl_sqlite_set_error("conexao sqlite invalida");
        return -1;
    }
    int rc = sqlite3_close(conn);
    if (rc != SQLITE_OK) {
        kl_sqlite_set_error(sqlite3_errmsg(conn));
        return -1;
    }
    kl_sqlite_handles[db] = NULL;
    kl_sqlite_set_error("");
    return 0;
}

int kl_sqlite_exec(int db, const char* sql) {
    sqlite3* conn = kl_sqlite_get(db);
    if (!conn) {
        kl_sqlite_set_error("conexao sqlite invalida");
        return -1;
    }

    char* err = NULL;
    int rc = sqlite3_exec(conn, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        kl_sqlite_set_error(err ? err : sqlite3_errmsg(conn));
        sqlite3_free(err);
        return -1;
    }

    kl_sqlite_set_error("");
    return 0;
}

char* kl_sqlite_value(int db, const char* sql) {
    sqlite3* conn = kl_sqlite_get(db);
    if (!conn) {
        kl_sqlite_set_error("conexao sqlite invalida");
        return "";
    }

    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        kl_sqlite_set_error(sqlite3_errmsg(conn));
        return "";
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW || sqlite3_column_count(stmt) == 0) {
        sqlite3_finalize(stmt);
        kl_sqlite_set_error("");
        return "";
    }

    const unsigned char* text = sqlite3_column_text(stmt, 0);
    char* result = NULL;
    if (text) {
        size_t len = strlen((const char*)text);
        result = (char*)malloc(len + 1);
        strcpy(result, (const char*)text);
    } else {
        result = (char*)malloc(1);
        result[0] = '\0';
    }

    sqlite3_finalize(stmt);
    kl_sqlite_set_error("");
    return result;
}

static void kl_json_append(char** buffer, size_t* len, size_t* cap, const char* text) {
    size_t add = strlen(text);
    if (*len + add + 1 > *cap) {
        while (*len + add + 1 > *cap) *cap *= 2;
        *buffer = (char*)realloc(*buffer, *cap);
    }
    memcpy(*buffer + *len, text, add);
    *len += add;
    (*buffer)[*len] = '\0';
}

static void kl_json_append_string(char** buffer, size_t* len, size_t* cap, const char* text) {
    kl_json_append(buffer, len, cap, "\"");
    for (const char* p = text ? text : ""; *p; p++) {
        char tmp[3] = {0};
        if (*p == '"' || *p == '\\') {
            tmp[0] = '\\';
            tmp[1] = *p;
            kl_json_append(buffer, len, cap, tmp);
        } else if (*p == '\n') {
            kl_json_append(buffer, len, cap, "\\n");
        } else if (*p == '\r') {
            kl_json_append(buffer, len, cap, "\\r");
        } else if (*p == '\t') {
            kl_json_append(buffer, len, cap, "\\t");
        } else {
            tmp[0] = *p;
            kl_json_append(buffer, len, cap, tmp);
        }
    }
    kl_json_append(buffer, len, cap, "\"");
}

char* kl_sqlite_row_json(int db, const char* sql) {
    sqlite3* conn = kl_sqlite_get(db);
    if (!conn) {
        kl_sqlite_set_error("conexao sqlite invalida");
        char* empty = (char*)malloc(3);
        strcpy(empty, "{}");
        return empty;
    }

    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        kl_sqlite_set_error(sqlite3_errmsg(conn));
        char* empty = (char*)malloc(3);
        strcpy(empty, "{}");
        return empty;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        kl_sqlite_set_error("");
        char* empty = (char*)malloc(3);
        strcpy(empty, "{}");
        return empty;
    }

    size_t cap = 256;
    size_t len = 0;
    char* json = (char*)malloc(cap);
    json[0] = '\0';
    kl_json_append(&json, &len, &cap, "{");

    int cols = sqlite3_column_count(stmt);
    for (int i = 0; i < cols; i++) {
        if (i > 0) kl_json_append(&json, &len, &cap, ", ");
        kl_json_append_string(&json, &len, &cap, sqlite3_column_name(stmt, i));
        kl_json_append(&json, &len, &cap, ": ");

        int type = sqlite3_column_type(stmt, i);
        if (type == SQLITE_NULL) {
            kl_json_append(&json, &len, &cap, "null");
        } else if (type == SQLITE_INTEGER || type == SQLITE_FLOAT) {
            char num[128];
            const unsigned char* text = sqlite3_column_text(stmt, i);
            snprintf(num, sizeof(num), "%s", text ? (const char*)text : "0");
            kl_json_append(&json, &len, &cap, num);
        } else {
            const unsigned char* text = sqlite3_column_text(stmt, i);
            kl_json_append_string(&json, &len, &cap, (const char*)text);
        }
    }

    kl_json_append(&json, &len, &cap, "}");
    sqlite3_finalize(stmt);
    kl_sqlite_set_error("");
    return json;
}

char* kl_sqlite_error(void) {
    return kl_sqlite_last_error;
}
