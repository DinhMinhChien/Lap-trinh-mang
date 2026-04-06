#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {

    if (argc != 4) {
        printf("Usage: %s <port> <file_chao> <file_luu>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    char *fileChao = argv[2];
    char *fileLuu = argv[3];

    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == -1) {
        perror("Không thể tạo socket");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Lỗi bind");
        return 1;
    }

    listen(server, 5);
    printf("Server dang cho o port %d...\n", port);

    while (1) {
        int client = accept(server, NULL, NULL);
        if (client < 0) {
            perror("accept()");
            continue;
        }

        printf("Co client ket noi!\n");

        FILE *f = fopen(fileChao, "r");
        if (f != NULL) {
            char chao[1024];
            fgets(chao, sizeof(chao), f);
            send(client, chao, strlen(chao), 0);
            fclose(f);
        }

        FILE *out = fopen(fileLuu, "a");
        char buffer[1024];
        int n;

        while ((n = recv(client, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[n] = '\0';
            printf("Nhan: %s", buffer);

            if (out != NULL) {
                fputs(buffer, out);
                fflush(out);
            }
        }

        if (out != NULL) fclose(out);
        close(client);
        printf("Client da ngat ket noi\n");
    }

    close(server);
    return 0;
}