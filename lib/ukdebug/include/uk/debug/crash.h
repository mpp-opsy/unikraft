/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2021, Karlsruhe Institute of Technology. All rights reserved.
 * Copyright (c) 2023, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the BSD-3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 */

#ifndef __UK_CRASH_H__
#define __UK_CRASH_H__

#include <uk/plat/bootstrap.h>
#include <uk/arch/lcpu.h>
#include <uk/essentials.h>

#ifdef __cplusplus
extern "C" {
#endif

enum uk_crash_reason {
	UK_CRASH_REASON_INVALID,
	/**
	 * The crash was caused by an explicit UK_CRASH
	 */
	UK_CRASH_REASON_EXPLICIT,
	/**
	 * The crash was caused by an unhandled trap.
	 */
	UK_CRASH_REASON_UNHANDLED_TRAP,
	/**
	 * The crash was caused by an unhandled page fault.
	 */
	UK_CRASH_REASON_PAGE_FAULT,
	/**
	 * The crash was caused by something else.
	 */
	UK_CRASH_REASON_OTHER,
};

struct uk_crash_descr {
	/**
	 * Contains the crash reason.
	 * The meaning of the following fields change depending on the reason.
	 */
	enum uk_crash_reason reason;

	/**
	 * Contains the errno code the systems exits with.
	 */
	int errno;

	/**
	 * For UK_CRASH_REASON_UNHANDLED_TRAP:
	 *   the architecture specific trap number
	 * For UK_CRASH_REASON_PAGE_FAULT:
	 *   the virtual address which was attempted to access
	 */
	__u64 arg1;

	/**
	 * For UK_CRASH_REASON_UNHANDLED_TRAP:
	 *   pointer to human-readable string of the trap number
	 * For UK_CRASH_REASON_PAGE_FAULT:
	 *   an architecture specific error code
	 */
	__u64 arg2;

	/**
	 * For UK_CRASH_REASON_UNHANDLED_TRAP:
	 *   error code of the trap
	 */
	__u64 arg3;
};

struct uk_event_crash_parameter {
	/** Optional register state at the crash location */
	struct __regs *regs;
	/** Optional explicit crash description */
	struct uk_crash_descr descr;
};

/**
 * Event for crashes. Will receive a uk_crash_event_param as data argument.
 */
#define UK_CRASH_EVENT uk_crash_event

#ifdef __cplusplus
}
#endif

#endif /* __UK_CRASH_H__ */
