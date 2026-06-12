#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVER_HOST "lebavui.io.vn"
#define SERVER_PORT 21
#define BUFFER_SIZE 4096

int connect_socket(const char *host, int port) {
    int sock;
    struct sockaddr_in server_addr;
    struct hostent *server;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Lỗi tạo socket");
        exit(1);
    }

    server = gethostbyname(host);
    if (server == NULL) {
        fprintf(stderr, "Lỗi: Không thể phân giải host %s\n", host);
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memmove((char *)&server_addr.sin_addr.s_addr, (char *)server->h_addr_list[0], server->h_length);
    server_addr.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Lỗi kết nối");
        exit(1);
    }
    return sock;
}

void send_command(int sock, const char *cmd) {
    printf("C->S: %s", cmd);
    send(sock, cmd, strlen(cmd), 0);
}

void recv_response(int sock, char *buffer) {
    memset(buffer, 0, BUFFER_SIZE);
    recv(sock, buffer, BUFFER_SIZE - 1, 0);
    printf("S->C: %s", buffer);
}

int enter_pasv(int control_sock) {
    char buffer[BUFFER_SIZE];
    send_command(control_sock, "PASV\r\n");
    recv_response(control_sock, buffer);

    int h1, h2, h3, h4, p1, p2;
    if (sscanf(buffer, "%*[^(](%d,%d,%d,%d,%d,%d)", &h1, &h2, &h3, &h4, &p1, &p2) != 6) {
        printf("Lỗi phân tích PASV!\n");
        exit(1);
    }

    char ip[32];
    sprintf(ip, "%d.%d.%d.%d", h1, h2, h3, h4);
    int data_port = p1 * 256 + p2;
    
    printf(">> Đang mở Data Socket tới %s:%d\n", ip, data_port);
    return connect_socket(ip, data_port);
}

void reverse_string(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = temp;
    }
}

int main() {
    char buffer[BUFFER_SIZE];
    char cmd[256];
    char question_file[128];
    char answer_file[128];
    char file_content[256] = {0};
    
    char input_user[64];
    char input_pass[64];

    printf("Nhập Username (ví dụ: user_20241234): ");
    fgets(input_user, sizeof(input_user), stdin);
    input_user[strcspn(input_user, "\n")] = 0; 

    printf("Nhập Password: ");
    fgets(input_pass, sizeof(input_pass), stdin);
    input_pass[strcspn(input_pass, "\n")] = 0; 
    printf("--------------------------------------------------\n");

    int control_sock = connect_socket(SERVER_HOST, SERVER_PORT);
    recv_response(control_sock, buffer); 


    sprintf(cmd, "USER %s\r\n", input_user);
    send_command(control_sock, cmd);
    recv_response(control_sock, buffer); 

    sprintf(cmd, "PASS %s\r\n", input_pass);
    send_command(control_sock, cmd);
    recv_response(control_sock, buffer); 

    if (strncmp(buffer, "230", 3) != 0) {
        printf(">> LỖI: Sai tài khoản hoặc mật khẩu!\n");
        close(control_sock);
        return 1;
    }

    int data_sock = enter_pasv(control_sock);
    send_command(control_sock, "NLST\r\n");
    recv_response(control_sock, buffer); 

    char list_buffer[BUFFER_SIZE] = {0};
    recv(data_sock, list_buffer, BUFFER_SIZE - 1, 0); 
    close(data_sock); 
    recv_response(control_sock, buffer); 

    char *ptr = strstr(list_buffer, "question_");
    if (ptr != NULL) {
        sscanf(ptr, "%s", question_file); 
        printf(">> Đã tìm thấy file: %s\n", question_file);
    } else {
        printf(">> Không tìm thấy file question_ trên server!\n");
        close(control_sock);
        return 1;
    }


    data_sock = enter_pasv(control_sock);
    sprintf(cmd, "RETR %s\r\n", question_file);
    send_command(control_sock, cmd);
    recv_response(control_sock, buffer); 

    recv(data_sock, file_content, sizeof(file_content) - 1, 0); 
    close(data_sock);
    recv_response(control_sock, buffer); 
    
    printf(">> Nội dung file gốc: %s\n", file_content);

    char *id_part = question_file + 9;
    sprintf(answer_file, "answer_%s", id_part);
    
    reverse_string(file_content);
    printf(">> Nội dung đảo ngược: %s\n", file_content);
    printf(">> Tên file trả lời: %s\n", answer_file);

    data_sock = enter_pasv(control_sock);
    sprintf(cmd, "STOR %s\r\n", answer_file);
    send_command(control_sock, cmd);
    recv_response(control_sock, buffer); 

    send(data_sock, file_content, strlen(file_content), 0); 
    close(data_sock);
    recv_response(control_sock, buffer); 

    printf(">> Đã upload thành công file %s lên server!\n", answer_file);

    send_command(control_sock, "QUIT\r\n");
    recv_response(control_sock, buffer);
    close(control_sock);

    return 0;
}