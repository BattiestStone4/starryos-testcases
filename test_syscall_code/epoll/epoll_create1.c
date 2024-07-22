#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <assert.h>
#include <string.h>

int main() {
    int pipefd[2];
    
    // 创建一个管道
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }
    
    int read_fd = pipefd[0];
    int write_fd = pipefd[1];
    
    // 使用 EPOLL_CLOEXEC 标志创建一个 epoll 实例
    int epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        return 1;
    }
    
    printf("Epoll file descriptor created: %d\n", epoll_fd);
    
    // 在管道的写入端写入一些数据
    const char *data = "Hello, world!";
    ssize_t bytes_written = write(write_fd, data, strlen(data));
    if (bytes_written == -1) {
        perror("write");
        return 1;
    }
    
    // 关闭管道的写入端
    close(write_fd);
    
    // Fork 一个子进程
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }
    
    if (pid == 0) {
        // 在子进程中
        
        // 尝试在子进程中使用 epoll 实例
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = read_fd;
        int result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, read_fd, &event);
        
        // 期望子进程无法使用 epoll 实例，因为它在父进程中已经被关闭
        assert(result == -1);
        printf("子进程无法使用 epoll 实例，测试通过！\n");
    } else {
        // 在父进程中
        
        // 等待子进程退出
        int status;
        waitpid(pid, &status, 0);
    }
    
    return 0;
}