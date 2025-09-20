// packet_sniffer.c
// Requires root. Uses AF_PACKET raw socket to capture Ethernet frames and parse IP/TCP/UDP headers.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <net/ethernet.h> // struct ether_header
#include <netinet/ip.h>   // struct iphdr
#include <netinet/tcp.h>  // struct tcphdr
#include <netinet/udp.h>  // struct udphdr
#include <linux/if_packet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <time.h>

#define BUF_SIZE 65536

void print_mac(unsigned char *mac) {
    printf("%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

int main(int argc, char *argv[]) {
    int raw_sock;
    unsigned char *buffer = (unsigned char *)malloc(BUF_SIZE);
    if (!buffer) { perror("malloc"); return 1; }

    raw_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_sock < 0) {
        perror("Socket creation failed (need root)");
        return 1;
    }

    printf("Packet sniffer started (listening on all interfaces). Press Ctrl+C to stop.\n");

    while (1) {
        ssize_t data_size = recvfrom(raw_sock, buffer, BUF_SIZE, 0, NULL, NULL);
        if (data_size < 0) {
            perror("recvfrom");
            break;
        }

        struct ethhdr *eth = (struct ethhdr *)(buffer);
        printf("\n=== Frame: %ld bytes ===\n", data_size);
        printf("Ethernet | SRC: "); print_mac(eth->h_source);
        printf(" DST: "); print_mac(eth->h_dest);
        printf(" Type: 0x%04x\n", ntohs(eth->h_proto));

        if (ntohs(eth->h_proto) == ETH_P_IP) {
            struct iphdr *ip = (struct iphdr*)(buffer + sizeof(struct ethhdr));
            struct in_addr src, dst;
            src.s_addr = ip->saddr;
            dst.s_addr = ip->daddr;
            char srcip[INET_ADDRSTRLEN], dstip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &src, srcip, INET_ADDRSTRLEN);
            inet_ntop(AF_INET, &dst, dstip, INET_ADDRSTRLEN);
            printf("IP   | SRC: %s DST: %s TTL: %d Proto: %d\n", srcip, dstip, ip->ttl, ip->protocol);

            if (ip->protocol == IPPROTO_TCP) {
                struct tcphdr *tcp = (struct tcphdr*)(buffer + sizeof(struct ethhdr) + ip->ihl*4);
                printf("TCP  | SRC PORT: %u DST PORT: %u SEQ: %u ACK: %u FLAGS: ",
                       ntohs(tcp->source), ntohs(tcp->dest), ntohl(tcp->seq), ntohl(tcp->ack_seq));
                if (tcp->urg) printf("URG ");
                if (tcp->ack) printf("ACK ");
                if (tcp->psh) printf("PSH ");
                if (tcp->rst) printf("RST ");
                if (tcp->syn) printf("SYN ");
                if (tcp->fin) printf("FIN ");
                printf("\n");
            } else if (ip->protocol == IPPROTO_UDP) {
                struct udphdr *udp = (struct udphdr*)(buffer + sizeof(struct ethhdr) + ip->ihl*4);
                printf("UDP  | SRC PORT: %u DST PORT: %u LEN: %u\n",
                       ntohs(udp->source), ntohs(udp->dest), ntohs(udp->len));
            } else {
                printf("Other IP protocol: %d\n", ip->protocol);
            }
        } else {
            printf("Non-IP frame. Type: 0x%04x\n", ntohs(eth->h_proto));
        }
    }

    close(raw_sock);
    free(buffer);
    return 0;
}
