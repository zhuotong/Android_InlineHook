
.global r_dump_start
.global r_dump_end
.global r_hk_info

.hidden r_dump_start
.hidden r_dump_end
.hidden r_hk_info


.data

r_dump_start:                    //用于读写寄存器/栈，需要自己解析参数，不能读写返回值，不能阻止原函数(被hook函数)的执行
                                //从行为上来我觉得更偏向dump，所以起名为dump。
    push    {r0-r4}                     //r0=r0,中转用 r1=sp(push之前的sp), r2=r14/lr, r3=pc, r4=cpsr
    mrs     r0, cpsr
    str     r0, [sp, #0x10]             //r4的位置存放cpsr
    str     r14, [sp, #8]               //r2的位置存放lr
    add     r14, sp, #0x14
    str     r14, [sp, #4]               //r1的位置存放真实sp
    pop     {r0}                        //恢复r0
    push    {r0-r12}                    //保存r-r12，之后是r13-r16/cpsr
    mov     r0, sp
    ldr     r1, r_hk_info;
    ldr     r3, [r1, #8];               //pre_callback
    str     pc, [sp, #0x3c];            //存储pc
    blx     r3

to_call_oriFun:
    ldr     r0, [sp, #0x40]             //cpsr
    msr     cpsr, r0
    ldmfd   sp!, {r0-r12}               //恢复r0-r12
    ldr     r14, [sp, #4]               //恢复r14/lr
    ldr     sp, [r13]                   //恢复sp(push之前的sp)
    ldr     lr, r_hk_info
    ldr     lr, [lr, #4];               //pOriFuncAddr
    blx     lr;

to_aft_callback:
    push    {r0-r4}                     //r0=r0,中转用 r1=sp(push之前的sp), r2=r14/lr, r3=pc, r4=cpsr
    mrs     r0, cpsr
    str     r0, [sp, #0x10]             //r4的位置存放cpsr
    str     r14, [sp, #8]               //r2的位置存放lr
    add     r14, sp, #0x14
    str     r14, [sp, #4]               //r1的位置存放真实sp
    pop     {r0}                        //恢复r0
    push    {r0-r12}                    //保存r-r12，之后是r13-r16/cpsr
    mov     r0, sp
    ldr     r1, r_hk_info;
    ldr     r3, [r1, #0x10];            //aft_callback
    str     pc, [sp, #0x3c];            //存储pc
    blx     r3

to_popreg:
    ldr     r0, [sp, #0x40]             //cpsr
    msr     cpsr, r0
    ldmfd   sp!, {r0-r12}               //恢复r0-r12
    ldr     r14, [sp, #4]               //恢复r14/lr
    ldr     sp, [r13]                   //恢复sp(push之前的sp)
    mov     pc, lr;

r_hk_info:                               //结构体STR_HK_INFO
.word 0x12345678

r_dump_end:

.end
