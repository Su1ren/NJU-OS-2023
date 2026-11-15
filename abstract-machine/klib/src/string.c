#include <klib-macros.h>
#include <klib.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

// 获取字符串 s 的长度，不包括终止符 '\0'
size_t strlen(const char *s) {
  assert(s != NULL);
  size_t len = 0;
  while (*s++)
    ++len;
  return len;
}

// 将字符串 src 复制到 dst，返回 dst 的指针
char *strcpy(char *dst, const char *src) {
  assert(dst != NULL);
  assert(src != NULL);
  char *ret = dst;
  while ((*dst++ = *src++))
    ;
  return ret;
}

// 将字符串 src 的前 n 个字符复制到 dst，返回 dst 的指针
char *strncpy(char *dst, const char *src, size_t n) {
  assert(dst != NULL);
  assert(src != NULL);
  char *ret = dst;

  // 先复制最多 n 个字节，遇到 '\0' 停止。
  while (n > 0 && (*dst = *src)) {
    dst++;
    src++;
    n--;
  }
  // src 比 n 短时，剩余位置补 '\0'。
  while (n > 0) {
    *dst++ = '\0';
    n--;
  }
  return ret;
}

// 将字符串 src 追加到 dst 的末尾，返回 dst 的指针
char *strcat(char *dst, const char *src) {
  assert(dst != NULL);
  assert(src != NULL);
  char *ret = dst;
  while (*dst)
    ++dst; // 找到 dst 的末尾
  while ((*dst++ = *src++))
    ; // 追加 src 到 dst
  return ret;
}

// 比较两个字符串 s1 和 s2 的大小，返回值 <0, 0, >0 分别表示 s1<s2, s1==s2,
// s1>s2
int strcmp(const char *s1, const char *s2) {
  assert(s1 != NULL);
  assert(s2 != NULL);
  // 比较两个字符串的每个字符，直到遇到不同的字符或到达字符串末尾
  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  // 返回两个字符的差值，若 s1<s2 则返回负数，若 s1>s2 则返回正数，若相等则返回
  // 0
  return *(unsigned char *)s1 - *(unsigned char *)s2;
}

// 比较两个字符串 s1 和 s2 的前 n 个字符的大小，返回值 <0, 0, >0 分别表示 s1<s2,
// s1==s2, s1>s2
int strncmp(const char *s1, const char *s2, size_t n) {
  assert(s1 != NULL);
  assert(s2 != NULL);
  // 比较两个字符串的前 n 个字符，直到遇到不同的字符或到达字符串末尾
  while (n-- && *s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  // 如果 n 为 0，表示前 n 个字符相等，返回 0
  if (n == (size_t)-1)
    return 0;
  // 返回两个字符的差值，若 s1<s2 则返回负数，若 s1>s2 则返回正数，若相等则返回
  // 0
  return *(unsigned char *)s1 - *(unsigned char *)s2;
}

// 将字符 c（一个无符号字符）复制到字符串 s 的前 n 个字符中，返回 s 的指针
void *memset(void *s, int c, size_t n) {
  assert(s != NULL || n == 0);
  unsigned char *p = s;
  while (n--) {
    *p++ = (unsigned char)c;
  }
  return s;
}

// 将内存区域 src 的前 n 个字节移动到内存区域 dst 中，返回 dst 的指针
void *memmove(void *dst, const void *src, size_t n) {
  assert((dst != NULL && src != NULL) || n == 0);
  unsigned char *d = dst;
  const unsigned char *s = src;
  // 如果 d 地址低于 s 地址，说明 dst 和 src 没有重叠，可以直接从前往后复制
  if (d < s) {
    while (n--) {
      *d++ = *s++;
    }
  } else {
    // 否则，说明 dst 和 src
    // 有可能的重叠，需要从后往前复制，避免覆盖未复制的数据
    d += n;
    s += n;
    while (n--) {
      *--d = *--s;
    }
  }
  return dst;
}

// 将内存区域 in 的前 n 个字节复制到内存区域 out 中，返回 out 的指针
void *memcpy(void *out, const void *in, size_t n) {
  // 断言 out 和 in 不能重叠，否则可能会导致未定义行为
  assert((out != NULL && in != NULL) || n == 0);
  if (n > 0) {
    uintptr_t d = (uintptr_t)out, s = (uintptr_t)in;
    assert(d + n <= s || s + n <= d);
  }
  unsigned char *d = out;
  const unsigned char *s = in;
  while (n--) {
    *d++ = *s++;
  }
  return out;
}

// 比较内存区域 s1 和 s2 的前 n 个字节的大小，返回值 <0, 0, >0 分别表示 s1<s2,
// s1==s2, s1>s2
int memcmp(const void *s1, const void *s2, size_t n) {
  assert((s1 != NULL && s2 != NULL) || n == 0);
  const unsigned char *p1 = s1, *p2 = s2;
  while (n--) {
    if (*p1 != *p2) {
      return *p1 - *p2;
    }
    p1++;
    p2++;
  }
  return 0;
}

#endif
