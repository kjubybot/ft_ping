#include "ft_ping.h"
#include <stdio.h>
#include <sys/time.h>

ft_ping_t ft_ping;

void send_ping(int sig) {
    payload_t request;

    ft_ping.seq++;
    ft_ping.packets++;

    memset(&request, 0, sizeof(request));
    request.icmp.type = ICMP_ECHO;
    request.icmp.un.echo.sequence = ft_ping.seq;
    gettimeofday(&request.time, NULL);
    if (sendto(ft_ping.sock, &request, sizeof(request), 0, ft_ping.addr, ft_ping.addrlen) < 0) {
        perror(PROG_NAME": sendto");
        exit(1);
    }
    alarm(1);
}

void terminate(int sig) {
    struct timeval now;
    gettimeofday(&now, NULL);
    size_t elapsed = (now.tv_usec - ft_ping.start_time.tv_usec) / 1000;
    printf("\n0o0 %s ping statistics 0o0\n", ft_ping.name);
    printf("%d packets transmitted, %d recieved, %d errors, time %lu ms\n", ft_ping.packets, ft_ping.packets_recv, ft_ping.packets_lost, elapsed);
    exit(0);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fputs("put usage here", stderr);
        exit(1);
    }

    struct addrinfo *addr;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(argv[1], NULL, &hints, &addr)) {
        perror(PROG_NAME);
        exit(1);
    }

    ft_ping.addr = addr->ai_addr;
    ft_ping.addrlen = addr->ai_addrlen;
    sprintf(ft_ping.name, "%hhu.%hhu.%hhu.%hhu",
            addr->ai_addr->sa_data[2],
            addr->ai_addr->sa_data[3],
            addr->ai_addr->sa_data[4],
            addr->ai_addr->sa_data[5]);

    printf("FT_PING %s (%s) %lu bytes of data\n", argv[1], ft_ping.name, sizeof(payload_t));

    ft_ping.sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (ft_ping.sock < 0) {
        perror(PROG_NAME": socket");
        exit(1);
    }
    int enable = 1;
    if (setsockopt(ft_ping.sock, SOL_IP, IP_RECVERR, &enable, sizeof(enable))) {
        perror(PROG_NAME": setsockopt");
        exit(1);
    }
    if (setsockopt(ft_ping.sock, SOL_IP, IP_RECVOPTS, &enable, sizeof(enable))) {
        perror(PROG_NAME": setsockopt");
        exit(1);
    }
    if (setsockopt(ft_ping.sock, SOL_IP, IP_RECVTTL, &enable, sizeof(enable))) {
        perror(PROG_NAME": setsockopt");
        exit(1);
    }

    signal(SIGALRM, send_ping);
    signal(SIGINT, terminate);
    alarm(1);
    reciever(&ft_ping);
}
