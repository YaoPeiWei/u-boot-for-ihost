#ifndef __BOOTCUBE_H
#define __BOOTCUBE_H

#include <common.h>
#include <command.h>

#define MISC_PARTITION "misc"
#define RECOVERY_PARTITION "recovery"
#define BOOT_PARTITION "boot"

struct misc_part_info {
    char command[32];
    char status[32];
    char recovery[768];
    char stage[32];
    char slot_suffix[32];
    char reserved[192];
};

int do_bootcube(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[]);

int try_boot_from_emmc(void);

void try_boot_from_sdcard(void);

#endif
