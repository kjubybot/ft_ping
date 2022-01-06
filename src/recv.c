#include "ft_ping.h"

static void collect_data(recv_t *response, struct msghdr *message) {
    response->payload = (payload_t*)message->msg_iov[0].iov_base;
    for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(message); cmsg != NULL; cmsg = CMSG_NXTHDR(message, cmsg)) {
        if (cmsg->cmsg_level == SOL_IP) {
            if (cmsg->cmsg_type == IP_TTL) {
                response->ttl = *(int*)CMSG_DATA(cmsg);
            } else if (cmsg->cmsg_type == IP_RECVERR) {
                response->err = (struct sock_extended_err*)CMSG_DATA(cmsg);
            }
        }
    }
}

void reciever(ft_ping_t *ft_ping) {
    struct msghdr message;
    struct iovec iov;
    char aux[CONTROL_LEN];
    char iov_base[IOV_LEN];

    iov.iov_base = iov_base;
    iov.iov_len = IOV_LEN;

    message.msg_name = ft_ping->addr;
    message.msg_namelen = ft_ping->addrlen;
    message.msg_iov = &iov;
    message.msg_iovlen = 1;
    message.msg_control = aux;

    recv_t response;
    struct timeval now;

    while (1) {
        message.msg_controllen = CONTROL_LEN;
        ssize_t rec = recvmsg(ft_ping->sock, &message, 0);
        if (rec < 0) {
            ft_ping->packets_lost++;

            if (ft_ping->opts.quiet != 1) {
                rec = recvmsg(ft_ping->sock, &message, MSG_ERRQUEUE);
                collect_data(&response, &message);
                char *err;

                if (response.err->ee_type == ICMP_DEST_UNREACH) {
                    switch (response.err->ee_code) {
                        case ICMP_NET_UNREACH:
                            err = "Network Unreachable";
                            break;
                        case ICMP_HOST_UNREACH:
                            err = "Destination Host Unreachable";
                            break;
                        case ICMP_PKT_FILTERED:
                            err = "Packet filtered";
                            break;
                    }
                } else if (response.err->ee_type == ICMP_TIME_EXCEEDED) {
                    switch (response.err->ee_code) {
                        case ICMP_EXC_TTL:
                            err = "TTL exceeded";
                            break;
                        case ICMP_EXC_FRAGTIME:
                            err = "Fragment Reass time exceeded";
                            break;
                    }
                }
                printf("From %s: icmp_seq=%d %s\n", ft_ping->name, response.payload->icmp.un.echo.sequence, err);
            }
        } else {
            ft_ping->packets_recv++;

            if (ft_ping->opts.quiet != 1) {
                collect_data(&response, &message);
                gettimeofday(&now, NULL);
                float elapsed = (float)(now.tv_usec - response.payload->time.tv_usec) / 1000;
                printf("%lu bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
                        rec,
                        ft_ping->name,
                        response.payload->icmp.un.echo.sequence,
                        response.ttl,
                        elapsed);
            }
        }
        if (ft_ping->opts.count == 0) {
            terminate(0);
        }
    }
}
