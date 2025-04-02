/*
 * loader.c
 * Author: Mpoki Mwaisela
 *
 * This user-space program loads and attaches an eBPF program to the
 * `execve` syscall tracepoint using the perf_event API.
 *
 * It raises the memory lock limit, loads the BPF object file,
 * finds the appropriate program, and attaches it to the tracepoint
 * `syscalls:sys_enter_execve`.
 *
 * Once attached, it streams log output from bpf_printk via trace_pipe.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <linux/perf_event.h>
#include <linux/bpf.h>
#include <bpf/libbpf.h>
#include <errno.h>

int get_tracepoint_id(const char *category, const char *event) {
    char path[256];
    snprintf(path, sizeof(path),
             "/sys/kernel/debug/tracing/events/%s/%s/id", category, event);
    FILE *f = fopen(path, "r");
    if (!f) {
        perror("fopen tracepoint id");
        return -1;
    }
    int id;
    fscanf(f, "%d", &id);
    fclose(f);
    return id;
}

int main() {
    // Step 1: Bump RLIMIT_MEMLOCK
    struct rlimit r = {RLIM_INFINITY, RLIM_INFINITY};
    if (setrlimit(RLIMIT_MEMLOCK, &r)) {
        perror("setrlimit");
        return 1;
    }

    // Step 2: Load BPF object
    struct bpf_object *obj;
    obj = bpf_object__open_file("./bpf_programs/execsnoop.bpf.o", NULL);
    if (!obj) {
        fprintf(stderr, "Failed to open BPF object\n");
        return 1;
    }

    if (bpf_object__load(obj)) {
        fprintf(stderr, "Failed to load BPF object\n");
        return 1;
    }

    // Step 3: Find program
    struct bpf_program *prog =
        bpf_object__find_program_by_title(obj, "tracepoint/syscalls/sys_enter_execve");
    if (!prog) {
        fprintf(stderr, "Failed to find program in object\n");
        return 1;
    }

    int prog_fd = bpf_program__fd(prog);
    if (prog_fd < 0) {
        perror("bpf_program__fd");
        return 1;
    }

    // Step 4: Attach using perf_event_open
    int trace_id = get_tracepoint_id("syscalls", "sys_enter_execve");
    if (trace_id < 0) {
        return 1;
    }

    struct perf_event_attr attr = {
        .type = PERF_TYPE_TRACEPOINT,
        .size = sizeof(attr),
        .config = trace_id,
        .sample_type = PERF_SAMPLE_RAW,
        .wakeup_events = 1,
    };

    int perf_fd = syscall(__NR_perf_event_open, &attr, -1, 0, -1, 0);
    if (perf_fd < 0) {
        perror("perf_event_open");
        return 1;
    }

    if (ioctl(perf_fd, PERF_EVENT_IOC_SET_BPF, prog_fd) < 0) {
        perror("PERF_EVENT_IOC_SET_BPF");
        return 1;
    }

    if (ioctl(perf_fd, PERF_EVENT_IOC_ENABLE, 0) < 0) {
        perror("PERF_EVENT_IOC_ENABLE");
        return 1;
    }

    printf("BPF program attached. Listening on trace_pipe:\n\n");
    system("cat /sys/kernel/debug/tracing/trace_pipe");

    return 0;
}
