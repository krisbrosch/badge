#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stdint.h>

struct ringbuffer {
  uint8_t *buf;
  uint32_t start;
  uint32_t end;
  uint32_t mask;
  uint32_t size;
};

// Initialize a ring buffer; size must be a power of 2
void rb_init(struct ringbuffer *rb, uint8_t *buf, uint32_t size);
int rb_full(struct ringbuffer *rb);
int rb_empty(struct ringbuffer *rb);
void rb_insert(struct ringbuffer *rb, uint8_t val);
uint8_t rb_remove(struct ringbuffer *rb);
int32_t rb_peek(struct ringbuffer *rb);

#endif
