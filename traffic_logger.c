// traffic_logger.c
// Simple traffic logger using AF_PACKET raw socket. Aggregates packet count & bytes and dumps stats every interval.
// Requires root.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>

#define BUF_SIZE 65536
#define STATS_INTERVAL 5       // seconds
#define LOG_FILE "traffic_log.csv"

volatile sig_atomic_t stop_flag = 0;
void handle_sigint(int sig) { stop_flag = 1; }

int main(int argc, char *argv[]) {
    int raw_sock;
    unsigned char *buffer = malloc(BUF_SIZE);
    if (!buffer) { perror("malloc"); return 1; }

    signal(SIGINT, handle_sigint);

    raw_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_sock < 0) {
        perror("Socket creation failed (need root)");
        return 1;
    }

    FILE *fp = fopen(LOG_FILE, "w");
    if (!fp) {
        perror("fopen");
        close(raw_sock);
        return 1;
    }
    fprintf(fp, "timestamp,interval_s,packets,bytes\n");
    fflush(fp);

    unsigned long interval_packets = 0;
    unsigned long interval_bytes = 0;

    time_t last_time = time(NULL);
    time_t start_time = last_time;

    printf("Traffic logger started. Logging to %s every %d seconds. Ctrl+C to stop.\n", LOG_FILE, STATS_INTERVAL);

    while (!stop_flag) {
        ssize_t data_size = recvfrom(raw_sock, buffer, BUF_SIZE, 0, NULL, NULL);
        if (data_size <= 0) {
            if (errno == EINTR) continue;
            perror("recvfrom");
            break;
        }
        interval_packets++;
        interval_bytes += data_size;

        time_t now = time(NULL);
        if (now - last_time >= STATS_INTERVAL) {
            double interval = difftime(now, last_time);
            char timestr[64];
            struct tm ttm;
            localtime_r(&now, &ttm);
            strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", &ttm);

            printf("[%s] Interval %.0f s — Packets: %lu, Bytes: %lu, Throughput: %.2f KB/s\n",
                   timestr, interval, interval_packets, interval_bytes, (interval_bytes/1024.0)/interval);

            fprintf(fp, "%s,%.0f,%lu,%lu\n", timestr, interval, interval_packets, interval_bytes);
            fflush(fp);

            interval_packets = 0;
            interval_bytes = 0;
            last_time = now;
        }
    }

    printf("Stopping logger. Finalizing file: %s\n", LOG_FILE);
    fclose(fp);
    close(raw_sock);
    free(buffer);
    return 0;
}
