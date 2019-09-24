#define NETLINK_USER 31
struct sock *netlink_socket = NULL;
struct pid *pid_struct;
struct task_struct *task, *curr;
struct list_head *pos;
int lev = 0, num = 0;
int pid;
struct process {
    char *comm;
    int pid;
    int level;
} proc[1000];


