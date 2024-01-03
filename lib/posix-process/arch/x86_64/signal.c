/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2023, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the BSD-3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 */

#include <signal.h>
#include <uk/arch/ctx.h>

void pprocess_signal_arch_jmp_handler(struct ukarch_execenv *execenv,
				      int signum, siginfo_t *si,
				      ucontext_t *ctx,
				      void *handler, void *sp)
{
	struct ukarch_sysctx uk_sysctx;

	ukarch_sysctx_store(&uk_sysctx);
	ukarch_sysctx_load(&execenv->sysctx);

	__asm__ volatile ("movq    %%rsp, %%r12\t\n" /* save uk sp */
			  "movq    %4, %%rsp\t\n"    /* switch to handler sp */
			  "movq    %0, %%rdi\t\n"    /* arg0 signum */
			  "movq    %1, %%rsi\t\n"    /* arg1 siginfo */
			  "movq    %2, %%rdx\t\n"    /* arg2 ucontext */
			  "call    *%3\t\n"          /* call handler */
			  "movq    %%r12, %%rsp\t\n" /* restore uk sp */
			  :
			  : "r" ((unsigned long)signum), "r" (si),
			    "r" (ctx), "r" (handler), "r" (sp)
			  : "rdi", "rsi", "rdx", "memory");

	ukarch_sysctx_store(&execenv->sysctx);
	ukarch_sysctx_load(&uk_sysctx);
}
