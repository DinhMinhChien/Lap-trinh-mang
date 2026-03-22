#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in server_addr;

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client == -1) {
        perror("socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    if (connect(client, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        close(client);
        return 1;
    }

    printf("Connected to server %s:%d\n", server_ip, port);

    char mssv[50], hoTen[100], ngaySinh[50];
    float diemTB;

    printf("MSSV: ");
    scanf("%s", mssv);

    printf("Ho ten: ");
    getchar();
    fgets(hoTen, sizeof(hoTen), stdin);
    hoTen[strcspn(hoTen, "\n")] = 0;

    printf("Ngay sinh (YYYY-MM-DD): ");
    scanf("%s", ngaySinh);

    printf("Diem TB: ");
    scanf("%f", &diemTB);

    char buffer[BUFFER_SIZE];
    snprintf(buffer, sizeof(buffer), "%s %s %s %.2f", mssv, hoTen, ngaySinh, diemTB);

    send(client, buffer, strlen(buffer), 0);

    printf("Da gui du lieu len server!\n");

    close(client);
    return 0;
}