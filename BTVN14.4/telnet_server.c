#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <ctype.h>

#define MAX_CLIENTS 64
#define BUFFER_SIZE 1024

typedef struct {
    int fd;
    int logged_in; 
} ClientInfo;

void clean_string(char *str) {
    int i, j = 0;
    char temp[BUFFER_SIZE];
    for (i = 0; str[i] != '\0'; i++) {
        if (isprint((unsigned char)str[i])) {
            temp[j++] = str[i];
        }
    }
    temp[j] = '\0';
    char *start = temp;
    while(isspace((unsigned char)*start)) start++;
    if(*start == 0) { str[0] = '\0'; return; }
    char *end = start + strlen(start) - 1;
    while(end > start && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
    strcpy(str, start);
}

int check_login(char *credentials) {
    FILE *f = fopen("database.txt", "r");
    if (f == NULL) return 0;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strcmp(line, credentials) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {AF_INET, htons(9999), INADDR_ANY};
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

    printf("Telnet Server đang chạy ở cổng 9999...\n");

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
                        clients[nfds].logged_in = 0;
                        nfds++;
                        send(client_sk, "Nhap user pass:\n", 16, 0);
                    } else close(client_sk);
                } else {
                    char buf[BUFFER_SIZE];
                    int n = recv(fds[i].fd, buf, sizeof(buf) - 1, 0);
                    if (n <= 0) {
                        printf("Socket %d ngắt kết nối.\n", fds[i].fd);
                        close(fds[i].fd);
                        fds[i] = fds[nfds - 1];
                        clients[i] = clients[nfds - 1];
                        nfds--; i--; continue;
                    }
                    buf[n] = '\0';
                    clean_string(buf);

                    if (clients[i].logged_in == 0) {
                        if (check_login(buf)) {
                            clients[i].logged_in = 1;
                            send(fds[i].fd, "OK! Nhap lenh:\n", 15, 0);
                        } else send(fds[i].fd, "Sai! Nhap lai:\n", 15, 0);
                    } else {
                        if (strlen(buf) == 0) continue;

                        printf("Socket %d thuc thi: [%s]\n", fds[i].fd, buf);
                        
                        char cmd[2048];
                        const char *out_file = "out.txt";
                        FILE *f_log = fopen(out_file, "a");
                        if (f_log) {
                            fprintf(f_log, "\n--- Command: %s ---\n", buf);
                            fclose(f_log);
                        }
                        char tmp_file[64];
                        sprintf(tmp_file, "tmp_%d.txt", fds[i].fd);
                        snprintf(cmd, sizeof(cmd), "%s > %s 2>&1 && cat %s >> %s", buf, tmp_file, tmp_file, out_file);

                        system(cmd);
                        FILE *f_tmp = fopen(tmp_file, "r");
                        if (f_tmp) {
                            char res[BUFFER_SIZE];
                            while (fgets(res, sizeof(res), f_tmp)) {
                                send(fds[i].fd, res, strlen(res), 0);
                            }
                            fclose(f_tmp);
                            remove(tmp_file);
                        }
                        send(fds[i].fd, "\nNext command:\n", 15, 0);
                    }
                }
            }
        }
    }
    return 0;
}