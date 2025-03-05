/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Rockchip Electronic Co.,Ltd
 */

#include <linux/linkage.h>
#include <asm/assembler.h>

ENTRY(spl_optee_entry)
#ifdef CONFIG_ROCKCHIP_OPTEE_BINARY
	ldr r1, =CONFIG_TEXT_BASE
#else
	ldr lr, =CONFIG_TEXT_BASE
#endif
	mov pc, r3
ENDPROC(spl_optee_entry)
