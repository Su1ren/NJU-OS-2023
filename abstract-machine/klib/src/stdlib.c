#include <am.h>
#include <klib-macros.h>
#include <klib.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;

static char *arena_cur; // 当前分配的内存指针

int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next / 65536) % 32768;
}

void srand(unsigned int seed) { next = seed; }

int abs(int x) { return (x < 0 ? -x : x); }

int atoi(const char *nptr) {
  assert(nptr != NULL);

  while (*nptr == ' ' || *nptr == '\t' || *nptr == '\n' || *nptr == '\v' ||
         *nptr == '\f' || *nptr == '\r')
    nptr++;

  // 先判断符号位，如果有 '+' 或 '-'，则记录符号并移动指针
  int sign = 1;
  if (*nptr == '+' || *nptr == '-') {
    if (*nptr == '-')
      sign = -1;
    nptr++;
  }

  int x = 0;
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + (*nptr - '0');
    nptr++;
  }
  return sign * x;
}

void *malloc(size_t size) {
  // 只处理 size > 0 的情况，size == 0 时返回 NULL
  if (size == 0)
    return NULL;
    // On native, malloc() will be called during initializaion of C runtime.
    // Therefore do not call panic() here, else it will yield a dead recursion:
    //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  if (heap.start == NULL || heap.end == NULL) {
    panic("heap is not initialized");
  }
  assert(heap.start != NULL);
  assert(heap.end != NULL);
  assert((uintptr_t)heap.start <= (uintptr_t)heap.end);
  if (arena_cur == NULL) {
    // 初始化 arena_cur 为 heap.start
    // 对齐到 16 字节边界
    arena_cur = (char *)ROUNDUP((uintptr_t)heap.start, 16);
  }
  assert(((uintptr_t)arena_cur & 15) == 0);
  assert((uintptr_t)arena_cur <= (uintptr_t)heap.end);
  // 检查是否有足够的空间分配
  if (size > (size_t)-1 - 15)
    return NULL;                    // 防止溢出
  size = (size + 15) & ~(size_t)15; // 向上对齐到 16 字节
  assert((size & 15) == 0);
  if (size > (size_t)((char *)heap.end - arena_cur)) {
    return NULL; // 没有足够的空间
  }
  assert((uintptr_t)arena_cur + size <= (uintptr_t)heap.end);
  void *p = arena_cur;
  arena_cur += size; // 移动 arena_cur 指针
  return p;
#endif
  return NULL;
}

void free(void *ptr) {
  // 移动内存分配的指针，释放内存
  // 这里不做任何操作，因为我们没有实现内存回收机制
  (void)ptr; // 避免编译器警告
}

#endif
