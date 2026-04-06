#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8686

int main() {
    int server_fd, client_sock;

    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, 5);

    printf("Server dang cho...\n");

    client_sock = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

    int path_len;
    recv(client_sock, &path_len, sizeof(int), 0);

    char path[path_len + 1];
    recv(client_sock, path, path_len, 0);
    path[path_len] = '\0';

    printf("%s\n", path);

    int count;
    recv(client_sock, &count, sizeof(int), 0);

    for (int i = 0; i < count; i++) {
        int name_len;
        recv(client_sock, &name_len, sizeof(int), 0);

        char name[name_len + 1];
        recv(client_sock, name, name_len, 0);
        name[name_len] = '\0';

        long size;
        recv(client_sock, &size, sizeof(long), 0);

        printf("%s - %ld bytes\n", name, size);
    }

    close(client_sock);
    close(server_fd);
    return 0;
}