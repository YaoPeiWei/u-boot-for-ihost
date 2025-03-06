// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 Rockchip Electronics Co., Ltd
 * Copyright (c) 2022 Edgeble AI Technologies Pvt. Ltd.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/hardware.h>
#include <asm/arch-rockchip/grf_rv1126.h>

#define FIREWALL_APB_BASE	0xffa60000
#define FW_DDR_CON_REG		0x80
#define GRF_BASE		0xFE000000

const char * const boot_devices[BROM_LAST_BOOTSOURCE + 1] = {
	[BROM_BOOTSOURCE_EMMC] = "/mmc@ffc50000",
	[BROM_BOOTSOURCE_SD] = "/mmc@ffc60000",
};

/* GRF_GPIO3A_IOMUX_L */
enum {
	GPIO3A3_SHIFT		= 12,
	GPIO3A3_MASK		= GENMASK(14, 12),
	GPIO3A3_GPIO		= 0,
	GPIO3A3_UART2_RX_M1,
	GPIO3A3_A7_JTAG_TMS_M1,

	GPIO3A2_SHIFT		= 8,
	GPIO3A2_MASK		= GENMASK(10, 8),
	GPIO3A2_GPIO		= 0,
	GPIO3A2_UART2_TX_M1,
	GPIO3A2_A7_JTAG_TCK_M1,
};

/* GRF_IOFUNC_CON2 */
enum {
	UART2_IO_SEL_SHIFT	= 8,
	UART2_IO_SEL_MASK	= GENMASK(8, 8),
	UART2_IO_SEL_M0		= 0,
	UART2_IO_SEL_M1,
};

void board_debug_uart_init(void)
{
	static struct rv1126_grf * const grf = (void *)GRF_BASE;

	/* Enable early UART2 channel m1 on the rv1126 */
	rk_clrsetreg(&grf->iofunc_con2, UART2_IO_SEL_MASK,
		     UART2_IO_SEL_M1 << UART2_IO_SEL_SHIFT);

	/* Switch iomux */
	rk_clrsetreg(&grf->gpio3a_iomux_l,
		     GPIO3A3_MASK | GPIO3A2_MASK,
		     GPIO3A3_UART2_RX_M1 << GPIO3A3_SHIFT |
		     GPIO3A2_UART2_TX_M1 << GPIO3A2_SHIFT);
}

/************ *uart3* *************/
enum {
	GPIO3A0_SHIFT		= 0,
	GPIO3A0_MASK		= GENMASK(2, 0),
	GPIO3A0_GPIO		= 0,
	GPIO3A0_RESERVED0,
	GPIO3A0_RESERVED1,
	GPIO3A0_CAN_RXD_M0,
	GPIO3A0_UART3_TX_M2,

	GPIO3A1_SHIFT		= 4,
	GPIO3A1_MASK		= GENMASK(6, 4),
	GPIO3A1_GPIO		= 0,
	GPIO3A1_RESERVED0,
	GPIO3A1_RESERVED1,
	GPIO3A1_CAN_TXD_M0,
	GPIO3A1_UART3_RX_M2,

	UART3_IO_SEL_SHIFT	= 10,
	UART3_IO_SEL_MASK	= GENMASK(11, 10),
	UART3_IO_SEL_M0		= 0,
	UART3_IO_SEL_M1,
	UART3_IO_SEL_M2,
};

void board_uart3_init(void){
	static struct rv1126_grf * const grf = (void *)GRF_BASE;
	/* UART3 m2*/
	rk_clrsetreg(&grf->iofunc_con2, UART3_IO_SEL_MASK,
		UART3_IO_SEL_M2 << UART3_IO_SEL_SHIFT);
	
	/* Switch iomux */
	rk_clrsetreg(&grf->gpio3a_iomux_l,
			GPIO3A1_MASK | GPIO3A0_MASK,
			GPIO3A1_UART3_RX_M2 << GPIO3A1_SHIFT |
			GPIO3A0_UART3_TX_M2 << GPIO3A0_SHIFT);
}

#ifndef CONFIG_TPL_BUILD
int arch_cpu_init(void)
{
	/**
	 * Set dram area unsecure in spl
	 *
	 * usb & mmc & sfc controllers can read data to dram
	 * since they are unsecure.
	 * (Note: only secure-world can access this register)
	 */
	if (IS_ENABLED(CONFIG_SPL_BUILD))
		writel(0, FIREWALL_APB_BASE + FW_DDR_CON_REG);

	return 0;
}
#endif
