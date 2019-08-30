/*
 * Copyright (c) 2006 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Based on ipt_random and ipt_nth by Fabrice MARIE <fabrice@netfilter.org>.
 */

#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/skbuff.h>
#include <linux/net.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>

#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/inet.h>
#include <linux/netfilter/xt_statistic.h>
#include <linux/netfilter/x_tables.h>
#include <linux/module.h>

struct xt_statistic_priv {
    atomic_t count;
} ____cacheline_aligned_in_smp;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Patrick McHardy <kaber@trash.net>");
MODULE_DESCRIPTION("Xtables: statistics-based matching (\"Nth\", random)");
MODULE_ALIAS("ipt_statistic");
MODULE_ALIAS("ip6t_statistic");

#define LENGTH_IP 16
#define LENGTH_DOMAIN 30
#define MAX_POD 6
static char clusterIP[LENGTH_IP];
static char targetDomain[LENGTH_DOMAIN];
static char targetIP[LENGTH_DOMAIN];
static char podIP[MAX_POD][LENGTH_IP];
static int curPodSize = 0;

static ssize_t clusterIPwrite(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    int c;
    memset(clusterIP, 0, LENGTH_IP);

    if (*ppos > 0 || count > LENGTH_IP)
        return -EFAULT;
    if (copy_from_user(clusterIP, ubuf, count))
        return -EFAULT;

    c = strlen(clusterIP);
    clusterIP[c-1]=0; //remove new line
    printk(KERN_INFO "Read %s", clusterIP);
    *ppos = c;
    return c;
}

static ssize_t httpwrite(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    int c;
    char buf[LENGTH_DOMAIN + LENGTH_IP] = {0};
    char *token, *cur;

    memset(targetDomain, 0, LENGTH_DOMAIN);
    memset(targetIP, 0, LENGTH_IP);
    if (*ppos > 0 || count > (LENGTH_DOMAIN + LENGTH_IP))
        return -EFAULT;
    if (copy_from_user(buf, ubuf, count))
        return -EFAULT;

    c = strlen(buf);
    buf[c-1]=0; //remove new line

    printk(KERN_INFO "Read %s:%d", buf, c);

    //Use strsep to splite the string, I don't know why sscanf doesn't work here...
    cur = buf;
    token = strsep(&cur, ",");
    memcpy(targetDomain, token, strlen(token));
    token = strsep(&cur, ",");
    memcpy(targetIP, token, strlen(token));
    printk(KERN_INFO "Read %s:%s", targetDomain, targetIP);
    *ppos = c;
    return c;
}

static ssize_t podIPwrite(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
    int i;
    int c;
    char buf[MAX_POD * LENGTH_IP + 1] = {0};
    char *token, *cur;

    curPodSize = 0;
    //Init all podIP array
    for (i = 0; i < MAX_POD; i++) {
        memset(podIP[i], 0, LENGTH_IP);
    }

    if (*ppos > 0 || count > (MAX_POD * LENGTH_IP + 1))
        return -EFAULT;
    if (copy_from_user(buf, ubuf, count))
        return -EFAULT;

    c = strlen(buf);
    buf[c-1]=0; //remove new line

    cur = buf;
    while ( (token = strsep(&cur, ","))) {
        memcpy(podIP[curPodSize++], token, strlen(token));
    }

    *ppos = c;
    return c;
}

//proc operations
static struct file_operations clusterIPops =
{
    .owner = THIS_MODULE,
    .write = clusterIPwrite,
};

static struct file_operations httpops =
{
    .owner = THIS_MODULE,
    .write = httpwrite,
};

static struct file_operations podIPops =
{
    .owner = THIS_MODULE,
    .write = podIPwrite,
};



static struct proc_dir_entry *entClusterIP;
static struct proc_dir_entry *entHTTP;
static struct proc_dir_entry *entPodIP;

static unsigned int getDestinationIP(const struct sk_buff *skb)
{
    struct iphdr *iph;          /* IPv4 header */
    iph = ip_hdr(skb);          /* get IP header */

    //We only check UDP
    if (iph->protocol != IPPROTO_UDP)
        return 0;
    return iph->daddr;
}


enum MATCH_RESULT {
    ROLL_BACK = 0,
    SUCCESS = 1,
    FAIL = 2,
};
/*
 *  0 -> roll back to probability
 *  1 -> match success
 *  2 -> match fail
 */
static enum MATCH_RESULT checkL7LB(struct sk_buff *skb)
{
    struct udphdr *udph;        /* UDP header */
    unsigned char *user_data;   /* UDP data begin pointer */
    enum MATCH_RESULT ret = ROLL_BACK;

    udph = udp_hdr(skb);        /* get UDP header */
    /* Calculate pointers for begin and end of UDP packet data */
    user_data = (unsigned char *)((unsigned char *)udph + sizeof(struct udphdr));

    //Try to prinout the current IP
    //We use the first 4 bit for counter, k8s use the 12th-16th bit
    printk(KERN_INFO "current IP is %s", podIP[(skb->mark&(0x000f))]);

    if (strlen(user_data) >= strlen(targetDomain)) {
        if (0 != (strncmp(user_data, targetDomain, strlen(targetDomain))))
            goto END;

        if (strlen(podIP[(skb->mark&(0x000f))]) != strlen(targetIP))
	{
	    ret = FAIL;	
            goto END;
	}
        //Return true if cuurent IP is target IP
        if (0 != (strncmp(podIP[(skb->mark&(0x000f))], targetIP, strlen(targetIP))))
       	{
	    ret = FAIL;	
            goto END;
	}

        printk(KERN_INFO "Find the target !\n");
        ret = SUCCESS;
    }

END:
    skb->mark++;
    return ret;
}

    static bool
statistic_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
    const struct xt_statistic_info *info = par->matchinfo;
    bool ret = info->flags & XT_STATISTIC_INVERT;
    int nval, oval;

    switch (info->mode) {
        case XT_STATISTIC_MODE_RANDOM:
            //If we set the clusterIP, try to match.
            if (strlen(clusterIP)!=0) {
                unsigned int cluster_ip = in_aton(clusterIP);
                unsigned int dest_ip = getDestinationIP(skb);
                enum MATCH_RESULT result;

                printk(KERN_INFO "try to match %d:%d", cluster_ip, dest_ip);
                if (cluster_ip == dest_ip) {
                    printk(KERN_INFO "match the IP address");
                    result = checkL7LB((struct sk_buff*)skb);

                    if (result == SUCCESS)
                        return true;
                    else if (result == FAIL)
                        return false;
                }
            }
            if ((prandom_u32() & 0x7FFFFFFF) < info->u.random.probability)
                ret = !ret;
            break;
        case XT_STATISTIC_MODE_NTH:
            do {
                oval = atomic_read(&info->master->count);
                nval = (oval == info->u.nth.every) ? 0 : oval + 1;
            } while (atomic_cmpxchg(&info->master->count, oval, nval) != oval);
            if (nval == 0)
                ret = !ret;
            break;
    }

    return ret;
}

static int statistic_mt_check(const struct xt_mtchk_param *par)
{
    struct xt_statistic_info *info = par->matchinfo;

    if (info->mode > XT_STATISTIC_MODE_MAX ||
            info->flags & ~XT_STATISTIC_MASK)
        return -EINVAL;

    info->master = kzalloc(sizeof(*info->master), GFP_KERNEL);
    if (info->master == NULL)
        return -ENOMEM;
    atomic_set(&info->master->count, info->u.nth.count);

    return 0;
}

static void statistic_mt_destroy(const struct xt_mtdtor_param *par)
{
    const struct xt_statistic_info *info = par->matchinfo;

    kfree(info->master);
}

static struct xt_match xt_statistic_mt_reg __read_mostly = {
    .name       = "statistic",
    .revision   = 0,
    .family     = NFPROTO_UNSPEC,
    .match      = statistic_mt,
    .checkentry = statistic_mt_check,
    .destroy    = statistic_mt_destroy,
    .matchsize  = sizeof(struct xt_statistic_info),
    .usersize   = offsetof(struct xt_statistic_info, master),
    .me         = THIS_MODULE,
};

char *dirname="k8s";
struct proc_dir_entry *parent;
static int __init statistic_mt_init(void)
{
    parent = proc_mkdir(dirname, NULL);
    if (parent == NULL)
        return -ENOMEM;
    entClusterIP = proc_create("clusterIP",0220, parent, &clusterIPops);
    if (entClusterIP == NULL)
        return -ENOMEM;
    entHTTP = proc_create("http",0220, parent, &httpops);
    if (entHTTP == NULL)
        return -ENOMEM;
    entPodIP = proc_create("podIP",0220, parent, &podIPops);
    if (entPodIP == NULL)
        return -ENOMEM;
    printk(KERN_INFO "create three proc fs \n");
    return xt_register_match(&xt_statistic_mt_reg);
}

static void __exit statistic_mt_exit(void)
{
    //Debug Message
    int i = 0;
    printk(KERN_INFO "exit module \n");
    printk(KERN_INFO "%s\n", clusterIP);
    printk(KERN_INFO "%s->%s\n", targetDomain,targetIP);
    printk(KERN_INFO "podIP size %d", curPodSize);
    for (i = 0; i < curPodSize; i++)
        printk(KERN_INFO "%s\n", podIP[i]);

    if (entClusterIP)
        proc_remove(entClusterIP);
    if (entHTTP)
        proc_remove(entHTTP);
    if (entPodIP)
        proc_remove(entPodIP);
    if (parent)
        proc_remove(parent);
    xt_unregister_match(&xt_statistic_mt_reg);
}

module_init(statistic_mt_init);
module_exit(statistic_mt_exit);
