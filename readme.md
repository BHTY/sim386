win32emu To-do List
- Fix import data directory walking
- Get more programs working (make sure to try a wide variety of CRTs)
  - Get Reversi working @ 100% (fix icons in dialogs)
  - Get FreeCell working
- Fix the heap manager
- ``VirtualAlloc``
- Put more stuff for kernel32, gdi32, user32 inside of the DLL (written in C & linked with the asm thunks) rather than on the host-thunk side
- Make sure the CRT initialization code actually works and pushes the correct parameters for WinMain/main
- Iron out CPU bugs (sim386)
  - This isn't a bug so much as a missing feature, but it might be nice to add support for the PEB (since some applications might actually fuck around with that) and thus adding (at least limited) support for the FS segment register
- Reorganize the thunks - split them off from ``pe_ldr.c`` with a separate C file for each DLL's thunks (also the low-order byte of the thunk ID should identify the DLL and the upper 24-bits store the function ID)
  - In addition to this, even if the host-side thunks should probably still be done one-by-one (even if with manual assistance), generating the asm for the simulator-side thunks should be fully automated - you should give it a header file with a list of function prototypes and it'll just chew through them and pop out a completely-baked ASM file
- Abstract out the parts of ``pe_ldr.c`` that directly manipulate the stack (while this technically isn't CPU-dependent, it is calling convention-dependent, and win32 has different calling conventions on x86 vs MIPS)
  - Instead of directly reading from the stack inside of the thunk functions, they should call a function like ``uint32_t get_arg(i386* cpu, int arg_id);``
    - On i386, such a function would read off of the stack while on MIPS, it would read out of a register
  - Similarly, for functions that have to pass arguments back to the CPU for reverse thunking (i.e. ``dummy_WndProc``), they should use a generic function as well
    - A varargs function (with a fixed parameter indicating the number of arguments) is probably the best bet here (on the MIPS and most other RISC architectures, it'll put the first four arguments in designated registers and then push the rest onto the stack after a 16-byte home space)
- Add a MIPS R4000 CPU core to swap with the i386 core

Good news on the portability front: In pe_ldr.c, aside from the thunks themselves, the only functions that make any major assumptions about the CPU being an 80386 are ``handle_syscall`` and ``debug_step``
- ``debug_step``: Setting a breakpoint pokes ``0xCC`` (``int 3``) into memory at the desired point and there's various parts of the function that explicitly mention ``cpu->eip`` (including the ``g`` instruction to move the program counter, and the breakpoint fixups). This is easily fixed with a ``set_breakpoint()`` function and a ``set_pc()`` function that performs the operation
- ``handle_syscall``: Assumes that the function ID is held in ``eax`` and that the return value will be too. This is easily addressed with special functions to ``get_syscall_id`` and ``set_return_value`` inside of the relevant CPU core implementation
- The rest of the functions that actually interact with the CPU only do so to allocate blocks of virtual memory. Since the paging system is effectively an implementation detail, the MIPS core will likely inherit the same paging hierarchy as the 386.
- **Potential Concern**: There's a lot of code which assumes that everything is 32-bit (pointers, items on the stack, etc.). This shouldn't cause issues with MIPS but it could cause problems with the Alpha (though it shouldn't cause any more problems for AXP64 than AXP32 assuming the virtual memory system works in such a way that all addresses are still within 32-bits)

Core System DLLs (still very barebones)
- GDI32: 
- KERNEL32: 
- USER32:

Less Core System DLLs
- SHELL32.DLL: Thunk for GetVersionA but that's it
- MSVFW32.DLL - Video for Windows: Unimplemented
- WINMM.DLL - Windows Multimedia: Includes stubs for timeBeginPeriod and timeGetTime but that's it
- Microsoft Visual C++ runtimes: Unimplemented
- WSOCK32.DLL / WS2_32.DLL - WinSock: Unimplemented (why do programs import from this by ordinal???)
