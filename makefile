CLANG      ?= clang-15
CC         ?= gcc
BPFTOOL    ?= bpftool
CFLAGS     ?= -Wall -g
BPF_CFLAGS ?= -O2 -g -Wall -target bpf

BPF_DIR    := bpf_programs
VMLINUX_H  := $(BPF_DIR)/vmlinux.h

INCLUDES   = -I$(BPF_DIR) -I/usr/include -I/usr/include/bpf

BPF_SRCS   := $(wildcard $(BPF_DIR)/*.bpf.c)
BPF_OBJS   := $(BPF_SRCS:.bpf.c=.bpf.o)

USER_SRCS  := $(filter-out %.bpf.c, $(wildcard *.c))
USER_BINS  := $(USER_SRCS:.c=)

.PHONY: all clean run help $(VMLINUX_H)

all: $(VMLINUX_H) $(BPF_OBJS) $(USER_BINS)

$(BPF_DIR)/%.bpf.o: $(BPF_DIR)/%.bpf.c $(VMLINUX_H)
	@echo "Compiling $< -> $@"
	$(CLANG) $(BPF_CFLAGS) $(INCLUDES) -c $< -o $@

%: %.c
	@echo "Compiling $< -> $@"
	$(CC) $(CFLAGS) -o $@ $< -lbpf -lelf

$(VMLINUX_H):
	@if [ -f /sys/kernel/btf/vmlinux ]; then \
		echo "Generating $(VMLINUX_H) from kernel BTF..."; \
		$(BPFTOOL) btf dump file /sys/kernel/btf/vmlinux format c > $@; \
	else \
		echo "ERROR: /sys/kernel/btf/vmlinux not found"; \
		echo "Install a kernel with CONFIG_DEBUG_INFO_BTF=y"; \
		exit 1; \
	fi

run:
	@echo "Specify a user binary to run:"
	@echo " make run BIN=loader"
ifdef BIN
	sudo ./$(BIN)
endif

clean:
	rm -f $(BPF_OBJS) $(USER_BINS) $(VMLINUX_H)

help:
	@echo "eBPF Project Makefile"
	@echo ""
	@echo "  make              - Build everything"
	@echo "  make run BIN=x    - Run user binary with sudo"
	@echo "  make clean        - Clean generated files"
	@echo "  make help         - Show this help"
