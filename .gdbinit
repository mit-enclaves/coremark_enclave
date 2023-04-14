set arch riscv:rv64
#set riscv use_compressed_breakpoint off
layout split
foc cmd
set trace-commands on
set logging on
target remote localhost:1234
symbol-file ../secure_shared_memory/build/sm.elf
add-symbol-file ../secure_shared_memory/build/sm.enclave.elf 0x86002000
add-symbol-file build/coremark.elf 0x20000000
add-symbol-file build/payload.elf 0x82000000
