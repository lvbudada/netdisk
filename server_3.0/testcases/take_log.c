#include <func.h>

int main()
{
    int fd = open("log.log", O_WRONLY|O_APPEND|O_CREAT, 0666);
    ERROR_CHECK(fd ,-1, "open1");
    write(fd, "helloworld\n", 11);
    close(fd);
    fd = open("log.log", O_WRONLY|O_APPEND|O_CREAT, 0666);
    write(fd, "nihao\n", 5);
    close(fd);
    return 0;
}

