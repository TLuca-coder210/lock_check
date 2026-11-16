# lock_check

A runtime deadlock detector for multi-threaded C/C++ programs on macOS. It intercepts POSIX mutex operations at load time without requiring source modifications or recompilation of the target binary, builds a Resource Allocation Graph (RAG) from the execution trace, and checks for circular wait conditions.

## How it works

The tool is split into two parts:

1. **Interposition library (`src/tracer.c`)**
   Injected into the target process via `DYLD_INSERT_LIBRARIES`. Using macOS's `DYLD_INTERPOSE`, it hooks `pthread_mutex_lock` and `pthread_mutex_unlock` to log `REQUEST`, `ACQUIRE`, and `RELEASE` events along with thread IDs and mutex addresses to `test.log`.

2. **Cycle analyzer (`src/CycleDetectionDeadLock.c`)**
   Parses the trace log and maintains a directed Resource Allocation Graph. Raw 64-bit `pthread_t` IDs and memory addresses are mapped to compact integer indices using Binary Search Trees. On every `REQUEST` event, it traverses the graph to detect if a cycle has formed and prints the exact threads and mutexes involved.

## Implementation note: avoiding hook re-entrancy

Standard buffered I/O (`fopen`, `fprintf`) internally acquires locks and allocates heap memory. Calling them inside a `pthread_mutex_lock` hook causes an immediate self-deadlock during `dyld` / `libSystem` initialization before `main()` even starts.

To avoid this, `tracer.c` uses a thread-local re-entrancy guard (`__thread bool in_hook`), formats log lines on the stack with `snprintf`, and writes directly to the file descriptor using raw syscalls (`open`, `write`, `close`).
