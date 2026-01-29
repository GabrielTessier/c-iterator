
#include <stdio.h>

#include "iterator.h"

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
  while (!iterator_finish(&counter_iterator)) {
    res = (long int) iterator_next(&counter_iterator);
    printf("%ld\n", res);
  }
  printf("\n");
  return 0;
}
