#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8088

int main() {
    int sock;
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        return 1;
    }

    printf("Connected to server!\n");

    char *messages[] = {
        "MINHMEO12",
        "SOICTSOICT01234",
        "567890123456789",
        "0123456789SOICT",
        "SOICT012345678",
        "9012345678"
    };

    int n = sizeof(messages) / sizeof(messages[0]);
    
    for (int i = 0; i < n; i++) {
        send(sock, messages[i], strlen(messages[i]), 0);
        printf("Sent: %s\n", messages[i]);

        sleep(1); 
    }

    close(sock);
    printf("Done.\n");

    return 0;
}