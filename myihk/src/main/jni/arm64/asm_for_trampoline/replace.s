//用于演示，因为不是最终方案，不写完整代码了。

//修改trampoline指向蹦床即可

.data

replace_start:

    ldr     x0, trampoline;             //如果每一个函数都在源码中新建一个shellcode的话，而不是动态复制生成，那么这个shellcode和蹦床可以合为一个。
    br      x0;                         //不能改变lr寄存器


trampoline:                           //蹦床的地址
.double 0xffffffffffffffff

replace_end:

//预先在一个section分配足够的空间，存储蹦床，其实和上面一样。
replace_start_0:

    ldr     x0, trampolines;
    br      x0;


trampolines:                           //蹦床的地址
.double 0xffffffffffffffff

replace_end_0:

//下面为伪代码，通过这样也可以实现不修改/保存寄存器、不修改hook函数，动态生成蹦床完成hook。

//优点是不用保存寄存器，缺点是因为正负64m的限制，hook函数应该和hook库是一起编译成动态库/可执行文件的。不能单独使用hook库。

//256个蹦床，实际使用可能要考虑这个hkArry内存对齐的问题(如果编译器未做内存对齐)。
//unsigned int hkArry[256 * 2] __attribute__ ((section(".my_sec")));

//void test_trampoline(){
    //仅用于演示，应该先设置内存为可读写/执行，
//    hkArry[0] = 0xf85f83e0;//或者memcpy, ldr     x0, [sp, #-0x8];
    //例如这样计算偏移，组合b指令。
//    unsigned long offset = (unsigned long) my_open - ((unsigned long) hkArry + 1 * 4);
//    hkArry[1] = 0x14000000;//计算偏移，生成指令，b       0;
//}