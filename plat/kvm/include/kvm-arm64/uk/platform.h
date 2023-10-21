/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2023, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the BSD-3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 */
#ifndef __KVM_ARM64_PLATFORM_H__
#define __KVM_ARM64_PLATFORM_H__

#if CONFIG_KVM_VMM_QEMU

#define UKPLAT_RAM_BASE		0x00000000
#define UKPLAT_RAM_LEN		(255UL * 0x40000000)
#define UKPLAT_RAM_END		(UKPLAT_RAM_BASE + UKPLAT_RAM_LEN)

#define UKPLAT_KVM_IMAGE_BASE	0x40000000

#elif CONFIG_KVM_VMM_FIRECRACKER

#define UKPLAT_RAM_BASE		0x00000000
#define UKPLAT_RAM_LEN		(1024UL * 0x40000000)
#define UKPLAT_RAM_END		(UKPLAT_RAM_BASE + UKPLAT_RAM_LEN)

#define UKPLAT_KVM_IMAGE_BASE	0x80000000

#endif /* CONFIG_KVM_VMM_FIRECRACKER  */

#define UKPLAT_KVM_DTB_SIZE	0x00100000

#endif /*  __KVM_ARM64_PLATFORM_H__ */
