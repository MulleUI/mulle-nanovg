# Debugging Crashes

<!-- Keywords: debug, crash, gdb, stacktrace, segfault -->

## Workflow

**When code crashes: STACKTRACE FIRST, fixes SECOND.**

### 1. Get stacktrace

```bash
gdb -batch -ex "run" -ex "bt" ./executable 2>&1 | tail -30
```

With arguments:
```bash
gdb -batch -ex "run arg1 arg2" -ex "bt" ./executable 2>&1 | tail -30
```

### 2. Read it

```
#0  0x5555555a121d in nvg__allocPathCache () at nanovg.c:246
#1  0x5555555a1985 in nvgCreateInternal (params=...) at nanovg.c:440
#2  0x555555569c04 in main (argc=1, argv=...) at main.c:259
```

Frame #0 = crash location. Look at that line in that file.

### 3. Fix the actual problem

Don't guess. Don't add random checks. Fix what the stacktrace shows.

## Interactive debugging

```bash
gdb ./executable
(gdb) run
(gdb) bt
(gdb) frame 2
(gdb) print variable
(gdb) info locals
```
