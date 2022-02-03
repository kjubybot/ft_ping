#include "ft_ping.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

ft_ping_t ft_ping;


void send_ping(int sig) {
    payload_t request;

    ft_ping.seq++;
    ft_ping.packets++;

    memset(&request, 0, sizeof(request));
    request.icmp.type = ICMP_ECHO;
    request.icmp.un.echo.sequence = ft_ping.seq;
    gettimeofday(&request.time, NULL);
    request.icmp.checksum = checksum(&request, sizeof(request));

    if (ft_ping.opts.is_raw) {
        struct iphdr ip = build_ip(PAYLOAD_SIZE + IP_HDR_LEN, ft_ping.opts.ttl, ft_ping.addr.sin_addr.s_addr);
        char request_raw[PAYLOAD_SIZE + IP_HDR_LEN];
        memcpy(request_raw, &ip, sizeof(ip));
        memcpy(request_raw + IP_HDR_LEN, &request, sizeof(request));

        if (sendto(ft_ping.sock, &request_raw, sizeof(request_raw), 0, (struct sockaddr *)&ft_ping.addr, ft_ping.addrlen) < 0) {
            perror(PROG_NAME": sendto");
            exit(1);
        }
    } else if (sendto(ft_ping.sock, &request, sizeof(request), 0, (struct sockaddr *)&ft_ping.addr, ft_ping.addrlen) < 0) {
        perror(PROG_NAME": sendto");
        exit(1);
    }

    if (ft_ping.opts.count != -1) {
        ft_ping.opts.count--;
    }
    if (ft_ping.opts.count != 0) {
        struct itimerval interval = {0};
        interval.it_value = ft_ping.opts.interval;
        setitimer(0, &interval, 0);
    }
}

void terminate(int sig) {
    struct timeval now;
    gettimeofday(&now, NULL);
    size_t elapsed = (now.tv_usec - ft_ping.start_time.tv_usec) / 1000;
    printf("\n0o0 %s ping statistics 0o0\n", ft_ping.name);
    printf("%d packets transmitted, %d recieved, %d errors, %d%% packet loss, time %lu ms\n",
            ft_ping.packets,
            ft_ping.packets_recv,
            ft_ping.packets_lost,
            100 - (ft_ping.packets_recv * 100) / ft_ping.packets,
            elapsed);
    exit(0);
}

void show_status(int sig) {
    printf("\r%d/%d packets, %d%% loss\n",
            ft_ping.packets_recv,
            ft_ping.packets,
            100 - (ft_ping.packets_recv * 100) / ft_ping.packets);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fputs(USAGE, stderr);
        exit(1);
    }

    if (parse_opts(argv, &ft_ping.opts)) {
        exit(1);
    }

    if (ft_ping.opts.help) {
        puts(USAGE);
        exit(0);
    }

    struct addrinfo *addr;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(argv[1], NULL, &hints, &addr)) {
        fprintf(stderr, PROG_NAME": cannot find host %s\n", argv[1]);
        exit(1);
    }

    ft_ping.addr.sin_addr.s_addr = ((struct sockaddr_in *)addr->ai_addr)->sin_addr.s_addr;
    ft_ping.addr.sin_port = 0;
    ft_ping.addr.sin_family = AF_INET;
    ft_ping.addrlen = addr->ai_addrlen;
    sprintf(ft_ping.name, "%hhu.%hhu.%hhu.%hhu",
            addr->ai_addr->sa_data[2],
            addr->ai_addr->sa_data[3],
            addr->ai_addr->sa_data[4],
            addr->ai_addr->sa_data[5]);
    freeaddrinfo(addr);

    ft_ping.sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
    if (ft_ping.sock < 0) {
        ft_ping.sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
        if (ft_ping.sock < 0) {
            perror(PROG_NAME": cannot create socket");
            exit(1);
        }
        ft_ping.opts.is_raw = 1;
    }

    int enable = 1;
    if (setsockopt(ft_ping.sock, SOL_IP, IP_RECVERR, &enable, sizeof(enable))) {
        perror(PROG_NAME": cannot set error recieving");
        exit(1);
    }
    if (setsockopt(ft_ping.sock, SOL_IP, IP_RECVTTL, &enable, sizeof(enable))) {
        perror(PROG_NAME": cannot set TTL recieving");
        exit(1);
    }
    if (ft_ping.opts.is_raw) {
        if (setsockopt(ft_ping.sock, SOL_IP, IP_HDRINCL, &enable, sizeof(enable))) {
            perror(PROG_NAME": cannot configure socket");
            exit(1);
        }
    } else if (setsockopt(ft_ping.sock, SOL_IP, IP_TTL, &ft_ping.opts.ttl, sizeof(ft_ping.opts.ttl))) {
        perror(PROG_NAME": cannot set TTL");
        exit(1);
    }

    signal(SIGALRM, send_ping);
    signal(SIGINT, terminate);
    signal(SIGQUIT, show_status);

    printf("FT_PING %s (%s) %lu bytes of data\n", argv[1], ft_ping.name, sizeof(payload_t));

    send_ping(0);
    reciever(&ft_ping);
}
