
#ifndef _NETINET_IN_H_
#define _NETINET_IN_H_
#include <C:\Users\Hp\OneDrive\Desktop\PT\Samba Server\Arduino\libsmb2\portable-endian.h>
#include <C:\Users\Hp\OneDrive\Desktop\PT\Samba Server\Arduino\libsmb2\socket.h>


__BEGIN_DECLS
#define INET_ADDRSTRLEN 16
typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;
int bindresvport(int, struct sockaddr_in*);
#if __ANDROID_API__ >= __ANDROID_API_N__
extern const struct in6_addr in6addr_any __INTRODUCED_IN(24);
extern const struct in6_addr in6addr_loopback __INTRODUCED_IN(24);
#else
static const struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;
static const struct in6_addr in6addr_loopback = IN6ADDR_LOOPBACK_INIT;
#endif /* __ANDROID_API__ >= __ANDROID_API_N__ */
__END_DECLS