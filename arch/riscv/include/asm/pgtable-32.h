#ifndef _ASM_RISCV_PGTABLE_32_H
#define _ASM_RISCV_PGTABLE_32_H

#include <asm-generic/pgtable-nopmd.h>
#include <linux/const.h>

/* Size of region mapped by a page global directory */
#define PGDIR_SHIFT     22
#define PGDIR_SIZE      (_AC(1,UL) << PGDIR_SHIFT)
#define PGDIR_MASK      (~(PGDIR_SIZE - 1))

#endif /* _ASM_RISCV_PGTABLE_32_H */
