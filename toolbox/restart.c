#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <cutils/properties.h>

int restart_main(int argc, char *argv[])
{
    char buf[1024];

    if(argc > 1) {
        property_set("ctl.stop", argv[1]);
        property_set("ctl.start", argv[1]);
    } else {
        /* defaults to stopping and starting the common services */
        property_set("ctl.stop", "zygote");
        property_set("ctl.stop", "surfaceflinger");
        property_set("ctl.start", "surfaceflinger");
        property_set("ctl.start", "zygote");
    }

    return 0;
}
