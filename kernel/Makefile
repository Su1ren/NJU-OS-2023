NAME := kernel
SRCS := kernel.c
export MODULE  := L0
export AM_HOME := $(PWD)/../abstract-machine
ifeq ($(ARCH),)
export ARCH := x86_64-qemu
endif

# GCC 13 reports a false -Warray-bounds in am/src/x86/qemu/ioe.c (MMIO at 0x0).
export CFLAGS += -Wno-array-bounds

include $(AM_HOME)/Makefile
include ../oslabs.mk
image: git
