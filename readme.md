win32emu To-do List
- Fix import data directory walking
- Get FreeCell working (then WinMine, then Doom)
- Fix the heap manager
- VirtualAlloc
- Read resource data directory & implement (rather than stub out) associaed functions
- Iron out CPU bugs (sim386)
- Reorganize the thunks - split them off from pe_ldr.c with a separate C file for each DLL's thunks (also the low-order byte of the thunk ID should identify the DLL and the upper 24-bits store the function ID)
- Abstract out the parts of pe_ldr.c that directly manipulate the stack (while this technically isn't CPU-dependent, it is calling convention-dependent, and win32 has different calling conventions on x86 vs MIPS)
  - Instead of directly reading from the stack inside of the thunk functions, they should call a function like uint32_t get_arg(i386* cpu, int arg_id);
    - On i386, such a function would read off of the stack while on MIPS, it would read out of a register
  - Similarly, for functions that have to pass arguments back to the CPU for reverse thunking (i.e. dummy_WndProc), they should use a generic function as well
    - A varargs function (with a fixed parameter indicating the number of arguments) is probably the best bet here (on the MIPS and most other RISC architectures, it'll put the first four arguments in designated registers and then push the rest onto the stack after a 16-byte home space)
- Add a MIPS R4000 CPU core to swap with the i386 core
Good news on the portability front: In pe_ldr.c, aside from the thunks themselves, the only functions that make any major assumptions about the CPU being an 80386 are ``a``
