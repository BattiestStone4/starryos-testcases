#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define SHM_PATH "/dev/shm/test_file.txt"
#define BUFFER_SIZE 100

int main() {
    int fd;
    char buffer[BUFFER_SIZE];
    const char *test_data = "This is a test file created in /dev/shm.\n";

    // 创建并打开文件
    fd = open(SHM_PATH, O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    // 写入文本
    ssize_t bytes_written = write(fd, test_data, strlen(test_data));
    if (bytes_written == -1) {
        perror("Error writing to file");
        close(fd);
        exit(EXIT_FAILURE);
    }

    printf("Successfully wrote %zd bytes to %s\n", bytes_written, SHM_PATH);

    // 将文件指针移回文件开头
    if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("Error seeking file");
        close(fd);
        return 1;
    }

    // 读取数据
    ssize_t bytes_read = read(fd, buffer, BUFFER_SIZE - 1);
    if (bytes_read == -1) {
        perror("Error reading from file");
        close(fd);
        return 1;
    }

    buffer[bytes_read] = '\0';  // 确保字符串正确终止

    // 输出读取的数据
    printf("Read from /dev/shm: %s", buffer);

    // 关闭文件
    close(fd);

    printf("File closed successfully. You can check the contents using 'cat %s'\n", SHM_PATH);

    return 0;
}
