#include "bootcube.h"
#include <mmc.h>
#include <part.h>
#include <malloc.h>
#include <asm/cache.h>
#include <bootm.h>
#include <image.h>

enum boot_mode {
    BOOT_MODE_EMMC = 0,
    BOOT_MODE_SD = 1,
};

enum storage_type {
    STORAGE_TYPE_EMMC = 0,
    STORAGE_TYPE_SD = 1,
};

static struct blk_desc *emmc_dev_desc;
static struct blk_desc *sdcard_dev_desc;

int do_bootcube(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
    printf("bootcube start ......\n");

    struct mmc *mmc;
    // done: judge the sd card whether exists or not
    mmc = find_mmc_device(1);
    if (mmc_init(mmc) != 0) {
        printf("sd card not found\n");
        try_boot_from_emmc();
    } else {
        printf("sd card found\n");
        try_boot_from_sdcard();
    }   


    return 0;
}

static struct blk_desc *try_get_bootdev(enum storage_type storage_type) {
    if (storage_type == STORAGE_TYPE_EMMC && emmc_dev_desc) {
        return emmc_dev_desc;
    } else if (storage_type == STORAGE_TYPE_SD && sdcard_dev_desc) {
        return sdcard_dev_desc;
    }
    char *devnum = storage_type == STORAGE_TYPE_EMMC ? "0" : "1";
    int ret = blk_get_device_by_str("mmc", devnum, storage_type == STORAGE_TYPE_EMMC ? &emmc_dev_desc : &sdcard_dev_desc);
    if (ret < 0) {
        printf("%s: can't get mmc %s device_bolck\n", __func__, devnum);
        return NULL;
    }
    return storage_type == STORAGE_TYPE_EMMC ? emmc_dev_desc : sdcard_dev_desc;
}

static int recovery_judge(struct blk_desc *emmc){
    int recovery = 0;
    struct disk_partition *misc_part = malloc(sizeof(struct disk_partition));
    if (part_get_info_by_name(emmc, MISC_PARTITION, misc_part) < 0) {
        printf("%s: misc partition is not exists\n", __func__);
        return 0;
    }

    int cnt = DIV_ROUND_UP(sizeof(struct misc_part_info), misc_part->blksz);
    struct misc_part_info *misc_part_info = memalign(ARCH_DMA_MINALIGN, cnt * misc_part->blksz);
    // this is a magic number, ihost misc partition content is 16kb offset, block size is 512
    // so the offset is 0x20
    u32 bcb_offset = 0x20;
    if (blk_dread(emmc, misc_part->start + bcb_offset, cnt, misc_part_info) != cnt) {
        recovery = 0;
    } else {
        recovery = !strcmp(misc_part_info->command, "boot-recovery");
        if (recovery) {
            printf("boot mode: recovery(misc)\n");
        }
    }

    free(misc_part);
    free(misc_part_info);
    return recovery;
}

static int boot_from_emmc(char *boot_partition) {
    struct blk_desc *emmc = try_get_bootdev(STORAGE_TYPE_EMMC);
    if (!emmc) {
        return CMD_RET_FAILURE;
    }
    struct disk_partition *boot_part;
    if (part_get_info_by_name(emmc, boot_partition, boot_part) < 0) {
        printf("%s: %s partition is not exists\n", __func__, boot_partition);
        return CMD_RET_FAILURE;
    }
    int cnt = boot_part->size;
    void *boot_addr = memalign(ARCH_DMA_MINALIGN, cnt * boot_part->blksz);
    if (blk_dread(emmc, boot_part->start, cnt, boot_addr) != cnt) {
        printf("%s: read %s partition failed\n", __func__, boot_partition);
        return CMD_RET_FAILURE;
    }
    char *bootm_args[1];
    char fit_addr[12];
    env_set("bootm-no-reloc", "y");
    snprintf(fit_addr, sizeof(fit_addr), "0x%lx", (ulong)boot_addr);
    bootm_args[0] = fit_addr;
    printf("%s try to boot at %s with size 0x%08lx\n", boot_partition, fit_addr, cnt * boot_part->blksz);

    int ret = do_bootm_states(NULL, 0, ARRAY_SIZE(bootm_args), bootm_args,
            BOOTM_STATE_START |
            BOOTM_STATE_FINDOS | BOOTM_STATE_FINDOTHER |
            BOOTM_STATE_LOADOS |
    #ifdef CONFIG_SYS_BOOT_RAMDISK_HIGH
            BOOTM_STATE_RAMDISK |
    #endif
            BOOTM_STATE_OS_PREP | BOOTM_STATE_OS_FAKE_GO |
            BOOTM_STATE_OS_GO, &images, 1);

    if (ret) {
        printf("%s: boot %s failed\n", __func__, boot_partition);
        return CMD_RET_FAILURE;
    }

    return CMD_RET_SUCCESS;
}

int try_boot_from_emmc(void)
{
    printf("try to boot from emmc\n");
    if (run_command("mmc dev 0", 0) != 0) {
        printf("%s: can not switch to emmc\n", __func__);
        return CMD_RET_FAILURE;
    }
    // 1. judge misc partition content to start different system
    // 2. if misc partition is empty or is not exists, start ihost default system
    // 3. if misc partition is not empty, start the system specified in misc partition
    struct blk_desc *emmc = try_get_bootdev(STORAGE_TYPE_EMMC);
    if (!emmc) {
        printf("%s: can not find emmc\n", __func__);
        return CMD_RET_FAILURE;
    }
    char *boot_partition;
    int recovery = recovery_judge(emmc);
    if (!recovery) {
        printf("boot mode: normal\n");
        boot_partition = BOOT_PARTITION;
    } else {
        boot_partition = RECOVERY_PARTITION;
    }
    return boot_from_emmc(boot_partition);
}

void try_boot_from_sdcard(void)
{
    printf("try to boot from sd\n");
    // todo: listen to the uart3 and get the flag to start from sd card or emmc
    // if flag try to start sd card system
    // else try to start system
    if(run_command("run bootcmd_mmc1", 0) != 0) {
        try_boot_from_emmc();
    }
}

U_BOOT_CMD(
    bootcube, 1, 1, do_bootcube,
    "cube systemboot",
    "bootcube\t\t- cube system boot"
);
