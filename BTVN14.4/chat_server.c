#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <time.h>

#define MAX_CLIENTS 64
#define BUFFER_SIZE 1024

typedef struct {
    int fd;
    char name[64];
    int registered; 
} ClientInfo;

void get_timestamp(char *buf) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buf, 64, "%Y/%m/%d %I:%M:%S%p", t);
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) { perror("socket"); exit(1); }

    struct sockaddr_in addr = {AF_INET, htons(8888), INADDR_ANY};
    
    int opt = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1);
    }
    listen(listener, 5);

    struct pollfd fds[MAX_CLIENTS];
    ClientInfo clients[MAX_CLIENTS];
    int nfds = 1;

    fds[0].fd = listener;
    fds[0].events = POLLIN;

    printf("Server đang chạy trên port 8888...\n");

    while (1) {
        int ret = poll(fds, nfds, -1);
        if (ret < 0) break;

        for (int i = 0; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == listener) {
                    int client_sk = accept(listener, NULL, NULL);
                    if (nfds < MAX_CLIENTS) {
                        fds[nfds].fd = client_sk;
                        fds[nfds].events = POLLIN;
                        clients[nfds].fd = client_sk;
                        clients[nfds].registered = 0;
                        nfds++;
                        char *msg = "Hay nhap ten theo cu phap 'client_id: client_name':\n";
                        send(client_sk, msg, strlen(msg), 0);
                    } else {
                        close(client_sk);
                    }
                } else {
                    char buf[BUFFER_SIZE];
                    int n = recv(fds[i].fd, buf, sizeof(buf) - 1, 0);
                    if (n <= 0) {
                        close(fds[i].fd);
                        fds[i] = fds[nfds - 1];
                        clients[i] = clients[nfds - 1];
                        nfds--;
                        continue;
                    }
                    buf[n] = '\0';
                    if (buf[strlen(buf) - 1] == '\n') buf[strlen(buf) - 1] = '\0';
                    if (buf[strlen(buf) - 1] == '\r') buf[strlen(buf) - 1] = '\0';

                    if (clients[i].registered == 0) {
                        char name_part[64];
                        if (strncmp(buf, "client_id: ", 11) == 0) {
                            strcpy(clients[i].name, buf + 11);
                            clients[i].registered = 1;
                            send(fds[i].fd, "Dang ky thanh cong!\n", 20, 0);
                        } else {
                            char *err = "Sai cu phap! Nhap lai 'client_id: client_name':\n";
                            send(fds[i].fd, err, strlen(err), 0);
                        }
                    } else {
                        char time_buf[64];
                        char send_buf[2048]; 
                        
                        get_timestamp(time_buf);
                        snprintf(send_buf, sizeof(send_buf), "%s %s: %s\n", time_buf, clients[i].name, buf);

                        for (int j = 1; j < nfds; j++) {
                            if (j != i && clients[j].registered) {
                                send(fds[j].fd, send_buf, strlen(send_buf), 0);
                            }
                        }
                    }
                }
            }
        }
    }
    close(listener);
    return 0;
}