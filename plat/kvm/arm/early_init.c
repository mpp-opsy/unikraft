/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2023, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the BSD-3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 */

#include <uk/assert.h>
#include <uk/essentials.h>

#if CONFIG_LIBUKTTY_PL011_EARLY_CONSOLE
#include <uk/tty/pl011.h>
#endif /* CONFIG_LIBUKTTY_PL011_EARLY_CONSOLE */

#if CONFIG_LIBUKTTY_NS16550_EARLY_CONSOLE
#include <uk/tty/ns16550.h>
#endif /* CONFIG_LIBUKTTY_NS16550_EARLY_CONSOLE */

int kvm_early_init_fdt(void *fdt __maybe_unused)
{
	int rc __maybe_unused = 0;

#if CONFIG_LIBUKTTY_NS16550_EARLY_CONSOLE
	rc = ns16550_early_init();
#endif /* CONFIG_LIBUKTTY_NS16550_EARLY_CONSOLE */

#if CONFIG_LIBUKTTY_PL011_EARLY_CONSOLE
	rc = pl011_early_init();
#endif /* CONFIG_LIBUKTTY_PL011_EARLY_CONSOLE */

	return rc;
}
