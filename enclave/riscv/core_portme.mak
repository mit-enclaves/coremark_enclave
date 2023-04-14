# Copyright 2018 Embedded Microprocessor Benchmark Consortium (EEMBC)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# 
# Original Author: Shay Gal-on

#File : core_portme.mak

# Flag : OUTFLAG
#	Use this flag to define how to to get an executable (e.g -o)
OUTFLAG= -o
# Flag : CC
#	Use this flag to define compiler to use
CC 		= riscv64-unknown-elf-gcc
# Flag : LD
#	Use this flag to define compiler to use
LD		= riscv64-unknown-elf-ld
# Flag : AS
#	Use this flag to define compiler to use
AS		= riscv64-unknown-elf-as
# Flag : E_CFLAGS
#	Use this flag to define compiler options. Note, you can add compiler options from the command line using XCFLAGS="other flags"

ENCLAVE_LD := $(ENCLAVE_SRC_DIR)/enclave.lds

PORT_CFLAGS = $(CFLAGS) -T $(ENCLAVE_LD) $(LDFLAGS)
FLAGS_STR = "$(PORT_CFLAGS) $(XCFLAGS) $(XLFLAGS) $(LFLAGS_END)"
E_CFLAGS = $(PORT_CFLAGS) -I$(PORT_DIR) -I. -DFLAGS_STR=\"$(FLAGS_STR)\" 
#Flag : LFLAGS_END
#	Define any libraries needed for linking or other flags that should come at the end of the link line (e.g. linker scripts). 
#	Note : On certain platforms, the default clock_gettime implementation is supported but requires linking of librt.
# SEPARATE_COMPILE
# Flag : SEPARATE_COMPILE
# You must also define below how to create an object file, and how to link.
OBJOUT 	= -o
LFLAGS 	=  
ASFLAGS =
OFLAG 	= -o
COUT 	= -c

LFLAGS_END = 
# Flag : PORT_SRCS
# 	Port specific source files can be added here
#	You may also need cvt.c if the fcvt functions are not provided as intrinsics by your compiler!
#PORT_SRCS = $(PORT_DIR)/core_portme.c $(PORT_DIR)/ee_printf.c $(PORT_DIR)/cvt.c
PORT_SRCS = $(PORT_DIR)/core_portme.c
PORT_SRCS += $(PORT_DIR)/../../sbi/console.c
vpath %.c $(PORT_DIR)
vpath %.s $(PORT_DIR)

OEXT = .o
EXE = .elf
BIN = .bin

$(BUILD_DIR)$(PORT_DIR)/%$(OEXT) : %.c
	$(CC) $(E_CFLAGS) $(XCFLAGS) $(COUT) $< $(OBJOUT) $@

$(BUILD_DIR)%$(OEXT) : %.c
	$(CC) $(E_CFLAGS) $(XCFLAGS) $(COUT) $< $(OBJOUT) $@

$(BUILD_DIR)$(PORT_DIR)/%$(OEXT) : %.s
	$(AS) $(ASFLAGS) $< $(OBJOUT) $@
