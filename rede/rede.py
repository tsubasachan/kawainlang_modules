import socket


_sockets = {}


def _store(sock):
    fd = sock.fileno()
    _sockets[fd] = sock
    return fd


def _get(fd):
    sock = _sockets.get(fd)
    if sock is None:
        raise RuntimeError(f"Socket TCP inválido: {fd}")
    return sock


def kl_tcp_connect(host, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect((host, int(port)))
        return _store(sock)
    except Exception:
        sock.close()
        return -1


def kl_tcp_bind(host, port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((host, int(port)))
        return _store(sock)
    except Exception:
        sock.close()
        return -1


def kl_tcp_listen(server_socket, backlog):
    try:
        _get(server_socket).listen(int(backlog))
        return 0
    except Exception:
        return -1


def kl_tcp_accept(server_socket):
    try:
        client, _ = _get(server_socket).accept()
        return _store(client)
    except Exception:
        return -1


def kl_tcp_send(socket_fd, data):
    try:
        return _get(socket_fd).send(data.encode("utf-8"))
    except Exception:
        return -1


def kl_tcp_recv(socket_fd, max_bytes):
    try:
        data = _get(socket_fd).recv(int(max_bytes))
        return data.decode("utf-8", errors="replace")
    except Exception:
        return ""


def kl_tcp_close(socket_fd):
    sock = _sockets.pop(socket_fd, None)
    if sock is None:
        return -1
    try:
        sock.close()
        return 0
    except Exception:
        return -1
