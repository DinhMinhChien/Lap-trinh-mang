#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage: %s <IP> <Port>\n", argv[0]);
        return 1;
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);

    if (connect(client, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("connect");
        return 1;
    }

    printf("Connected to server %s:%d\n", ip, port);

    char buffer_recv[1024];
    int n = recv(client, buffer_recv, sizeof(buffer_recv) - 1, 0);
    if (n > 0) {
        buffer_recv[n] = '\0';
        printf("Server: %s\n", buffer_recv);
    }

    char buffer_send[1024];
    printf("Nhap du lieu: ");
    fgets(buffer_send, sizeof(buffer_send), stdin);

    send(client, buffer_send, strlen(buffer_send), 0);

    close(client);

    return 0;
}