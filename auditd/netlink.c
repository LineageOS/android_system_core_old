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
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include <cutils/klog.h>

#include "auditd.h"
#include "libaudit.h"

/* Relying on static to initialize to 0, these are used to initialize structs to 0 */
static const struct audit_message _new_req;
static const struct sockaddr_nl _new_addr;

/**
 * Copies the netlink message data to the reply structure.
 *
 * When the kernel sends a response back, we must adjust the response from the nlmsghdr.
 * All the data is is rep->msg but belongs in type enforced fields in the struct.
 *
 * @param rep
 *  The response
 * @param len
 *  The length of ther message, len must never be less than 0!
 * @return
 *  This function returns 0 on success.
 */
static int set_internal_fields(struct audit_reply *rep, ssize_t len) {

	int rc = 0;
	int line = 0;

	/*
	 * We end up setting a specific field in the union, but since it
	 * is a union and they are all of type pointer, we can just clear
	 * one.
	 */
	rep->status = NULL;

	/* Set the response from the netlink message */
	rep->nlh      = &rep->msg.nlh;
	rep->len      = rep->msg.nlh.nlmsg_len;
	rep->type     = rep->msg.nlh.nlmsg_type;

	/* Check if the reply from the kernel was ok */
	if (!NLMSG_OK(rep->nlh, (size_t)len)) {
		rc = (len == sizeof(rep->msg)) ? EFBIG : EBADE;
		line = __LINE__;
		goto err;
	}

	/* Next we'll set the data structure to point to msg.data. This is
	 * to avoid having to use casts later. */
	if (rep->type == NLMSG_ERROR) {
		rep->error = NLMSG_DATA(rep->nlh);
	}
	else if (rep->type == AUDIT_GET) {
		rep->status  = NLMSG_DATA(rep->nlh);
	}
	else if (rep -> type == AUDIT_LIST_RULES) {
		rep->ruledata = NLMSG_DATA(rep->nlh);
	}
	else if (rep->type == AUDIT_SIGNAL_INFO) {
		rep->signal_info = NLMSG_DATA(rep->nlh);
	}
	/* If it is not any of the above specific events, it must be a generic message */
	else {
		rep->message = NLMSG_DATA(rep->nlh);
	}
out:
	return rc;
err:
	errno = GETERRNO(rc);
	ERROR("%s    Failed in %s around line %d with error: %s\n", __FILE__, __FUNCTION__, line, strerror(errno));
	rc = -1;
	goto out;
}

/**
 * Waits for an ack from the kernel
 * @param fd
 *  The netlink socket fd
 * @param seq
 *  The current sequence number were acking on
 * @return
 *  0 on success
 */
static int get_ack(int fd, int16_t seq) {

	int rc = 0;
	int line = 0;

	struct audit_reply rep;

	/* Sanity check the input, this is an internal interface this shouldn't happen */
	if (fd < 0) {
		rc = EINVAL;
		line = __LINE__;
		goto err;
	}

	rc = -audit_get_reply(fd, &rep, GET_REPLY_BLOCKING, MSG_PEEK);
	if(rc < 0) {
		line = __LINE__;
		goto err;
	}
	else if (rep.type == NLMSG_ERROR) {
		(void)audit_get_reply(fd, &rep, GET_REPLY_BLOCKING, 0);
		if (rep.error->error) {
			line = __LINE__;
			rc = rep.error->error;
			goto err;
		}
	}
out:
	return rc;
err:
	errno = GETERRNO(rc);
	ERROR("%s    Failed in %s around line %d with error: %s\n", __FILE__, __FUNCTION__, line, strerror(errno));
	rc = -1;
	goto out;
}

/**
 * Opens a connection to the Audit netlink socket
 * @return
 *  A valid fd on success or < 0 on error with errno set.
 *  Returns the same errors as man 2 socket.
 */
int audit_open() {
	return socket(PF_NETLINK, SOCK_RAW, NETLINK_AUDIT);
}

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
 *  0 on success, errno on error
 */
int audit_get_reply(int fd, struct audit_reply *rep, reply_t block, int peek) {

	int rc;
	ssize_t len;
	int flags;
	int line = 0;

	struct sockaddr_nl nladdr;
	socklen_t nladdrlen = sizeof(nladdr);

	if (fd < 0) {
		rc = EBADF;
		line = __LINE__;
		goto err;
	}

	/* Set up the flags for recv from */
	flags = (block == GET_REPLY_NONBLOCKING) ? MSG_DONTWAIT : 0;
	flags |= peek;

	/*
	 * Get the data from the netlink socket but on error we need to be carefull,
	 * the interface shows that EINTR can never be returned, other errors, however,
	 * can be returned.
	 */
	do {

		len = recvfrom(fd, &rep->msg, sizeof(rep->msg), flags,
			(struct sockaddr*)&nladdr, &nladdrlen);

		/*
		 * EAGAIN and EINTR should be re-tried until success or
		 * another error manifests.
		 */
		if (len < 0 && errno != EINTR) {
			if (block == GET_REPLY_NONBLOCKING && errno == EAGAIN) {
				rc = 0;
				goto out;
			}
			rc = errno;
			line = __LINE__;
			goto err;
		}

	/* 0 or greater indicates success */
	} while (len < 0);

	if (nladdrlen != sizeof(nladdr)) {
		rc = EPROTO;
		line == __LINE__;
		goto err;
	}

	/* Make sure the netlink message was not spoof'd */
	if (nladdr.nl_pid) {
		rc = EINVAL;
		line == __LINE__;
		goto err;
	}

	rc = set_internal_fields(rep, len);
	if (rc < 0) {
		rc = errno;
		line == __LINE__;
		goto err;
	}

out:
	return rc;
err:
	errno = GETERRNO(rc);
	ERROR("%s    Failed in %s around line %d with error: %s\n", __FILE__, __FUNCTION__, line, strerror(errno));
	rc = -1;
	goto out;
}

/**
 * Closes the fd returned from audit_open()
 * @param fd
 *  The fd to close
 *
 * @note
 *  This function was left void to be complaint
 *  to an existing interface, however it clears
 *  errno on entry, so errno can be used to check
 *  if the fd you gave close was wrong.
 *
 */
void audit_close(int fd) {

	int line = 0;
	int rc = close(fd);
	if(rc) {
		line == __LINE__;
		rc = errno;
		goto err;
	}
out:
	return;

err:
	errno = GETERRNO(rc);
	ERROR("%s    Failed in %s around line %d with error: %s\n", __FILE__, __FUNCTION__, line, strerror(errno));
	goto out;
}

/**
 *
 * @param fd
 *  The netlink socket fd
 * @param type
 *  The type of netlink message
 * @param data
 *  The data to send
 * @param size
 *  The length of the data in bytes
 * @return
 *  Sequence number on success or -errno on error.
 */
int audit_send(int fd, int type, const void *data, unsigned int size) {

	int rc = 0;
	static int16_t sequence = 0;
	struct audit_message req = _new_req;
	struct sockaddr_nl addr = _new_addr;

	/* We always send netlink messaged */
	addr.nl_family = AF_NETLINK;

	/* Set up the netlink headers */
	req.nlh.nlmsg_type = type;
	req.nlh.nlmsg_len = NLMSG_SPACE(size);
	req.nlh.nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;

	/*
	 * Check for a valid fd, even though sendto would catch this, its easier to always
	 * blindly increment the sequence number
	 */
	if (fd < 0) {
		rc = EBADF;
		goto err;
	}

	/* Ensure the message is not too big */
	if (NLMSG_SPACE(size) > MAX_AUDIT_MESSAGE_LENGTH) {
		rc = EINVAL;
		goto err;
	}

	/* Only memcpy in the data if it was specified */
	if (size && data)
		memcpy(NLMSG_DATA(&req.nlh), data, size);

	/*
	 * Only increment the sequence number on a guarantee
	 * you will send it to the kernel.
	 *
	 * Also, the sequence is defined as a u32 in the kernel
	 * struct. Using an int here might not work on 32/64 bit splits. A
	 * signed 64 bit value can overflow a u32..but a u32
	 * might not fit in the response, so we need to use s32.
	 * Which is still kind of hackish since int could be 16 bits
	 * in size. The only safe type to use here is a signed 16
	 * bit value.
	 */
	req.nlh.nlmsg_seq = ++sequence;

	/* While failing and its due to interrupts */
	do {
		/* Try and send the netlink message */
		rc = sendto(fd, &req, req.nlh.nlmsg_len, 0,
			(struct sockaddr*)&addr, sizeof(addr));

	} while (rc < 0 && errno == EINTR);

	/* Not all the bytes were sent */
	if ((uint32_t)rc != req.nlh.nlmsg_len) {
		rc = EPROTO;
		goto err;
	}
	else if (rc < 0) {
		rc = errno;
		goto out;
	}

	/* We sent all the bytes, get the ack */
	rc = (get_ack(fd, sequence) == 0) ? (int)sequence : rc;

out:

	/* Don't let sequence roll to negative */
	sequence = (sequence < 0) ? 0 : sequence;

	return rc;

err:
	errno = rc;
	rc = -rc;
	goto out;
}

