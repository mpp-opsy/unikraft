/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the BSD-3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 */

#include <errno.h>
#include <string.h>

#include <uk/assert.h>
#include <uk/console.h>
#include <uk/spinlock.h>

#include "snprintf.h"
#include "uk_print_msg.h"

#define LOGMSG_ALIGN		16
#define LOGMSG_SIZE(_msg_len)	(sizeof(struct uk_print_logmsg) + (_msg_len))

struct uk_print_logmsg {
	struct uk_print_logmsg *next;
	struct uk_print_msg msg;
};

static __u8 logbuf[CONFIG_LIBUKPRINT_LOGBUF_SIZE];

static struct uk_print_logmsg *logbuf_head;
static struct uk_print_logmsg *logbuf_tail;

static uk_spinlock logbuf_lock = UK_SPINLOCK_INITIALIZER();

/* Returns the next free aligned position in the buffer, or wraps
 * around.
 */
static inline __uptr logbuf_next(void)
{
	__uptr next;

	if (!logbuf_head)
		return (__uptr)logbuf;

	next = ALIGN_UP((__uptr)logbuf_tail + LOGMSG_SIZE(logbuf_tail->msg.len),
			LOGMSG_ALIGN);

	if (next >= (__uptr)logbuf + sizeof(logbuf))
		next = (__uptr)logbuf;

	return next;
}

static inline void update_head(struct uk_print_logmsg *logmsg, __sz msg_len)
{
	struct uk_print_logmsg *lm;
	int found = 0; /* bool */

	if (RANGE_OVERLAP((__uptr)logbuf_head, LOGMSG_SIZE(logbuf_head->msg.len),
			  (__uptr)logmsg, LOGMSG_SIZE(msg_len))) {
		for (lm = logbuf_head; lm != lm->next; lm = lm->next) {
			if ((__uptr)lm->next > (__uptr)logmsg + LOGMSG_SIZE(msg_len)) {
				logbuf_head = lm->next;
				found = 1;
				break;
			}
		}

		if (!found) {
			/* This logmsg is the last element in the buffer.
			 * Set head to the first element.
			 */
			logbuf_head = (struct uk_print_logmsg *)logbuf;
		}
	}
}

int uk_print_logbuf_write(struct uk_print_msg *msg)
{
	struct uk_print_logmsg *logmsg;
	va_list cp;

	uk_spin_lock(&logbuf_lock);

	va_copy(cp, msg->ap);
	msg->len = vsnprintf(msg->msg, 0, msg->fmt, cp);
	msg->len++; /* NUL byte */

	if (LOGMSG_SIZE(msg->len) > sizeof(logbuf)) {
		uk_spin_unlock(&logbuf_lock);
		return -EMSGSIZE;
	}

	logmsg = (struct uk_print_logmsg *)logbuf_next();

	/* If we exceed the buffer, wrap around */
	if ((__uptr)logmsg + LOGMSG_SIZE(msg->len) > (__uptr)logbuf + sizeof(logbuf))
		logmsg = (struct uk_print_logmsg *)logbuf;

	/* Update logbuf_head before we overwrite any message objects.
	 * The first needs to be checked / updated upon every new write
	 * after the first wraparound.
	 */
	if (logbuf_head)
		update_head(logmsg, msg->len);

	memcpy(&logmsg->msg, msg, sizeof(*msg));

	/* Format the message now. Once the caller returns,
	 * fmt and va_list will no longer be available.
	 */
	va_copy(cp, msg->ap);
	vsnprintf(logmsg->msg.msg, msg->len, msg->fmt, cp);

	if (!logbuf_head) {
		logbuf_head = logmsg;
		logbuf_tail = logmsg;
	}
	logmsg->next = logmsg;

	logbuf_tail->next = logmsg; /* update previous last */
	logbuf_tail = logmsg;

	uk_spin_unlock(&logbuf_lock);

	return 0;
}

void uk_print_dmesg(void)
{
#if CONFIG_LIBUKCONSOLE
	struct uk_print_logmsg *lm;

	uk_spin_lock(&logbuf_lock);

	if (!logbuf_head) {
		uk_spin_unlock(&logbuf_lock);
		return;
	}

	for (lm = logbuf_head; lm != lm->next; lm = lm->next)
		uk_console_out(lm->msg.msg, lm->msg.len);
	uk_console_out(lm->msg.msg, lm->msg.len);

	uk_spin_unlock(&logbuf_lock);
#endif /* CONFIG_LIBUKCONSOLE */
}
