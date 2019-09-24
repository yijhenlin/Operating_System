#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/skbuff.h>
#include <linux/pid.h>
#include <linux/string.h>
#include "ksimple_pstree.h"

int atoi(char *s)
{
    int sum = 0;
    int i;
    for(i = 1; s[i] != '\0'; i++) {
        sum = sum*10+s[i]-'0';
    }
    return sum;
}
int flag_t = 0;
int find_sibling(struct task_struct *, int);
int find_all_tree(struct task_struct *);

int find_thread(struct task_struct *tar, int flag) // flag = 0 all tree, flag = 1 sibling
{
    tar = tar->group_leader;
    if(!list_empty(&(tar->thread_node))) {
        struct list_head *p = NULL;
//        struct task_struct *t;
        struct list_head *head = &(tar->thread_node);
        list_for_each(p, head) {
            struct task_struct *t = list_entry(p, struct task_struct, thread_node);
            if(t->pid < 1)return 0;
            if(flag == 0) {
                lev--;
                find_all_tree(t);
                lev++;
            } else if(flag == 1)find_sibling(t, 1);
        }
        flag_t = 0;
    }
    return 0;
}

int find_sibling(struct task_struct *tar, int flag)        //head = &(task->sibling), flag = 0 proc, flag = 1 thread
{
    struct list_head *p = NULL;
    struct task_struct *t;
    struct list_head *head;
    find_thread(tar->parent, 1);
    if(!list_empty(&(tar->sibling)) && !flag) {
        head = &(tar->parent->children);
        list_for_each(p, head) {
            t = list_entry(p, struct task_struct, sibling);
            if(t->pid < 1)return 0;
            proc[num].comm = t->comm;
            proc[num].pid = t->pid;
            proc[num].level = 0;
            num++;
        }
    }
    if(!list_empty(&(tar->children)) && flag) {
        head = &(tar->children);
        list_for_each(p, head) {
            t = list_entry(p, struct task_struct, sibling);
            if(t->pid < 1)return 0;
            proc[num].comm = t->comm;
            proc[num].pid = t->pid;
            proc[num].level = 0;
            num++;
        }
    }
    return 0;
}

int find_all_tree(struct task_struct *st)       //head = &(task->children)
{
    if(!list_empty(&(st->children))) {
        struct list_head *head = &(st->children);
        struct list_head *p = NULL;
        struct task_struct *t;
        lev ++;
        list_for_each(p, head) {
            t = list_entry(p, struct task_struct, sibling);
            if(t->pid < 1)return 0;
            proc[num].comm = t->comm;
            proc[num].pid = t->pid;
            proc[num].level = lev;
            num++;
            if(flag_t == 0) {
                flag_t = 1;		//the same thread_group, avoid repeat
                find_thread(t, 0);
            }
            // printk("%s(%d)\n", t->comm, t->pid);
            if(!list_empty(&t->children))find_all_tree(t);

        }
        lev--;
    }
    return 0;
}

int find_parent(struct task_struct *ptr)
{
    struct task_struct *t;
    t = ptr->group_leader->real_parent;
    lev++;
    if(t != NULL && t->pid >= 1) {
        proc[num].comm = t->comm;
        proc[num].pid = t->pid;
        proc[num].level = lev;
        // printk("%s(%d)\n", t->comm, t->pid);
        num++;
        find_parent(t);
    } else;
    return 0;
}

static void send_msg(char *msg)
{
    struct nlmsghdr *nlh;
    struct sk_buff *skb_out;
    int msg_size = strlen(msg);
    int res;

    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out) {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }
    nlh = nlmsg_put(skb_out, 0, 0, 0, msg_size, NLM_F_MULTI);
    NETLINK_CB(skb_out).dst_group = 0;
    strncpy(nlmsg_data(nlh), msg, msg_size);
    nlh = NLMSG_NEXT(nlh, msg_size);
    nlh->nlmsg_type = NLMSG_DONE;
    res = nlmsg_unicast(netlink_socket, skb_out, pid);
    if (res < 0)
        printk(KERN_INFO "Error while sending bak to user\n");
}

char *packet(int f)
{
    char *result = (char *)kmalloc(6000, GFP_KERNEL);
    memset(result, 0, 6000);
    char n[20];
    int i, j;
    if(f == 0) {
        for(i = 0; i < num; i++) {
            for(j = 0; j < proc[i].level; j++) {
                strcat(result, "    ");
            }
            snprintf(n, 10, "%d", proc[i].pid);
            strcat(result, proc[i].comm);
            strcat(result, "(");
            strcat(result, n);
            strcat(result, ")\n");

        }
    } else {
        for(i = num-1; i >= 0; i--) {
            for(j = 0; j < proc[i].level; j++) {
                strcat(result, "    ");
            }
            snprintf(n, 10, "%d", proc[i].pid);
            strcat(result, proc[i].comm);
            strcat(result, "(");
            strcat(result, n);
            strcat(result, ")\n");
        }
    }
//	printk("%s", result);
    return result;
}


static void hello_nl_recv_msg(struct sk_buff *skb)
{

    struct nlmsghdr *nlh;
    int p_id;
    int f = 0;
//    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);
    nlh = (struct nlmsghdr *)skb->data;
    pid = nlh->nlmsg_pid;
//    printk(KERN_INFO "Netlink received msg payload: %s\n", (char *)nlmsg_data(nlh));
    char *option = (char *)nlmsg_data(nlh);
    p_id = atoi(option);
    pid_struct = find_get_pid(p_id);
    if(pid_struct == NULL) {
        printk("no pid_struct\n");
        send_msg("");
        return;
    }
    task = pid_task(pid_struct, PIDTYPE_PID);
    if(task == NULL) {
        printk("no task\n");
        send_msg("");
        return;
    }
    if(option[0] == 'c') {
        proc[num].comm = task->comm;
        proc[num].pid = task->pid;
        proc[num].level = 0;
        num++;
        find_thread(task, 0);
        find_all_tree(task);
        f = 0;
    } else if(option[0] == 's') {
        find_sibling(task, 0);//(&(task->sibling));
        f = 0;
    } else if(option[0] == 'p') {
        proc[num].comm = task->comm;
        proc[num].pid = task->pid;
        proc[num].level = 0;
        num++;
        find_parent(task);
        int i;
        for(i = 0; i < num; i++) {
            proc[i].level = lev-proc[i].level-1;
        }
        f = 1;
    } else {
        printk("option error : %s", option);
        send_msg("");
        return;
    }
//    pid = nlh->nlmsg_pid; /*pid of sending process */
    char *msg = packet(f);
    send_msg(msg);
    memset(msg, 0, 6000);
    memset(proc, 0, sizeof(proc));
    num = 0;
    lev = 0;
}

static int __init hello_init(void)
{

    struct netlink_kernel_cfg cfg = {
        .groups  = 0,
        .input = hello_nl_recv_msg,
    };

//    printk("Entering: %s\n", __FUNCTION__);
    netlink_socket = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);

//     netlink_socket = netlink_kernel_create(&init_net, NETLINK_USER, 0, hello_nl_recv_msg, NULL, THIS_MODULE);
    if (!netlink_socket) {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    return 0;
}

static void __exit hello_exit(void)
{

    printk(KERN_INFO "exiting hello module\n");
    netlink_kernel_release(netlink_socket);
}
module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");

