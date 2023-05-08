#ifndef TRIDENT_H
#define TRIDENT_H
#include <stdint.h>
#include <sys/ioctl.h>

enum imgdata_type { RAW, RLE, RLE1 };

typedef struct drawfren_s {
  int fb_fd;
  struct fb_fix_screeninfo finfo;
  struct fb_var_screeninfo vinfo;
  long screensize;
  uint8_t *fbp;
} drawfren_t;

typedef struct pixmap_s {
  int32_t width;
  int32_t height;
  size_t n_pixels;
  char datatype;
  uint32_t *data;
} pixmap_t;

#define GET_VINFO(fren) {                                  \
  ioctl(fren->fb_fd, FBIOGET_VSCREENINFO, &fren->vinfo);   \
}

#define GET_FINFO(fren) {                                  \
  ioctl(fren->fb_fd, FBIOGET_FSCREENINFO, &fren->finfo);   \
}

#define GET_INFOS(fren)  {  \
  GET_VINFO(fren);          \
  GET_FINFO(fren);          \
}
#endif /* TRIDENT_H */
