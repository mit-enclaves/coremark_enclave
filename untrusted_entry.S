#include <parameters.h>

.option norvc
.section .text.entry

li sp, (RAM_BASE+RAM_SIZE)  # Top of memory
li t0, STACK_SIZE
mul t0, t0, a0
sub sp, sp, t0 # sp = (RAM_BASE+RAM_SIZE) - (mhartid*STACK_SIZE)

la ra, untrusted_entry
jr ra

# untrusted_entry should not return!
.hang:
  j .hang
