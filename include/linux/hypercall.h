#ifndef HYPERCALL_H
#define HYPERCALL_H

//static bool is_replay = false;

static inline void igloo_hypercall(uint32_t num, uint32_t a1) {
#ifdef CONFIG_MIPS
  //if (is_replay) printk(KERN_EMERG "Igloo hypercall %d arg %x\n", num, a1);

  //if (num == 1058) is_replay = true;
  asm volatile(
    "movz $0, %[num], %[a1]": : [num] "r" (num), [a1] "r" (a1) //: "memory"
    );
#else
#error "No igloo_hypercall support for architecture"
#endif
}

// Block until target_ptr contains a value that isn't 01
static inline uint32_t block_until_hypercall_result(uint32_t hc_num) {
  volatile uint32_t blocker = -1;
  printk(KERN_EMERG "Blocking until HC result %d\n", hc_num);
  while (blocker == -1) igloo_hypercall(hc_num, (uint32_t)&blocker);

  printk(KERN_EMERG "\tunblocked from %d with result %d\n", hc_num, blocker);
  return blocker;
}

#endif
