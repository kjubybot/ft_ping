#ifndef FT_PING
#define FT_PING

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>
#include <unistd.h>
#include <linux/errqueue.h>

#define IOV_LEN 192
#define CONTROL_LEN 256
#define PROG_NAME "ft_ping"

typedef struct {
    int count;
    int timestamp;
    int help;
    float interval;
    int quiet;
    int ttl;
    int verbose;
} opts_t;

typedef struct {
    char name[16];
    struct sockaddr *addr;
    socklen_t addrlen;
    int sock;
    uint16_t seq;
    int packets;
    int packets_recv;
    int packets_lost;
    struct timeval start_time;
    opts_t opts;
} ft_ping_t;

typedef struct {
    struct icmphdr icmp;
    struct timeval time;
    char padding[40];
} payload_t;

typedef struct {
    payload_t *payload;
    int ttl;
    struct sock_extended_err *err;
} recv_t;

void reciever(ft_ping_t *);

#endif
