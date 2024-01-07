/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2023, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the BSD-3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 */

#include <uk/process.h>
#include <uk/refcount.h>
#include <uk/semaphore.h>

#include "process.h"
#include "sigset.h"
#include "signal.h"

/* Enqueue signal for a thread or a process
 *
 * Setting pthread to NULL signifies that the signal
 * targets the process.
 */
static int pprocess_signal_enqueue(struct posix_process *pproc,
				   struct posix_thread *pthread,
				   struct uk_signal *sig)
{
	int signum;

	UK_ASSERT(sig);
	UK_ASSERT(pproc);

	if (pthread)
		UK_ASSERT(pthread->process == pproc);

	if (unlikely(pproc->signal->queued_count >= pproc->signal->queued_max))
		return -EAGAIN;

	signum = sig->siginfo.si_signo;

	if (pthread) {
		/* Standard signals can be queued once */
		if (signum < SIGRTMIN && IS_PENDING(pthread->signal->sigqueue, signum))
			return 0;

		uk_pr_debug("Queueing signal %d for tid %d (pid %d)\n",
			    signum, pthread->tid, pproc->pid);

		uk_list_add_tail(&sig->list_head,
				 &pthread->signal->sigqueue.list_head[signum]);
		SET_PENDING(pproc->signal->sigqueue, signum);
	} else {
		/* Standard signals can be queued once */
		if (signum < SIGRTMIN && IS_PENDING(pproc->signal->sigqueue, signum))
			return 0;

		uk_pr_debug("Queueing signal %d for pid %d\n",
			    signum, pproc->pid);

		uk_list_add_tail(&sig->list_head,
				 &pproc->signal->sigqueue.list_head[signum]);
		SET_PENDING(pproc->signal->sigqueue, signum);
	}

	pproc->signal->queued_count++;

	return 0;
}

struct uk_signal *pprocess_signal_dequeue(struct posix_process *pproc,
					  struct posix_thread *pthread,
					  int signum)
{
	struct uk_signal_queue *sigqueue;
	struct uk_signal *sig;

	UK_ASSERT(pproc);

	if (pthread)
		UK_ASSERT(pthread->process == pproc);

	if (pthread)
		sigqueue = &pthread->signal->sigqueue;
	else
		sigqueue = &pproc->signal->sigqueue;

	sig = uk_list_first_entry_or_null(&sigqueue->list_head[signum],
					  struct uk_signal,
					  list_head);
	if (!sig)
		return sig;

	uk_list_del(&sig->list_head);

	/* Reset pending bit if the signal queue is empty */
	if (uk_list_empty(&sigqueue->list_head[signum])) {
		if (pthread)
			RESET_PENDING(pthread->signal->sigqueue, signum);
		else
			RESET_PENDING(pproc->signal->sigqueue, signum);
	}

	pproc->signal->queued_count--;

	return sig;
}

struct uk_signal *pprocess_signal_next_pending_t(struct posix_thread *pthread)
{
	struct posix_process *pproc;
	struct uk_signal *sig;

	pproc = tid2pprocess(pthread->tid);
	UK_ASSERT(pproc);

	for (int i = 1; i < SIG_ARRAY_COUNT; i++) {
		if (!pprocess_signal_is_deliverable(pthread, i))
			continue;

		sig = pprocess_signal_dequeue(pproc, pthread, i);
		if (sig)
			return sig;
	}

	return NULL;
}

void pprocess_signal_clear_pending(struct posix_process *proc, int signum)
{
	struct posix_thread *thread, *threadn;
	struct uk_signal *sig;

	if (IS_PENDING(proc->signal->sigqueue, signum)) {
		while ((sig = pprocess_signal_dequeue(proc, NULL, signum)))
			uk_signal_free(proc->_a, sig);
	}

	uk_process_foreach_pthread(proc, thread, threadn) {
		if (!IS_PENDING(thread->signal->sigqueue, signum))
			continue;
		while ((sig = pprocess_signal_dequeue(proc, thread, signum)))
			uk_signal_free(thread->_a, sig);
	}
}

void pprocess_signal_sigaction_clear(struct kern_sigaction *ks)
{
	ks->ks_handler = SIG_DFL;
	ks->ks_flags = 0;
	ks->ks_restorer = NULL;
	uk_sigemptyset(&ks->ks_mask);
}

int pprocess_signal_sigaction_new(struct uk_alloc *alloc, struct uk_signal_pdesc *pd)
{
	pd->sigaction = uk_malloc(alloc, sizeof(struct uk_sigaction));
	if (unlikely(!pd->sigaction)) {
		uk_pr_err("Could not allocate memory\n");
		return -ENOMEM;
	}

	pd->sigaction->alloc = alloc;
	uk_refcount_init(&pd->sigaction->refcnt, 1);

	return 0;
}

int pprocess_signal_pdesc_alloc(struct posix_process *process)
{
	UK_ASSERT(process);

	process->signal = uk_malloc(process->_a,
				    sizeof(struct uk_signal_pdesc));
	if (unlikely(!process->signal)) {
		uk_pr_err("Could not allocate memory\n");
		return -ENOMEM;
	}

	return 0;
}

int pprocess_signal_tdesc_alloc(struct posix_thread *thread)
{
	UK_ASSERT(thread);

	thread->signal = uk_malloc(thread->_a, sizeof(struct uk_signal_tdesc));
	if (unlikely(!thread->signal)) {
		uk_pr_err("Could not allocate memory\n");
		return -ENOMEM;
	}

	return 0;
}

int pprocess_signal_pdesc_init(struct posix_process *process)
{
	struct uk_signal_pdesc *pd;
	int rc;

	UK_ASSERT(process);
	UK_ASSERT(process->signal);

	pd = process->signal;

	pd->queued_count = 0;
	pd->queued_max = _POSIX_SIGQUEUE_MAX;

	uk_sigemptyset(&pd->sigqueue.pending);
	for (__sz i = 0; i < SIG_ARRAY_COUNT; i++)
		UK_INIT_LIST_HEAD(&pd->sigqueue.list_head[i]);

	rc = pprocess_signal_sigaction_new(process->_a, pd);
	if (unlikely(rc))
		return rc;

	for (__sz i = 0; i < SIG_ARRAY_COUNT; i++)
		pprocess_signal_sigaction_clear(KERN_SIGACTION(process, i));

	return 0;
}

int pprocess_signal_tdesc_init(struct posix_thread *thread)
{
	struct uk_signal_tdesc *td;

	UK_ASSERT(thread);
	UK_ASSERT(thread->signal);

	td = thread->signal;

	uk_sigemptyset(&td->mask);
	uk_sigemptyset(&td->sigqueue.pending);

	for (size_t i = 0; i < SIG_ARRAY_COUNT; i++)
		UK_INIT_LIST_HEAD(&td->sigqueue.list_head[i]);

	uk_semaphore_init(&thread->signal->pending_semaphore, 0);
	uk_semaphore_init(&thread->signal->deliver_semaphore, 0);

	return 0;
}

void pprocess_signal_pdesc_free(struct posix_process *process)
{
	struct uk_signal *sig;

	UK_ASSERT(process);
	UK_ASSERT(process->signal);

	for (__sz i = 0; i < SIG_ARRAY_COUNT; i++)
		while ((sig = pprocess_signal_dequeue(process, NULL, i)))
			uk_signal_free(process->_a, sig);

	pprocess_signal_sigaction_release(process->signal->sigaction);
}

void pprocess_signal_tdesc_free(struct posix_thread *thread)
{
	struct posix_process *pproc;
	struct uk_signal *sig;

	UK_ASSERT(thread);
	UK_ASSERT(thread->signal);

	pproc = tid2pprocess(thread->tid);
	UK_ASSERT(pproc);

	for (int i = 1; i < SIG_ARRAY_COUNT; i++)
		while ((sig = pprocess_signal_dequeue(pproc, thread, i)))
			uk_signal_free(thread->_a, sig);
}
