
#include <stdio.h>

#include "iterator.h"

void counterBIS(Iterator *iter, void *args) {
  (void)args;
  for (long int i=0; i<=10; i++) {
    iterator_yield(iter, (void*)i);
  }
}

void counter_plus_55(Iterator *iter, void *args) {
  (void)args;
  for (long int i=0; i<=10; i++) {
    Iterator bis;
    iterator_init(&bis, &counterBIS, NULL);
    long int res = 0;
    while (true) {
      res += (long int) iterator_next(&bis);
      if (iterator_finish(&bis)) {
        break;
      }
    }
    // res = 55;
    iterator_yield(iter, (void*)(i+res));
  }
}

void counter(Iterator *iter, void *args) {
  int *param = (int*) args;
  int a = param[0];
  int b = param[1];
  for (long int i=a; i<=b; i++) {
    iterator_yield(iter, (void*)i);
  }
}

int main(void) {
  int vals[] = {3, 6};
  Iterator counter_iterator;
  iterator_init(&counter_iterator, &counter, vals);
  long int res;
  // print : 3, 4, 5, 6,
  while (true) {
    res = (long int) iterator_next(&counter_iterator);
    if (iterator_finish(&counter_iterator)) {
      break;
    }
    printf("%ld, ", res);
  }
  printf("\n");

  iterator_init(&counter_iterator, &counter_plus_55, vals);
  res = 0;
  // print : 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65,
  while (true) {
    res = (long int) iterator_next(&counter_iterator);
    if (iterator_finish(&counter_iterator)) {
      break;
    }
    printf("%ld, ", res);
  }
  printf("\n");

  return 0;
}
