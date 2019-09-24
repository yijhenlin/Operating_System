#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "simple_pstree.h"

void prepare_socket(char *send)
{

    netlink_socket = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (netlink_socket < 0) return;

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); /* self pid */

    bind(netlink_socket, (struct sockaddr *)&src_addr, sizeof(src_addr));

    memset(&dest_addr, 0, sizeof(dest_addr));
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; /* For Linux Kernel */
    dest_addr.nl_groups = 0; /* unicast */

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    strcpy(NLMSG_DATA(nlh), send);

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
}

int main(int argc, char* argv[])
{
    int pid = getpid();
    char p[10];
    sprintf(p, "%d", pid);
//    char *option = (char *)malloc(20);
    char option[20];
    memset(option, 0, 20);
    if(argc == 1) {
        strcpy(option, "c1");
    } else {
        if(argv[1][0] == '-') {
            char *arg = strtok(argv[1], "-");
            strcpy(option, arg);
            if(strlen(argv[1]) > 2);
            else if(argv[1][1] == 'c') strcat(option, "1");
            else if(argv[1][1] == 's' || argv[1][1] == 'p')
                strcat(option, p);
            else return 1;
        } else {
            strcpy(option, "c");
            strcat(option, argv[1]);
        }
    }
    prepare_socket(option);
//    printf("Sending message to kernel\n");
    sendmsg(netlink_socket, &msg, 0);
//    printf("Waiting for message from kernel\n");

    iov2.iov_base = buf;
    iov2.iov_len = sizeof(buf);
    msg2.msg_name = (void *)&dest_addr;
    msg2.msg_namelen = sizeof(dest_addr);
    msg2.msg_iov = &iov2;
    msg2.msg_iovlen = 1;
    msg2.msg_control = NULL;
    msg2.msg_controllen = 0;
    msg2.msg_flags = 0;
    len = recvmsg(netlink_socket, &msg2, 0);
//   printf("recv: %d\n", len);
    for(nh2 = (struct nlmsghdr *)buf; NLMSG_OK(nh2, len); nh2 = NLMSG_NEXT(nh2, len)) {
        if(nh2->nlmsg_type == NLMSG_DONE) {
            printf("done\n");
            break;
        }
        printf("%s\n", NLMSG_DATA(nh2));
    }
//    free(option);
    close(netlink_socket);
}
