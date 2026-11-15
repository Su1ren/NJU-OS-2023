#include <am.h>
#include <klib-macros.h>

bool __am_has_ioe = false; // 标识是否支持 IOE（Input/Output Emulator）
static bool ioe_init_done = false; // 标识 IOE 是否已经初始化完成

// 以下为 AM 设备的初始化函数声明，这些函数在不同的设备驱动中实现
void __am_timer_init();
void __am_gpu_init();
void __am_input_init();
void __am_audio_init();
void __am_disk_init();
void __am_input_config(AM_INPUT_CONFIG_T *);
void __am_timer_config(AM_TIMER_CONFIG_T *);
void __am_timer_rtc(AM_TIMER_RTC_T *);
void __am_timer_uptime(AM_TIMER_UPTIME_T *);
void __am_input_keybrd(AM_INPUT_KEYBRD_T *);
void __am_gpu_config(AM_GPU_CONFIG_T *);
void __am_gpu_status(AM_GPU_STATUS_T *);
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *);
void __am_audio_config(AM_AUDIO_CONFIG_T *);
void __am_audio_ctrl(AM_AUDIO_CTRL_T *);
void __am_audio_status(AM_AUDIO_STATUS_T *);
void __am_audio_play(AM_AUDIO_PLAY_T *);
void __am_disk_config(AM_DISK_CONFIG_T *cfg);
void __am_disk_status(AM_DISK_STATUS_T *stat);
void __am_disk_blkio(AM_DISK_BLKIO_T *io);
static void __am_uart_config(AM_UART_CONFIG_T *cfg) { cfg->present = false; }
static void __am_net_config(AM_NET_CONFIG_T *cfg) { cfg->present = false; }

typedef void (*handler_t)(void *buf);
// 定义一个查找表（lookup table），用于将设备寄存器号映射到对应的处理函数
static void *lut[128] = {
    [AM_TIMER_CONFIG] = __am_timer_config,
    [AM_TIMER_RTC] = __am_timer_rtc,
    [AM_TIMER_UPTIME] = __am_timer_uptime,
    [AM_INPUT_CONFIG] = __am_input_config,
    [AM_INPUT_KEYBRD] = __am_input_keybrd,
    [AM_GPU_CONFIG] = __am_gpu_config,
    [AM_GPU_FBDRAW] = __am_gpu_fbdraw,
    [AM_GPU_STATUS] = __am_gpu_status,
    [AM_UART_CONFIG] = __am_uart_config,
    [AM_AUDIO_CONFIG] = __am_audio_config,
    [AM_AUDIO_CTRL] = __am_audio_ctrl,
    [AM_AUDIO_STATUS] = __am_audio_status,
    [AM_AUDIO_PLAY] = __am_audio_play,
    [AM_DISK_CONFIG] = __am_disk_config,
    [AM_DISK_STATUS] = __am_disk_status,
    [AM_DISK_BLKIO] = __am_disk_blkio,
    [AM_NET_CONFIG] = __am_net_config,
};

// 返回 true 表示 IOE 初始化成功，false 表示失败
// 该函数必须在 ioe_read/ioe_write 之前调用，否则会触发 panic
bool ioe_init() {
  panic_on(cpu_current() != 0, "call ioe_init() in other CPUs");
  panic_on(ioe_init_done, "double-initialization");
  __am_has_ioe = true;
  return true;
}

static void fail(void *buf) { panic("access nonexist register"); }

// 执行 AM 设备的初始化操作，确保所有设备都已准备好使用
void __am_ioe_init() {
  for (int i = 0; i < LENGTH(lut); i++)
    if (!lut[i])
      lut[i] = fail;
  __am_timer_init();
  __am_gpu_init();
  __am_input_init();
  __am_audio_init();
  __am_disk_init();
  ioe_init_done = true;
}

static void do_io(int reg, void *buf) {
  if (!ioe_init_done) {
    __am_ioe_init();
  }
  ((handler_t)lut[reg])(buf);
}

// ioe 的读写都通过 do_io 这个函数来实现
// do_io 会根据寄存器号 reg 查找对应的处理函数，并将 buf 传递给它
void ioe_read(int reg, void *buf) { do_io(reg, buf); }
void ioe_write(int reg, void *buf) { do_io(reg, buf); }
