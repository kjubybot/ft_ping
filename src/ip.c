#include "ft_ping.h"

uint16_t checksum(void *data, size_t len) {
    uint16_t res = 0;
    uint32_t sum = 0;
    uint16_t *payload = data;
    len >>= 1;

    for (size_t i = 0; i < len; i++) {
        sum += payload[i];
    }

    sum = (sum >> 16) + (sum & 0xffff);
    res = ~sum;

    return res;
}

struct iphdr build_ip(size_t len, int ttl, in_addr_t addr) {
    struct iphdr res;

    res.version = 4;
    res.ihl = 5;
    res.tos = 0;
    res.tot_len = len;
    res.id = 0;
    res.frag_off = 0;
    res.ttl = ttl;
    res.protocol = 1;
    res.check = 0;
    res.saddr = INADDR_ANY;
    res.daddr = addr;
    res.check = checksum(&res, sizeof(res));

    return res;
}
