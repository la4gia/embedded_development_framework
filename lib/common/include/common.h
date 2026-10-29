#ifndef COMMON_H
#define COMMON_H

/**
 * @file common.h
 * @brief Common utilities for bare-metal firmware
 */

#include <stdint.h>
#include <stddef.h>

/* ─── Bit manipulation ────────────────────────────────────────────────────── */

#define BIT(n)          (1UL << (n))
#define ARRAY_SIZE(a)   (sizeof(a) / sizeof((a)[0]))
#define UNUSED(x)       ((void)(x))

/* ─── Assert (loops forever on failure) ───────────────────────────────────── */

#ifdef NDEBUG
#define ASSERT(expr) ((void)0)
#else
#define ASSERT(expr)                        \
    do {                                    \
        if (!(expr)) {                      \
            assert_failed(__FILE__, __LINE__); \
        }                                   \
    } while (0)

static inline void assert_failed(const char *file, int line)
{
    (void)file;
    (void)line;
    /* Hang here — attach debugger to inspect file:line */
    while (1)
        ;
}
#endif

/* ─── Memory barrier helpers ──────────────────────────────────────────────── */

#define DSB() __asm volatile ("dsb" ::: "memory")
#define ISB() __asm volatile ("isb" ::: "memory")
#define DMB() __asm volatile ("dmb" ::: "memory")

/* ─── Interrupt control ───────────────────────────────────────────────────── */

static inline void disable_irq(void)
{
    __asm volatile ("cpsid i" ::: "memory");
}

static inline void enable_irq(void)
{
    __asm volatile ("cpsie i" ::: "memory");
}

#endif /* COMMON_H */
