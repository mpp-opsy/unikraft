/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2023, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the BSD-3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 */

#ifndef __KVM_X86_64_PLATFORM_H__
#define __KVM_X86_64_PLATFORM_H__

#define UKPLAT_RAM_BASE		0x00000000
#define UKPLAT_RAM_LEN		0x100000000 /* 4GiB */
#define UKPLAT_RAM_END		(UKPLAT_RAM_BASE + UKPLAT_RAM_LEN)

#endif /*  __KVM_X86_64_PLATFORM_H__ */
