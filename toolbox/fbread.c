/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/fb.h>
#include <cutils/log.h>

#define r(fd, ptr, size) (read((fd), (ptr), (size)) != (int)(size))
#define w(fd, ptr, size) (write((fd), (ptr), (size)) != (int)(size))

/* This version number defines the format of the fbinfo struct.
   It must match versioning in ddms where this data is consumed. */
#define DDMS_RAWIMAGE_VERSION 1
struct fbinfo {
    unsigned int version;
    unsigned int bpp;
    unsigned int size;
    unsigned int width;
    unsigned int height;
    unsigned int red_offset;
    unsigned int red_length;
    unsigned int blue_offset;
    unsigned int blue_length;
    unsigned int green_offset;
    unsigned int green_length;
    unsigned int alpha_offset;
    unsigned int alpha_length;
} __attribute__((packed));

static int readfb_devfb (int fd)
{
    struct fb_var_screeninfo vinfo;
    int fb, offset;
    char x[256];
    int rv = -ENOTSUP;

    struct fbinfo fbinfo;
    unsigned i, bytespp;

    fb = open("/dev/graphics/fb0", O_RDONLY);
    if (fb < 0) goto done;

    if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) < 0) goto done;
    fcntl(fb, F_SETFD, FD_CLOEXEC);

    bytespp = vinfo.bits_per_pixel / 8;

    fbinfo.version = DDMS_RAWIMAGE_VERSION;
    fbinfo.bpp = vinfo.bits_per_pixel;
    fbinfo.size = vinfo.xres * vinfo.yres * bytespp;
    fbinfo.width = vinfo.xres;
    fbinfo.height = vinfo.yres;
    fbinfo.red_offset = vinfo.red.offset;
    fbinfo.red_length = vinfo.red.length;
    fbinfo.green_offset = vinfo.green.offset;
    fbinfo.green_length = vinfo.green.length;
    fbinfo.blue_offset = vinfo.blue.offset;
    fbinfo.blue_length = vinfo.blue.length;
    fbinfo.alpha_offset = vinfo.transp.offset;
    fbinfo.alpha_length = vinfo.transp.length;

    /* HACK: for several of our 3d cores a specific alignment
     * is required so the start of the fb may not be an integer number of lines
     * from the base.  As a result we are storing the additional offset in
     * xoffset. This is not the correct usage for xoffset, it should be added
     * to each line, not just once at the beginning */
    offset = vinfo.xoffset * bytespp;

    offset += vinfo.xres * vinfo.yoffset * bytespp;

    rv = 0;

    if (w(fd, &fbinfo, sizeof(fbinfo))) goto done;

    lseek(fb, offset, SEEK_SET);
    for (i = 0; i < fbinfo.size; i += 256) {
      if (r(fb, &x, 256)) goto done;
      if (w(fd, &x, 256)) goto done;
    }

    if (r(fb, &x, fbinfo.size % 256)) goto done;
    if (w(fd, &x, fbinfo.size % 256)) goto done;

done:
    if (fb >= 0) close(fb);
    return rv;
}

#ifdef GRALLOC_FB_READ_SUPPORTED

static int fill_format(struct fbinfo* info, int format)
{
    // bpp, red, green, blue, alpha

    static const int format_map[][9] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0},   // INVALID
        {32, 0, 8, 8, 8, 16, 8, 24, 8}, // HAL_PIXEL_FORMAT_RGBA_8888
        {32, 0, 8, 8, 8, 16, 8, 0, 0}, // HAL_PIXEL_FORMAT_RGBX_8888
        {24, 16, 8, 8, 8, 0, 8, 0, 0},  // HAL_PIXEL_FORMAT_RGB_888
        {16, 11, 5, 5, 6, 0, 5, 0, 0},  // HAL_PIXEL_FORMAT_RGB_565
        {32, 16, 8, 8, 8, 0, 8, 24, 8}, // HAL_PIXEL_FORMAT_BGRA_8888
        {16, 11, 5, 6, 5, 1, 5, 0, 1},  // HAL_PIXEL_FORMAT_RGBA_5551
        {16, 12, 4, 8, 4, 4, 4, 0, 4}   // HAL_PIXEL_FORMAT_RGBA_4444
    };
    const int *p;

    if (format == 0 || format > HAL_PIXEL_FORMAT_RGBA_4444)
        return -ENOTSUP;

    p = format_map[format];

    info->bpp = *(p++);
    info->red_offset = *(p++);
    info->red_length = *(p++);
    info->green_offset = *(p++);
    info->green_length = *(p++);
    info->blue_offset = *(p++);
    info->blue_length = *(p++);
    info->alpha_offset = *(p++);
    info->alpha_length = *(p++);

    return 0;
}

static int readfb_gralloc (int fd)
{
    struct fbinfo fbinfo;
    struct gralloc_module_t *gralloc;
    struct framebuffer_device_t *fbdev = 0;
    struct alloc_device_t *allocdev = 0;
    buffer_handle_t buf = 0;
    unsigned char* data = 0;
    int stride;
    int rv;
    int linebytes;
    unsigned i;

    rv = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, (const hw_module_t**)&gralloc);
    if (rv != 0)
        goto done;
    rv = framebuffer_open(&gralloc->common, &fbdev);
    if (rv != 0) goto done;
    if (!fbdev->read) {
        rv = -ENOTSUP;
        goto done;
    }
    rv = gralloc_open(&gralloc->common, &allocdev);
    if (rv != 0) goto done;

    rv = allocdev->alloc(allocdev, fbdev->width, fbdev->height,
                         fbdev->format, GRALLOC_USAGE_SW_READ_OFTEN,
                         &buf, &stride);
    if (rv != 0) goto done;

    rv = fbdev->read(fbdev, buf);
    if (rv != 0) goto done;

    rv = gralloc->lock(gralloc, buf, GRALLOC_USAGE_SW_READ_OFTEN, 0, 0,
                       fbdev->width, fbdev->height, (void**)&data);
    if (rv != 0) goto done;

    fbinfo.version = DDMS_RAWIMAGE_VERSION;

    rv = fill_format(&fbinfo, fbdev->format);
    if (rv != 0) goto done;

    stride *= (fbinfo.bpp >> 3);
    linebytes = fbdev->width * (fbinfo.bpp >> 3);
    fbinfo.size = linebytes * fbdev->height;
    fbinfo.width = fbdev->width;
    fbinfo.height = fbdev->height;

    // point of no return: don't attempt alternative means of reading
    // after this
    rv = 0;

    // write fbinfo contents
    if (w(fd, &fbinfo, sizeof(fbinfo))) goto done;

    // write data
    for (i = 0; i < fbinfo.height; i++) {
        if (w(fd, data, linebytes)) goto done;
        data += stride;
    }

 done:
    if (data)
        gralloc->unlock(gralloc, buf);
    if (buf)
        allocdev->free(allocdev, buf);
    if (allocdev)
        gralloc_close(allocdev);
    if (fbdev)
        framebuffer_close(fbdev);

    return rv;
}
#endif

int main()
{
    int out = fileno(stdout);
    int rv = -1;

#ifdef GRALLOC_FB_READ_SUPPORTED
    rv = readfb_gralloc(out);
#endif

    if (rv < 0)
        rv = readfb_devfb(out);

    close(out);

    return rv;
}
