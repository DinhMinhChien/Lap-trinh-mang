#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT 8888
#define DB_FILE "database.txt"

int check_login(char *user, char *pass) {
    FILE *f = fopen(DB_FILE, "r");
    if (f == NULL) return 0;

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

void handle_client(int client_sock) {
    char buf[1024], user[50], pass[50], cmd[1024], sys_cmd[1100];
    
    send(client_sock, "Username: ", 10, 0);
    int len = recv(client_sock, buf, sizeof(buf) - 1, 0);
    buf[len] = '\0'; sscanf(buf, "%s", user);

    send(client_sock, "Password: ", 10, 0);
    len = recv(client_sock, buf, sizeof(buf) - 1, 0);
    buf[len] = '\0'; sscanf(buf, "%s", pass);

    if (!check_login(user, pass)) {
        send(client_sock, "Login failed. Bye!\n", 19, 0);
        close(client_sock);
        exit(0);
    }

    send(client_sock, "Login success! Enter commands:\n", 31, 0);

    while (1) {
        send(client_sock, "$ ", 2, 0);
        len = recv(client_sock, buf, sizeof(buf) - 1, 0);
        if (len <= 0) break;

        buf[len] = '\0';
        if (buf[len-1] == '\n') buf[len-1] = '\0';
        if (buf[len-2] == '\r') buf[len-2] = '\0';

        if (strlen(buf) <= 0) continue;

        sprintf(sys_cmd, "%s > out.txt 2>&1", buf);
        system(sys_cmd);
        FILE *fout = fopen("out.txt", "r");
        if (fout) {
            while (fgets(cmd, sizeof(cmd), fout) != NULL) {
                send(client_sock, cmd, strlen(cmd), 0);
            }
            fclose(fout);
        }
    }

    close(client_sock);
    exit(0); 
}

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    bind(server_sock, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_sock, 5);

    printf("Server telnet dang chay tai port %d...\n", PORT);

    signal(SIGCHLD, SIG_IGN);

    while (1) {
        int client_sock = accept(server_sock, NULL, NULL);
        printf("Co ket noi moi!\n");

        if (fork() == 0) {
            close(server_sock); 
            handle_client(client_sock);
        } else {
            close(client_sock); 
        }
    }

    return 0;
}