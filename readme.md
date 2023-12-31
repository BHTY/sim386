win32emu To-do List
- Fix import data directory walking
- Get more programs working (make sure to try a wide variety of CRTs - right now the Visual C++ 4.0 one seems to work)
  - Get Reversi working @ 100% (fix icons in dialogs)
    - The issue here is that ``DialogBoxIndirectParam`` expects any icons (and probably certain other resources) found in the dialog template to be found in the ``HINSTANCE`` that you pass it (which is not the case here since the emulated instance handles != the real ones) so any icons just won't show up (though the ``ICON`` controls will still be created, they'll be drawing icons that don't exist -> they'll be blank)
    - Short of rewriting the dialog manager, the only real option seems to be to scan the dialog template for any resource references, then enumerate over the child windows of the dialog box and "fix" those references. Right now, when a dialog box is created, I ``EnumChildWindows`` and set a default icon for every control with the ``SS_ICON`` style
  - Get FreeCell working (I'm certain there's other fixes that we'll need(beyond implementing additional imports) 
    - Fix ``LoadStringA``: basically, divide the string ID by 16 and there's your string table ID, then go through the string table and find the right string (don't assume it's 0/1/whatever like Reversi)
  - CL seems like a daunting task but it could perhaps be made to work? DoomGeneric/Win32? The sky is the limit
- Memory Management
  - Make the heap allocator actually work and not be a giant hack (it can't free and can't grow heaps)
  - ``VirtualAlloc`` - this'll be important for things like ``CreateDIBSection`` down the road
- Add dynamic support for turning the debugger on and off at runtime
- Make sure the CRT initialization code actually works and pushes the correct parameters for WinMain/main (I'm pretty sure this one is pretty much fixed) - the really important thing (eventually) will be to call DLL entry points
- Iron out CPU bugs (sim386) - make the simulator into fast386
  - This isn't a bug so much as a missing feature, but it might be nice to add support for the PEB (since some applications might actually fuck around with that) and thus adding (at least limited) support for the FS segment register
  - A lot of programs just flat-out will refuse to start because of the lack of an FPU
  - The CPU is also pretty slow (a little over 1 MIPS on a fairly beefy modern machine), though caching the stack and data pages should help matters (caching the code page broke dialogs receiving WM_COMMAND messages though)
    - Whenever you do a "far" jump/call (i.e. a jump into another code section), the block is invalidated and replaced
- Reorganize the thunks - split them off from ``pe_ldr.c`` with a separate C file for each DLL's thunks (also the low-order byte of the thunk ID should identify the DLL and the upper 24-bits store the function ID)
  - In addition to this, even if the host-side thunks should probably still be done one-by-one (even if with manual assistance), generating the asm for the simulator-side thunks should be fully automated - you should give it a header file with a list of function prototypes and it'll just chew through them and pop out a completely-baked ASM file
  - More code should be moved INSIDE of the DLLs (written in C & linked with the asm thunks) rather than on the host-thunk side (this should help simplify and clean up the code a good bit)
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
![Alt text](image.png?raw=true "Reversi!")
