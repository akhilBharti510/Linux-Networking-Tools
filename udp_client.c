// udp_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 2048

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return -1;
    }

    int port = atoi(argv[2]);
    int sockfd;
    struct sockaddr_in servaddr;
    char sendbuf[BUFFER_SIZE];
    char recvbuf[BUFFER_SIZE];
    socklen_t addrlen = sizeof(servaddr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("UDP socket creation failed");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sockfd);
        return -1;
    }

    snprintf(sendbuf, sizeof(sendbuf), "Hello UDP server (from client pid %d)", getpid());
    if (sendto(sockfd, sendbuf, strlen(sendbuf), 0, (struct sockaddr *)&servaddr, addrlen) < 0) {
        perror("sendto failed");
    } else {
        ssize_t n = recvfrom(sockfd, recvbuf, sizeof(recvbuf)-1, 0, NULL, NULL);
        if (n > 0) {
            recvbuf[n] = '\0';
            printf("Server reply: %s\n", recvbuf);
        }
    }

    close(sockfd);
    return 0;
}
