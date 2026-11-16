#include "image.h"
#include <am.h>
#include <amdev.h>

#define TILE 32 // 每次绘制的 tile 大小为 32x32 像素

#define KEYNAME(key) [AM_KEY_##key] = #key,
static const char *key_names[] = {AM_KEYS(KEYNAME)};

static inline void puts(const char *s) {
  for (; *s; s++)
    putch(*s);
}

// 把源图按最近邻缩放到屏幕
static void draw_image(void) {
  AM_GPU_CONFIG_T info = {0};
  ioe_read(AM_GPU_CONFIG, &info);
  int W = info.width, H = info.height;

  static uint32_t pixels[TILE * TILE];

  for (int y = 0; y < H; y += TILE) {
    int h = (y + TILE <= H) ? TILE : H - y;
    for (int x = 0; x < W; x += TILE) {
      int w = (x + TILE <= W) ? TILE : W - x;
      // 确定绘制位置的宽度和高度，防止超出屏幕边界
      for (int j = 0; j < h; j++) {
        int sy = (y + j) * IMAGE_H / H;
        for (int i = 0; i < w; i++) {
          int sx = (x + i) * IMAGE_W / W;
          pixels[j * w + i] = image[sy][sx];
        }
      }

      AM_GPU_FBDRAW_T event = {
          .x = x,
          .y = y,
          .w = w,
          .h = h,
          .sync = 1,
          .pixels = pixels,
      };
      ioe_write(AM_GPU_FBDRAW, &event);
    }
  }
}

// Operating system is a C program!
int main(const char *args) {
  ioe_init();

  puts("mainargs = \"");
  puts(args); // make run mainargs=xxx
  puts("\"\n");

  draw_image();

  puts("Press any key to see its key code...\n");
  while (1) {
    AM_INPUT_KEYBRD_T ev = {.keycode = AM_KEY_NONE};
    ioe_read(AM_INPUT_KEYBRD, &ev);

    if (ev.keydown && ev.keycode == AM_KEY_ESCAPE) {
      halt(0);
    }
    // 打印其他按键的名称
    if (ev.keycode != AM_KEY_NONE && ev.keydown) {
      puts("Key pressed: ");
      puts(key_names[ev.keycode]);
      puts("\n");
    }
  }
  return 0;
}
