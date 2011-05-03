#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/reboot.h>
#include <unistd.h>

#ifdef RECOVERY_WRITE_MISC_PART
#include <fcntl.h>
#include <sys/mount.h>
#include <mtd/mtd-user.h>

struct bootloader_message {
    char command[32];
    char status[32];
    char recovery[1024];
    char stub[2048 - 32 - 32 - 1024];
};

char command[2048];
int mtdnum = -1;
int mtdsize = 0;
int mtderasesize = 0;
char mtdname[64];
char mtddevname[32];

#define MTD_PROC_FILENAME   "/proc/mtd"

int init_mtd_info() {
	if (mtdnum >= 0) {
		return 0;
	}
    int fd = open(MTD_PROC_FILENAME, O_RDONLY);
    if (fd < 0) {
        return (mtdnum = -1);
    }
    int nbytes = read(fd, command, sizeof(command) - 1);
    close(fd);
    if (nbytes < 0) {
        return (mtdnum = -2);
    }
    command[nbytes] = '\0';
	char *cursor = command;
	while (nbytes-- > 0 && *(cursor++) != '\n'); // skip one line
	while (nbytes > 0) {
		int matches = sscanf(cursor, "mtd%d: %x %x \"%63s[^\"]", &mtdnum, &mtdsize, &mtderasesize, mtdname);
                if (matches == ( RECOVERY_WRITE_MISC_PART )) {
                        if (strncmp("misc", mtdname, ( RECOVERY_WRITE_MISC_PART )) == 0) {
				sprintf(mtddevname, "/dev/mtd/mtd%d", mtdnum);
				printf("Partition for parameters: %s\n", mtddevname);
				return 0;
			}
			while (nbytes-- > 0 && *(cursor++) != '\n'); // skip a line
		}
	}
    return (mtdnum = -3);
}

char* get_message() {
	int fd;
	int pos = 2048;
	if (init_mtd_info() != 0) {
		return NULL;
	}
	fd = open(mtddevname, O_RDONLY);
    if (fd < 0) {
        return NULL;
    }
	if (lseek(fd, pos, SEEK_SET) != pos) {
		return NULL;
	}
	pos = read(fd, command, sizeof(command));
	//printf("Read %d bytes\n", pos);
	if (pos < 0) {
        return NULL;
    }
	if (command[0] != 0 && command[0] != 255) {
	   //command[8] = 0;
       printf("Boot command: %s\n", command);
	   return NULL;
    } else {
       printf("No boot command\n");
	}
	close(fd);
    return command;
}

int set_message(char* cmd) {
	int fd;
	int pos = 2048;
	if (init_mtd_info() != 0) {
		return -9;
	}
	fd = open(mtddevname, O_RDWR);
    if (fd < 0) {
        return fd;
    }
    struct erase_info_user erase_info;
    erase_info.start = 0;
    erase_info.length = mtderasesize;
    if (ioctl(fd, MEMERASE, &erase_info) < 0) {
		fprintf(stderr, "mtd: erase failure at 0x%d (%s)\n", pos, strerror(errno));
    }
	if (lseek(fd, pos, SEEK_SET) != pos) {
		close(fd);
		return pos;
	}
	memset(&command, 0, sizeof(command));
	strncpy(command, cmd, strlen(cmd));
	pos = write(fd, command, sizeof(command));
	//printf("Written %d bytes\n", pos);
	if (pos < 0) {
		close(fd);
        return pos;
    }
	close(fd);
    return 0;
}
#endif // RECOVERY_WRITE_MISC_PART

int reboot_main(int argc, char *argv[])
{
    int ret;
    int nosync = 0;
    int poweroff = 0;

    opterr = 0;
    do {
        int c;

        c = getopt(argc, argv, "np");
        
        if (c == EOF) {
            break;
        }
        
        switch (c) {
        case 'n':
            nosync = 1;
            break;
        case 'p':
            poweroff = 1;
            break;
        case '?':
            fprintf(stderr, "usage: %s [-n] [-p] [rebootcommand]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    } while (1);

    if(argc > optind + 1) {
        fprintf(stderr, "%s: too many arguments\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if(!nosync)
        sync();

    if(poweroff)
        ret = __reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_POWER_OFF, NULL);
    else if(argc > optind) {
#ifdef RECOVERY_PRE_COMMAND
        if (!strncmp(argv[optind],"recovery",8))
            system( RECOVERY_PRE_COMMAND );
#endif
#ifdef RECOVERY_WRITE_MISC_PART
        set_message(argv[optind]);
#endif
        ret = __reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART2, argv[optind]);
    } else
        ret = reboot(RB_AUTOBOOT);
    if(ret < 0) {
        perror("reboot");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "reboot returned\n");
    return 0;
}
