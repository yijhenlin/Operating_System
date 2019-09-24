#include <sys/socket.h>
#include <linux/netlink.h>

#define NETLINK_USER 31

#define MAX_PAYLOAD 6000 /* maximum payload size*/
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL, *nh2;
struct iovec iov;
int netlink_socket;
struct msghdr msg;
int len;
char buf[MAX_PAYLOAD];
struct iovec iov2;
struct msghdr msg2;


