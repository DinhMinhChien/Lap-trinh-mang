#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main() {

    int server_sock;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    printf("UDP Server dang chay tren port 8080...\n");

    while (1) {

        int recv_len = recvfrom(server_sock, buffer, BUFFER_SIZE - 1, 0,(struct sockaddr*)&client_addr, &addr_len);

        buffer[recv_len] = '\0';
        printf("Server nhan duoc: %s\n", buffer);
        sendto(server_sock, buffer, strlen(buffer), 0, (struct sockaddr*)&client_addr, addr_len);
    }

    close(server_sock);
    return 0;
}