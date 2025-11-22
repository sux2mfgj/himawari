#include <stdarg.h>

static void (*putc)(char c);

void register_putc(void (*func)(char)) { putc = func; }

static void puts(const char *text) {
  for (; *text; text++)
    putc(*text);
}

static void put_hex_num(unsigned int x) {

  char buf[sizeof(unsigned int)];

  int buf_idx = 0;
  for (; x != 0; x /= 0x10, buf_idx++) {
    buf[buf_idx] = "0123456789abcdef"[x % 0x10];
  }

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
    // case 'd': {
    //   int d = va_arg(args, int);
    //   put_dec_num(d);
    //   break;
    // }
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
