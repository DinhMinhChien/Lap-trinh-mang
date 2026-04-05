#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define SERVER_PORT 8686

int main() {
    int sock;
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));

    char path[1024];
    getcwd(path, sizeof(path));

    int path_len = strlen(path);
    send(sock, &path_len, sizeof(int), 0);
    send(sock, path, path_len, 0);

    DIR *dir = opendir(".");
    struct dirent *entry;

    int count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            count++;
        }
    }
    closedir(dir);


    send(sock, &count, sizeof(int), 0);

    dir = opendir(".");
    while ((entry = readdir(dir)) != NULL) {

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        struct stat st;
        stat(entry->d_name, &st);

        int name_len = strlen(entry->d_name);
        long size = st.st_size;

        send(sock, &name_len, sizeof(int), 0);
        send(sock, entry->d_name, name_len, 0);

        send(sock, &size, sizeof(long), 0);
    }

    closedir(dir);
    close(sock);
    return 0;
}