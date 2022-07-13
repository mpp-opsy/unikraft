/* SPDX-License-Identifier: BSD-2-Clause */
/*-
 * This code is based on:
 * https://github.com/freebsd/freebsd/blob/master/sys/arm64/arm64/locore.S
 *
 * Authors: Andrew Turner
 *          Wei Chen <Wei.Chen@arm.com>
 *
 * Copyright (c) 2012-2014 Andrew Turner. All rights reserved.
 * Copyright (c) 2018 Arm Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */
#include <uk/arch/limits.h>
#include <uk/arch/paging.h>
#include <uk/asm.h>
#include <kvm-arm/mm.h>
#include <arm/cpu_defs.h>

/* FIXME */
#define PTE_TYPE_BLOCK 0b01
#define PTE_TYPE_TABLE 0b11
#define PTE_TYPE_PAGE 0b11

/* Unikraft to VMSAv8-A */
#define PT_LVL_3	0
#define PT_LVL_2	1
#define PT_LVL_1	2
#define PT_LVL_0	3

/**
 * Outputs a single PTE
 *
 * @param upper_attr upper attribute bits
 * @param paddr physical address
 * @param lower_attr lower attribute bits
 */
.macro pte_tbl_desc paddr
       .quad \paddr + PTE_TYPE_TABLE
.endm

/* FIXME Can be reduced to 
 *
 * .quad (\attr_upper << 50) | (\paddr << (some_macro)) | \attr_lower | PTE_TYPE_???
 */
.macro pte_mapping paddr, attr
.quad \paddr + \attr
.endm

/* Outputs a number of block / table descriptors for a contiguous mapping starting at
 * the provided physical address.
 */
.macro pte_fill paddr, lvl, pages, attr
.ifle \pages
       .exitm
.endif
	pte_mapping (\paddr + PAGE_Lx_SIZE(\lvl) * 0), \attr
.ifgt (\pages - 8)
	/* Have to do some unrolling to not exceed max nested macros */
	pte_mapping (\paddr + PAGE_Lx_SIZE(\lvl) * 1), \attr
	pte_mapping (\paddr + PAGE_Lx_SIZE(\lvl) * 2), \attr
	pte_mapping (\paddr + PAGE_Lx_SIZE(\lvl) * 3), \attr
	pte_mapping (\paddr + PAGE_Lx_SIZE(\lvl) * 4), \attr
	pte_mapping (\paddr + PAGE_Lx_SIZE(\lvl) * 5), \attr
	pte_mapping (\paddr + PAGE_Lx_SIZE(\lvl) * 6), \attr
	pte_mapping (\paddr + PAGE_Lx_SIZE(\lvl) * 7), \attr
	pte_fill    (\paddr + PAGE_Lx_SIZE(\lvl) * 8), \lvl, (\pages - 8), \attr
.else
	pte_fill (\paddr + PAGE_Lx_SIZE(\lvl) * 1), \lvl, (\pages - 1), \attr
.endif
.endm

/* Outputs a number of invalid PTEs
 *
 * @param paddr physical address (not used)
 * @param pages number of PTEs
 */
.macro pte_zero paddr=0, pages
       .fill \pages, 0x8, 0
.endm

/* KVM memory map (QEMU virt):
 *
 * 0x0000000000000000 - 0x0000000007ffffff	Hole:          0    - 128MiB
 * 0x0000000008000000 - 0x000000003fffffff	Devices:     128MiB -   1GiB
 * 0x0000000040000000 - 0x0000007fffffffff	Kernel:        1GiB - 512GiB
 * 0x0000008000000000 - 0x000000ffffffffff	PCIe hi mem: 512GiB -   1TiB
 */

#if 0
.global aarch64_bpt_l0_pt0
.global aarch64_bpt_l1_pt0
.global aarch64_bpt_l1_pt1
.global aarch64_bpt_l2_pt0
.global aarch64_bpt_l2_pt1
.global aarch64_bpt_l3_pt0
#endif

.section .data

/* L0: 0 - 2TiB
 *
 */
.align 12
aarch64_bpt_l0_pt0:
       pte_tbl_desc    aarch64_bpt_l1_pt0
       pte_tbl_desc    aarch64_bpt_l1_pt1
       pte_zero        , 509
#ifdef CONFIG_PAGING
	pte_tbl_desc	aarch64_bpt_l1_pt511
#else
	pte_zero 	, 1
#endif /* CONFIG_PAGING */

/* L1: 0 - 512GiB (1GiB / entry)
 * 0x0000000000000000 - 0x000000003fffffff	Table descriptor to 1st L2 table
 * 0x0000000040000000 - 0x000000007fffffff	Table descriptor to 2nd L2 table
 * 0x0000000080000000 - 0x000000003fffffff	RAM       @   2GiB
 * 0x0000000400000000 - 0x0000007fffffffff	PCIe ECAM @ 256GiB
 */
.align 12
aarch64_bpt_l1_pt0:
       pte_tbl_desc    aarch64_bpt_l2_pt0
       pte_tbl_desc    aarch64_bpt_l2_pt1
       pte_fill        0x0000000080000000, 2, 254, SECT_ATTR_NORMAL
       pte_fill        0x0000004000000000, 2, 256, SECT_ATTR_DEVICE_nGnRnE

/* L1: 512GiB - 1TiB (1GiB / entry)
 * 0x0000008000000000 - 0x000000ffffffffff	PCIe hi-mem @ 512GiB
 */
.align 12
aarch64_bpt_l1_pt1:
       pte_fill        0x0000008000000000, 2, 512, SECT_ATTR_DEVICE_nGnRE

#ifdef CONFIG_PAGING
/* L1: 255.5 TiB - 2TiB (1GiB / entry)
 * 0x0000ff8000000000 - 0x0000ffffffffffff	Direct-mapped
 */
aarch64_bpt_l1_pt511:
	pte_fill	0x0000000000000000, 2, 512, SECT_ATTR_NORMAL
#endif /* CONFIG_PAGING */

/* L2: 0 - 1GiB (2MiB / entry)
 * 0x0000000000000000 - 0x0000000007ffffff	Hole:   @ 0
 * 0x0000000008000000 - 0x000000003fffffff	Devices @ 128MiB
 */
.align 12
aarch64_bpt_l2_pt0:
       pte_zero        0x0000000000000000, 64
       pte_fill        0x0000000008000000, 1, 448, SECT_ATTR_DEVICE_nGnRE

/* L2: 1GiB - 2GiB (2MiB per entry)
 * 0x0000000000000000 - 0x00000000001fffff	Table descriptor to 1st L3 table
 * 0x0000000000200000 - 0x0000000000000000	RAM	@ 1.2GiB - 2GiB
 */
.align 12
aarch64_bpt_l2_pt1:
       pte_tbl_desc  aarch64_bpt_l3_pt0
       pte_fill      0x0000000040200000, 1, 511, SECT_ATTR_NORMAL

/* L3: 1GiB - 1.2GiB (4KiB / entry)
 * 0x0000000040000000 - 0x0000007fffffffff	Kernel:        1GiB - 512GiB
 */
.align 12
aarch64_bpt_l3_pt0:
       pte_fill        0x0000000040000000, 0, 512, SECT_ATTR_NORMAL_RWX


/* FIXME:
 * - This does not fit images >2MiB (should be OK)
 *   On boot: 1st 2MiB of the image will be mapped in 4KiB, the rest in 2MiB pages.
 *   This is a problem for permissions (we need to also get the image to be managed by paging)
 *   If that worked, the image mappings > 1MiB would be mapped into newly created PTs.
 *
 * TODO:
 * - Set permissions upon entry
 * - Then enable WXN
 */

.text
/*
 * The following is the Memory Layout of AArch64 Virtual Machine
 * | 0-0x7FFFFFF | 0x8000000-0x3FFFFFFF | 0x40000000-0x7FFFFFFFFF | 512GB-1TB |
 * ----------------------------------------------------------------------------
 * | HOLE        | DEVICES MMIO         | <1>                     | <2>       |
 * ----------------------------------------------------------------------------
 * <1> DTB|TEXT|DATA|BSS|PAGETABLE|BOOTSTACK
 * <2> PCI-e High Mem
 */
#define HOLE_START 0
#define HOLE_MEM_ENTRIES  64 /* 128M */
#define DEVICE_ADDR_START 0x8000000
#define DEVICE_ENTRIES    448 /* 512-64 */
#define RAM_ADDR_START    0x40000000
#define RAM_L2_ENTRIES    255
#define RAM_ADDR_SIZE     (0x40000000 * RAM_L2_ENTRIES)
#define PCIE_ECAM_START   0x4000000000
#define PCIE_ECAM_L2_ENTRIES 256
#define PCIE_ADDR_START   0x8000000000
#define PCIE_L2_ENTRIES   512
#define PCIE_ADDR_SIZE    0x8000000000
/*
 * As we use VA == PA mapping, so the VIRT_BITS must be the same
 * as PA_BITS. We can get PA_BITS from ID_AA64MMFR0_EL1.PARange.
 */
virt_bits:
	.byte 32, 36, 40, 42, 44, 48

ENTRY(start_mmu)
	/*
	 * Using dsb here to guarantee the create_pagetables has
	 * been done.
	 */
	dsb sy

	/* Load ttbr0, pagetable starts from _end */
	ldr x27, =aarch64_bpt_l0_pt0
	msr ttbr0_el1, x27
	isb

	/* Clear the Monitor Debug System control register */
	msr mdscr_el1, xzr

	/* Invalidate the TLB to avoid stale one */
	tlbi vmalle1
	dsb nsh

	ldr x2, =MAIR_INIT_ATTR
	msr mair_el1, x2

	/* Get VIRT_BITS from id_aa64mmfr0_el1.PARange */
	mrs x3, id_aa64mmfr0_el1
	ldr x5, =virt_bits
	ubfx x4, x3, #0, #4
	ldrb w4, [x5, x4]

	/* Setup TCR_TxSZ(64 - VIRT_BITS) for TCR_INIT_FLAGS */
	mov x5, #64
	sub x5, x5, x4
	mov x4, x5
	lsl x5, x5, #TCR_T1SZ_SHIFT
	orr x5, x4, x5
	ldr x2, =TCR_INIT_FLAGS
	orr x2, x5, x2
	bfi x2, x3, #32, #3
	msr tcr_el1, x2

	/* save lr */
	mov x22, x30

        /*
	 * Invalidate the D-Cache to avoid using invalid data that existed
	 * in D-Cache. Invalidate ranges that may have been modified:
	 * DATA, BSS, and BOOTSTACK.
	 */
	ldr x0, =_data
	ldr x1, =_end
	add x1, x1, #__STACK_SIZE
	sub x1, x1, x0
	bl clean_and_invalidate_dcache_range

	/* Setup SCTLR */
	ldr x2, =SCTLR_SET_BITS
	ldr x3, =SCTLR_CLEAR_BITS
	mrs x1, sctlr_el1
	bic x1, x1, x3	/* Clear the required bits */
	orr x1, x1, x2	/* Set the required bits */
	msr sctlr_el1, x1
	isb

	/* restore lr */
	mov x30, x22

	ret
END(start_mmu)

