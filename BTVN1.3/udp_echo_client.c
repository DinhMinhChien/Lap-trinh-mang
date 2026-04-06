#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main() {
    int client_sock;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);

    client_sock = socket(AF_INET, SOCK_DGRAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    while (1) {
        
        printf("Nhap message: ");
        fgets(buffer, BUFFER_SIZE, stdin);

        sendto(client_sock, buffer, strlen(buffer), 0,(struct sockaddr*)&server_addr, addr_len);

        int recv_len = recvfrom(client_sock, buffer, BUFFER_SIZE - 1, 0, NULL, NULL);

        buffer[recv_len] = '\0';
        printf("Server tra ve: %s\n", buffer);
    }

    close(client_sock);
    return 0;
}