/*
 * Copyright (C) 2011 Motorola
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <cutils/partition_utils.h>
#include <sys/mount.h>
#include "ext4_utils.h"
#include "ext4.h"
#include "fs_mgr_priv.h"
/* Avoid redefinition warnings */
#undef __le32
#undef __le16
#include <cryptfs.h>

/* These come from cryptfs.c */
#define CRYPT_KEY_IN_FOOTER "footer"
#define CRYPT_MAGIC         0xD0B5B1C4

#define F2FS_SUPER_MAGIC 0xF2F52010
#define EXT4_SUPER_MAGIC 0xEF53

#define INVALID_BLOCK_SIZE -1

int fs_mgr_is_partition_encrypted(struct fstab_rec *fstab)
{
    int fd = -1;
    struct stat statbuf;
    unsigned int sectors;
    off64_t offset;
    __le32 crypt_magic = 0;
    int ret = 0;

    if (!(fstab->fs_mgr_flags & MF_CRYPT))
        return 0;

    if (fstab->key_loc[0] == '/') {
        if ((fd = open(fstab->key_loc, O_RDWR)) < 0) {
            goto out;
        }
    } else if (!strcmp(fstab->key_loc, CRYPT_KEY_IN_FOOTER)) {
        if ((fd = open(fstab->blk_device, O_RDWR)) < 0) {
            goto out;
        }
        if ((ioctl(fd, BLKGETSIZE, &sectors)) == -1) {
            goto out;
        }
        offset = ((off64_t)sectors * 512) - CRYPT_FOOTER_OFFSET;
        if (lseek64(fd, offset, SEEK_SET) == -1) {
            goto out;
        }
    } else {
        goto out;
    }

    if (read(fd, &crypt_magic, sizeof(crypt_magic)) != sizeof(crypt_magic)) {
        goto out;
    }
    if (crypt_magic != CRYPT_MAGIC) {
        goto out;
    }

    /* It's probably encrypted! */
    ret = 1;

out:
    if (fd >= 0) {
        close(fd);
    }
    return ret;
}

#define TOTAL_SECTORS 4         /* search the first 4 sectors */
static int is_f2fs(char *block)
{
    __le32 *sb;
    int i;

    for (i = 0; i < TOTAL_SECTORS; i++) {
        sb = (__le32 *)(block + (i * 512));     /* magic is in the first word */
        if (le32_to_cpu(sb[0]) == F2FS_SUPER_MAGIC) {
            return 1;
        }
    }

    return 0;
}

static int is_ext4(char *block)
{
    struct ext4_super_block *sb = (struct ext4_super_block *)block;
    int i;

    for (i = 0; i < TOTAL_SECTORS * 512; i += sizeof(struct ext4_super_block), sb++) {
        if (le32_to_cpu(sb->s_magic) == EXT4_SUPER_MAGIC) {
            return 1;
        }
    }

    return 0;
}

/* Examine the superblock of a block device to see if the type matches what is
 * in the fstab entry.
 */
int fs_mgr_identify_fs(struct fstab_rec *fstab)
{
    char *block = NULL;
    int fd = -1;
    char rc = -1;

    block = calloc(1, TOTAL_SECTORS * 512);
    if (!block) {
        goto out;
    }
    if ((fd = open(fstab->blk_device, O_RDONLY)) < 0) {
        goto out;
    }
    if (read(fd, block, TOTAL_SECTORS * 512) != TOTAL_SECTORS * 512) {
        goto out;
    }

    if ((!strncmp(fstab->fs_type, "f2fs", 4) && is_f2fs(block)) ||
        (!strncmp(fstab->fs_type, "ext4", 4) && is_ext4(block))) {
        rc = 0;
    } else {
        ERROR("Did not recognize file system type %s on %s\n", fstab->fs_type, fstab->blk_device);
    }

out:
    if (fd >= 0) {
        close(fd);
    }
    if (block) {
        free(block);
    }
    return rc;
}
