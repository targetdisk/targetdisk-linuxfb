/*
 *  2023 Andrew "targetdisk" Rogers
 *
 *  Compile with:
 *    gcc -Wall -g -O0 -o trident trident.c
 */

#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "img_data.h"
#include "trident.h"

void setup_fren(drawfren_t *fren) {
  fren->fb_fd = open("/dev/fb0", O_RDWR);

  GET_VINFO(fren);
  fren->vinfo.grayscale = 0;
  fren->vinfo.bits_per_pixel = 32;
  GET_VINFO(fren);
  GET_INFOS(fren);

  fren->screensize = fren->vinfo.yres_virtual * fren->finfo.line_length;

  fren->fbp = mmap(0, fren->screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
                   fren->fb_fd, (off_t)0);
}

static inline long pixel_location(long x, long y, drawfren_t *df) {
  return ( (x + df->vinfo.xoffset) * (df->vinfo.bits_per_pixel / 8)
           + (y + df->vinfo.yoffset) * df->finfo.line_length );
}

static inline uint32_t pixel_color(uint8_t r, uint8_t g, uint8_t b, drawfren_t* df) {
  return (r << df->vinfo.red.offset) | (g << df->vinfo.green.offset) | (b << df->vinfo.blue.offset);
}

void fill_screen(uint32_t color, drawfren_t *df) {
  long x, y, location;
  for (y = 0; y < df->vinfo.yres; y++) {
    for (x = 0; x < df->vinfo.xres; x++) {
      location = pixel_location(x, y, df);
      *((uint32_t*)(df->fbp + location)) = color;
    }
  }
}

static inline int bounded_rand(int lower, int upper) {
  return (rand() % (upper - lower + 1)) + lower;
}

void draw_trident(drawfren_t *df, pixmap_t *pixmap) {
  long start_x = bounded_rand(0, df->vinfo.xres - pixmap->width);
  long x = start_x;
  long y = bounded_rand(0, df->vinfo.yres - pixmap->height);
  {
    uint32_t *data_ptr = pixmap->data;
    for (size_t pi = 0; pi < pixmap->n_pixels; pi++) {
      if (pi % pixmap->width == 0) {
        x = start_x;
        y++;
      } else {
        x++;
      }
      if (*data_ptr & 0x000000ff) {
        *((uint32_t*)(df->fbp + pixel_location(x, y, df)))
          = pixel_color((*data_ptr >> 24),
            ((*data_ptr & 0x00ff0000) >> 16),
            ((*data_ptr & 0x0000ff00) >> 8),
            df);
      }
      data_ptr++;
    }
  }
}

#ifndef RLE1_ONLY
void rle_decompress(pixmap_t *pixmap) {
  uint32_t *rledata = pixmap->data;
  uint32_t *rawdata = malloc(sizeof(uint32_t) * pixmap->n_pixels);
  if (rawdata) {
    pixmap->data = rawdata;
    uint32_t *pixel;
    for (size_t pi = 0; pi < pixmap->n_pixels; rledata++) {
      pixel = rledata++;
      for (uint32_t pi_end = pi + *rledata; pi < pi_end; pi++) {
        *rawdata++ = *pixel;
      }
    }
  } else {
    fprintf(stderr, "ERROR: malloc failed on pixmap data!!\n");
    exit(ENOMEM);
  }
}
#endif /* RLE1_ONLY */

static inline uint32_t *rle1_put_pixels(uint32_t pixel, size_t reps, uint32_t *rawdata) {
  for (size_t n = 0; n < reps; n++)
    *rawdata++ = pixel;
  return rawdata;
}

void rle1_decompress(pixmap_t *pixmap) {
  uint32_t *rledata = pixmap->data;
  uint32_t *fgcolor = rledata++;
  uint32_t *rawdata = malloc(sizeof(uint32_t) * pixmap->n_pixels);
  if (rawdata) {
    pixmap->data = rawdata;
    unsigned char byte;
    size_t reps;
    unsigned char packlocat = 24;
    for (size_t pi = 0; pi < pixmap->n_pixels;) {
      byte = (unsigned char)(*rledata >> packlocat);
      reps = (size_t)(byte & 127);
      pi += reps;
      rawdata = rle1_put_pixels((byte >> 7 ? *fgcolor : 0), reps, rawdata);
      if (!packlocat) {
        packlocat = 24;
        rledata++;
      } else {
        packlocat -= 8;
      }
    }
  } else {
    fprintf(stderr, "ERROR: malloc failed on pixmap data!!\n");
    exit(ENOMEM);
  }
}

void decompress(pixmap_t *pixmap) {
  switch (pixmap->datatype) {
    case RAW:
    case RAW_BL:
      return;
#ifndef RLE1_ONLY
    case RLE:
    case RLE_BL:
      rle_decompress(pixmap);
      return;
#endif /* RLE1_ONLY */
    case RLE1:
    case RLE1_BL:
      rle1_decompress(pixmap);
      return;
    default:
      fprintf(stderr, "ERROR: image data type not implemented!!\n");
      exit(EPROTONOSUPPORT);
  }
}

void print_frenfo(drawfren_t *df) {
  printf("%ux%u @ %ubpp (line length %u)\n",
      df->vinfo.xres, df->vinfo.yres, df->vinfo.bits_per_pixel, df->finfo.line_length);
}

int main(void) {
  drawfren_t fren;
  setup_fren(&fren);
  print_frenfo(&fren);
  decompress(&trident);

  for (int i = 0; i < 2048; i++) {
    fill_screen(pixel_color(BG_R, BG_G, BG_B, &fren), &fren);
    draw_trident(&fren, &trident);
    sleep(5);
  }

  return 0;
}
