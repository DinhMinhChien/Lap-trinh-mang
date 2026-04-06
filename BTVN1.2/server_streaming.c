#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8088
#define TARGET "0123456789"
#define TARGET_LEN 10

int count_target_occurrences(char *data) {
    int count = 0;
    int len = strlen(data);

    for (int i = 0; i <= len - TARGET_LEN; i++) {
        if (strncmp(data + i, TARGET, TARGET_LEN) == 0) {
            count++;
        }
    }
    return count;
}

int main() {
    int server, client;
    struct sockaddr_in server_addr, client_addr;

    socklen_t client_len = sizeof(client_addr);

    char buffer[1024];
    char prev[20] = "";
    int total_count = 0;

    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (struct sockaddr*)&server_addr, sizeof(server_addr));

    listen(server, 5);
    printf("Server listening on port %d...\n", PORT);

    client = accept(server, (struct sockaddr*)&client_addr, &client_len);
    printf("Client connected!\n");

    while (1) {

        memset(buffer, 0, sizeof(buffer));

        int bytes = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;

        buffer[bytes] = '\0';

        char temp[2048];

        strcpy(temp, prev);
        strcat(temp, buffer);

        int found = count_target_occurrences(temp);
        total_count += found;

        printf("Nhan du lieu: %s\n", buffer);
        printf("Tim thay %d lan, Tong = %d\n\n", found, total_count);

        int len = strlen(temp);
        
        if (len >= TARGET_LEN - 1) {
            strncpy(prev, temp + len - (TARGET_LEN - 1), TARGET_LEN - 1);
            prev[TARGET_LEN - 1] = '\0';
        } else {
            strcpy(prev, temp);
        }
    }

    close(client);
    close(server);

    return 0;
}