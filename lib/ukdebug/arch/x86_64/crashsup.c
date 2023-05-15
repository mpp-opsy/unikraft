/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2024, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the BSD-3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 */

#include <uk/essentials.h>
#include <uk/debug/crash.h>
#include <uk/plat/lcpu.h>
#include <x86/traps.h>

#include "crashdump.h"

extern __u8 _uk_debug_explicit_crash;

void uk_crash_event_param(struct ukarch_trap_ctx *ctx,
			  struct uk_event_crash_parameter *param)
{
	param->regs = ctx->regs;

	if (_uk_debug_explicit_crash) {
		param->descr.reason = UK_CRASH_REASON_EXPLICIT;
		param->descr.errno = 0;
		return;
	}

	if (ctx->trapnr == TRAP_page_fault) {
		param->descr.reason = UK_CRASH_REASON_PAGE_FAULT;
		param->descr.errno = ctx->uk_errno;
		param->descr.arg1 = ctx->fault_address;
		param->descr.arg2 = ctx->error_code;
		return;
	}

	param->descr.reason = UK_CRASH_REASON_UNHANDLED_TRAP;
	param->descr.errno = 0;
	param->descr.arg1 = ctx->trapnr;
	param->descr.arg2 = (__u64)ctx->str;
	param->descr.arg3 = ctx->error_code;
}
