/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Marc Rittinghaus <marc.rittinghaus@kit.edu>
 * Authors: Michalis Pappas <michalis.pappas@opensynergy.com>
 *
 * Copyright (c) 2021, Karlsruhe Institute of Technology (KIT).
 *                     All rights reserved.
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

#if 0
#ifndef __PLAT_CMN_ARCH_PAGING_H__
#error Do not include this header directly
#endif
#else /* FIXME */
#define CONFIG_PAGING
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

#if PT_LEVELS < 4
#error "Unsupported number of page table levels"
#endif

/*
 * We expect the physical memory to be mapped 1:1 into the following area in
 * every virtual address space. This allows us to easily translate from virtual
 * to physical page table addresses and vice versa. We also access the metadata
 * for the frame allocators through this area.
 */
#define DIRECTMAP_AREA_START	0xffff000000000000
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
pgarch_pte_change_prot(__pte_t pte, unsigned long new_prot,
		       unsigned int level __unused)
{
	/* No write-only pages in VMSAv8-64 */
	UK_ASSERT(new_prot & PAGE_PROT_READ);

	pte &= ~ATTR_AP_MASK;
	pte &= ~ATTR_PXN;

	if (!(new_prot & PAGE_PROT_WRITE))
		pte |= ATTR_AP(ATTR_AP_RO);

	if (!(new_prot & PAGE_PROT_EXEC))
		pte |= ATTR_PXN;

	return pte;
}

static inline unsigned long
pgarch_prot_from_pte(__pte_t pte, unsigned int level __unused)
{
	unsigned long prot = PAGE_PROT_READ;

	if (!(pte & ATTR_AP(ATTR_AP_RO)))
		prot |= PAGE_PROT_WRITE;

	if (!(pte & ATTR_PXN))
		prot |= PAGE_PROT_EXEC;

	return prot;
}

/*
 * Page tables
 */
static inline __vaddr_t
pgarch_pt_pte_to_vaddr(struct uk_pagetable *pt __unused, __pte_t pte,
		       unsigned int level __maybe_unused)
{
	return arm64_directmap_paddr_to_vaddr(PT_Lx_PTE_PADDR(pte, level));
}

/* Create a Block or Page Descriptor */
static inline __pte_t
pgarch_pte_create(__paddr_t paddr, unsigned long prot, unsigned int level,
		  __pte_t template, unsigned int template_level __unused)
{
	__pte_t pte;

	UK_ASSERT(PAGE_ALIGNED(paddr));
	UK_ASSERT(level <= ARM64_PT_MAP_LEVEL_MAX);

	pte = paddr & arm64_pte_lx_map_paddr_mask[level];

	pte |= PTE_Lx_VALID_BIT;

	if (!(prot & PAGE_PROT_WRITE))
		pte |= ATTR_AP(ATTR_AP_RO);

	if (!(prot & PAGE_PROT_EXEC))
		pte |= ATTR_PXN;

	pte |= template & (ATTR_CONTIGUOUS | ATTR_DBM | ATTR_nG |
			   ATTR_AF | ATTR_SH_MASK | ATTR_IDX_MASK);

	return pte;
}

/* Create a Table Descriptor */
static inline __pte_t
pgarch_pt_pte_create(struct uk_pagetable *pt __unused, __paddr_t pt_paddr,
		     unsigned int level __unused, __pte_t template,
		     unsigned int template_level __unused)
{
	__pte_t pt_pte;

	UK_ASSERT(PAGE_ALIGNED(pt_paddr));

	pt_pte = pt_paddr & ARM64_PTE_Lx_TABLE_PADDR_MASK;

	pt_pte |= (PTE_Lx_TYPE_TABLE << PTE_Lx_TYPE_SHIFT);
	pt_pte |= PTE_Lx_VALID_BIT;

	/* TODO Add flags from template */

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

static inline int pgarch_init(void)
{
	__u64 reg;

	unsigned int pa_range;
	unsigned int pa_bits[] = {
		PARANGE_0000, PARANGE_0001, PARANGE_0010, PARANGE_0011,
		PARANGE_0100, PARANGE_0101, PARANGE_0110
	};

	unsigned int ia_size;
	unsigned int oa_size;

	/* Check FEAT_TTST is implemented */
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
	printf("%d-bit PA range\n", pa_range);

	/* Check TTBR0 walks are enabled */
	if (SYSREG_READ64(TCR_EL1) & TCR_EL1_EPD0_BIT)
		UK_CRASH("TTBR0_EL1 table walks are not enabled\n");

	/* Check TTBR1 walks are disabled */
	/* FIXME Disable on entry 
	if (!(SYSREG_READ64(TCR_EL1) & TCR_EL1_EPD1_BIT))
		UK_CRASH("TTBR1_EL1 table walks are not disabled\n");
	*/

	/* Check supported granule size */
	reg = (SYSREG_READ64(ID_AA64MMFR0_EL1) >> ID_AA64MMFR0_EL1_TGRAN4_SHIFT) &
	       ID_AA64MMFR0_EL1_TGRAN4_MASK; 
	if (reg != ID_AA64MMFR0_EL1_TGRAN4_SUPPORTED)
		UK_CRASH("4KiB granule not supported\n");

	printf("Granule size: 4KiB\n"); /* FIXME all messages */

	/* Check configured granule size */
	if ((SYSREG_READ64(TCR_EL1) & TCR_EL1_TG0_MASK) != TCR_EL1_TG0_4K)
		UK_CRASH("Invalid granule size\n");

	/* Check IA size. This should be set at entry. */
	ia_size = arm64_paddr_bits();
	printf("IA size: %d\n", ia_size);
	if (ia_size != PA_RANGE_MAX)
		UK_CRASH("Invalid IA size\n");

	/* Check OA size == IA size. This should be set at entry. */
	oa_size = arm64_vaddr_bits();
	printf("OA size: %d\n", oa_size);
	if (ia_size != oa_size)
		UK_CRASH("IA - OA size mismatch");

	/* Check addressing mode set to 48-bit */
	if (SYSREG_READ64(TCR_EL1) & TCR_EL1_DS_BIT)
		UK_CRASH("48-bit addressing in not enabled\n");

	return 0;
}

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
