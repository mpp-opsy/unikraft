/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2023, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the BSD-3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 */

#ifndef __UK_DEBUG_ARCH_CRASH_H__
#define __UK_DEBUG_ARCH_CRASH_H__

#include <uk/essentials.h>

#ifdef __cplusplus
extern "C" {
#endif

extern __u8 _uk_debug_explicit_crash;

#define ukarch_trigger_crash()						\
	do {								\
		__asm__ __volatile__ (					\
			"strb %w[one], %[indicator]\n"			\
			".inst 0xdeff\n"				\
			: [indicator] "=m" (_uk_debug_explicit_crash)	\
			: [one] "r" (1)					\
		);							\
		__builtin_unreachable();				\
	} while (0)

#ifdef __cplusplus
}
#endif

#endif /* __UK_DEBUG_ARCH_CRASH_H__ */
