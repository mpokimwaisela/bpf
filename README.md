# eBPF

A simple tutorial for demonstrating how to write and load an eBPF programs. It includes:

- Kernel-space BPF program (`*.bpf.c`)
- User-space loader (`loader.c`)
- Auto-generated `vmlinux.h`
- A general-purpose Makefile

## Requirements

I have tested and run the programs in the following system

- Linux Kernel **5.15+** with **BTF (BPF Type Format)** support
- Ubuntu 20.04
- `sudo` access

### System Dependencies

If needed, install these packages:

```bash
sudo apt update
sudo apt install -y clang-15 llvm-15 lld-15 libbpf-dev libelf-dev bpftool linux-headers-$(uname -r)

```

## Code structure
.
├── Makefile              # Build system
├── loader.c              # User-space loader
├── bpf_programs/
│   ├── execsnoop.bpf.c   # eBPF program (tracepoint)
│   └── vmlinux.h         # Auto-generated kernel type definitions

## Build and running

I only have one bpf program at the moment called execsnoop.bpf.c. 
This program attaches to the linux execve system call using a tracepoint. 
Specifically in our case we check if the process name matches bash,ls or python then logs a message using bpf_printk if it matches.

```bash
make
sudo ./loader
```


