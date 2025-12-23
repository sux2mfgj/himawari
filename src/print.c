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

static void put_hex_num(unsigned int x, int width, int zero_pad) {
  char buf[16];
  int buf_idx = 0;

  // Convert number to hex digits
  if (x == 0) {
    buf[buf_idx++] = '0';
  } else {
    for (; x != 0; x /= 0x10, buf_idx++)
      buf[buf_idx] = "0123456789abcdef"[x % 0x10];
  }

  // Apply padding if needed
  int padding = width - buf_idx;
  if (padding > 0) {
    char pad_char = zero_pad ? '0' : ' ';
    for (int i = 0; i < padding; i++)
      putc(pad_char);
  }

  // Print digits in reverse order
  for (int i = buf_idx - 1; i >= 0; i--)
    putc(buf[i]);
}

static void put_hex_num_long(unsigned long x, int width, int zero_pad) {
  char buf[32];
  int buf_idx = 0;

  // Convert number to hex digits
  if (x == 0) {
    buf[buf_idx++] = '0';
  } else {
    for (; x != 0; x /= 0x10, buf_idx++)
      buf[buf_idx] = "0123456789abcdef"[x % 0x10];
  }

  // Apply padding if needed
  int padding = width - buf_idx;
  if (padding > 0) {
    char pad_char = zero_pad ? '0' : ' ';
    for (int i = 0; i < padding; i++)
      putc(pad_char);
  }

  // Print digits in reverse order
  for (int i = buf_idx - 1; i >= 0; i--)
    putc(buf[i]);
}

static void put_dec_num(int d, int width, int zero_pad) {
  char buf[32];
  int buf_idx = 0;

  // Convert number to decimal digits
  if (d == 0) {
    buf[buf_idx++] = '0';
  } else {
    for (; d != 0; d /= 10, buf_idx++)
      buf[buf_idx] = "0123456789"[d % 10];
  }

  // Apply padding if needed
  int padding = width - buf_idx;
  if (padding > 0) {
    char pad_char = zero_pad ? '0' : ' ';
    for (int i = 0; i < padding; i++)
      putc(pad_char);
  }

  // Print digits in reverse order
  for (int i = buf_idx - 1; i >= 0; i--)
    putc(buf[i]);
}

static void put_dec_num_long(long d, int width, int zero_pad) {
  char buf[32];
  int buf_idx = 0;

  // Convert number to decimal digits
  if (d == 0) {
    buf[buf_idx++] = '0';
  } else {
    for (; d != 0; d /= 10, buf_idx++)
      buf[buf_idx] = "0123456789"[d % 10];
  }

  // Apply padding if needed
  int padding = width - buf_idx;
  if (padding > 0) {
    char pad_char = zero_pad ? '0' : ' ';
    for (int i = 0; i < padding; i++)
      putc(pad_char);
  }

  // Print digits in reverse order
  for (int i = buf_idx - 1; i >= 0; i--)
    putc(buf[i]);
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

    // Parse flags and width
    int zero_pad = 0;
    int width = 0;
    int is_long = 0;

    // Check for '0' flag (zero padding)
    if (*cur == '0') {
      zero_pad = 1;
      cur++;
    }

    // Parse width
    while (*cur >= '0' && *cur <= '9') {
      width = width * 10 + (*cur - '0');
      cur++;
    }

    // Check for 'l' length modifier
    if (*cur == 'l') {
      is_long = 1;
      cur++;
    }

    // Parse format specifier
    switch (*cur) {
    case 'd': {
      if (is_long) {
        long d = va_arg(args, long);
        put_dec_num_long(d, width, zero_pad);
      } else {
        int d = va_arg(args, int);
        put_dec_num(d, width, zero_pad);
      }
      break;
    }
    case 'x': {
      if (is_long) {
        unsigned long x = va_arg(args, unsigned long);
        put_hex_num_long(x, width, zero_pad);
      } else {
        unsigned int x = va_arg(args, unsigned int);
        put_hex_num(x, width, zero_pad);
      }
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
