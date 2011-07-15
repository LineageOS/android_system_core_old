#include <reboot/reboot.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/reboot.h>

#ifdef RECOVERY_SHELL
#include "../../../bootable/recovery/libcrecovery/common.h"
#define system __system
#endif

#ifndef REBOOT_REASON_DEFAULT
#define REBOOT_REASON_DEFAULT NULL
#endif

int reboot_wrapper(const char* reason) {
    int reboot_with_reason = 0;
    int need_clear_reason = 0;
    int ret;

    if (reason != NULL) {
        // pass the reason to the kernel when we reboot
        reboot_with_reason = 1;

    #ifdef RECOVERY_PRE_COMMAND
        if (!strncmp(reason,"recovery",8)) {
            system( RECOVERY_PRE_COMMAND );

            #ifdef RECOVERY_PRE_COMMAND_CLEAR_REASON
            need_clear_reason = 1;
            #endif
        }
    #endif
    #ifdef REBOOT_PRE_COMMAND

        // called for all reboot reasons, could be a script or binary tool.
        char cmd[256];
        sprintf(cmd, REBOOT_PRE_COMMAND " %s", reason);

        system(cmd);

        #ifdef REBOOT_PRE_COMMAND_CLEAR_REASON
        need_clear_reason = 1;
        #endif

    #endif
    }
    else
        reason = REBOOT_REASON_DEFAULT;

    // although we have a reason, ignore it on reboot
    if (need_clear_reason) {
        reboot_with_reason = 0;
    }

    if (reboot_with_reason)
        ret = __reboot(LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART2, (char *)reason);
    else
        ret = reboot(RB_AUTOBOOT);

    return ret;
}
