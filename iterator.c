
#include <unistd.h>
#include <sys/mman.h>

#include "iterator.h"

#define STACK_CAPACITY (1024*getpagesize())

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

void __attribute__((naked)) iterator_end(void) {
  asm("popq %rdi\n\t"
      "movb $1, 24(%rdi)\n\t"
      "movq $0, %rsi\n\t"
      "call iterator_yield");
}

void iterator_init(Iterator *iter, void (*f)(Iterator*, void*), void *args) {
  iter->stack_base = mmap(NULL, STACK_CAPACITY, PROT_WRITE|PROT_READ, MAP_PRIVATE|MAP_STACK|MAP_ANONYMOUS|MAP_GROWSDOWN, -1, 0);
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
  iterator_save_context();
  asm("movq %rdi, %rdx\n\t"      // iter
      "movq 16(%rdx), %rdx\n\t"  // iter->rsp_caller => %rdx
      "movq %rsp, (%rdi)\n\t"    // %rsp => iter->rsp
      "movq %rdx, %rsp\n\t"      // %rdx (iter->rsp_caller) => %rsp
      "movq %rsi, %rax\n\t"      // return val
      "popq %r15\n\t"
      "popq %r14\n\t"
      "popq %r13\n\t"
      "popq %r12\n\t"
      "popq %rbx\n\t"
      "popq %rbp\n\t"
      "popq %rsi\n\t"
      "popq %rdi");
  asm("popq %rbp\n\t"
      "ret");
}

void* iterator_next(Iterator *iter) {
  iterator_save_context();
  asm("movq %rsp, 16(%rdi)\n\t"
      "movq (%rdi), %rsp\n\t"
      "popq %r15\n\t"
      "popq %r14\n\t"
      "popq %r13\n\t"
      "popq %r12\n\t"
      "popq %rbx\n\t"
      "popq %rbp\n\t"
      "popq %rsi\n\t"
      "popq %rdi\n\t"
      "ret");
}

bool iterator_finish(Iterator *iter) {
  return iter->finish;
}
