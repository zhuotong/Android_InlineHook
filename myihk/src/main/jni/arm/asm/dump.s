.global _dump_start
.global _dump_end
.global _hk_info
.global _oriFuc

.hidden _dump_start
.hidden _dump_end
.hidden _hk_info
.hidden _oriFuc

//可用于标准的c/c++函数、非标准函数、函数的一部分(用于读写寄存器)，前提都是字节长度足够
//非标准函数即非c/c++编译的函数，那么手写汇编可能存在并不遵守约定的情况，比如我们使用了sp寄存器，并在未使用的栈上保存寄存器
//但是可能不是满递减而是反过来满递增，或者不遵守栈平衡，往栈上写数据，但是并不改变sp寄存器。当然应该是很少见的。

.data

_dump_start:                    //用于读写寄存器/栈，需要自己解析参数，不能读写返回值，不能阻止原函数(被hook函数)的执行
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
    ldr     r1, _hk_info;
    ldr     r3, [r1];                   //onPreCallBack
    str     pc, [sp, #0x3c];            //存储pc,arm模式，所以pc+8,指向ldr     r0, [sp, #0x40]
    blx     r3
    ldr     r0, [sp, #0x40]             //cpsr
    msr     cpsr, r0
    ldmfd   sp!, {r0-r12}               //恢复r0-r12
    ldr     r14, [sp, #4]               //恢复r14/lr
    ldr     sp, [r13]                   //恢复sp(push之前的sp)
    ldr     pc, _oriFuc

_oriFuc:                                //备份/修复的原方法
.word 0x12345678

_hk_info:                               //结构体STR_HK_INFO
.word 0x12345678

_dump_end:

.end
