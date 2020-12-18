#define _GNU_SOURCE 1
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <termios.h>

#define BUFFER_SIZE 1024
#define PORT 22334 

//隐藏终端输入的退格字符
void hint_backspace()
{
    struct termios term;
    memset(&term, 0, sizeof(term));

    if (tcgetattr(STDIN_FILENO, &term) == -1)
        perror("tcgetattr error");

    term.c_cc[VERASE] = '\b';
    if (tcsetattr(STDIN_FILENO, TCSANOW, &term) == -1)
        perror("tcsetattr error");

    return;
}

int main(int argc, char* argv[])
{
    int connfd;
    char read_buf[BUFFER_SIZE];
    char write_buf[BUFFER_SIZE];
    char End_buf[BUFFER_SIZE] = "END\n";
    memset(read_buf, 0, sizeof(read_buf));
    memset(write_buf, 0, sizeof(write_buf));

    hint_backspace();

    struct sockaddr_in6 srvaddr, peeraddr;
    socklen_t addr_len = sizeof(srvaddr);
    bzero(&srvaddr, sizeof(srvaddr));
    srvaddr.sin6_family = AF_INET6;
    srvaddr.sin6_port = htons(PORT);
    srvaddr.sin6_addr = in6addr_any;

    bzero(&peeraddr, sizeof(peeraddr));
    peeraddr.sin6_family = AF_INET6;
    peeraddr.sin6_port = htons(PORT-1);
    peeraddr.sin6_addr = in6addr_any;


    if ((connfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
        perror("client socket error");

    bind(connfd, (struct sockaddr*)&srvaddr, sizeof(srvaddr));
    printf("\033[1;35;35m****************************************\033[0m\n");
    printf("\033[1;35;35m******已和对等方建立连接，开始通信******\033[0m\n");
    printf("\033[1;35;35m****************************************\033[0m\n");

    struct pollfd fds[2];
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    fds[1].fd = connfd;
    fds[1].events = POLLIN;
    fds[1].revents = 0;

    while (1)
    {
        int ret = poll(fds, 2, -1);
        if (ret < 0)
        {
            printf("poll error");
            break;
        }

        if (fds[0].revents & POLLIN) {  //向对等方发送数据
            fgets(write_buf, BUFFER_SIZE-1, stdin);
            ret = sendto(connfd, write_buf, BUFFER_SIZE-1, 0, (struct sockaddr*)&peeraddr, addr_len);
            if (strcmp(write_buf, End_buf) == 0) {
                printf("\033[1;35;35m****************************************\033[0m\n");
                printf("\033[1;35;35m************你已经关闭了连接************\033[0m\n");
                printf("\033[1;35;35m****************************************\033[0m\n");
                break;
            }

            memset(write_buf, 0, sizeof(write_buf));
        }

        if (fds[1].revents & POLLIN) { //接收对等方数据
            memset(read_buf, 0, sizeof(read_buf));
            recvfrom(fds[1].fd, read_buf, BUFFER_SIZE-1, 0, NULL, NULL); 
            if (strcmp(read_buf, End_buf) == 0) {
                printf("\033[1;35;35m****************************************\033[0m\n");
                printf("\033[1;35;35m************对等方关闭了连接************\033[0m\n");
                printf("\033[1;35;35m****************************************\033[0m\n");
                break;
            }

            printf("对等方:%s", read_buf);
        }
    }

    close(connfd);
    return 0;
}

