/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2021, Karlsruhe Institute of Technology. All rights reserved.
 * Copyright (c) 2023, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the BSD-3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 */

#include <stdarg.h>
#include <stddef.h>

#include <uk/arch/traps.h>
#include <uk/config.h>
#include <uk/debug/crash.h>
#include <uk/essentials.h>
#include <uk/event.h>
#include <uk/plat/lcpu.h>
#include <uk/plat/bootstrap.h>
#include <uk/plat/time.h>
#include <uk/preempt.h>

#if CONFIG_LIBUKSCHED && CONFIG_LIBUKNOFAULT
#include <uk/nofault.h>
#include <uk/thread.h>
#endif /* CONFIG_LIBUKSCHED && CONFIG_LIBUKNOFAULT */

#include "crashdump.h"

#define __CRASH_REBOOT_HINT(delay)					\
	"System rebooting in " #delay " seconds...\n"
#define _CRASH_REBOOT_HINT(delay)					\
	__CRASH_REBOOT_HINT(delay)
#define CRASH_REBOOT_HINT						\
	_CRASH_REBOOT_HINT(CONFIG_LIBUKDEBUG_CRASH_REBOOT_DELAY)

__u8 _uk_debug_explicit_crash; /* 0: not explicit */

#if CONFIG_LIBUKDEBUG_CRASH_SCREEN
static void crash_print_thread_info(void)
{
#if CONFIG_LIBUKSCHED && CONFIG_LIBUKNOFAULT
	struct uk_thread *current = uk_thread_current();
	const char *name;

	if (!current)
		return;

	if (uk_nofault_probe_r((__vaddr_t)current, sizeof(struct uk_thread),
			       0) != sizeof(struct uk_thread)) {
		crash_printk("Current thread information corrupted\n");
		return;
	}
	name = uk_nofault_probe_r((__vaddr_t)current->name, 1, 0) == 1
	       ? current->name
	       : "(corrupted)";

	crash_printk("Thread \"%s\"@%p\n", name, current);

#endif /* CONFIG_LIBUKSCHED && CONFIG_LIBUKNOFAULT */
}
#endif /* CONFIG_LIBUKDEBUG_CRASH_SCREEN */

__noreturn static void crash_shutdown(void)
{
#if CONFIG_LIBUKDEBUG_CRASH_ACTION_REBOOT
	__nsec until;
	__nsec now;

	now = ukplat_monotonic_clock();
	until = now + ukarch_time_sec_to_nsec(CONFIG_LIBUKDEBUG_CRASH_REBOOT_DELAY);

	if (until > 0) {
		crash_printk(CRASH_REBOOT_HINT);
		/* Interrupts are disabled. Just busy wait... */
		while (until > ukplat_monotonic_clock())
			; /* do nothing */
	}
	ukplat_restart();
#else /* !CONFIG_LIBUKDEBUG_CRASH_ACTION_REBOOT */
	ukplat_crash();
#endif /* !CONFIG_LIBUKDEBUG_CRASH_ACTION_REBOOT */
}

UK_EVENT(UK_CRASH_EVENT);

int uk_debug_crash_handler(void *data)
{
	struct uk_event_crash_parameter param;
	struct ukarch_trap_ctx *ctx;
#ifdef UKPLAT_LCPU_MULTICORE
	static __s32 crash_cpu = -1;
	int cpu_id = ukplat_lcpu_id();
#endif /* UKPLAT_LCPU_MULTICORE */

	UK_ASSERT(data);

	ctx = (struct ukarch_trap_ctx *)data;

#if CONFIG_UK_ARCH_X86_64 /* FIXME */
	lcpu_push_nested_exceptions();
#endif /* CONFIG_UK_ARCH_X86_64 */
	ukplat_lcpu_disable_irq();
	uk_preempt_disable();

#ifdef UKPLAT_LCPU_MULTICORE
	#warning The crash code does not support multicore systems

	/* Only let one CPU perform the crash */
	if (ukarch_compare_exchange_sync(&crash_cpu, -1, cpu_id) != cpu_id) {
		/* TODO: Finish SMP Support
		 * Freeze CPU or wait until the crash_cpu initiates a freeze
		 * (e.g., through IPI). For now, just busy wait.
		 */
		ukplat_lcpu_halt();
	}
#endif /* UKPLAT_LCPU_MULTICORE */

#ifdef CONFIG_LIBUKDEBUG_CRASH_SCREEN
	if (!_uk_debug_explicit_crash) {
		crash_printk("Unikraft Panic - " STRINGIFY(UK_CODENAME)
			     " (" STRINGIFY(UK_FULLVERSION) ")\n");
		crash_print_thread_info();
		crash_printk("    _      \n");
		crash_printk("  cx xo    \n");
		crash_printk("  (|O|)/V  \n");
		crash_printk("           \n");
		crash_print_crashdump(ctx->regs);
	}
#endif /* CONFIG_LIBUKDEBUG_CRASH_SCREEN */

	/* Fill-in arch-specific event param from ctx */
	uk_crash_event_param(ctx, &param);

	/* We ignore the return value, because we cannot handle it anyway. */
	uk_raise_event(UK_CRASH_EVENT, &param);

	/* Halt or reboot the system */
	crash_shutdown();
}

UK_EVENT_HANDLER_PRIO(UK_EVENT_UNHANDLED_EXCEPTION, uk_debug_crash_handler,
		      UK_PRIO_LATEST);
