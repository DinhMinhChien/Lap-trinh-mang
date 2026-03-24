#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <port> <logfile>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    char *logfile = argv[2];

    int server, client;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    char buffer[BUFFER_SIZE];

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        perror("socket loi");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind loi");
        return 1;
    }

    // Listen
    if (listen(server, 5) < 0) {
        perror("listen loi");
        return 1;
    }

    printf("Server listening on port %d...\n", port);

    while (1) {
        // Chờ client
        client = accept(server, (struct sockaddr*)&client_addr, &addr_len);
        if (client < 0) {
            perror("accept loi");
            continue;
        }

        int bytes = recv(client, buffer, BUFFER_SIZE - 1, 0);

        if (bytes <= 0) {
            close(client);
            continue;
        }

        buffer[bytes] = '\0';

        char *ip = inet_ntoa(client_addr.sin_addr);

        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char time_str[64];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

        FILE *fp = fopen(logfile, "a");
        if (fp == NULL) {
            perror("fopen loi");
            close(client);
            continue;
        }

        fprintf(fp, "%s %s %s\n", ip, time_str, buffer);
        fclose(fp);

        printf("%s %s %s\n", ip, time_str, buffer);

        close(client);
    }

    close(server);
    return 0;
}