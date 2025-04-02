/*
 * execsnoop.bpf.c
 * Author: Mpoki Mwaisela
 *
 * This eBPF program attaches to the Linux `execve` syscall via a tracepoint.
 * It runs every time a new process is executed on the system.
 *
 * Specifically, it checks if the process name matches "bash", "ls", or "python".
 * If there's a match, it logs the event using "bpf_printk", which can be read
 * from /sys/kernel/debug/tracing/trace_pipe.
 *
 * This is a lightweight way to trace selected process executions in real time
 * entirely from within the kernel.
 */



#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

char LICENSE[] SEC("license") = "GPL";

SEC("tracepoint/syscalls/sys_enter_execve")
int trace_execve(struct trace_event_raw_sys_enter *ctx) {
    char comm[16];
    bpf_get_current_comm(&comm, sizeof(comm));

    // Filter: only print for these programs i.e bash, ls, python 
    if (__builtin_memcmp(comm, "bash", 4) == 0 ||
        __builtin_memcmp(comm, "ls", 2) == 0 ||
        __builtin_memcmp(comm, "python", 6) == 0) {
        bpf_printk("execve by: %s\n", comm);
    }

    return 0;
}
