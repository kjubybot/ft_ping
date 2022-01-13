#ifndef FT_PING
#define FT_PING

#include <ctype.h>
#include <netinet/ip.h>
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
#define IP_HDR_LEN 20

#define DEFAULT_TTL 64
#define DEFAULT_INTERVAL 1.0

#define PROG_NAME "ft_ping"
#define USAGE "Usage:\n" \
                "ft_ping [options] address\n\n" \
                "Valid options are:\n" \
                "    -c N    send N packets and stop\n" \
                "    -D      show timestamp\n" \
                "    -h      show this help\n" \
                "    -q      quiet mode, display only starting line and summary\n" \
                "    -t N    set TTL. Default is 64\n" \
                "    -v      verbose mode\n"

typedef struct {
    int count;
    int timestamp;
    int help;
    struct timeval interval;
    int quiet;
    int ttl;
    int verbose;
    int is_raw;
} opts_t;

typedef struct {
    char name[16];
    struct sockaddr_in addr;
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
    struct iphdr ip;
    payload_t data;
} payload_raw_t;

typedef struct {
    payload_t *payload;
    int ttl;
    struct sock_extended_err *err;
} recv_t;

void reciever(ft_ping_t *);
int parse_opts(char **, opts_t *);
void terminate(int);

uint16_t checksum(void *, size_t);
struct iphdr build_ip(size_t, int, in_addr_t);

#endif
