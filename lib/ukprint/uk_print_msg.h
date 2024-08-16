/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the BSD-3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 */

#ifndef __UK_PRINT_MSG_H__
#define __UK_PRINT_MSG_H__

#include <uk/arch/time.h>
#include <uk/essentials.h>

extern unsigned int uk_print_console_lvl;

struct uk_print_msg {
	int flags;
	__u16 libid;
#if CONFIG_LIBUKPRINT_PRINT_SRCNAME
	const char *srcname;
	unsigned int srcline;
#endif /* CONFIG_LIBUKPRINT_PRINT_SRCNAME */
#if CONFIG_LIBUKPRINT_PRINT_TIME
	__nsec timestamp;
#endif /* CONFIG_LIBUKPRINT_PRINT_TIME */
#if CONFIG_LIBUKPRINT_PRINT_THREAD
	const char *threadname; /* may be NULL */
#endif /* CONFIG_LIBUKPRINT_PRINT_THREAD */
#if CONFIG_LIBUKPRINT_PRINT_CALLER
	__uptr retaddr;
	__uptr frameaddr;
#endif /* CONFIG_LIBUKPRINT_PRINT_CALLER */
	const char *fmt;
	va_list ap;
#if CONFIG_LIBUKPRINT_LOGBUF
	__sz len;
	char msg[];
#endif /* CONFIG_LIBUKPRINT_LOGBUF */
};

#if CONFIG_LIBUKCONSOLE
void uk_print_console_write(struct uk_print_msg *msg);
#endif /* CONFIG_LIBUKCONSOLE */

#if CONFIG_LIBUKPRINT_LOGBUF
int uk_print_logbuf_write(struct uk_print_msg *msg);
#endif /* CONFIG_LIBUKPRINT_LOGBUF */

#endif /* __UK_PRINT_MSG_H__ */
