#include <hm/string.h>

#include <stdarg.h>
#include <stdint.h>

/* Context for snprintf buffer writing */
typedef struct {
  char *buf;
  size_t size;
  size_t pos;
} snprintf_ctx_t;

int memcmp(const void *a, const void *b, size_t size) {
  const uint8_t *ap = (const uint8_t *)a;
  const uint8_t *bp = (const uint8_t *)b;

  for (size_t i = 0; i < size; i++) {
    if (ap[i] != bp[i])
      return ap[i] - bp[i];
  }

  return 0;
}

void memset(void *a, uint8_t byte, size_t size) {
  uint8_t *ap = (uint8_t *)a;
  for (int i = 0; i < size; i++)
    ap[i] = byte;
}

void memcpy(void *dst, void *src, size_t size) {
  uint8_t *src_p = (uint8_t *)src;
  uint8_t *dst_p = (uint8_t *)dst;

  for (int i = 0; i < size; i++)
    dst_p[i] = src_p[i];
}

/* Helper function to write a character to the buffer */
static void buf_putc(snprintf_ctx_t *ctx, char c) {
  if (ctx->pos < ctx->size - 1) {
    ctx->buf[ctx->pos] = c;
  }
  ctx->pos++;
}

/* Helper function to write a string to the buffer */
static void buf_puts(snprintf_ctx_t *ctx, const char *s) {
  for (; *s; s++) {
    buf_putc(ctx, *s);
  }
}

/* Convert hex number to string and write to buffer */
static void buf_put_hex_num(snprintf_ctx_t *ctx, unsigned int x) {
  if (x == 0) {
    buf_putc(ctx, '0');
    return;
  }

  char buf[16];
  int buf_idx = 0;
  for (; x != 0; x /= 0x10, buf_idx++)
    buf[buf_idx] = "0123456789abcdef"[x % 0x10];

  buf_idx--;

  for (; buf_idx >= 0; buf_idx--)
    buf_putc(ctx, buf[buf_idx]);
}

/* Convert decimal number to string and write to buffer */
static void buf_put_dec_num(snprintf_ctx_t *ctx, int d) {
  if (d == 0) {
    buf_putc(ctx, '0');
    return;
  }

  if (d < 0) {
    buf_putc(ctx, '-');
    d = -d;
  }

  char buf[32];
  int buf_idx = 0;
  for (; d != 0; d /= 10, buf_idx++)
    buf[buf_idx] = "0123456789"[d % 10];

  buf_idx--;

  for (; buf_idx >= 0; buf_idx--)
    buf_putc(ctx, buf[buf_idx]);
}

static int vsnprintf(char *buf, size_t size, const char *fmt, va_list args) {
  if (!buf || size == 0) {
    return 0;
  }

  snprintf_ctx_t ctx = {.buf = buf, .size = size, .pos = 0};

  for (const char *cur = fmt; *cur; cur++) {
    if (*cur != '%') {
      buf_putc(&ctx, *cur);
      continue;
    }

    /* skip '%' */
    cur++;
    switch (*cur) {
    case 'd': {
      int d = va_arg(args, int);
      buf_put_dec_num(&ctx, d);
      break;
    }
    case 'x': {
      unsigned int x = va_arg(args, unsigned int);
      buf_put_hex_num(&ctx, x);
      break;
    }
    case 's': {
      char *s = va_arg(args, char *);
      buf_puts(&ctx, s);
      break;
    }
    case '%': {
      buf_putc(&ctx, '%');
      break;
    }
    default: {
      buf_putc(&ctx, '%');
      buf_putc(&ctx, *cur);
      break;
    }
    }
  }

  /* Null-terminate the string */
  if (ctx.pos < size) {
    buf[ctx.pos] = '\0';
  } else {
    buf[size - 1] = '\0';
  }

  return ctx.pos;
}

/* snprintf implementation */
int snprintf(char *buf, size_t size, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int ret = vsnprintf(buf, size, fmt, args);
  va_end(args);
  return ret;
}
