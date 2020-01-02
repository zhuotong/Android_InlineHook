.global replace_start
.global replace_end
.global p_hk_info

.hidden replace_start
.hidden replace_end
.hidden p_hk_info

.data

//这种方式尽量用于标准的c/c++函数，因为通过hook函数再调用原函数，只能保证参数寄存器和lr寄存器是一致的，其他寄存器可能被修改。

replace_start:                      //如果只是替换/跳到hook函数，其实是不用保存寄存器的，只是重新写比较麻烦，所以在之前的基础上

    push    {r0-r4}                     //r0=r0,中转用 r1=sp(push之前的sp), r2=r14/lr, r3=pc, r4=cpsr
    mrs     r0, cpsr
    str     r0, [sp, #0x10]             //r4的位置存放cpsr
    str     r14, [sp, #8]               //r2的位置存放lr
    add     r14, sp, #0x14
    str     r14, [sp, #4]               //r1的位置存放真实sp
    pop     {r0}                        //恢复r0
    push    {r0-r12}                    //保存r-r12，之后是r13-r16/cpsr
    mov     r0, sp
    ldr     r1, p_hk_info;
    ldr     r3, [r1, #8];               //pre_callback，保存lr
    str     pc, [sp, #0x3c];            //存储pc
    blx     r3

to_call_oriFun:
    ldr     r0, [sp, #0x40]             //cpsr
    msr     cpsr, r0
    ldmfd   sp!, {r0-r12}               //恢复r0-r12
    ldr     r14, [sp, #4]               //恢复r14/lr
    ldr     sp, [r13]                   //恢复sp(push之前的sp)
    ldr     lr, p_hk_info
    ldr     lr, [lr, #0x14];            //pHkFunAddr
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
    ldr     r1, p_hk_info;
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

p_hk_info:                               //结构体STR_HK_INFO
.word 0x12345678


replace_end:

.end