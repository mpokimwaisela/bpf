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
src/  
├── Makefile              
├── loader.c                    
├── bpf_programs/                
│   ├── execsnoop.bpf.c          

### Description
 - `loader.c`: User-space loader that loads and attaches the BPF program to a tracepoint.
 - `bpf_programs/`: Contains all eBPF-related files.
    - `execsnoop.bpf.c`: The eBPF program that hooks into the execve syscall via a tracepoint.

## Build and running

- The project currently includes one eBPF program: `execsnoop.bpf.c`.  
- It attaches to the Linux `execve` syscall using a tracepoint.  
- When a process named `bash`, `ls`, or `python` is executed, it logs the event using `bpf_printk`.

```bash
cd ./src
make
sudo ./loader
```
- After starting the loader, open another terminal to test the program.  
- Run commands like `bash`, `ls`, or `python` to trigger the tracepoint.  
- These events will be logged in the loader's terminal via `bpf_printk`.



