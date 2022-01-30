/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Author(s): Marc Rittinghaus <marc.rittinghaus@kit.edu>
 *
 * Copyright (c) 2021, Karlsruhe Institute of Technology. All rights reserved.
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

#ifndef __UK_EVENT_H__
#define __UK_EVENT_H__

#include <uk/essentials.h>
#include <uk/prio.h>

#ifdef CONFIG_LIBUKDEBUG
#include <uk/assert.h>
#endif /* CONFIG_LIBUKDEBUG */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_LIBUKDEBUG
#define __event_assert(x) UK_ASSERT(x)
#else
#define __event_assert(x)	\
	do {			\
	} while (0)
#endif /* CONFIG_LIBUKDEBUG */

/**
 * Signature for event handler functions.
 *
 * @param data
 *   Optional data parameter. The data is supplied when raising the event
 * @return
 *   One of the UK_EVENT_* macros on success, errno on < 0
 */
typedef int (*uk_event_handler_t)(void *data);

#define UK_EVENT_NOT_HANDLED 0  /* The handler did not handle the event */
#define UK_EVENT_HANDLED     1  /* The handler handled the event */

struct uk_event {
	const uk_event_handler_t *handlers;
};

/* Register an event along with a list of handlers.
 *
 * @param name
 * 	Name of the event. The name may contain any characters that can be used
 *	in C identifiers.
 * @param hndl
 * 	NULL-terminated array of event handlers. Each member must be an instance of
 * 	uk_event_handler_t. When the event is raised, the handlers are invoked in
 * 	the order they are declared in the array.
 */
#define UK_EVENT(name, hndl)	\
struct uk_event name = {	\
	.handlers = hndl	\
}

/**
 * Raise an event by pointer and invoke the handler chain until the first
 * handler successfully handled the event.
 *
 * @param event
 *   Pointer to the event to raise.
 * @param data
 *   Optional data supplied to the event handlers
 * @return
 *   One of the UK_EVENT_* macros on success, errno on < 0
 */
static inline int uk_raise_event(struct uk_event *event, void *data)
{
	const uk_event_handler_t *h;
	int ret = UK_EVENT_NOT_HANDLED;

	__event_assert(event);

	for (size_t i = 0; (h = &event->handlers[i]) != NULL; i++) {
		ret = ((*h)(data));
		if (ret)
			break;
	}

	return ret;
}

/**
 * Helper macro to raise an event by name. Invokes the handler chain until the
 * first handler successfully handled the event.
 *
 * @param event
 *   Name of the event to raise.
 * @param data
 *   Optional data supplied to the event handlers
 * @return
 *   One of the UK_EVENT_* macros on success, errno on < 0
 */
#define uk_raise_event_by_name(name, data) \
	uk_raise_event(&name, data)

#ifdef __cplusplus
}
#endif

#endif /* __UK_EVENT_H__ */
