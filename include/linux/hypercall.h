#ifndef HYPERCALL_H
#define HYPERCALL_H

#include <linux/kernel.h> // for panic

extern bool USE_IGLOO_HYPERCALLS; // Should we report system state via HCs?
extern bool USE_IGLOO_VPN_HYPERCALLS; // Can we *block* on HCs while waiting for the VPN?

//static bool is_replay = false;

static inline void igloo_hypercall(uint32_t num, uint32_t a1) {
  if (!USE_IGLOO_HYPERCALLS) {
    printk(KERN_EMERG "Ignoring hypercall %d UIH is %d\n", num, USE_IGLOO_HYPERCALLS);
    return;
  }

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
  if (!USE_IGLOO_HYPERCALLS || !USE_IGLOO_VPN_HYPERCALLS)
      panic("Blocking on a hypercall when hypercalls are disabled");

  printk(KERN_EMERG "Blocking until HC result %d\n", hc_num);
  while (blocker == -1) igloo_hypercall(hc_num, (uint32_t)&blocker);

  //printk(KERN_EMERG "\tunblocked from %d with result %d\n", hc_num, blocker);
  return blocker;
}

#endif
