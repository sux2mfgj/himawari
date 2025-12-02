#include <stdarg.h>

static void (*putc)(char c);

void register_putc(void (*func)(char)) { putc = func; }

void puts(const char *text) {
  for (; *text; text++)
    putc(*text);
}

void putsn(const char *text, unsigned long len) {
  for (unsigned long i = 0; i < len; i++)
    putc(text[i]);
}

static void put_hex_num(unsigned int x) {

  if (x == 0) {
    putc('0');
    return;
  }

  char buf[16];

  int buf_idx = 0;
  for (; x != 0; x /= 0x10, buf_idx++)
    buf[buf_idx] = "0123456789abcdef"[x % 0x10];

  buf_idx--;

  for (; buf_idx >= 0; buf_idx--)
    putc(buf[buf_idx]);
}

static void put_dec_num(int d) {
  if (d == 0) {
    putc('0');
    return;
  }

  char buf[32];
  int buf_idx = 0;
  for (; d != 0; d /= 10, buf_idx++)
    buf[buf_idx] = "0123456789"[d % 10];

  buf_idx--;

  for (; buf_idx >= 0; buf_idx--)
    putc(buf[buf_idx]);
}

void kprintf(const char *fmt, ...) {

  va_list args;
  va_start(args, fmt);

  for (const char *cur = fmt; *cur; cur++) {
    if (*cur != '%') {
      putc(*cur);
      continue;
    }

    // skip '%'
    cur++;
    switch (*cur) {
    case 'd': {
      int d = va_arg(args, int);
      put_dec_num(d);
      break;
    }
    case 'x': {
      unsigned int x = va_arg(args, unsigned int);
      put_hex_num(x);
      break;
    }
    case 's': {
      char *s = va_arg(args, char *);
      puts(s);
      break;
    }
    default: {
      puts("\nUnsupported format is detectd: ");
      putc(*cur);
      putc('\n');
      break;
    }
    }
  }

  va_end(args);
}
