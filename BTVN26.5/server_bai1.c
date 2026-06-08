#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>

#define SERVER_PORT 8080

void handle_calc_logic(int client_fd) {
    char raw_data[4096] = {0};
    recv(client_fd, raw_data, sizeof(raw_data) - 1, 0);

    char http_method[16], path_info[256];
    sscanf(raw_data, "%s %s", http_method, path_info);

    char operator[16] = "";
    double val1 = 0, val2 = 0;
    int is_valid = 0;

    if (strcmp(http_method, "GET") == 0) {
        char *q_ptr = strchr(path_info, '?');
        if (q_ptr && sscanf(q_ptr + 1, "op=%[^&]&a=%lf&b=%lf", operator, &val1, &val2) == 3) is_valid = 1;
    } 
    else if (strcmp(http_method, "POST") == 0) {
        char *body_ptr = strstr(raw_data, "\r\n\r\n");
        if (body_ptr && sscanf(body_ptr + 4, "op=%[^&]&a=%lf&b=%lf", operator, &val1, &val2) == 3) is_valid = 1;
    }

    char html_output[1024];
    if (is_valid) {
        double calc_res = 0;
        char symbol = '?';
        if (strcmp(operator, "add") == 0) { calc_res = val1 + val2; symbol = '+'; }
        else if (strcmp(operator, "sub") == 0) { calc_res = val1 - val2; symbol = '-'; }
        else if (strcmp(operator, "mul") == 0) { calc_res = val1 * val2; symbol = '*'; }
        else if (strcmp(operator, "div") == 0 && val2 != 0) { calc_res = val1 / val2; symbol = '/'; }
        
        snprintf(html_output, sizeof(html_output), "<html><body><h1>Ket qua: %.2g %c %.2g = %.2g</h1></body></html>", val1, symbol, val2, calc_res);
    } else {
        snprintf(html_output, sizeof(html_output), "<html><body><h2>Thong bao: Sai cu phap. Format: ?op=add&a=10&b=5</h2></body></html>");
    }

    char final_response[2048];
    snprintf(final_response, sizeof(final_response), "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n%s", html_output);
    send(client_fd, final_response, strlen(final_response), 0);
    close(client_fd);
}

int main() {
    signal(SIGCHLD, SIG_IGN); 
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    int enable = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

    struct sockaddr_in server_addr = {AF_INET, htons(SERVER_PORT), INADDR_ANY};
    bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_sock, 5);
    
    printf("Server Calculator dang khoi chay tai port %d...\n", SERVER_PORT);

    while (1) {
        int client_sock = accept(server_sock, NULL, NULL);
        if (fork() == 0) {
            close(server_sock);
            handle_calc_logic(client_sock);
            exit(0);
        }
        close(client_sock);
    }
    return 0;
}