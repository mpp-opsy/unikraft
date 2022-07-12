/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Michalis Pappas <michalis.pappas@opensynergy.com>
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

#ifndef __PLAT_CMN_ARCH_PAGING_H__
#error Do not include this header directly
#endif

#include <arm/arm64/cpu.h>
#include <arm/arm64/cpu_defs.h>

#include <uk/arch/limits.h>
#include <uk/essentials.h>
#include <uk/plat/paging.h>
#include <uk/fallocbuddy.h>
#include <uk/print.h>

#include <errno.h>

#define PA_RANGE_MIN	44 /* FIXME can we go lower, like 40-bits? */
#define PA_RANGE_MAX	48

/* ---------------- FIXME ------------------------ */
/*These are also / should be defined in plat */

#define PTE_Lx_VALID_BIT	1UL

#define PTE_Lx_TYPE_BIT		(1UL << 1)
#define PTE_Lx_TYPE_SHIFT	1

#define PTE_Lx_TYPE_BLOCK	0
#define PTE_Lx_TYPE_TABLE	1

/* D5-4815 - D5-4816 */
#define PTE_Lx_TABLE_PADDR_MASK	0x0000fffffffff000 /* [47-12] */

#define PTE_L2_BLOCK_PADDR_MASK	0x0000ffffc0000000 /* [47-30] */
#define PTE_L1_BLOCK_PADDR_MASK	0x0000ffffffe00000 /* [47-21] */
#define PTE_L0_PAGE_PADDR_MASK	0x0000fffffffff000 /* [47-12] */
/* ---------------- FIXME ----------------------- */

/*
 * We expect the physical memory to be mapped 1:1 into the following area in
 * every virtual address space. This allows us to easily translate from virtual
 * to physical page table addresses and vice versa. We also access the metadata
 * for the frame allocators through this area.
 */

#define DIRECTMAP_AREA_START	0x0000ff8000000000
#define DIRECTMAP_AREA_END	0xffffffffffffffff
#define DIRECTMAP_AREA_SIZE	(DIRECTMAP_AREA_END - DIRECTMAP_AREA_START + 1)

static inline __vaddr_t
arm64_directmap_paddr_to_vaddr(__paddr_t paddr)
{
	UK_ASSERT(paddr < DIRECTMAP_AREA_SIZE);
	return (__vaddr_t)paddr + DIRECTMAP_AREA_START;
}

static inline __paddr_t
arm64_directmap_vaddr_to_paddr(__vaddr_t vaddr)
{
	UK_ASSERT(vaddr >= DIRECTMAP_AREA_START);
	UK_ASSERT(vaddr < DIRECTMAP_AREA_END);
	return (__paddr_t)(vaddr - DIRECTMAP_AREA_START);
}

static inline __pte_t
pgarch_pte_create(__paddr_t paddr, unsigned long attr, unsigned int level,
		  __pte_t template, unsigned int template_level __unused)
{
	__pte_t pte;

	unsigned long pte_lx_map_paddr_mask[] = {
		PTE_L0_PAGE_PADDR_MASK,
		PTE_L1_BLOCK_PADDR_MASK,
		PTE_L2_BLOCK_PADDR_MASK,
	};

	UK_ASSERT(PAGE_ALIGNED(paddr));
	UK_ASSERT(level <= PT_MAP_LEVEL_MAX);

	pte = paddr & pte_lx_map_paddr_mask[level];

	if (!template)
		pte |= (SECT_ATTR_DEFAULT | ATTR_IDX(NORMAL_WB));
	else
		pte |= template & (ATTR_CONTIGUOUS | ATTR_DBM | ATTR_nG |
				   ATTR_AF | ATTR_SH_MASK | ATTR_IDX_MASK);

	if (level == PAGE_LEVEL)
		pte |= Ln_PAGE;
	else
		pte |= Ln_BLOCK;

	if (!(attr & PAGE_ATTR_PROT_WRITE))
		pte |= ATTR_AP(ATTR_AP_RO);

	if (!(attr & PAGE_ATTR_PROT_EXEC))
		pte |= ATTR_XN;

	return pte;
}

static inline __pte_t
pgarch_pte_change_attr(__pte_t pte, unsigned long new_attr,
		       unsigned int level __unused)
{
	(void)pte;
	(void)new_attr;

	UK_CRASH("%s: TRACE", __func__);

	return 0;
}

static inline unsigned long
pgarch_attr_from_pte(__pte_t pte, unsigned int level __unused)
{
	unsigned long attr = PAGE_ATTR_PROT_READ;

	if (!(pte & ATTR_AP(ATTR_AP_RO)))
		attr |= PAGE_ATTR_PROT_WRITE;

	if (!(pte & ATTR_PXN))
		attr |= PAGE_ATTR_PROT_EXEC;

	return attr;
}

/*
 * Page tables
 */
static inline __vaddr_t
pgarch_pt_pte_to_vaddr(struct uk_pagetable *pt __unused, __pte_t pte,
		       unsigned int level __maybe_unused)
{
	(void)pt;

	__vaddr_t ret = arm64_directmap_paddr_to_vaddr(PT_Lx_PTE_PADDR(pte, level));

	return ret;
}

/* Create a table descriptor */
static inline __pte_t
pgarch_pt_pte_create(struct uk_pagetable *pt __unused, __paddr_t pt_paddr,
		     unsigned int level __unused, __pte_t template,
		     unsigned int template_level __unused)
{
	__pte_t pt_pte;

	UK_ASSERT(PAGE_ALIGNED(pt_paddr));

	pt_pte = pt_paddr & PTE_Lx_TABLE_PADDR_MASK;

	pt_pte |= (PTE_Lx_TYPE_TABLE << PTE_Lx_TYPE_SHIFT);
	pt_pte |= PTE_Lx_VALID_BIT;

	/* TODO Add flags from template */
	UK_ASSERT(!template);

	return pt_pte;
}

static inline __vaddr_t
pgarch_pt_map(struct uk_pagetable *pt __unused, __paddr_t pt_paddr,
	      unsigned int level __unused)
{
	return arm64_directmap_paddr_to_vaddr(pt_paddr);
}

static inline __paddr_t
pgarch_pt_unmap(struct uk_pagetable *pt __unused, __vaddr_t pt_vaddr,
		unsigned int level __unused)
{
	return arm64_directmap_vaddr_to_paddr(pt_vaddr);
}

static inline unsigned long arm64_vaddr_bits(void)
{
	__u64 TCR_IPS_MAP[] = {
		TCR_IPS_32BIT, TCR_IPS_36BIT,
		TCR_IPS_40BIT, TCR_IPS_42BIT,
		TCR_IPS_44BIT, TCR_IPS_48BIT,
		TCR_IPS_52BIT
	};
	__u64 reg = 0;

	__asm__ __volatile__("mrs %0, tcr_el1" : "=r" (reg));

	return TCR_IPS_MAP[(reg >> TCR_IPS_SHIFT) & TCR_IPS_MASK];
}

static inline unsigned int arm64_paddr_bits(void)
{
	__u64 reg;

	__asm__ __volatile__("mrs %0, tcr_el1" : "=r" (reg));

	return 64 - ((reg & TCR_T0SZ_MASK) >> TCR_T0SZ_SHIFT);
}

static inline int pgarch_init(void)
{
#if 0
	__u64 reg;

	unsigned int pa_range;
	unsigned int pa_bits[] = {
		PARANGE_0000, PARANGE_0001, PARANGE_0010, PARANGE_0011,
		PARANGE_0100, PARANGE_0101, PARANGE_0110
	};

	unsigned int ia_size;
	unsigned int oa_size;

	/* Check FEAT_TTST is implemented (max TxSZ 48) */
	if (!((SYSREG_READ64(ID_AA64MMFR2_EL1) >> ID_AA64MMFR2_EL1_ST_SHIFT) &
	    ID_AA64MMFR2_EL1_ST_MASK))
		UK_CRASH("FEAT_TTST is not implemented\n");
	/* Check supported PA size */
	reg = (SYSREG_READ64(ID_AA64MMFR0_EL1) >>
	       ID_AA64MMFR0_EL1_PARANGE_SHIFT) &
	       ID_AA64MMFR0_EL1_PARANGE_MASK;

	UK_ASSERT(reg < ARRAY_SIZE(pa_bits));

	pa_range = pa_bits[reg];

	if (pa_range < PA_RANGE_MIN || pa_range > PA_RANGE_MAX)
		UK_CRASH("%dbit PAs not supported\n", pa_range);
	printf("PA range: %d bits\n", pa_range);

	/* Check TTBR0 walks are enabled */
	if (SYSREG_READ64(TCR_EL1) & TCR_EL1_EPD0_BIT)
		UK_CRASH("TTBR0_EL1 table walks are not enabled\n");

	/* TODO
	 * Disable at entry
	 * Flush TLB for that range upon disable.
	 */
	/* Check TTBR1 walks are disabled */
	if (!(SYSREG_READ64(TCR_EL1) & TCR_EL1_EPD1_BIT))
		UK_CRASH("TTBR1_EL1 table walks are not disabled\n");

	/* Check supported granule size */
	reg = (SYSREG_READ64(ID_AA64MMFR0_EL1) >> ID_AA64MMFR0_EL1_TGRAN4_SHIFT) &
	       ID_AA64MMFR0_EL1_TGRAN4_MASK; 

	if (reg != ID_AA64MMFR0_EL1_TGRAN4_SUPPORTED)
		UK_CRASH("4KiB granule not supported\n");

	/* Check configured granule size */
	if ((SYSREG_READ64(TCR_EL1) & TCR_EL1_TG0_MASK) != TCR_EL1_TG0_4K)
		UK_CRASH("Invalid granule size\n");

	printf("Granule size: 4KiB\n"); /* FIXME all messages */

	/* TODO Check OA size == PA size */

	/* Check IA size. This should be set at entry. */
	ia_size = arm64_paddr_bits();
	printf("IA size: %d\n", ia_size);
	if (ia_size != PA_RANGE_MAX)
		UK_CRASH("Invalid IA size\n");

	/* Check OA size == IA size. This should be set at entry. */
#if 0 /* FIXME */
	oa_size = arm64_vaddr_bits();
	printf("OA size: 0x%lx\n", oa_size);
	if (ia_size != oa_size)
		UK_CRASH("IA - OA size mismatch");
#endif
	/* Check addressing mode set to 48-bit */
	if (SYSREG_READ64(TCR_EL1) & TCR_EL1_DS_BIT)
		UK_CRASH("48-bit addressing in not enabled\n");
#endif
	return 0;
}

#if 0 /* TODO */
static __paddr_t x86_pg_maxphysaddr;

#define X86_PG_VALID_PADDR(paddr)	((paddr) < x86_pg_maxphysaddr)

int ukarch_paddr_range_isvalid(__paddr_t start, __paddr_t end)
{
	UK_ASSERT(start <= end);

	return (X86_PG_VALID_PADDR(start) && X86_PG_VALID_PADDR(end));
}
#endif

static inline int
pgarch_pt_add_mem(struct uk_pagetable *pt, __paddr_t start, __sz len)
{
	unsigned long pages = len >> PAGE_SHIFT;
	void *fa_meta;
	__sz fa_meta_size;
	__vaddr_t dm_off;
	int rc;

	UK_ASSERT(start <= __PADDR_MAX - len);
	UK_ASSERT(ukarch_paddr_range_isvalid(start, start + len));

	/* Reserve space for the metadata at the beginning of the area. Note
	 * that the metadata area will be a bit too large because we eat away
	 * from the frames by placing the metadata in the first frames.
	 */
	fa_meta = (void *)arm64_directmap_paddr_to_vaddr(start);
	fa_meta_size = uk_fallocbuddy_metadata_size(pages);

	if (unlikely(fa_meta_size >= len))
		return -ENOMEM;

	start = PAGE_ALIGN_UP(start + fa_meta_size);
	pages = (len - fa_meta_size) >> PAGE_SHIFT;

	dm_off = arm64_directmap_paddr_to_vaddr(start);

	UK_ASSERT(pt->fa->addmem);
	rc = pt->fa->addmem(pt->fa, fa_meta, start, pages, dm_off);
	if (unlikely(rc))
		return rc;

	return 0;
}

static inline int
pgarch_pt_init(struct uk_pagetable *pt, __paddr_t start, __sz len)
{
	__sz fa_size;
	int rc;

	/* Reserve space for the allocator at the beginning of the area. */
	pt->fa = (struct uk_falloc *)arm64_directmap_paddr_to_vaddr(start);

	fa_size = ALIGN_UP(uk_fallocbuddy_size(), 8);
	if (unlikely(fa_size >= len))
		return -ENOMEM;

	rc = uk_fallocbuddy_init(pt->fa);
	if (unlikely(rc))
		return rc;

	rc = pgarch_pt_add_mem(pt, start + fa_size, len - fa_size);
	if (unlikely(rc))
		return rc;

	return 0;
}
