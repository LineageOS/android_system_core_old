#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

extern int getxattr(const char *, const char *, void *, size_t);

static int usage(const char *s)
{
    fprintf(stderr, "Usage: %s -n name pathname\n", s);
    fprintf(stderr, "  -n name      name of the extended attribute to set\n");
    fprintf(stderr, "  -h           display this help and exit\n");

    exit(10);
}

int getfattr_main(int argc, char **argv)
{
    int i;
    char *name = NULL;
    char value[256];
    size_t valuelen = 0;

    for (;;) {
        int ret;

        ret = getopt(argc, argv, "hn:");

        if (ret < 0)
            break;

        switch(ret) {
            case 'h':
                usage(argv[0]);
                break;
            case 'n':
                name = optarg;
                break;
        }
    }

    if (!name || optind == argc)
        usage(argv[0]);

    for (i = optind ; i < argc ; i++) {
        memset(value, 0, sizeof(value));
        valuelen = getxattr(argv[i], name, value, sizeof(value)-1);
        if (valuelen > 0) {
            printf("%s: %s=%s\n", argv[i], name, value);
        }
    }

    return 0;
}
