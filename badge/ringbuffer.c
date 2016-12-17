#include "ringbuffer.h"

void rb_init(struct ringbuffer *rb, uint8_t *buf, uint32_t size) {
  rb->start = rb->end = 0;
  rb->buf = buf;
  rb->size = size;
  rb->mask = size - 1;
}

int rb_full(struct ringbuffer *rb) {
  return (((rb->end + 1) & rb->mask) == rb->start);
}

int rb_empty(struct ringbuffer *rb) {
  return (rb->start == rb->end);
}

void rb_insert(struct ringbuffer *rb, uint8_t val) {
  uint32_t newEnd;
  if(rb_full(rb))
    return;
  newEnd = (rb->end + 1) & rb->mask;
  rb->buf[rb->end] = val;
  rb->end = newEnd;
}

uint8_t rb_remove(struct ringbuffer *rb) {
  uint32_t newStart;
  uint8_t val;
  if(rb_empty(rb))
    return 0;
  newStart = (rb->start + 1) & rb->mask;
  val = rb->buf[rb->start];
  rb->start = newStart;
  return val;
}

int32_t rb_peek(struct ringbuffer *rb) {
  if(rb_empty(rb))
    return -1;
  return rb->buf[rb->start];
}

