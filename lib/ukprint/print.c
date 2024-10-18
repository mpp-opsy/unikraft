/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024, Unikraft GmbH and The Unikraft Authors.
 * Copyright (c) 2017, NEC Europe Ltd., NEC Corporation. All rights reserved.
 * Licensed under the BSD-3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 */

#include <stdint.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>

#include <uk/essentials.h>
#include <uk/print.h>

#include "snprintf.h"
#include "uk_print_msg.h"

#if CONFIG_LIBUKPRINT_PRINT_SRCNAME
#define UK_PRINT_MSG_ARGS_SRCNAME(_msg, _srcname, _srcline)	\
	(_msg).srcname = (_srcname);				\
	(_msg).srcline = (_srcline)
#else
#define UK_PRINT_MSG_ARGS_SRCNAME(_msg, _srcname, _srcline)
#endif /* CONFIG_LIBUKPRINT_PRINT_SRCNAME */

#if CONFIG_LIBUKPRINT_PRINT_CALLER
#define UK_PRINT_MSG_ARGS_CALLER(_msg)				\
	(_msg).retaddr = __return_addr(0);			\
	(_msg).frameaddr = __frame_addr(0)
#else
#define UK_PRINT_MSG_ARGS_CALLER(_msg)
#endif /* CONFIG_LIBUKPRINT_PRINT_CALLER */

#define UK_PRINT_MSG_ALLOC(_msg, _flags, _libid, _srcname, _srcline,	\
			   _fmt, _ap)					\
	do {								\
		(_msg) = (struct uk_print_msg) {			\
			.flags = (_flags),				\
			.libid = (_libid),				\
			.fmt = (_fmt),					\
		};							\
		UK_PRINT_MSG_ARGS_SRCNAME((_msg), (_srcname),		\
					  (_srcline));			\
		UK_PRINT_MSG_ARGS_CALLER((_msg));			\
		va_copy((_msg).ap, (_ap));				\
	} while (0)

#define UK_PRINT_MSG_FREE(_msg)						\
		va_end((_msg).ap)					\

#if CONFIG_LIBUKPRINT_PRINTK
void _uk_vprintk(int flags, __u16 libid,
		 const char *srcname __maybe_unused,
		 unsigned int srcline __maybe_unused,
		 const char *fmt, va_list ap)
{
	struct uk_print_msg msg;

	UK_PRINT_MSG_ALLOC(msg, flags, libid, srcname, srcline, fmt, ap);

#if CONFIG_LIBUKPRINT_LOGBUF
	uk_print_logbuf_write(&msg);
#endif /* CONFIG_LIBUKPRINT_LOGBUF */

#if CONFIG_LIBUKCONSOLE
	uk_print_console_write(&msg);
#endif /* CONFIG_LIBUKCONSOLE */

	UK_PRINT_MSG_FREE(msg);
}

void _uk_printk(int flags, __u16 libid,
		const char *srcname __maybe_unused,
		unsigned int srcline __maybe_unused,
		const char *fmt, ...)
{
	struct uk_print_msg msg;
	va_list ap;

	va_start(ap, fmt);
	UK_PRINT_MSG_ALLOC(msg, flags, libid, srcname, srcline, fmt, ap);

#if CONFIG_LIBUKPRINT_LOGBUF
	uk_print_logbuf_write(&msg);
#endif /* CONFIG_LIBUKPRINT_LOGBUF */

#if CONFIG_LIBUKCONSOLE
	uk_print_console_write(&msg);
#endif /* CONFIG_LIBUKCONSOLE */

	UK_PRINT_MSG_FREE(msg);
	va_end(ap);
}
#endif /* CONFIG_LIBUKPRINT_PRINTK */
