/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2018, Arm Ltd. All rights reserved.
 * Copyright (c) 2022, OpenSynergy GmbH. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __UKARCH_LCPU_H__
#error Do not include this header directly
#endif

#include <uk/arch/types.h>
#include <uk/asm.h>

/**************************************************************************
 * AArch64 System Register Definitions
 *************************************************************************/

/* CTR_EL0, Cache Type Register */
#define CTR_DMINLINE_SHIFT	16
#define CTR_DMINLINE_WIDTH	4
#define CTR_IMINLINE_MASK	0xf
#define CTR_BYTES_PER_WORD	4

/* ID_AA64MMFR0_EL1 - Memory Model Feature Register 1 */
#define ID_AA64MMFR0_EL1_PARANGE_SHIFT	U(0)
#define ID_AA64MMFR0_EL1_PARANGE_MASK	ULL(0xf)

#define PARANGE_0000	U(32)
#define PARANGE_0001	U(36)
#define PARANGE_0010	U(40)
#define PARANGE_0011	U(42)
#define PARANGE_0100	U(44)
#define PARANGE_0101	U(48)
#define PARANGE_0110	U(52)

#define ID_AA64MMFR0_EL1_ECV_SHIFT		U(60)
#define ID_AA64MMFR0_EL1_ECV_MASK		ULL(0xf)
#define ID_AA64MMFR0_EL1_ECV_NOT_SUPPORTED	ULL(0x0)
#define ID_AA64MMFR0_EL1_ECV_SUPPORTED		ULL(0x1)
#define ID_AA64MMFR0_EL1_ECV_SELF_SYNCH	ULL(0x2)

#define ID_AA64MMFR0_EL1_FGT_SHIFT		U(56)
#define ID_AA64MMFR0_EL1_FGT_MASK		ULL(0xf)
#define ID_AA64MMFR0_EL1_FGT_SUPPORTED		ULL(0x1)
#define ID_AA64MMFR0_EL1_FGT_NOT_SUPPORTED	ULL(0x0)

#define ID_AA64MMFR0_EL1_TGRAN4_SHIFT		U(28)
#define ID_AA64MMFR0_EL1_TGRAN4_MASK		ULL(0xf)
#define ID_AA64MMFR0_EL1_TGRAN4_SUPPORTED	ULL(0x0)
#define ID_AA64MMFR0_EL1_TGRAN4_NOT_SUPPORTED	ULL(0xf)

#define ID_AA64MMFR0_EL1_TGRAN64_SHIFT		U(24)
#define ID_AA64MMFR0_EL1_TGRAN64_MASK		ULL(0xf)
#define ID_AA64MMFR0_EL1_TGRAN64_SUPPORTED	ULL(0x0)
#define ID_AA64MMFR0_EL1_TGRAN64_NOT_SUPPORTED	ULL(0xf)

#define ID_AA64MMFR0_EL1_TGRAN16_SHIFT		U(20)
#define ID_AA64MMFR0_EL1_TGRAN16_MASK		ULL(0xf)
#define ID_AA64MMFR0_EL1_TGRAN16_SUPPORTED	ULL(0x1)
#define ID_AA64MMFR0_EL1_TGRAN16_NOT_SUPPORTED	ULL(0x0)

/* ID_AA64MMFR2_EL1 - Memory Model Feature Register 2 */
#define ID_AA64MMFR2_EL1		S3_0_C0_C7_2

#define ID_AA64MMFR2_EL1_ST_SHIFT	U(28)
#define ID_AA64MMFR2_EL1_ST_MASK	ULL(0xf)

#define ID_AA64MMFR2_EL1_CCIDX_SHIFT	U(20)
#define ID_AA64MMFR2_EL1_CCIDX_MASK	ULL(0xf)
#define ID_AA64MMFR2_EL1_CCIDX_LENGTH	U(4)

#define ID_AA64MMFR2_EL1_CNP_SHIFT	U(0)
#define ID_AA64MMFR2_EL1_CNP_MASK	ULL(0xf)

/* MAIR_EL1 - Memory Attribute Indirection Register */
#define MAIR_ATTR_MASK(idx)	(0xff << ((n)* 8))
#define MAIR_ATTR(attr, idx)	((attr) << ((idx) * 8))

/* Device-nGnRnE memory */
#define MAIR_DEVICE_nGnRnE	0x00
/* Device-nGnRE memory */
#define MAIR_DEVICE_nGnRE	0x04
/* Device-GRE memory */
#define MAIR_DEVICE_GRE		0x0C
/* Outer Non-cacheable + Inner Non-cacheable */
#define MAIR_NORMAL_NC		0x44
/* Outer + Inner Write-through non-transient */
#define MAIR_NORMAL_WT		0xbb
/* Outer + Inner Write-back non-transient */
#define MAIR_NORMAL_WB		0xff

/* SCTLR_EL1 - System Control Register */
#define SCTLR_M		(UL(1) << 0)	/* MMU enable */
#define SCTLR_A		(UL(1) << 1)	/* Alignment check enable */
#define SCTLR_C		(UL(1) << 2)	/* Data/unified cache enable */
#define SCTLR_SA	(UL(1) << 3)	/* Stack alignment check enable */
#define SCTLR_SA0	(UL(1) << 4)	/* Stack Alignment Check Enable for EL0 */
#define SCTLR_CP15BEN	(UL(1) << 5)	/* System instruction memory barrier enable */
#define SCTLR_ITD	(UL(1) << 7)	/* IT disable */
#define SCTLR_SED	(UL(1) << 8)	/* SETEND instruction disable */
#define SCTLR_UMA	(UL(1) << 9)	/* User mask access */
#define SCTLR_I		(UL(1) << 12)	/* Instruction access Cacheability control */
#define SCTLR_DZE	(UL(1) << 14)	/* Traps EL0 DC ZVA instructions to EL1 */
#define SCTLR_UCT	(UL(1) << 15)	/* Traps EL0 accesses to the CTR_EL0 to EL1 */
#define SCTLR_nTWI	(UL(1) << 16)	/* Don't trap EL0 WFI to EL1 */
#define SCTLR_nTWE	(UL(1) << 18)	/* Don't trap EL0 WFE to EL1 */
#define SCTLR_WXN	(UL(1) << 19)	/* Write permission implies XN */
#define SCTLR_EOE	(UL(1) << 24)	/* Endianness of data accesses at EL0 */
#define SCTLR_EE	(UL(1) << 25)	/* Endianness of data accesses at EL1 */
#define SCTLR_UCI	(UL(1) << 26)	/* Traps EL0 cache instructions to EL1 */

/* Reserve to 1 */
#define SCTLR_RES1_B11	(UL(1) << 11)
#define SCTLR_RES1_B20	(UL(1) << 20)
#define SCTLR_RES1_B22	(UL(1) << 22)
#define SCTLR_RES1_B23	(UL(1) << 23)
#define SCTLR_RES1_B28	(UL(1) << 28)
#define SCTLR_RES1_B29	(UL(1) << 29)

/* Reserve to 0 */
#define SCTLR_RES0_B6	(UL(1) << 6)
#define SCTLR_RES0_B10	(UL(1) << 10)
#define SCTLR_RES0_B13	(UL(1) << 13)
#define SCTLR_RES0_B17	(UL(1) << 17)
#define SCTLR_RES0_B21	(UL(1) << 21)
#define SCTLR_RES0_B27	(UL(1) << 27)
#define SCTLR_RES0_B30	(UL(1) << 30)
#define SCTLR_RES0_B31	(UL(1) << 31)

/* TCR_EL1 - Translation Control Register */
#define TCR_ASID_16	(1 << 36)

#define TCR_IPS_SHIFT	32
#define TCR_IPS_MASK	0x7
#define TCR_IPS_32BIT	(UL(0) << TCR_IPS_SHIFT)
#define TCR_IPS_36BIT	(UL(1) << TCR_IPS_SHIFT)
#define TCR_IPS_40BIT	(UL(2) << TCR_IPS_SHIFT)
#define TCR_IPS_42BIT	(UL(3) << TCR_IPS_SHIFT)
#define TCR_IPS_44BIT	(UL(4) << TCR_IPS_SHIFT)
#define TCR_IPS_48BIT	(UL(5) << TCR_IPS_SHIFT)
#define TCR_IPS_52BIT	(UL(6) << TCR_IPS_SHIFT)
 

#define TCR_TG1_SHIFT	30
#define TCR_TG1_16K	(1 << TCR_TG1_SHIFT)
#define TCR_TG1_4K	(2 << TCR_TG1_SHIFT)
#define TCR_TG1_64K	(3 << TCR_TG1_SHIFT)

#define TCR_TG0_SHIFT	14
#define TCR_TG0_4K	(0 << TCR_TG0_SHIFT)
#define TCR_TG0_64K	(1 << TCR_TG0_SHIFT)
#define TCR_TG0_16K	(2 << TCR_TG0_SHIFT)

#define TCR_SH1_SHIFT	28
#define TCR_SH1_IS	(0x3 << TCR_SH1_SHIFT)
#define TCR_ORGN1_SHIFT	26
#define TCR_ORGN1_WBWA	(0x1 << TCR_ORGN1_SHIFT)
#define TCR_IRGN1_SHIFT	24
#define TCR_IRGN1_WBWA	(0x1 << TCR_IRGN1_SHIFT)
#define TCR_SH0_SHIFT	12
#define TCR_SH0_IS	(0x3 << TCR_SH0_SHIFT)
#define TCR_ORGN0_SHIFT	10
#define TCR_ORGN0_WBWA	(0x1 << TCR_ORGN0_SHIFT)
#define TCR_IRGN0_SHIFT	8
#define TCR_IRGN0_WBWA	(0x1 << TCR_IRGN0_SHIFT)

#define TCR_CACHE_ATTRS ((TCR_IRGN0_WBWA | TCR_IRGN1_WBWA) | \
			(TCR_ORGN0_WBWA | TCR_ORGN1_WBWA))

#define TCR_SMP_ATTRS	(TCR_SH0_IS | TCR_SH1_IS)

#define TCR_T1SZ_SHIFT	16
#define TCR_T0SZ_SHIFT	0
#define TCR_T0SZ_MASK	0x1f
#define TCR_T1SZ(x)	((x) << TCR_T1SZ_SHIFT)
#define TCR_T0SZ(x)	((x) << TCR_T0SZ_SHIFT)
#define TCR_TxSZ(x)	(TCR_T1SZ(x) | TCR_T0SZ(x))

/**************************************************************************
 * VMSAv8-64 Definitions
 *************************************************************************/

/* The following definitions are for Stage 1 translations using
 * 4KiB granule and a 48-bit VA space.
 *
 * Notice: Here we use the VMSAv8-A table indexing convention,
 *         and not the one of Unikraft's paging API.
 */

#define PT_LEVELS		4
#define PT_PTES_PER_LEVEL	512

#define PAGE_SIZE		UL(0x1000)
#define PAGE_MASK		(~(PAGE_SIZE - 1))
#define PAGE_SHIFT		12

/* Level 0 table, 512GiB per entry */
#define L0_SHIFT	39
#define L0_SIZE		(1ul << L0_SHIFT)
#define L0_OFFSET	(L0_SIZE - 1ul)
#define L0_TABLE	0x3 /* A next-level table */

/* Level 1 table, 1GiB per entry */
#define L1_SHIFT	30
#define L1_SIZE 	(1 << L1_SHIFT)
#define L1_OFFSET 	(L1_SIZE - 1)
#define L1_TABLE	L0_TABLE

/* Level 2 table, 2MiB per entry */
#define L2_SHIFT	21
#define L2_SIZE 	(1 << L2_SHIFT)
#define L2_OFFSET 	(L2_SIZE - 1)
#define L2_TABLE	L1_TABLE

#define L2_BLOCK_MASK	UL(0xffffffe00000)

/* Level 3 table, 4KiB per entry */
#define L3_SHIFT	12
#define L3_SIZE 	(1 << L3_SHIFT)
#define L3_OFFSET 	(L3_SIZE - 1)
#define L3_SHIFT	12
#define L3_PAGE		0x3

#define L0_ENTRIES_SHIFT 9
#define L0_ENTRIES	(1 << L0_ENTRIES_SHIFT)
#define L0_ADDR_MASK	(L0_ENTRIES - 1)

#define Ln_ENTRIES_SHIFT 9
#define Ln_ENTRIES	(1 << Ln_ENTRIES_SHIFT)
#define Ln_ADDR_MASK	(Ln_ENTRIES - 1)
#define Ln_TABLE_MASK	((1 << 12) - 1)

#define Ln_TABLE	0x3
#define Ln_BLOCK	0x1
#define Ln_INVAL	0x0

/*
 * PTE Definitions
 */
#define PT_LEVEL_SHIFT		9
#define PT_OFFS_BITS		12

#define PTE_Lx_VALID_BIT	UL(1)

#define PTE_Lx_TYPE_BIT		(UL(1) << 1)
#define PTE_Lx_TYPE_SHIFT	1

#define PTE_Lx_TYPE_BLOCK	0
#define PTE_Lx_TYPE_TABLE	1

#define PTE_Lx_TABLE_PADDR_MASK	0x0000fffffffff000 /* [47-12] */

#define PTE_L2_BLOCK_PADDR_MASK	0x0000ffffffe00000 /* [47-30] */
#define PTE_L1_BLOCK_PADDR_MASK	0x0000ffffc0000000 /* [47-21] */
#define PTE_L0_PAGE_PADDR_MASK	0x0000fffffffff000 /* [47-12] */

/* Definitions for Block and Page descriptors */
#define ATTR_MASK_H	UL(0xfff0000000000000)
#define ATTR_MASK_L	UL(0x0000000000000fff)
#define ATTR_MASK	(ATTR_MASK_H | ATTR_MASK_L)
/* Bits 58:55 are reserved for software */
#define ATTR_SW_MANAGED	(UL(1) << 56)
#define ATTR_SW_WIRED	(UL(1) << 55)
#define ATTR_UXN	(UL(1) << 54)
#define ATTR_PXN	(UL(1) << 53)
#define ATTR_XN		(ATTR_PXN | ATTR_UXN)
#define ATTR_CONTIGUOUS	(UL(1) << 52)
#define ATTR_DBM	(UL(1) << 51)
#define ATTR_nG		(1 << 11)
#define ATTR_AF		(1 << 10)
#define ATTR_SH(x)	((x) << 8)
#define ATTR_SH_MASK	ATTR_SH(3)
#define ATTR_SH_NS	0		/* Non-shareable */
#define ATTR_SH_OS	2		/* Outer-shareable */
#define ATTR_SH_IS	3		/* Inner-shareable */
#define ATTR_AP_RW_BIT	(1 << 7)
#define ATTR_AP(x)	((x) << 6)
#define ATTR_AP_MASK	ATTR_AP(3)
#define ATTR_AP_RW	(0 << 1)
#define ATTR_AP_RO	(1 << 1)
#define ATTR_AP_USER	(1 << 0)
#define ATTR_NS		(1 << 5)
#define ATTR_IDX(x)	((x) << 2)
#define ATTR_IDX_MASK	(7 << 2)

#define ATTR_DESCR_MASK	3

