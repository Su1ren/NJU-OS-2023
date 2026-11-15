#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

// 有界输出：buf/cap 用于 vsnprintf，put 用于 printf 流式输出。
struct out {
  char *buf;
  size_t cap;
  size_t len;        // 本应写入的总长度（用于 snprintf 返回值）
  void (*put)(char); // 非 NULL 时直接流式输出
};

static void out_init_buf(struct out *o, char *buf, size_t cap) {
  o->buf = buf;
  o->cap = cap;
  o->len = 0;
  o->put = NULL;
}

static void out_init_put(struct out *o, void (*put)(char)) {
  o->buf = NULL;
  o->cap = 0;
  o->len = 0;
  o->put = put;
}

static void outc(struct out *o, char ch) {
  if (o->put) {
    o->put(ch);
  } else if (o->buf && o->len + 1 < o->cap) {
    o->buf[o->len] = ch; // 预留最后一个字节给 '\0'
  }
  o->len++;
}

static void out_finish(struct out *o) {
  if (o->buf && o->cap > 0) {
    size_t end = o->len < o->cap ? o->len : o->cap - 1;
    o->buf[end] = '\0';
  }
}

static void emit_repeat(struct out *o, char ch, int n) {
  while (n-- > 0)
    outc(o, ch);
}

// 输出 value 的 base 进制表示；prefix 可以是 ""、"-"、"0x"。
static void emit_unsigned(struct out *o, unsigned long long value, int base,
                          bool upper, const char *prefix, int width, bool zero,
                          bool left) {
  char tmp[32];
  const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
  int n = 0;

  if (value == 0)
    tmp[n++] = '0';
  while (value > 0) {
    tmp[n++] = digits[value % (unsigned)base];
    value /= (unsigned)base;
  }

  int plen = 0;
  while (prefix[plen])
    plen++;
  int total = plen + n;
  int pad = width > total ? width - total : 0;

  if (left) {
    for (int i = 0; i < plen; i++)
      outc(o, prefix[i]);
    while (n > 0)
      outc(o, tmp[--n]);
    emit_repeat(o, ' ', pad);
  } else if (zero) {
    for (int i = 0; i < plen; i++)
      outc(o, prefix[i]);
    emit_repeat(o, '0', pad);
    while (n > 0)
      outc(o, tmp[--n]);
  } else {
    emit_repeat(o, ' ', pad);
    for (int i = 0; i < plen; i++)
      outc(o, prefix[i]);
    while (n > 0)
      outc(o, tmp[--n]);
  }
}

static void format(struct out *o, const char *fmt, va_list ap) {
  const char *p = fmt;

  while (*p) {
    if (*p != '%') {
      outc(o, *p++);
      continue;
    }

    p++;
    if (*p == '\0') {
      outc(o, '%');
      break;
    }
    if (*p == '%') {
      outc(o, '%');
      p++;
      continue;
    }

    bool left = false;
    bool zero = false;
    while (*p == '-' || *p == '0') {
      if (*p == '-')
        left = true;
      else
        zero = true;
      p++;
    }

    int width = 0;
    while (*p >= '0' && *p <= '9') {
      width = width * 10 + (*p - '0');
      p++;
    }

    int lmod = 0;
    while (*p == 'l') {
      lmod++;
      p++;
    }

    if (*p == '\0') {
      outc(o, '%');
      break;
    }

    switch (*p) {
    case 'd':
    case 'i': {
      long long v;
      if (lmod >= 2)
        v = va_arg(ap, long long);
      else if (lmod == 1)
        v = va_arg(ap, long);
      else
        v = va_arg(ap, int);

      bool neg = v < 0;
      unsigned long long mag;
      if (neg)
        mag = (unsigned long long)(-(v + 1)) + 1;
      else
        mag = (unsigned long long)v;
      emit_unsigned(o, mag, 10, false, neg ? "-" : "", width, zero, left);
      break;
    }
    case 'u': {
      unsigned long long v;
      if (lmod >= 2)
        v = va_arg(ap, unsigned long long);
      else if (lmod == 1)
        v = va_arg(ap, unsigned long);
      else
        v = va_arg(ap, unsigned int);
      emit_unsigned(o, v, 10, false, "", width, zero, left);
      break;
    }
    case 'x':
    case 'X': {
      unsigned long long v;
      if (lmod >= 2)
        v = va_arg(ap, unsigned long long);
      else if (lmod == 1)
        v = va_arg(ap, unsigned long);
      else
        v = va_arg(ap, unsigned int);
      emit_unsigned(o, v, 16, *p == 'X', "", width, zero, left);
      break;
    }
    case 'p': {
      uintptr_t v = (uintptr_t)va_arg(ap, void *);
      emit_unsigned(o, (unsigned long long)v, 16, false, "0x", width, zero,
                    left);
      break;
    }
    case 'c': {
      char ch = (char)va_arg(ap, int);
      int pad = width > 1 ? width - 1 : 0;
      if (!left)
        emit_repeat(o, ' ', pad);
      outc(o, ch);
      if (left)
        emit_repeat(o, ' ', pad);
      break;
    }
    case 's': {
      const char *s = va_arg(ap, const char *);
      if (s == NULL)
        s = "(null)";
      int len = 0;
      while (s[len])
        len++;
      int pad = width > len ? width - len : 0;
      if (!left)
        emit_repeat(o, ' ', pad);
      for (int i = 0; i < len; i++)
        outc(o, s[i]);
      if (left)
        emit_repeat(o, ' ', pad);
      break;
    }
    default:
      outc(o, '%');
      outc(o, *p);
      break;
    }

    p++;
  }
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  assert(out != NULL);
  assert(fmt != NULL);
  struct out o;
  out_init_buf(&o, out, (size_t)-1);
  format(&o, fmt, ap);
  out_finish(&o);
  return (int)o.len;
}

int sprintf(char *out, const char *fmt, ...) {
  assert(out != NULL);
  assert(fmt != NULL);
  va_list ap;
  va_start(ap, fmt);
  int cnt = vsprintf(out, fmt, ap);
  va_end(ap);
  return cnt;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  assert(fmt != NULL);
  assert(out != NULL || n == 0);
  struct out o;
  out_init_buf(&o, out, n);
  format(&o, fmt, ap);
  out_finish(&o);
  return (int)o.len;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  assert(fmt != NULL);
  assert(out != NULL || n == 0);
  va_list ap;
  va_start(ap, fmt);
  int cnt = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return cnt;
}

static void putch_sink(char ch) { putch(ch); }

int printf(const char *fmt, ...) {
  assert(fmt != NULL);
  struct out o;
  out_init_put(&o, putch_sink);
  va_list ap;
  va_start(ap, fmt);
  format(&o, fmt, ap);
  va_end(ap);
  return (int)o.len;
}

#endif
