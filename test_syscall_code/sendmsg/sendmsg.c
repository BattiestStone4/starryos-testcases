#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 5555
#define BUFFER_SIZE 1024

int main() {
    int sockfd, connfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[BUFFER_SIZE];
    struct msghdr msg = {0};
    struct iovec iov[1];
    char send_buf[] = "Hello, world!";
    char recv_buf[BUFFER_SIZE];
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
    
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    if (listen(sockfd, 5) < 0) {
        perror("listen failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d\n", PORT);
    socklen_t len = sizeof(cliaddr);
    if ((connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len)) < 0) {
        perror("server accept failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    iov[0].iov_base = send_buf;
    iov[0].iov_len = sizeof(send_buf);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    
    if (sendmsg(connfd, &msg, 0) < 0) {
        perror("sendmsg failed");
        close(connfd);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    printf("Message sent\n");
    ssize_t n = recv(connfd, recv_buf, sizeof(recv_buf) - 1, 0);
    if (n < 0) {
        perror("recv failed");
        close(connfd);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    recv_buf[n] = '\0';  // Null-terminate the received data
    printf("Message received: %s\n", recv_buf);
    close(connfd);
    close(sockfd);
    return 0;

}
