#ifndef HYPERCALL_H
#define HYPERCALL_H

static inline void igloo_hypercall(uint32_t num, uint32_t a1) {
#ifdef CONFIG_MIPS
  asm volatile(
    "movz $0, %[num], %[a1]": : [num] "r" (num), [a1] "r" (a1) //: "memory"
    );
#else
#error "No igloo_hypercall support for architecture"
#endif
}

#endif
