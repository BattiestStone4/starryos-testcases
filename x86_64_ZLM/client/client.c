#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define SERVER_IP "10.3.10.10"
#define SERVER_PORT 5555
#define FILE_PATH "/www/live/test/1970-01-01/00/00-00_5.ts"

void send_file(const char *file_path, int sockfd) {
    int filefd = open(file_path, O_RDONLY);
    if (filefd < 0) {
        perror("Error opening file");
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(filefd, buffer, sizeof(buffer))) > 0) {
        if (send(sockfd, buffer, bytes_read, 0) == -1) {
            perror("Error sending file");
            close(filefd);
            return;
        }
    }

    if (bytes_read < 0) {
        perror("Error reading file");
    }

    close(filefd);
}

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    send_file(FILE_PATH, sockfd);

    close(sockfd);
    return 0;
}

