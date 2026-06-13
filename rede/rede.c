#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <sys/select.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int kl_tcp_resolve(const char* host, int port, struct sockaddr_in* out) {
    if (!host || !out) return -1;

    memset(out, 0, sizeof(*out));
    out->sin_family = AF_INET;
    out->sin_port = htons((uint16_t)port);

    if (inet_pton(AF_INET, host, &out->sin_addr) == 1) return 0;

    struct hostent* entry = gethostbyname(host);
    if (!entry || entry->h_addrtype != AF_INET || !entry->h_addr_list[0]) {
        return -1;
    }

    memcpy(&out->sin_addr, entry->h_addr_list[0], sizeof(struct in_addr));
    return 0;
}

int kl_tcp_connect(const char* host, int port) {
    struct sockaddr_in addr;
    if (kl_tcp_resolve(host, port, &addr) != 0) return -1;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        close(fd);
        return -1;
    }

    return fd;
}

int kl_tcp_bind(const char* host, int port) {
    struct sockaddr_in addr;
    if (kl_tcp_resolve(host, port, &addr) != 0) return -1;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        close(fd);
        return -1;
    }

    return fd;
}

int kl_tcp_listen(int server_socket, int backlog) {
    return listen(server_socket, backlog) == 0 ? 0 : -1;
}

int kl_tcp_accept(int server_socket) {
    return accept(server_socket, NULL, NULL);
}

int kl_tcp_send(int socket_fd, const char* data) {
    if (!data) return -1;
    ssize_t sent = send(socket_fd, data, strlen(data), 0);
    return sent < 0 ? -1 : (int)sent;
}

char* kl_tcp_recv(int socket_fd, int max_bytes) {
    if (max_bytes <= 0) max_bytes = 1;

    char* buffer = (char*)malloc((size_t)max_bytes + 1);
    if (!buffer) return "";

    ssize_t received = recv(socket_fd, buffer, (size_t)max_bytes, 0);
    if (received < 0) {
        free(buffer);
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    buffer[received] = '\0';
    return buffer;
}

int kl_tcp_close(int socket_fd) {
    return close(socket_fd) == 0 ? 0 : -1;
}

int kl_tcp_selecionar(int socket_fd, int timeout_ms) {
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(socket_fd, &read_fds);

    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int activity = select(socket_fd + 1, &read_fds, NULL, NULL, &tv);
    if (activity < 0) return -1;
    if (activity == 0) return 0;
    return FD_ISSET(socket_fd, &read_fds) ? 1 : 0;
}
