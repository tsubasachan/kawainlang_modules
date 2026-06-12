import sqlite3


_connections = {}
_next_id = 1
_last_error = ""


def _set_error(message):
    global _last_error
    _last_error = str(message)


def _get_connection(db):
    conn = _connections.get(int(db))
    if conn is None:
        raise RuntimeError(f"Conexao SQLite invalida: {db}")
    return conn


def kl_sqlite_open(path):
    global _next_id
    try:
        conn = sqlite3.connect(path)
        conn.row_factory = sqlite3.Row
        db_id = _next_id
        _next_id += 1
        _connections[db_id] = conn
        _set_error("")
        return db_id
    except Exception as exc:
        _set_error(exc)
        return -1


def kl_sqlite_close(db):
    conn = _connections.pop(int(db), None)
    if conn is None:
        _set_error(f"Conexao SQLite invalida: {db}")
        return -1
    try:
        conn.close()
        _set_error("")
        return 0
    except Exception as exc:
        _set_error(exc)
        return -1


def kl_sqlite_exec(db, sql):
    try:
        conn = _get_connection(db)
        conn.execute(sql)
        conn.commit()
        _set_error("")
        return 0
    except Exception as exc:
        _set_error(exc)
        return -1


def kl_sqlite_value(db, sql):
    try:
        conn = _get_connection(db)
        row = conn.execute(sql).fetchone()
        _set_error("")
        if row is None or len(row) == 0 or row[0] is None:
            return ""
        return str(row[0])
    except Exception as exc:
        _set_error(exc)
        return ""


def kl_sqlite_row_json(db, sql):
    try:
        conn = _get_connection(db)
        cursor = conn.execute(sql)
        row = cursor.fetchone()
        _set_error("")
        if row is None:
            return "{}"
        keys = row.keys()
        parts = []
        for key in keys:
            parts.append(f"{_json_string(str(key))}: {_json_value(row[key])}")
        return "{" + ", ".join(parts) + "}"
    except Exception as exc:
        _set_error(exc)
        return "{}"


def kl_sqlite_error():
    return _last_error


def _json_string(value):
    escaped = (
        value.replace("\\", "\\\\")
        .replace('"', '\\"')
        .replace("\n", "\\n")
        .replace("\r", "\\r")
        .replace("\t", "\\t")
    )
    return '"' + escaped + '"'


def _json_value(value):
    if value is None:
        return "null"
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, (int, float)):
        return str(value)
    return _json_string(str(value))
