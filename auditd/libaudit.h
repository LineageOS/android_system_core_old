#ifndef _LIBAUDIT_H_
#define _LIBAUDIT_H_
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/audit.h>

#define MAX_AUDIT_MESSAGE_LENGTH    8970

typedef enum { GET_REPLY_BLOCKING=0, GET_REPLY_NONBLOCKING } reply_t;
typedef enum { WAIT_NO, WAIT_YES } rep_wait_t;

struct audit_sig_info {
	uid_t uid;
	pid_t pid;
	char ctx[0];
};

struct audit_message {
    struct nlmsghdr nlh;
    char   data[MAX_AUDIT_MESSAGE_LENGTH];
};

struct audit_reply {
    int                      type;
    int                      len;
    struct nlmsghdr         *nlh;
    struct audit_message     msg;

    union {
        struct audit_status     *status;
        struct audit_rule_data  *ruledata;
        const char              *message;
        struct nlmsgerr         *error;
        struct audit_sig_info   *signal_info;
    };
};

extern int  audit_open(void);
extern void audit_close(int fd);
extern int  audit_get_reply(int fd, struct audit_reply *rep, reply_t block,
               int peek);
extern int  audit_set_pid(int fd, uint32_t pid, rep_wait_t wmode);

#endif
