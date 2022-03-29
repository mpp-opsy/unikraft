/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Marc Rittinghaus <marc.rittinghaus@kit.edu>
 *
 * Copyright (c) 2021, Karlsruhe Institute of Technology (KIT).
 *                     All rights reserved.
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

#ifndef __UKARCH_PAGING_H__
#error Do not include this header directly
#endif

#ifndef __ASSEMBLY__
#include <uk/config.h>
#include <uk/arch/lcpu.h>
#include <uk/arch/types.h>

typedef __u64 __pte_t;

/* architecture-dependent extension for page table */
struct ukarch_pagetable {
	/* nothing */
};
#endif /* !__ASSEMBLY__ */

/* The Unikraft API uses the x86_64 pagetable indexing convention,
 * where higher index numbers denote tables of larger mappings,
 * ie pagetable walks are in the order of:
 * L3 -> L2 -> L1 -> L0
 *
 * This is conflicting with VMSA-8 that uses low-to-high indexing
 * instead, ie pagetable walks are in the order of:
 * L0 -> L1 -> L2 -> L3
 *
 * Here we will be using the convention of Unikraft's paging API.
 */

#define PAGE_LEVEL		0

/* The maximum level that we can create mappings on */
#define PT_MAP_LEVEL_MAX	PT_LEVELS - 2

__u64 TCR_IPS_MAP[] = {
	TCR_IPS_32BIT, TCR_IPS_36BIT,
	TCR_IPS_40BIT, TCR_IPS_42BIT,
	TCR_IPS_44BIT, TCR_IPS_48BIT,
	TCR_IPS_52BIT
};

/* Returns the mask of the address bits of a
 * block or page descriptor for a given level.
 */
__u64 PTE_Lx_MAP_PADDR_MASK[] = {
	PTE_L0_PAGE_PADDR_MASK,
	PTE_L1_BLOCK_PADDR_MASK,
	PTE_L2_BLOCK_PADDR_MASK
};

/* Here we use the same address with x86, for consistency.
 * This address is not a valid PA, as it is larger than the
 * maximum supported 48-bit (and 52-bit) address.
 * TODO Is this correct? Consider the PAuth / Tag bits.
 */
#define ARM64_INVALID_ADDR		0xBAADBAADBAADBAADUL

#define __VADDR_INV			ARM64_INVALID_ADDR
#define __PADDR_INV			ARM64_INVALID_ADDR

#define ARM64_PT_Lx_SHIFT(lvl)				\
	(PT_OFFS_BITS + (PT_LEVEL_SHIFT * lvl))

#define PT_Lx_IDX(vaddr, lvl)				\
	(((vaddr) >> ARM64_PT_Lx_SHIFT(lvl)) & (PT_PTES_PER_LEVEL - 1))

#define PT_Lx_PTES(lvl)			PT_PTES_PER_LEVEL

#define PT_Lx_PTE_PRESENT(pte, lvl) \
	((pte & PTE_Lx_VALID_BIT))

#define PT_Lx_PTE_CLEAR_PRESENT(pte, lvl) \
	(pte & ~PTE_Lx_VALID_BIT)

#define PT_Lx_PTE_INVALID(lvl)		0x0ULL

#define PAGE_Lx_SHIFT(lvl) ARM64_PT_Lx_SHIFT(lvl)

/* Has this level the ability to describe a mapping? */
#define PAGE_Lx_HAS(lvl) ((lvl) <= PT_MAP_LEVEL_MAX)

/* Is this PTE describing a mapping? */
#define PAGE_Lx_IS(pte, lvl)				\
	((lvl == PAGE_LEVEL) || ((pte) & PTE_Lx_TYPE_BIT) == PTE_Lx_TYPE_BLOCK)

#ifndef __ASSEMBLY__

#define ARM64_VADDR_MASK ((1ULL << arm64_vaddr_bits()) - 1)

#define ARM64_PADDR_MAX ((1ULL << arm64_paddr_bits()) - 1)
#define ARM64_VADDR_MAX ((1ULL << arm64_vaddr_bits()) - 1)

#define ARM64_PADDR_VALID(paddr) (paddr <= (__paddr_t)ARM64_VADDR_MAX)
#define ARM64_VADDR_VALID(vaddr) (vaddr <= (__vaddr_t)ARM64_VADDR_MAX)

static inline unsigned int arm64_paddr_bits(void)
{
	__u64 reg;

	__asm__ __volatile__("mrs %0, tcr_el1" : "=r" (reg));

	return 64 - ((reg & TCR_T0SZ_MASK) >> TCR_T0SZ_SHIFT);
}

static inline unsigned int arm64_vaddr_bits(void)
{
	__u64 reg = 0;


	__asm__ __volatile__("mrs %0, tcr_el1" : "=r" (reg));

	return TCR_IPS_MAP[(reg >> TCR_IPS_SHIFT) & TCR_IPS_MASK];
}

static inline __paddr_t PT_Lx_PTE_PADDR(__pte_t pte, unsigned int lvl)
{
	if (PAGE_Lx_IS(pte, lvl)) {
		if (unlikely(lvl > PT_MAP_LEVEL_MAX))
			return __PADDR_INV;

		return pte & PTE_Lx_MAP_PADDR_MASK[lvl];
	} else {
		if (unlikely(lvl <= PAGE_LEVEL || lvl >= PT_LEVELS))
			return __PADDR_INV;

		return pte & PTE_Lx_TABLE_PADDR_MASK;
	}
}

static int ukarch_vaddr_range_isvalid(__vaddr_t start, __vaddr_t end)
{
	UK_ASSERT(start <= end);

	return (ARM64_VADDR_VALID(end)) && (ARM64_VADDR_VALID(start));
}

static int ukarch_paddr_range_isvalid(__paddr_t start, __paddr_t end)
{
	UK_ASSERT(start <= end);

	return (ARM64_PADDR_VALID(end)) && (ARM64_PADDR_VALID(start));
}

static int ukarch_pte_read(__vaddr_t pt_vaddr, unsigned int lvl, unsigned int idx,
			   __pte_t *pte)
{
	(void)lvl;

	UK_ASSERT(idx < PT_Lx_PTES(lvl));

	*pte = *((__pte_t *)pt_vaddr + idx);

	return 0;
}

static int ukarch_pte_write(__vaddr_t pt_vaddr, unsigned int lvl, unsigned int idx,
			    __pte_t pte)
{
	(void)lvl;

	UK_ASSERT(idx < PT_Lx_PTES(lvl));

	*((__pte_t *)pt_vaddr + idx) = pte;

	return 0;
}

static __paddr_t ukarch_pt_read_base(void)
{
	__paddr_t reg;

	__asm__ __volatile__("mrs %x0, ttbr0_el1\n" : "=r" (reg));

	return reg;
}

static int ukarch_pt_write_base(__paddr_t pt_paddr)
{
	__asm__ __volatile__("msr ttbr0_el1, %x0\n" :: "r" (pt_paddr));

	/* TODO Does this implicitly flush the TLB? Remember adding barriers if needed */

	return 0;
}

static void ukarch_tlb_flush_entry(__vaddr_t vaddr)
{
	__asm__ __volatile__(
		"	dsb ishst\n"       /* wait for writes to complete */
		"	tlbi vaae1, %x0\n" /* invalidate by vaddr, any ASID */
		"	dsb ish\n"         /* wait for invalidate complete */
		"	isb\n"             /* sync context */
		:: "r" (vaddr) : "memory");
}

static void ukarch_tlb_flush(void)
{
	__asm__ __volatile__(
		"	dsb ishst\n"      /* wait for writes to complete */
		"	tlbi vmalle1is\n" /* invalidate all inner-shareable */
		"	dsb ish\n"        /* wait for invalidate complete */
		"	isb\n"            /* sync context */
		::: "memory");
}
#endif /* !__ASSEMBLY__ */
