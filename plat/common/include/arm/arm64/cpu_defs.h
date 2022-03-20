/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Wei Chen <wei.chen@arm.com>
 *
 * Copyright (c) 2018, Arm Ltd. All rights reserved.
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
#ifndef __CPU_ARM_64_DEFS_H__
#define __CPU_ARM_64_DEFS_H__

#include <uk/arch/lcpu.h>

/* Registers and Bits definitions for MMU */
/*
 * Memory types, these values are the indexs of the attributes
 * that defined in MAIR_EL1.
 */
#define DEVICE_nGnRnE	0
#define DEVICE_nGnRE	1
#define DEVICE_GRE	2
#define NORMAL_NC	3
#define NORMAL_WT	4
#define NORMAL_WB	5

#define MAIR_INIT_ATTR	\
		(MAIR_ATTR(MAIR_DEVICE_nGnRnE, DEVICE_nGnRnE) | \
		MAIR_ATTR(MAIR_DEVICE_nGnRE, DEVICE_nGnRE) |   \
		MAIR_ATTR(MAIR_DEVICE_GRE, DEVICE_GRE) |       \
		MAIR_ATTR(MAIR_NORMAL_NC, NORMAL_NC) |         \
		MAIR_ATTR(MAIR_NORMAL_WT, NORMAL_WT) |         \
		MAIR_ATTR(MAIR_NORMAL_WB, NORMAL_WB))

/*
 * As we use VA == PA mapping, so the VIRT_BITS must be the same
 * as PA_BITS. We can get PA_BITS from ID_AA64MMFR0_EL1.PARange.
 * So the TxSZ will be calculate dynamically.
 */
#define TCR_INIT_FLAGS	(TCR_ASID_16 | TCR_TG0_4K | \
			TCR_CACHE_ATTRS | TCR_SMP_ATTRS)

/* Bits to set */
#define SCTLR_SET_BITS	\
		(SCTLR_UCI | SCTLR_nTWE | SCTLR_nTWI | SCTLR_UCT | \
		SCTLR_DZE | SCTLR_I | SCTLR_SED | SCTLR_SA0 | SCTLR_SA | \
		SCTLR_C | SCTLR_M | SCTLR_CP15BEN | SCTLR_RES1_B11 | \
		SCTLR_RES1_B20 | SCTLR_RES1_B22 | SCTLR_RES1_B23 | \
		SCTLR_RES1_B28 | SCTLR_RES1_B29)

/* Bits to clear */
#define SCTLR_CLEAR_BITS \
		(SCTLR_EE | SCTLR_EOE | SCTLR_WXN | SCTLR_UMA | \
		SCTLR_ITD | SCTLR_A | SCTLR_RES0_B6 | SCTLR_RES0_B10 | \
		SCTLR_RES0_B13 | SCTLR_RES0_B17 | SCTLR_RES0_B21 | \
		SCTLR_RES0_B27 | SCTLR_RES0_B30 | SCTLR_RES0_B31)

/*
 * Define the attributes of pagetable descriptors
 */
#define ATTR_DEFAULT	(ATTR_AF | ATTR_SH(ATTR_SH_IS))

#define SECT_ATTR_DEFAULT	\
		(Ln_BLOCK | ATTR_DEFAULT)
#define SECT_ATTR_NORMAL	\
		(SECT_ATTR_DEFAULT | ATTR_XN | \
		ATTR_IDX(NORMAL_WB))
#define SECT_ATTR_NORMAL_RO	\
		(SECT_ATTR_DEFAULT | ATTR_XN | \
		ATTR_AP_RW_BIT | ATTR_IDX(NORMAL_WB))
#define SECT_ATTR_NORMAL_EXEC	\
		(SECT_ATTR_DEFAULT | ATTR_UXN | \
		ATTR_AP_RW_BIT | ATTR_IDX(NORMAL_WB))
#define SECT_ATTR_DEVICE_nGnRE	\
		(SECT_ATTR_DEFAULT | ATTR_XN | \
		ATTR_IDX(DEVICE_nGnRE))
#define SECT_ATTR_DEVICE_nGnRnE	\
		(SECT_ATTR_DEFAULT | ATTR_XN | \
		ATTR_IDX(DEVICE_nGnRnE))

#endif /* __CPU_ARM_64_DEFS_H__ */
