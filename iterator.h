#pragma once

#include <stdbool.h>

typedef struct {
  void *rsp;
  void *stack_base;
  void *rsp_caller;
  bool finish;
} Iterator;

void iterator_init(Iterator *iter, void (*f)(Iterator*, void*), void *args);
bool iterator_finish(Iterator *iter);
void* iterator_next(Iterator *iter);
void* iterator_yield(Iterator *iter, void *val);
