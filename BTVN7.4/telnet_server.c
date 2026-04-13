#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int fd;
    int is_logged_in;
    char username[50]; 
} Client;

void clean_string(char *str) {
    int j = 0;
    char temp[BUFFER_SIZE];
    for (int i = 0; str[i] != '\0'; i++) {
        if (isprint(str[i])) temp[j++] = str[i];
    }
    temp[j] = '\0';
    strcpy(str, temp);
}

int check_auth(char *user, char *pass) {
    FILE *f = fopen("users.txt", "r");
    if (!f) return 0;
    char f_user[50], f_pass[50];
    while (fscanf(f, "%s %s", f_user, f_pass) != EOF) {
        if (strcmp(user, f_user) == 0 && strcmp(pass, f_pass) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_addr.s_addr = htonl(INADDR_ANY), .sin_port = htons(8888) };
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)) || listen(listener, 10)) return 1;

    Client clients[MAX_CLIENTS] = {0};
    fd_set readfds;
    printf("Telnet Server đang chạy trên port 8888...\n");

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(listener, &readfds);
        int max_fd = listener;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd > 0) {
                FD_SET(clients[i].fd, &readfds);
                if (clients[i].fd > max_fd) max_fd = clients[i].fd;
            }
        }

        select(max_fd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(listener, &readfds)) {
            int new_fd = accept(listener, NULL, NULL);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].fd == 0) {
                    clients[i].fd = new_fd;
                    clients[i].is_logged_in = 0;
                    printf("Server: Có kết nối mới (FD: %d)\n", new_fd);
                    send(new_fd, "User Pass:\n", 11, 0);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int fd = clients[i].fd;
            if (fd > 0 && FD_ISSET(fd, &readfds)) {
                char buf[BUFFER_SIZE] = {0};
                int n = recv(fd, buf, sizeof(buf) - 1, 0);

                if (n <= 0) { 
                    if (clients[i].is_logged_in) {
                        printf("Server: Client '%s' (FD: %d) đã thoát.\n", clients[i].username, fd);
                    } else {
                        printf("Server: Một client chưa đăng nhập (FD: %d) đã thoát.\n", fd);
                    }
                    close(fd);  
                    clients[i].fd = 0; 
                    clients[i].is_logged_in = 0;
                    continue;
                }

                clean_string(buf);

                if (!clients[i].is_logged_in) {
                    char u[50], p[50];
                    if (sscanf(buf, "%s %s", u, p) == 2 && check_auth(u, p)) {
                        clients[i].is_logged_in = 1;
                        strcpy(clients[i].username, u);
                        char *success_msg = "Đăng nhập thành công. Nhập lệnh\n";
                        send(fd, success_msg, strlen(success_msg), 0);
                        printf("Server: Client '%s' đã đăng nhập thành công.\n", u);
                    } else {
                        char *fail_msg = "Lỗi đăng nhập. Thử lại:\n";
                        send(fd, fail_msg, strlen(fail_msg), 0);
                    }
                } else {
                    char cmd[BUFFER_SIZE + 50];
                    snprintf(cmd, sizeof(cmd), "%s > out.txt 2>&1", buf);
                    system(cmd);
                    FILE *f = fopen("out.txt", "r");
                    if (f) {
                        char f_buf[BUFFER_SIZE];
                        while (fgets(f_buf, sizeof(f_buf), f)) send(fd, f_buf, strlen(f_buf), 0);
                        fclose(f);
                    }
                    send(fd, "\n--- Xong ---\n", 14, 0);
                }
            }
        }
    }
    return 0;
}