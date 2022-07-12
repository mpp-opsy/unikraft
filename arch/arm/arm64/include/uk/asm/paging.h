/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Marc Rittinghaus <marc.rittinghaus@kit.edu>
 *          Michalis Pappas <michalis.pappas@opensynergy.com>
 *
 * Copyright (c) 2021, Karlsruhe Institute of Technology (KIT).
 *                     All rights reserved.
 *
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

#ifndef __UKARCH_PAGING_H__
#error Do not include this header directly
#endif

#ifndef __ASSEMBLY__
#include <uk/config.h>
#include <uk/arch/paging.h>
#include <uk/arch/types.h>

#ifdef CONFIG_LIBUKDEBUG
#include <uk/assert.h>
#endif /* CONFIG_LIBUKDEBUG */

typedef __u64 __pte_t;		/* page table entry */

/* architecture-dependent extension for page table */
struct ukarch_pagetable {
	/* nothing */
};
#endif /* !__ASSEMBLY__ */

/* ---------------- FIXME ------------------------ */
/*These are also / should be defined in plat */
#define PTE_Lx_VALID_BIT	1UL
#define PTE_Lx_TYPE_BIT		(1UL << 1)
#define PTE_Lx_TYPE_BLOCK	0

/* D5-4815 - D5-4816 */
#define PTE_Lx_TABLE_PADDR_MASK	0x0000fffffffff000 /* [47-12] */

#define PTE_L2_BLOCK_PADDR_MASK	0x0000ffffc0000000 /* [47-30] */
#define PTE_L1_BLOCK_PADDR_MASK	0x0000ffffffe00000 /* [47-21] */
#define PTE_L0_PAGE_PADDR_MASK	0x0000fffffffff000 /* [47-12] */
/* ---------------- FIXME ----------------------- */

#define PT_LEVELS			4
#define PT_PTES_PER_LEVEL		512
#define PT_LEVEL_SHIFT			9

/* 48-bit IA using a 4KiB translation granule (Figure D5-3)
 *
 *  63           47         38         29         20         11
 *  -----------------------------------------------------------------
 * | Upper bits | L0 index | L1 index | L2 index | L3 index | Offset |
 *  -----------------------------------------------------------------
 *
 * Levels here use the VMSAv8-A indexing.
 *
 * Notice that the Unikraft paging API uses the x86_64 convention
 * to index PT levels, that is pages are defined at L0, and the first
 * page table is at L3. This is the opposite of the convention use by
 * VMSAv8-A, where pages are defined by L3 and the top level table is
 * defined at L0 / L-1.
 */
#define PAGE_LEVEL			0
#define PAGE_SHIFT			12
#define PAGE_SIZE			0x1000UL
#define PAGE_MASK			(~(PAGE_SIZE - 1))

#define PAGE_ATTR_PROT_NONE		0x00 /* Page is not accessible */
#define PAGE_ATTR_PROT_READ		0x01 /* Page is readable */
#define PAGE_ATTR_PROT_WRITE		0x02 /* Page is writeable */
#define PAGE_ATTR_PROT_EXEC		0x04 /* Page is executable */

#define ARM64_PADDR_BITS		48
#define ARM64_VADDR_BITS		48

#define PT_Lx_PTES(lvl)			PT_PTES_PER_LEVEL

#define PT_Lx_PTE_PRESENT(pte, lvl) \
	((pte & PTE_Lx_VALID_BIT))

#define PT_Lx_PTE_CLEAR_PRESENT(pte, lvl) \
	(pte & ~PTE_Lx_VALID_BIT)

#define PT_MAP_LEVEL_MAX	PT_LEVELS - 2

#define PAGE_Lx_HAS(lvl) ((lvl) <= PT_MAP_LEVEL_MAX)

#define PAGE_Lx_IS(pte, lvl)				\
	((lvl == PAGE_LEVEL) || ((pte) & PTE_Lx_TYPE_BIT) == PTE_Lx_TYPE_BLOCK)

#define PAGE_Lx_SHIFT(lvl)				\
	(PAGE_SHIFT + (PT_LEVEL_SHIFT * lvl))

#define PT_Lx_IDX(vaddr, lvl)				\
	(((vaddr) >> PAGE_Lx_SHIFT(lvl)) & (PT_PTES_PER_LEVEL - 1))

/* Any address controlled by TTBR1 */
#define ARM64_INVALID_ADDR		0xBAADBAADBAADBAADUL

#define __VADDR_INV					\
	PAGE_Lx_ALIGN_DOWN(ARM64_INVALID_ADDR, PT_MAP_LEVEL_MAX)

#define __PADDR_INV					\
	PAGE_Lx_ALIGN_DOWN(ARM64_INVALID_ADDR, PT_MAP_LEVEL_MAX)

#define PT_Lx_PTE_INVALID(lvl)		0x0UL

#define ARM64_PADDR_MAX ((1ULL << ARM64_PADDR_BITS) - 1)
#define ARM64_VADDR_MAX ((1ULL << ARM64_VADDR_BITS) - 1)

#define ARM64_PADDR_VALID(paddr) (paddr <= (__paddr_t)ARM64_VADDR_MAX)
#define ARM64_VADDR_VALID(vaddr) (vaddr <= (__vaddr_t)ARM64_VADDR_MAX)

#ifndef __ASSEMBLY__
static inline __paddr_t PT_Lx_PTE_PADDR(__pte_t pte, unsigned int lvl)
{
	/* FIXME we also define this in plat */
	__u64 pte_lx_map_paddr_mask[] = {
		PTE_L0_PAGE_PADDR_MASK,
		PTE_L1_BLOCK_PADDR_MASK,
		PTE_L2_BLOCK_PADDR_MASK
	};

	if (PAGE_Lx_IS(pte, lvl)) {
#ifdef CONFIG_LIBUKDEBUG
		UK_ASSERT(lvl <= PT_MAP_LEVEL_MAX);
#endif /* CONFIG_LIBUKDEBUG */
		return pte & pte_lx_map_paddr_mask[lvl];
	} else {
#ifdef CONFIG_LIBUKDEBUG
		UK_ASSERT(lvl > PAGE_LEVEL && lvl < PT_LEVELS);
#endif /* CONFIG_LIBUKDEBUG */
		return pte & PTE_Lx_TABLE_PADDR_MASK;
	}

	return 0;
}

static inline int ukarch_paddr_range_isvalid(__paddr_t start, __paddr_t end)
{
#ifdef CONFIG_LIBUKDEBUG
	UK_ASSERT(start <= end);
#endif /* CONFIG_LIBUKDEBUG */

	return (ARM64_PADDR_VALID(end)) && (ARM64_PADDR_VALID(start));
}

static inline int ukarch_vaddr_range_isvalid(__vaddr_t start, __vaddr_t end)
{
#ifdef CONFIG_LIBUKDEBUG
	UK_ASSERT(start <= end);
#endif /* CONFIG_LIBUKDEBUG */

	return (ARM64_VADDR_VALID(end)) && (ARM64_VADDR_VALID(start));
}

static inline int ukarch_pte_read(__vaddr_t pt_vaddr, unsigned int lvl,
				  unsigned int idx, __pte_t *pte)
{
	(void)lvl;

#ifdef CONFIG_LIBUKDEBUG
	UK_ASSERT(idx < PT_Lx_PTES(lvl));
#endif /* CONFIG_LIBUKDEBUG */

	*pte = *((__pte_t *)pt_vaddr + idx);

	return 0;
}

static inline int ukarch_pte_write(__vaddr_t pt_vaddr, unsigned int lvl,
				   unsigned int idx, __pte_t pte)
{
	(void)lvl;

#ifdef CONFIG_LIBUKDEBUG
	UK_ASSERT(idx < PT_Lx_PTES(lvl));
#endif /* CONFIG_LIBUKDEBUG */

	*((__pte_t *)pt_vaddr + idx) = pte;

	return 0;
}

static inline __paddr_t ukarch_pt_read_base(void)
{
	__paddr_t reg;

	__asm__ __volatile__("mrs %x0, ttbr0_el1\n" : "=r" (reg));

	return reg;
}

static inline int ukarch_pt_write_base(__paddr_t pt_paddr)
{
	__asm__ __volatile__("msr ttbr0_el1, %x0\n"
			     "isb \n"
			     :: "r" (pt_paddr));

	/* TODO Does this implicitly flush the TLB? Remember adding barriers if needed */

	return 0;
}

static inline void ukarch_tlb_flush_entry(__vaddr_t vaddr)
{
	__asm__ __volatile__(
		"	dsb ishst\n"       /* make sure writes complete */
		"	tlbi vaae1, %x0\n" /* invalidate by vaddr, any ASID */
		"	dsb ish\n"         /* make sure invalidate completes */
		"	isb\n"             /* sync context */
		:: "r" (vaddr) : "memory");
}

static inline void ukarch_tlb_flush(void)
{
	__asm__ __volatile__(
		"	dsb ishst\n"      /* wait for writes to complete */
		"	tlbi vmalle1is\n" /* invalidate all inner-shareable */
		"	dsb ish\n"        /* wait for invalidate complete */
		"	isb\n"            /* sync context */
		::: "memory");
}
#endif /* !__ASSEMBLY__ */
