/*
 * Copyright 2012, Samsung Telecommunications of America
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Written by William Roberts <w.roberts@sta.samsung.com>
 */

#ifndef _LIBAUDIT_H_
#define _LIBAUDIT_H_

#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/audit.h>

#define MAX_AUDIT_MESSAGE_LENGTH    8970

#define AUDIT_OFF       0
#define AUDIT_ON        1
#define AUDIT_LOCKED    2

typedef enum {
    GET_REPLY_BLOCKING=0,
    GET_REPLY_NONBLOCKING
} reply_t;

typedef enum {
    WAIT_NO,
    WAIT_YES
} rep_wait_t;

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

/**
 * Opens a connection to the Audit netlink socket
 * @return
 *  A valid fd on success or < 0 on error with errno set.
 *  Returns the same errors as man 2 socket.
 */
extern int  audit_open(void);

/**
 * Closes the fd returned from audit_open()
 * @param fd
 *  The fd to close
 */
extern void audit_close(int fd);

/**
 *
 * @param fd
 *  The fd returned by a call to audit_open()
 * @param rep
 *  The response struct to store the response in.
 * @param block
 *  Whether or not to block on IO
 * @param peek
 *  Whether or not we are to remove the message from
 *  the queue when we do a read on the netlink socket.
 * @return
 *  This function returns 0 on success, else -errno.
 */
extern int  audit_get_reply(int fd, struct audit_reply *rep, reply_t block,
               int peek);

/**
 * Sets a pid to recieve audit netlink events from the kernel
 * @param fd
 *  The fd returned by a call to audit_open()
 * @param pid
 *  The pid whom to set as the reciever of audit messages
 * @param wmode
 *  Whether or not to block on the underlying socket io calls.
 * @return
 *  This function returns 0 on success, -errno on error.
 */
extern int  audit_set_pid(int fd, uint32_t pid, rep_wait_t wmode);

/**
 * Sends a command to the audit netlink socket
 * @param fd
 *  The fd returned by a call to audit_open()
 * @param type
 *  message type, see audit.h in the kernel
 * @param data
 *  opaque data pointer
 * @param size
 *  size of data in *data
 * @return
 *  This function returns 0 on success, -errno on error.
 */
extern int  audit_send(int fd, int type, const void *data, unsigned int size);

/**
 * Allocates a rule and adds a directory to watch, defaults to all permissions.
 * Call audit_update_watch_perms() subsequently to update permissions.
 * @param rulep
 *  double pointer to an unallocated audit_rule_data, which will be allocated. This must be freed
 * @param path
 *  path to add to the rule
 * @return
 *  This function returns 0 on success, -errno on error.
 */
extern int  audit_add_dir(struct audit_rule_data **rulep, const char *path);

/**
 * Sets enabled flag, 0 for audit off, 1 for audit on, 2 for audit locked
 * @param fd
 *  file descripter returned by audit_open()
 * @param state
 *  0 for audit off, 1 for audit on, 2 for audit locked
 * @return
 *  This function returns 0 on success, -errno on error, -1 if already locked
 */
extern int  audit_set_enabled(int fd, uint32_t state);

/**
 * Sets permissions for an already allocated watch rule
 * @param rule
 *  rule to set permissions on
 * @param perms
 *  permissions to set, AUDIT_PERM_{READ,WRITE,EXEC,ATTR}
 * @return
 *  This function returns 0 on success, -1 if rule is NULL and -2 if there are too many fields
 */
extern int  audit_update_watch_perms(struct audit_rule_data *rule, int perms);

/**
 * Sets permissions for an already allocated watch rule
 * @param rule
 *  rule to add field to
 * @param field
 *  field from audit.h AUDIT_PID...AUDIT_FILETYPE
 * @param oper
 *  operator from audit.h AUDIT_EQUAL|AUDIT_NOT_EQUAL|AUDIT_BIT_MASK
 * @param value
 *  value to match for the field (e.g., uid=1000, 1000 is the value)
 * @return
 *  This function returns 0 on success, -1 if rule is NULL and -2 if there are too many fields
 */
extern int  audit_add_field(struct audit_rule_data *rule, int field, int oper, char *value);

#endif
