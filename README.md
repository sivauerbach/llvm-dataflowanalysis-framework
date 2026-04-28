# Assignment 3 Code

This package contains:
- `unifiedpass.cpp`: LLVM plugin with
  - reusable fixed-point dataflow engine skeleton
  - set-print helper utilities
  - a fully wired Dominators, Dead Code Elimination, Loop Invariant Code Motion
  - pass registration for
  - `dominators`
  - `my-dce`
  - `licm`
- `Makefile`: build + run targets
- `tests/`: 2 provided test inputs for each pass (`*.bc`)

## Build

```bash
make
```

This builds `build/unifiedpass.so`.

## Run all tests

```bash
make tests
```

This generates:
- `build/tests/*-m2r.ll` (disassembled inputs)
- `build/tests/*-opt.ll` (outputs after running each pass)

## Run one pass manually

```bash
opt -bugpoint-enable-legacy-pm=1 \
  -load-pass-plugin=build/unifiedpass.so \
  -passes='dominators' tests/dominators-m2r.bc -o /tmp/out.bc
```

Replace `dominators` with one of: `my-dce`, `licm`.


