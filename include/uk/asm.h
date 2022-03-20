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
#ifndef __INLCUDE_UK_ASM_H__
#define __INLCUDE_UK_ASM_H__

#ifdef __ASSEMBLY__

/* Linkage for ASM */
#define __ALIGN .align 2
#define __ALIGN_STR ".align 2"

#define ENTRY(name)	\
.globl name;		\
__ALIGN;		\
name:

#define GLOBAL(name)	\
.globl name;		\
name:

#define END(name)	\
.size name, . - name

#define ENDPROC(name)	\
.type name, %function;	\
END(name)

/* Macros to avoid undefined or unintended behavior when using integer
 * constants in C. The GNU assembler does not support these suffixes,
 * so these evaluate to unsuffixed integer values in asm.
 */
# define   U(_x)        (_x)
# define  UL(_x)        (_x)
# define ULL(_x)        (_x)
# define   L(_x)        (_x)
# define  LL(_x)        (_x)

#else /* __ASSEMBLY */

# define  U_(_x)        (_x##U)
# define   U(_x)        U_(_x)
# define  UL(_x)        (_x##UL)
# define ULL(_x)        (_x##ULL)
# define   L(_x)        (_x##L)
# define  LL(_x)        (_x##LL)
#endif /* __ASSEMBLY __ */

#endif /* __INLCUDE_UK_ASM_H__ */
