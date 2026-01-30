
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <assert.h>

#include "iterator.h"

#define STACK_CAPACITY (1024*getpagesize())

#define DA_INIT_CAP 10

typedef struct {
  void **addrs;   // array of address (stack base)
  size_t capacity;
  size_t count;
} poll;

poll stack_poll = {0};

void poll_append(void *addr) {
  if (stack_poll.count >= stack_poll.capacity) {
    stack_poll.capacity = stack_poll.capacity == 0 ? DA_INIT_CAP : stack_poll.capacity*2;
    stack_poll.addrs = realloc(stack_poll.addrs, stack_poll.capacity*sizeof(*stack_poll.addrs));
    assert(stack_poll.addrs != NULL && "Buy more RAM lol");
  }
  stack_poll.addrs[stack_poll.count++] = addr;
}

void poll_remove_unordered(size_t i) {
  assert(i < stack_poll.count);
  stack_poll.addrs[i] = stack_poll.addrs[--stack_poll.count];
}

// Linux x86_64 call convention
// %rdi, %rsi, %rdx, %rcx, %r8, and %r9

#define iterator_save_context() asm(            \
  "pushq %rdi\n\t"                              \
  "pushq %rsi\n\t"                              \
  "pushq %rbp\n\t"                              \
  "pushq %rbx\n\t"                              \
  "pushq %r12\n\t"                              \
  "pushq %r13\n\t"                              \
  "pushq %r14\n\t"                              \
  "pushq %r15")

#define iterator_pop_context() asm(             \
  "popq %r15\n\t"                               \
  "popq %r14\n\t"                               \
  "popq %r13\n\t"                               \
  "popq %r12\n\t"                               \
  "popq %rbx\n\t"                               \
  "popq %rbp\n\t"                               \
  "popq %rsi\n\t"                               \
  "popq %rdi")

void __attribute__((naked)) iterator_end(void) {
  // add iterator stack to stack_poll
  asm("movq (%rsp), %rdi\n\t"
      "movq 8(%rdi), %rdi\n\t"
      "call poll_append");
  asm("popq %rdi\n\t"          // pop iterator struct
      "movb $1, 24(%rdi)\n\t"  // iter->finish = true
      "movq $0, %rsi\n\t"      // call iterator_yield(iter, 0);
      "call iterator_yield");
  // implicit yield with 0
}

void iterator_init(Iterator *iter, void (*f)(Iterator*, void*), void *args) {
  if (stack_poll.count != 0) {
    iter->stack_base = stack_poll.addrs[0];
    poll_remove_unordered(0);
  } else {
    iter->stack_base = mmap(NULL, STACK_CAPACITY, PROT_WRITE|PROT_READ, MAP_PRIVATE|MAP_STACK|MAP_ANONYMOUS|MAP_GROWSDOWN, -1, 0);
  }
  iter->finish = false;
  void **rsp = (void**)((char*)iter->stack_base + STACK_CAPACITY);
  // @arch
  *(--rsp) = iter; // needed for iterator_end
  *(--rsp) = iterator_end;
  *(--rsp) = f;
  *(--rsp) = iter; // push rdi
  *(--rsp) = args; // push rsi
  *(--rsp) = 0;    // push rbx
  *(--rsp) = 0;    // push rbp
  *(--rsp) = 0;    // push r12
  *(--rsp) = 0;    // push r13
  *(--rsp) = 0;    // push r14
  *(--rsp) = 0;    // push r15
  iter->rsp = rsp;
}

void* __attribute__((naked)) iterator_yield(Iterator *iter, void *val) {
  (void)iter;
  (void)val;
  iterator_save_context();
  // switch stack
  asm("movq 16(%rdi), %rdx\n\t"  // iter->rsp_caller => %rdx
      "movq %rsp, (%rdi)\n\t"    // %rsp => iter->rsp
      "movq %rdx, %rsp\n\t"      // %rdx (iter->rsp_caller) => %rsp
      "movq %rsi, %rax");        // return val
  iterator_pop_context();
  asm("ret");
}

void* __attribute__((naked)) iterator_next(Iterator *iter) {
  (void)iter;
  iterator_save_context();
  // switch stack
  asm("movq %rsp, 16(%rdi)\n\t"  // store rsp in iter->rsp_caller
      "movq (%rdi), %rsp");      // iter->rsp => %rsp
  iterator_pop_context();
  asm("ret");
}

bool iterator_finish(Iterator *iter) {
  return iter->finish;
}
