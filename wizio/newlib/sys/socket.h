////////////////////////////////////////////////////////////////////////////////////////
//
//      2021 Georgi Angelov
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////////////

#ifndef SOCKET_H_
#define SOCKET_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include "errno.h"

    typedef uint16_t in_port_t;
    typedef uint32_t in_addr_t;
    struct in_addr
    {
        in_addr_t s_addr;
    };
    typedef uint8_t sa_family_t;
    struct sockaddr_in
    {
        uint8_t sin_len;
        sa_family_t sin_family;
        in_port_t sin_port;
        struct in_addr sin_addr;
#define SIN_ZERO_LEN 8
        char sin_zero[SIN_ZERO_LEN];
    };
    struct sockaddr
    {
        uint8_t sa_len;
        sa_family_t sa_family;
        char sa_data[14];
    };

#if !defined(socklen_t)
    typedef uint32_t socklen_t; /* 32 */
#endif

    int socket(int32_t family, int32_t type, int32_t protocol);
    int connect(int fd, const struct sockaddr *addr, int32_t addrlen);
    static inline int closesocket(int fd) { return -1; }
    int setsockopt(int fd, int level, int opname, const void *opval, socklen_t oplen);
    int getsockopt(int fd, int level, int opname, const void *opval, socklen_t *oplen);
    int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    int listen(int sockfd, int backlog);
    int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    int shutdown(int sockfd, int how);
    ssize_t recv(int sockfd, void *buf, size_t len, int flags);
    ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
    ssize_t send(int sockfd, const void *buf, size_t len, int flags);
    ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
    //int select(int ignore, fd_set *rd, fd_set *wr, fd_set *er, struct timeval *timeout);

#define _htons(x) ((unsigned short)((((unsigned short)(x)&0x00ff) << 8) | (((unsigned short)(x)&0xff00) >> 8)))
#define _ntohs(x) ((unsigned short)((((unsigned short)(x)&0x00ff) << 8) | (((unsigned short)(x)&0xff00) >> 8)))

#define htons _htons
#define ntohs _ntohs

#define htonl(n) (((((unsigned long)(n)&0xFF)) << 24) | ((((unsigned long)(n)&0xFF00)) << 8) | \
                  ((((unsigned long)(n)&0xFF0000)) >> 8) | ((((unsigned long)(n)&0xFF000000)) >> 24))

#define ntohl(n) (((((unsigned long)(n)&0xFF)) << 24) | ((((unsigned long)(n)&0xFF00)) << 8) | \
                  ((((unsigned long)(n)&0xFF0000)) >> 8) | ((((unsigned long)(n)&0xFF000000)) >> 24))

#define IPPROTO_TCP 6       /* not used, need */
#define IPPROTO_UDP 17      /* not used, need */
#define AF_UNIX AF_UNSPEC   /* not used, need */
#define INET_ADDRSTRLEN 16  /* not used, need */
#define INET6_ADDRSTRLEN 46 /* not used, need */

/* printf helper */
#define ADDRESS_TO_IP4(A) (int)(A & 0xFF), (int)((A >> 8) & 0xFF), (int)((A >> 16) & 0xFF), (int)((A >> 24) & 0xFF)

    /* addres info */

    //#include <netdb.h>
    struct addrinfo
    {
        int ai_flags;
        int ai_family;
        int ai_socktype;
        int ai_protocol;
        size_t ai_addrlen;
        char *ai_canonname;
        struct sockaddr *ai_addr;
        struct addrinfo *ai_next;
    };

    //#define hostent ????_hostent_s
    //struct hostent *gethostbyname(const char *name);
    //int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
    //void freeaddrinfo(struct addrinfo *res);

#ifdef __cplusplus
}
#endif
#endif /* SOCKET_H_ */