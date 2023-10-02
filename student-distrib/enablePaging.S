#define ASM     1

.text
.globl loadPageDirectory 
.align 4

# global function that loads the pageDirectory on init
loadPageDirectory:
pushl %ebp
movl %esp, %ebp


movl 8(%ebp), %eax
movl %eax, %cr3    # load pageDirectory into cr3


movl %ebp, %esp
popl %ebp
ret

.globl enablePaging
.align 4

# global function that flags specific bits for our 
enablePaging:
pushl %ebp
movl %esp, %ebp


movl %cr4, %eax
orl  $0x00000010, %eax # set bit that indicates 4MB page_size
movl %eax, %cr4

movl %cr0, %eax
orl $0x80000001, %eax # set bits thath enable paging (PG) and segment protection (PE)
movl %eax, %cr0


movl %ebp, %esp
popl %ebp
ret
