#include "ft_ping.h"

static void collect_data(recv_t *response, struct msghdr *message, int is_raw) {
    if (is_raw) {
        response->payload = (payload_t*)(message->msg_iov[0].iov_base + IP_HDR_LEN);
    } else {
        response->payload = (payload_t*)message->msg_iov[0].iov_base;
    }
    for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(message); cmsg != NULL; cmsg = CMSG_NXTHDR(message, cmsg)) {
        if (cmsg->cmsg_level == SOL_IP) {
            if (cmsg->cmsg_type == IP_TTL) {
                response->ttl = *(int*)CMSG_DATA(cmsg);
            } else if (cmsg->cmsg_type == IP_RECVERR) {
                response->err = (struct sock_extended_err*)CMSG_DATA(cmsg);
                response->offender = (struct sockaddr_in *)SO_EE_OFFENDER(response->err);
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

    recv_t response;
    struct timeval now;

    while (1) {
        struct sockaddr_in addr = ft_ping->addr;

        message.msg_name = &addr;
        message.msg_namelen = ft_ping->addrlen;
        message.msg_iov = &iov;
        message.msg_iovlen = 1;
        message.msg_control = aux;
        message.msg_controllen = CONTROL_LEN;

        ssize_t rec = recvmsg(ft_ping->sock, &message, 0);
        if (addr.sin_addr.s_addr != ft_ping->addr.sin_addr.s_addr) {
            continue;
        }
        if (rec < 0)  {
            ft_ping->packets_lost++;

            if (ft_ping->opts.quiet != 1) {
                rec = recvmsg(ft_ping->sock, &message, MSG_ERRQUEUE);
                collect_data(&response, &message, ft_ping->opts.is_raw);
                char *err;

                if (response.err->ee_type == ICMP_DEST_UNREACH) {
                    switch (response.err->ee_code) {
                        case ICMP_NET_UNREACH:
                            err = "Network Unreachable";
                            break;
                        case ICMP_HOST_UNREACH:
                            err = "Destination Host Unreachable";
                            break;
                        case ICMP_PROT_UNREACH:
                            err = "Protocol unreachable";
                            break;
                        case ICMP_PORT_UNREACH:
                            err = "Port unreachable";
                            break;
                        case ICMP_FRAG_NEEDED:
                            err = "Fragmentation needed";
                            break;
                        case ICMP_SR_FAILED:
                            err = "Source route failed";
                            break;
                        case ICMP_NET_UNKNOWN:
                            err = "Network is unknown";
                            break;
                        case ICMP_HOST_UNKNOWN:
                            err = "Host is unknown";
                            break;
                        case ICMP_HOST_ISOLATED:
                            err = "Host isolated";
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

                printf("From %d.%d.%d.%d: icmp_seq=%d %s\n",
                        response.offender->sin_addr.s_addr & 0xff,
                        (response.offender->sin_addr.s_addr >> 8) & 0xff,
                        (response.offender->sin_addr.s_addr >> 16) & 0xff,
                        (response.offender->sin_addr.s_addr >> 24) & 0xff,
                        response.payload->icmp.un.echo.sequence, err);
                if (ft_ping->opts.verbose) {
                    printf("I dunno, have a header\n"HEADER_FORMAT, response.err->ee_type, response.err->ee_code);
                }
            }
        } else {
            ft_ping->packets_recv++;

            if (ft_ping->opts.quiet != 1) {
                collect_data(&response, &message, ft_ping->opts.is_raw);
                gettimeofday(&now, NULL);
                float elapsed = (now.tv_sec - response.payload->time.tv_sec) * 1000 + (float)(now.tv_usec - response.payload->time.tv_usec) / 1000;
                if (ft_ping->opts.timestamp) {
                    printf("[%lu.%lu] ", now.tv_sec, now.tv_usec);
                }
                if (ft_ping->opts.is_raw) {
                    rec -= IP_HDR_LEN;
                }
                printf("%lu bytes from %d.%d.%d.%d: icmp_seq=%d ttl=%d time=%.3f ms\n",
                        rec,
                        addr.sin_addr.s_addr & 0xff,
                        (addr.sin_addr.s_addr >> 8) & 0xff,
                        (addr.sin_addr.s_addr >> 16) & 0xff,
                        (addr.sin_addr.s_addr >> 24) & 0xff,
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
