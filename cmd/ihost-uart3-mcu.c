#include <common.h>
#include <command.h>
#include <debug_uart.h>
#include <ihost_uart3_common.h>


static int do_uart3_mcu_test(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    int ret = do_uart3_mcu(NULL, 0, 0, NULL);
    return ret;
}

U_BOOT_CMD(
    uart3_mcu_test, 1, 0, do_uart3_mcu_test,
    "Test UART3 MCU communication",
    ""
);