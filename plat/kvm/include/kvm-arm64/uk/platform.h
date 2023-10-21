/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2023, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the BSD-3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 */
#ifndef __KVM_ARM64_PLATFORM_H__
#define __KVM_ARM64_PLATFORM_H__

#if CONFIG_KVM_VMM_QEMU

/* Kernel image base */
#define KVM_RAM_BASE		0x40000000

/* Max memory range supported by the platform */
#define UKPLAT_RAM_BASE		0x00000000
#define UKPLAT_RAM_LEN		(255UL * 0x40000000)
#define UKPLAT_RAM_END		(UKPLAT_RAM_BASE + UKPLAT_RAM_LEN)

#elif CONFIG_KVM_VMM_FIRECRACKER

/* Kernel image base */
#define KVM_RAM_BASE		0x80000000

/* Max memory range supported by the platform */
#define UKPLAT_RAM_BASE		0x00000000
#define UKPLAT_RAM_LEN		(1024UL * 0x40000000)
#define UKPLAT_RAM_END		(RAM_BASE + RAM_LEN)

#endif /* CONFIG_KVM_VMM_FIRECRACKER  */

#define DTB_RESERVED_SIZE 0x100000

#endif /*  __KVM_ARM64_PLATFORM_H__ */
