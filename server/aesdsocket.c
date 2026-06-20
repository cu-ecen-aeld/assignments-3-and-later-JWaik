#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <syslog.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#define PORT     "9000"
#define DATAFILE "/var/tmp/aesdsocketdata"
#define BACKLOG  5
#define BUFSIZE  1024

volatile sig_atomic_t keep_running = 1;

void handle_signal(int sig)
{
    keep_running = 0;
}

static int setup_server_socket(void)
{
    struct addrinfo hints, *result, *rp;
    int sfd = -1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    int rc = getaddrinfo(NULL, PORT, &hints, &result);
    if (rc != 0) {
        syslog(LOG_ERR, "getaddrinfo: %s", gai_strerror(rc));
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        // create socket using family/type from getaddrinfo result (fixes AF_INET bug)
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1) continue;

        int opt = 1;
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) break;

        close(sfd);
        sfd = -1;
    }

    freeaddrinfo(result);

    if (sfd == -1) {
        syslog(LOG_ERR, "Failed to bind to port %s", PORT);
        return -1;
    }

    if (listen(sfd, BACKLOG) == -1) {
        syslog(LOG_ERR, "listen failed");
        close(sfd);
        return -1;
    }

    return sfd;
}

static int handle_client(int client_fd)
{
    FILE *f = fopen(DATAFILE, "a+");
    if (f == NULL) {
        perror("fopen failed");
        return -1;
    }

    char buf[BUFSIZE];
    ssize_t bytes;
    while ((bytes = recv(client_fd, buf, sizeof(buf), 0)) > 0) {
        if (fwrite(buf, 1, bytes, f) < (size_t)bytes) {
            perror("fwrite failed");
            fclose(f);
            return -1;
        }
        if (memchr(buf, '\n', bytes) != NULL)
            break;
    }

    rewind(f);
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0)
        send(client_fd, buf, n, 0);

    fclose(f);
    return 0;
}

int main(int argc, char *argv[])
{
    int daemon_mode = (argc == 2 && strcmp(argv[1], "-d") == 0);

    openlog("aesdsocketdata", LOG_PID | LOG_CONS, LOG_USER);
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // bind before fork so errors are visible to the shell
    int server_fd = setup_server_socket();
    if (server_fd == -1) return EXIT_FAILURE;

    if (daemon_mode) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork failed");
            close(server_fd);
            return EXIT_FAILURE;
        }
        if (pid > 0) return 0;  // parent exits — shell gets control back
        setsid();               // child detaches from terminal
    }

    struct sockaddr_storage client_addr;
    socklen_t addr_len;
    char ip_str[INET6_ADDRSTRLEN];

    while (keep_running) {
        addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd == -1) {
            if (errno == EINTR) break;
            perror("accept");
            break;
        }

        inet_ntop(AF_INET,
                  &((struct sockaddr_in *)&client_addr)->sin_addr,
                  ip_str, sizeof(ip_str));
        syslog(LOG_INFO, "Accepted connection from %s", ip_str);

        if (handle_client(client_fd) == -1)
            syslog(LOG_ERR, "handle_client failed for %s", ip_str);

        syslog(LOG_INFO, "Closed connection from %s", ip_str);
        close(client_fd);
    }

    syslog(LOG_INFO, "Caught signal, exiting");
    remove(DATAFILE);
    close(server_fd);
    closelog();
    return 0;
}
