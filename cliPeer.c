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
#define PORT 12345 

int main(int argc, char* argv[])
{
    int connfd;
    char buf[BUFFER_SIZE];
    char endBuf[BUFFER_SIZE] = "END\n";
    memset(buf, 0, sizeof(buf));

    struct sockaddr_in6 srvaddr, peeraddr;
    socklen_t addr_len = sizeof(srvaddr);
    bzero(&srvaddr, sizeof(srvaddr));
    srvaddr.sin6_family = AF_INET6;
    srvaddr.sin6_port = htons(PORT);
    srvaddr.sin6_addr = in6addr_any;

    bzero(&peeraddr, sizeof(peeraddr));
    peeraddr.sin6_family = AF_INET6;
    peeraddr.sin6_port = htons(PORT);
    inet_pton(AF_INET6, argv[1], &peeraddr.sin6_addr);

    if ((connfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
        perror("client socket error");
    bind(connfd, (struct sockaddr*)&srvaddr, sizeof(srvaddr));

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
            fgets(buf, BUFFER_SIZE-1, stdin);
            ret = sendto(connfd, buf, BUFFER_SIZE-1, 0, (struct sockaddr*)&peeraddr, addr_len);
            if (strcmp(buf, endBuf) == 0) {
                break;
            }

            memset(buf, 0, sizeof(buf));
        }

        if (fds[1].revents & POLLIN) { //接收对等方数据
            memset(buf, 0, sizeof(buf));
            recvfrom(fds[1].fd, buf, BUFFER_SIZE-1, 0, NULL, NULL); 
            if (strcmp(buf, endBuf) == 0) {
                break;
            }

            printf("peer:%s", buf);
        }
    }

    close(connfd);
    return 0;
}

