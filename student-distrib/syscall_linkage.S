#define ASM      1

.globl syscall_link

syscall_link:
    # we are in kernal space
    pushl   %ecx
    pushl   %edx
    pushl   %ebx
    pushl   %esp
    pushl   %ebp
    pushl   %esi
    pushl   %edi
    pushfl

    # call number: EAX
    # arg1: EBX
    # arg2: ECX
    # arg3: EDX

    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx

    cmpl $0, %eax
    jz syscall_error
    cmpl $10, %eax
    ja syscall_error

    call *sys_call_table(, %eax, 4)
    jmp syscall_end

syscall_error:
    movl $-1, %eax

syscall_end:
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp

    /* return to user space */
    # eax has return value
    
    popfl
    popl    %edi
    popl    %esi
    popl    %ebp
    popl    %esp
    popl    %ebx
    popl    %edx
    popl    %ecx
    
    iret

.align 4
sys_call_table:
    .long 0x0 
    .long halt
    .long execute
    .long read
    .long write
    .long open
    .long close
    .long getargs
    .long vidmap
    .long set_handler
    .long sigreturn

