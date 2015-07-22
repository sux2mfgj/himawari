#include "vectors.h"
.text
.align 16
.globl start_tasks
start_tasks: #void start_tasks(uint32_t* system_esp);
    movl 4(%esp), %eax
    movl %eax, %esp
    jmp restart1

# ----- pic -----
#define PIC_IRQ_HANDLER(irq) \
    pushl $irq    ;\
    call irq_handler 

#define PIC_MASTER_INTERRUPT(irq) \
    pusha ;\
    PIC_IRQ_HANDLER(irq) ;\
    add $4, %esp ;\
    jmp restart

.globl restart
restart: 
    movb $PIC_OCW2_EOI, %al 
    outb $PIC_MASTER_CMD_STATE_PORT 
restart1:
    popa 
    iret

.align 16
.globl hardware_interrupt0
hardware_interrupt0:    # clock
    PIC_MASTER_INTERRUPT(0)

.align 16
.globl hardware_interrupt1
hardware_interrupt1:    # keyboard
    PIC_MASTER_INTERRUPT(1)

.align 16
.globl hardware_interrupt2
hardware_interrupt2:    # cascade!
    PIC_MASTER_INTERRUPT(2)

.align 16
.globl hardware_interrupt3
hardware_interrupt3:    # second serial
    PIC_MASTER_INTERRUPT(3)

.align 16
.globl hardware_interrupt4
hardware_interrupt4:    # first serial
    PIC_MASTER_INTERRUPT(4)

.align 16
.globl hardware_interrupt5
hardware_interrupt5:    # XT winchester
    PIC_MASTER_INTERRUPT(5)

.align 16
.globl hardware_interrupt6
hardware_interrupt6:    # floppy
    PIC_MASTER_INTERRUPT(6)

.align 16
.globl hardware_interrupt7
hardware_interrupt7:    # printer
    PIC_MASTER_INTERRUPT(7)


#define PIC_SLAVE_INTERRUPT(irq) \
    pusha ;\
    PIC_IRQ_HANDLER(irq) ;\
    movb $PIC_OCW2_EOI, %al ;\
    outb $PIC_MASTER_CMD_STATE_PORT;\
    outb $PIC_SLAVE_CMD_STATE_PORT;\
    popa ;\
    iret

.align 16
.globl hardware_interrupt8
hardware_interrupt8:
    PIC_SLAVE_INTERRUPT(8)

.align 16
.globl hardware_interrupt9
hardware_interrupt9:
    PIC_SLAVE_INTERRUPT(9)

.align 16
.globl hardware_interrupt10
hardware_interrupt10:
    PIC_SLAVE_INTERRUPT(10)

.align 16
.globl hardware_interrupt11
hardware_interrupt11:
    PIC_SLAVE_INTERRUPT(11)

.align 16
.globl hardware_interrupt12
hardware_interrupt12:
    PIC_SLAVE_INTERRUPT(12)

.align 16
.globl hardware_interrupt13
hardware_interrupt13:
    PIC_SLAVE_INTERRUPT(13)

.align 16
.globl hardware_interrupt14
hardware_interrupt14:
    PIC_SLAVE_INTERRUPT(14)

.align 16
.globl hardware_interrupt15
hardware_interrupt15:
    PIC_SLAVE_INTERRUPT(15)

# TODO: when intterrup occured(system -> kernel), how change kernel stack to system stack.
# --- save ---
# save:
#     pusha



# ----- exception -----

#define IS_INT_IN_KERNEL(displ, label)    \
    cmpl   $2, displ(%esp)  ;\
    je  label

exception_entry:
    IS_INT_IN_KERNEL(12, exception_from_kernel)

# TODO: have to create from user trapframe and from kernel trapframe
exception_from_user:

exception_from_kernel:
    push %ds
    push %es
    push %fs
    push %gs

    pushal
    pushl %esp
    call exception_handler
    #iret

#define EXCEPTION_ERROR_CODE(vector) \
    pushl $vector; \
    jmp exception_entry

#define EXCEPTION_NO_ERROR_CODE(vector) \
    pushl $0 ;\
    EXCEPTION_ERROR_CODE(vector)

.globl divide_error
divide_error:
    EXCEPTION_NO_ERROR_CODE(DIVIDE_VECTOR)

.globl debug_fault
debug_fault:
    EXCEPTION_NO_ERROR_CODE(DEBUG_VECTOR)

.globl nmi_interrupt
nmi_interrupt:
    EXCEPTION_NO_ERROR_CODE(NMI_VECTOR)

.globl break_point
break_point:
    EXCEPTION_NO_ERROR_CODE(BRAKEPOINT_VECTOR)

.globl over_flow
over_flow:
    EXCEPTION_NO_ERROR_CODE(OVERFLOW_VECTOR)

.globl br_bound
br_bound:
    EXCEPTION_NO_ERROR_CODE(BR_BOUND_VECTOR)

.globl invalid_op
invalid_op:
    EXCEPTION_NO_ERROR_CODE(INVALID_OP_VECTOR)

.globl device_not
device_not:
    EXCEPTION_NO_ERROR_CODE(DEVICE_NOT_VECTOR)

.globl double_fault
double_fault:
    EXCEPTION_ERROR_CODE(DOUBLE_FAULT)

.globl coproc_segment_overrun
coproc_segment_overrun:
    EXCEPTION_NO_ERROR_CODE(COPROC_SEGMENT_VECTOR)

.globl invalid_tss
invalid_tss:
    EXCEPTION_ERROR_CODE(INVALID_TSS_VECTOR)

.globl segment_not_fault
segment_not_fault:
    EXCEPTION_ERROR_CODE(SEG_NOT_VECTOR)

.globl stack_exception
stack_exception:
    EXCEPTION_ERROR_CODE(STACK_EXCEPTION_VECTOR)


.globl general_protection
general_protection:
    EXCEPTION_ERROR_CODE(GENERAL_PROTECTION_VECTOR)

.globl page_fault
page_fault:
    EXCEPTION_ERROR_CODE(PAGE_FAULT_VECTOR)

.globl coproc_err
coproc_err:
    EXCEPTION_NO_ERROR_CODE(COPROC_ERROR_VECOTR)

.globl alignment_check
alignment_check:
    EXCEPTION_ERROR_CODE(ALIGNMENT_CHECK_VECTOR)

.globl machine_check
machine_check:
    EXCEPTION_NO_ERROR_CODE(MACHINE_CHECK_VECTOR)

.globl simd_exception
simd_exception:
    EXCEPTION_NO_ERROR_CODE(SIMD_EXCEPTION_VECTOR)

# ---- system call ----
.globl system_call
system_call:
    IS_INT_IN_KERNEL(8, system_call_by_kernel)

# TODO: have to create from user trapframe and from kernel trapframe
system_call_by_user:

system_call_by_kernel:
    pushl %eax
    call system_call_handler
    iret


