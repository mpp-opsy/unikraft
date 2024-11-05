/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2021, Karlsruhe Institute of Technology. All rights reserved.
 * Copyright (c) 2023, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the BSD-3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 */

#ifndef __UKDEBUG_INTERNAL_CRASHDUMP_H__
#define __UKDEBUG_INTERNAL_CRASHDUMP_H__

#include <uk/arch/lcpu.h>
#include <uk/arch/traps.h>
#include <uk/debug/crash.h>

#define crash_printk(fmt, ...)						\
	_uk_printk(UK_PRINT_KLVL_CRIT, UKLIBID_NONE, __NULL, 0, fmt,	\
		   ##__VA_ARGS__)

#define crash_vprintk(fmt, ap)						\
	_uk_vprintk(UK_PRINT_KLVL_CRIT, UKLIBID_NONE, __NULL, 0, fmt, ap)

void uk_crash_event_param(struct ukarch_trap_ctx *ctx,
			  struct uk_event_crash_parameter *param);

void crash_print_crashdump(struct __regs *regs);

void cdmp_gen_print_stack(unsigned long sp);
void cdmp_gen_print_call_trace_entry(unsigned long addr);

/* Must be implemented in the architecture-specific ukdebug support */
void cdmp_arch_print_registers(struct __regs *regs);
void cdmp_arch_print_stack(struct __regs *regs);
#if !__OMIT_FRAMEPOINTER__
void cdmp_arch_print_call_trace(struct __regs *regs);
#endif /* !__OMIT_FRAMEPOINTER__ */

#endif /* __UKDEBUG_INTERNAL_CRASHDUMP_H__ */
