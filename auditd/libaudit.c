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
 *
 */
#include <errno.h>
#include <string.h>

#include <cutils/klog.h>

#include "auditd.h"
#include "libaudit.h"

/* Only should be used internally */
extern int audit_send(int fd, int type, const void *data, unsigned int size);

/* Depending on static to initialize this to 0 */
static const struct audit_status _new_audit_status;

int audit_set_pid(int fd, uint32_t pid, rep_wait_t wmode) {

	int rc;
	int line;
	struct audit_reply rep;
	struct audit_status status = _new_audit_status;

	/*
	 * In order to set the auditd PID we send an audit message over the netlink socket
	 * with the pid field of the status struct set to our current pid, and the
	 * the mask set to AUDIT_STATUS_PID
	 */
	status.pid  = pid;
	status.mask = AUDIT_STATUS_PID;

	/* Let the kernel know this pid will be registering for audit events */
	rc = audit_send(fd, AUDIT_SET, &status, sizeof(status));
	if (rc < 0) {
		line = __LINE__;
		goto err;
	}

	/*
	 * In a request where we need to wait for a response, wait for the message
	 * and discard it. This message confirms and sync's us with the kernel.
	 * This daemon is now registered as the audit logger. Only wait if the
	 * wmode is != WAIT_NO
	 */
	if (wmode != WAIT_NO) {
		/* TODO
		 * If the daemon dies and restarts the message didn't come back,
		 * so I went to non-blocking and it seemed to fix the bug.
		 * Need to investigate further.
		 */
		(void)audit_get_reply(fd, &rep, GET_REPLY_NONBLOCKING, 0);
	}

out:
	return rc;

err:
	errno = GETERRNO(rc);
	ERROR("%s    Failed in %s around line %d with error: %s\n", __FILE__, __FUNCTION__, line, strerror(errno));
	rc = -1;
	goto out;
}
