.text
.globl call_uefi_function

//void call_uefi_function(void * func, number_of_args, ...);
// Calling convention of Linux x86_64
//  args: rdi, rsi, rdx, rcx, r8, r9, ...
// MS x64
//  args: rcx, rdx, r8, r9, ...
call_uefi_function:
    xchg %rcx, %rdx
    call *%rdi
    ret
