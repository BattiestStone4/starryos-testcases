#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pthread.h>

#define MAX_EVENTS 10
#define PORT 8080
#define BUF_SIZE 1024
#define MESSAGE "Hello, Server!\n"

void set_nonblocking(int sockfd) {
    int opts = fcntl(sockfd, F_GETFL);
    if (opts < 0) {
        perror("fcntl(F_GETFL)");
        exit(EXIT_FAILURE);
    }
    opts = (opts | O_NONBLOCK);
    if (fcntl(sockfd, F_SETFL, opts) < 0) {
        perror("fcntl(F_SETFL)");
        exit(EXIT_FAILURE);
    }
}

void *server_thread(void *arg) {
    int server_fd, new_socket, epoll_fd, nfds, n;
    struct sockaddr_in address;
    struct epoll_event ev, events[MAX_EVENTS];
    char buffer[BUF_SIZE];
    socklen_t addrlen = sizeof(address);

    // // 设置服务器运行时间(以秒为单位)
    // int server_timeout = 6;
    // time_t start_time = time(NULL);

    printf("Success set up new connection 3\n");

    // 创建套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 设置非阻塞
    set_nonblocking(server_fd);

    // 绑定
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 监听
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // 创建 epoll 实例
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    // 添加服务器套接字到 epoll 实例
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        perror("epoll_ctl: server_fd");
        exit(EXIT_FAILURE);
    }

    while (1) {
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);  // 设置超时为 1000 毫秒
        if (nfds == -1) {
            if (errno == EINTR) {
                continue;  // 如果是中断引起的错误，继续等待
            }
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        // // 检查服务器运行时间是否超过指定的超时时间
        // if (difftime(time(NULL), start_time) > server_timeout) {
        //     printf("Server timeout. Exiting...\n");
        //     break;
        // }

        for (n = 0; n < nfds; ++n) {
            if (events[n].data.fd == server_fd) {
                // 接受新连接

                new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
                if (new_socket == -1) {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }

                // 设置新连接非阻塞
                set_nonblocking(new_socket);

                // 添加新连接到 epoll 实例
                ev.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
                ev.data.fd = new_socket;
                printf("Success set up new connection 6 \n");
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &ev) == -1) {
                    perror("epoll_ctl: new_socket");
                    exit(EXIT_FAILURE);
                }
                printf("Success set up new connection 7\n");
            } else {
                // 处理已连接套接字上的事件
                printf("Success set up new connection 5 \n");

                int sockfd = events[n].data.fd;
                if (events[n].events & EPOLLRDHUP) {
                    printf("EPOLLRDHUP: Client disconnected\n");
                    close(sockfd);
                }
                
                if (events[n].events & EPOLLIN) {
                    int bytes_read;
                    while ((bytes_read = read(sockfd, buffer, BUF_SIZE)) > 0) {
                        write(STDOUT_FILENO, buffer, bytes_read);
                    }
                    if (bytes_read == -1 && errno != EAGAIN) {
                        perror("read");
                        close(sockfd);
                    }
                }
            }
        }
    }

    close(server_fd);
    return NULL;
}

void *client_thread(void *arg) {
    sleep(2);  // 确保服务器先运行
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return NULL;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    printf("Success set up new connection 4 \n");

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return NULL;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return NULL;
    }

    send(sock, MESSAGE, strlen(MESSAGE), 0);
    printf("Message sent\n");
    close(sock);

    // 通知服务器退出
    return NULL;
}

int main() {
    pthread_t server_tid, client_tid;
    
    printf("test start \n");

    // 创建服务器线程
    if (pthread_create(&server_tid, NULL, server_thread, NULL) != 0) {
        perror("Failed to create server thread");
        exit(EXIT_FAILURE);
    }

    // 创建客户端线程
    if (pthread_create(&client_tid, NULL, client_thread, NULL) != 0) {
        perror("Failed to create client thread");
        exit(EXIT_FAILURE);
    }

    // 等待两个线程结束
    pthread_join(server_tid, NULL);
    pthread_join(client_tid, NULL);

    return 0;
}