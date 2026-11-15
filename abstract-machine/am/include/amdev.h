#ifndef __AMDEV_H__
#define __AMDEV_H__

// **MAY SUBJECT TO CHANGE IN THE FUTURE**

// AM_DEVREG 表示一个设备寄存器的定义，
// 包含寄存器的 ID、名称、权限和结构体定义
#define AM_DEVREG(id, reg, perm, ...)                                          \
  enum { AM_##reg = (id) };                                                    \
  typedef struct {                                                             \
    __VA_ARGS__;                                                               \
  } AM_##reg##_T; // 宏定义展开，得到不同寄存器数据类型定义，变参携带传入的结构体成员

// 以下 1-24 是所有设备寄存器的定义，
// 其中 reg 是寄存器名称，perm 是权限（RD/WR），
// __VA_ARGS__ 是寄存器携带的数据结构定义
AM_DEVREG(1, UART_CONFIG, RD, bool present);
AM_DEVREG(2, UART_TX, WR, char data);
AM_DEVREG(3, UART_RX, RD, char data);
AM_DEVREG(4, TIMER_CONFIG, RD, bool present, has_rtc);
AM_DEVREG(5, TIMER_RTC, RD, int year, month, day, hour, minute, second);
AM_DEVREG(6, TIMER_UPTIME, RD, uint64_t us);
AM_DEVREG(7, INPUT_CONFIG, RD, bool present);
AM_DEVREG(8, INPUT_KEYBRD, RD, bool keydown; int keycode);
AM_DEVREG(9, GPU_CONFIG, RD, bool present, has_accel;
          int width, height, vmemsz);
AM_DEVREG(10, GPU_STATUS, RD, bool ready);
AM_DEVREG(11, GPU_FBDRAW, WR, int x, y; void *pixels; int w, h; bool sync);
AM_DEVREG(12, GPU_MEMCPY, WR, uint32_t dest; void *src; int size);
AM_DEVREG(13, GPU_RENDER, WR, uint32_t root);
AM_DEVREG(14, AUDIO_CONFIG, RD, bool present; int bufsize);
AM_DEVREG(15, AUDIO_CTRL, WR, int freq, channels, samples);
AM_DEVREG(16, AUDIO_STATUS, RD, int count);
AM_DEVREG(17, AUDIO_PLAY, WR, Area buf);
AM_DEVREG(18, DISK_CONFIG, RD, bool present; int blksz, blkcnt);
AM_DEVREG(19, DISK_STATUS, RD, bool ready);
AM_DEVREG(20, DISK_BLKIO, WR, bool write; void *buf; int blkno, blkcnt);
AM_DEVREG(21, NET_CONFIG, RD, bool present);
AM_DEVREG(22, NET_STATUS, RD, int rx_len, tx_len);
AM_DEVREG(23, NET_TX, WR, Area buf);
AM_DEVREG(24, NET_RX, WR, Area buf);

// Input
// 定义了键盘按键的枚举类型 AM_KEY_XXX
#define AM_KEYS(_)                                                             \
  _(ESCAPE)                                                                    \
  _(F1)                                                                        \
  _(F2)                                                                        \
  _(F3)                                                                        \
  _(F4) _(F5) _(F6) _(F7) _(F8) _(F9) _(F10) _(F11) _(F12) _(GRAVE) _(1) _(2)  \
      _(3) _(4) _(5) _(6) _(7) _(8) _(9) _(0) _(MINUS) _(EQUALS) _(BACKSPACE)  \
          _(TAB) _(Q) _(W) _(E) _(R) _(T) _(Y) _(U) _(I) _(O) _(P)             \
              _(LEFTBRACKET) _(RIGHTBRACKET) _(BACKSLASH) _(CAPSLOCK) _(A)     \
                  _(S) _(D) _(F) _(G) _(H) _(J) _(K) _(L) _(SEMICOLON)         \
                      _(APOSTROPHE) _(RETURN) _(LSHIFT) _(Z) _(X) _(C) _(V)    \
                          _(B) _(N) _(M) _(COMMA) _(PERIOD) _(SLASH) _(RSHIFT) \
                              _(LCTRL) _(APPLICATION) _(LALT) _(SPACE) _(RALT) \
                                  _(RCTRL) _(UP) _(DOWN) _(LEFT) _(RIGHT)      \
                                      _(INSERT) _(DELETE) _(HOME) _(END)       \
                                          _(PAGEUP) _(PAGEDOWN)
// 从 AM_KEY_ESCAPE 到 AM_KEY_PAGEDOWN 的键盘按键枚举值
#define AM_KEY_NAMES(key) AM_KEY_##key,
enum { AM_KEY_NONE = 0, AM_KEYS(AM_KEY_NAMES) };

// GPU
// 定义了 GPU 相关的寄存器和数据结构
#define AM_GPU_TEXTURE 1
#define AM_GPU_SUBTREE 2
#define AM_GPU_NULL 0xffffffff

// 定义了 GPU 画布的类型和属性
typedef uint32_t gpuptr_t;

// GPU 纹理描述符和画布结构体
struct gpu_texturedesc {
  uint16_t w, h;
  gpuptr_t pixels;
} __attribute__((packed));

// GPU 画布结构体，表示一个 GPU
// 画布的属性，包括类型、尺寸、位置、子画布和纹理描述符
struct gpu_canvas {
  uint16_t type, w, h, x1, y1, w1, h1;
  gpuptr_t sibling; // 指向同级画布的指针
  union {
    gpuptr_t child;
    struct gpu_texturedesc texture;
  }; // 如果是子画布，则 child 指向子画布；如果是纹理，则 texture 描述纹理属性
} __attribute__((packed));

#endif
