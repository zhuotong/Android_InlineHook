//
// Created by EDZ on 2019/12/18.
//

//#include "../include/mhk.h"
#include "mhk.h"

#include <jni.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/system_properties.h>


#include "iModel.h"

double retDou(double a, double b);
float retFlo(float a, float b);

/**
 *  用户自定义的stub函数，嵌入在hook点中，可直接操作寄存器等
 * @param regs      寄存器结构，保存寄存器当前hook点的寄存器信息
 * @param pInfo     保存了被hook函数、hook函数等的结构体
 */
void onPreCallBack(my_pt_regs *regs, HK_INFO *pInfo) //参数regs就是指向栈上的一个数据结构，由第二部分的mov r0, sp所传递。
{
    const char* name = "null";
    if (pInfo) {
        if (pInfo->methodName) {
            name = pInfo->methodName;
        } else {
            char buf[20];
            sprintf(buf, "%p", pInfo->pBeHookAddr);
            name = buf;
        }
    }
//    LE("tid=%d onPreCallBack:%s", gettid(), name);

#if defined(__aarch64__)

    LE("tid=%d, onPreCallBack:%s, "
       "x0=0x%llx, x1=0x%llx, x2=0x%llx, x3=0x%llx, x4=0x%llx, x5=0x%llx, x6=0x%llx, x7=0x%llx, x8=0x%llx, x9=0x%llx, x10=0x%llx, "
       "x11=0x%llx, x12=0x%llx, x13=0x%llx, x14=0x%llx, x15=0x%llx, x16=0x%llx, x17=0x%llx, x18=0x%llx, x19=0x%llx, x20=0x%llx, "
       "x21=0x%llx, x22=0x%llx, x23=0x%llx, x24=0x%llx, x25=0x%llx, x26=0x%llx, x27=0x%llx, x28=0x%llx, x29/FP=0x%llx, x30/LR=0x%llx, "
       "cur_sp=%p, ori_sp=%p, ori_sp/31=0x%llx, NZCV/32=0x%llx, x0/pc/33=0x%llx, cur_pc=%llx, arg8=%x, arg9=%x, arg10=%x, arg11=%x, "
       "arg12=%x, arg13=%x;"
       , gettid(), name,
       regs->uregs[0], regs->uregs[1], regs->uregs[2], regs->uregs[3], regs->uregs[4], regs->uregs[5],
       regs->uregs[6], regs->uregs[7], regs->uregs[8], regs->uregs[9], regs->uregs[10], regs->uregs[11],
       regs->uregs[12], regs->uregs[13], regs->uregs[14], regs->uregs[15], regs->uregs[16], regs->uregs[17],
       regs->uregs[18], regs->uregs[19], regs->uregs[20], regs->uregs[21], regs->uregs[22], regs->uregs[23],
       regs->uregs[24], regs->uregs[25], regs->uregs[26], regs->uregs[27], regs->uregs[28], regs->uregs[29], regs->uregs[30],
       regs, /*((char*)regs + 0x110)*/((char*)regs + 0x310), regs->uregs[31], regs->uregs[32], regs->uregs[33], regs->pc,
       SP(0), SP(1), SP(2), SP(3), SP(4), SP(5)
    );

    LE("d0=%.15f, d1=%.15f, d2=%.15f, d3=%.15f, d4=%.15f, d5=%.15f, d6=%.15f, d7=%.15f, "
        "d8=%.15f, d9=%.15f, d10=%.15f, d11=%.15f, d12=%.15f, d13=%.15f, d14=%.15f, d15=%.15f, "
        "d16=%.15f, d17=%.15f, d18=%.15f, d19=%.15f, d20=%.15f, d21=%.15f, d22=%.15f, d23=%.15f, "
        "d24=%.15f, d25=%.15f, d26=%.15f, d27=%.15f, d28=%.15f, d29=%.15f, d30=%.15f, d31=%.15f;",
        DREGS(0), DREGS(1), DREGS(2), DREGS(3), DREGS(4), DREGS(5), DREGS(6), DREGS(7),
        DREGS(8), DREGS(9), DREGS(10), DREGS(11), DREGS(12), DREGS(13), DREGS(14), DREGS(15),
        DREGS(16), DREGS(17), DREGS(18), DREGS(19), DREGS(20), DREGS(21), DREGS(22), DREGS(23),
        DREGS(24), DREGS(25), DREGS(26), DREGS(27), DREGS(28), DREGS(29), DREGS(30), DREGS(31)
    );
    LE("s0=%.15f, s1=%.15f, s2=%.15f, s3=%.15f, s4=%.15f, s5=%.15f, s6=%.15f, s7=%.15f, "
        "s8=%.15f, s9=%.15f, s10=%.15f, s11=%.15f, s12=%.15f, s13=%.15f, s14=%.15f, s15=%.15f, "
        "s16=%.15f, s17=%.15f, s18=%.15f, s19=%.15f, s20=%.15f, s21=%.15f, s22=%.15f, s23=%.15f, "
        "s24=%.15f, s25=%.15f, s26=%.15f, s27=%.15f, s28=%.15f, s29=%.15f, s30=%.15f, s31=%.15f;",
        FREGS(0), FREGS(1), FREGS(2), FREGS(3), FREGS(4), FREGS(5), FREGS(6), FREGS(7),
        FREGS(8), FREGS(9), FREGS(10), FREGS(11), FREGS(12), FREGS(13), FREGS(14), FREGS(15),
        FREGS(16), FREGS(17), FREGS(18), FREGS(19), FREGS(20), FREGS(21), FREGS(22), FREGS(23),
        FREGS(24), FREGS(25), FREGS(26), FREGS(27), FREGS(28), FREGS(29), FREGS(30), FREGS(31)
    );
    LE("q0=%.15f, q1=%.15f, q2=%.15f, q3=%.15f, q4=%.15f, q5=%.15f, q6=%.15f, q7=%.15f, "
        "q8=%.15f, q9=%.15f, q10=%.15f, q11=%.15f, q12=%.15f, q13=%.15f, q14=%.15f, q15=%.15f, "
        "q16=%.15f, q17=%.15f, q18=%.15f, q19=%.15f, q20=%.15f, q21=%.15f, q22=%.15f, q23=%.15f, "
        "q24=%.15f, q25=%.15f, q26=%.15f, q27=%.15f, q28=%.15f, q29=%.15f, q30=%.15f, q31=%.15f;",
        QREGS(0), QREGS(1), QREGS(2), QREGS(3), QREGS(4), QREGS(5), QREGS(6), QREGS(7),
        QREGS(8), QREGS(9), QREGS(10), QREGS(11), QREGS(12), QREGS(13), QREGS(14), QREGS(15),
        QREGS(16), QREGS(17), QREGS(18), QREGS(19), QREGS(20), QREGS(21), QREGS(22), QREGS(23),
        QREGS(24), QREGS(25), QREGS(26), QREGS(27), QREGS(28), QREGS(29), QREGS(30), QREGS(31)
    );


#elif defined(__arm__)
    LE("tid=%d, onPreCallBack:%s, "
       "r0=0x%lx, r1=0x%lx, r2=0x%lx, r3=0x%lx, r4=0x%lx, r5=0x%lx, r6=0x%lx, r7=0x%lx, r8=0x%lx, r9=0x%lx, r10=0x%lx, r11=0x%lx, r12=0x%lx, "
       "cur_sp=%p, ori_sp=%p, ori_sp/13=0x%lx, lr=0x%lx, cur_pc=0x%lx, cpsr=0x%lx, "
       "arg4=0x%lx, arg5=0x%lx, arg4=0x%lx, arg5=0x%lx;"
    , gettid(), name,
       regs->uregs[0], regs->uregs[1], regs->uregs[2], regs->uregs[3], regs->uregs[4], regs->uregs[5],
       regs->uregs[6], regs->uregs[7], regs->uregs[8], regs->uregs[9], regs->uregs[10], regs->uregs[11],
       regs->uregs[12],
       regs, ((char*)regs + 0x44), regs->uregs[13], regs->uregs[14], regs->uregs[15], regs->uregs[16],
       regs->uregs[17], regs->uregs[18], SP(0), SP(1)
    );
#endif

    if (pInfo) {
        LE("onPreCallBack: HK_INFO=%p", pInfo);
        if (pInfo->pBeHookAddr == open && regs->uregs[0]) {
            const char* name = (const char *)(regs->uregs[0]);
            LE("onPreCallBack: open: %s , %o, %o", name, regs->uregs[1], (mode_t)regs->uregs[2]);
        } /*else if (pInfo->pBeHookAddr == retDou) {
#if defined(__aarch64__)
//            LE("d0=%.15f, d1=%.15f, d2=%.15f, d3=%.15f", regs->neon.dregs[0], regs->neon.dregs[1], regs->neon.dregs[2], regs->neon.dregs[3]);
            LE("d0=%.15f, d1=%.15f, d2=%.15f, d3=%.15f", regs->neon.dregs(0), regs->neon.dregs(1), regs->neon.dregs(2), regs->neon.dregs(3));
            LE("s0=%.15f, s1=%.15f, s2=%.15f, s3=%.15f", (float)regs->neon.fregs(0), regs->neon.fregs(1), regs->neon.fregs(2), regs->neon.fregs(3));
            LE("q0=%.15llf, q1=%.15llf, q2=%.15llf, q3=%.15llf", (long double)regs->neon.qregs[0], regs->neon.qregs[1], regs->neon.qregs[2], regs->neon.qregs[3]);
#endif
        } else if (pInfo->pBeHookAddr == retFlo) {
#if defined(__aarch64__)
            LE("d0=%.15f, d1=%.15f, d2=%.15f, d3=%.15f", regs->neon.dregs(0), regs->neon.dregs(1), regs->neon.dregs(2), regs->neon.dregs(3));
            LE("s0=%.15f, s1=%.15f, s2=%.15f, s3=%.15f", (float)regs->neon.fregs(0), regs->neon.fregs(1), regs->neon.fregs(2), regs->neon.fregs(3));
            LE("q0=%.15llf, q1=%.15llf, q2=%.15llf, q3=%.15llf", (long double)regs->neon.qregs[0], regs->neon.qregs[1], regs->neon.qregs[2], regs->neon.qregs[3]);
#endif
        }*/
    }

}

void test_args_11(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, int a11){
    LE("a0=%d, a1=%d, a2=%d, a3=%d, a4=%d, a5=%d,"
       " a6=%d, a7=%d, a8=%d, a9=%d, a10=%d. a11=%d",
       a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}


typedef int (*old_open)(const char* pathname,int flags,/*mode_t mode*/...);
typedef int (*old__open)(const char* pathname,int flags,int mode);
typedef int (*old__openat)(int fd, const char *pathname, int flags, int mode);
typedef FILE* (*old_fopen)(const char* __path, const char* __mode);

old_open *ori_open;
old__open *ori__open;



/**
 * 用户自定义的stub函数，嵌入在hook点中，可直接操作寄存器等
 * 这里将R0寄存器锁定为0x333，一个远大于30的值
 * @param regs 寄存器结构，保存寄存器当前hook点的寄存器信息
 */
void onCallBack(my_pt_regs *regs, HK_INFO *pInfo) //参数regs就是指向栈上的一个数据结构，由第二部分的mov r0, sp所传递。
{
    const char* name = "null";
    if (pInfo) {
        if (pInfo->methodName) {
            name = pInfo->methodName;
        } else {
            char buf[20];
            sprintf(buf, "%p", pInfo->pBeHookAddr);
            name = buf;
        }
    }
//    LE("tid=%d onPreCallBack:%s", gettid(), name);

#if defined(__aarch64__)

    LE("tid=%d, onCallBack:%s, "
       "x0=0x%llx, x1=0x%llx, x2=0x%llx, x3=0x%llx, x4=0x%llx, x5=0x%llx, x6=0x%llx, x7=0x%llx, x8=0x%llx, x9=0x%llx, x10=0x%llx,"
       " x11=0x%llx, x12=0x%llx, x13=0x%llx, x14=0x%llx, x15=0x%llx, x16=0x%llx, x17=0x%llx, x18=0x%llx, x19=0x%llx, x20=0x%llx, "
       "x21=0x%llx, x22=0x%llx, x23=0x%llx, x24=0x%llx, x25=0x%llx, x26=0x%llx, x27=0x%llx, x28=0x%llx, x29/FP=0x%llx, x30/LR=0x%llx, "
       "cur_sp=%p, ori_sp=%p, ori_sp/31=0x%llx, NZCV/32=0x%llx, x0/pc/33=0x%llx, cur_pc=%llx, arg8=%x, arg9=%x, arg10=%x, arg11=%x, "
       "arg12=%x, arg13=%x;"
    , gettid(), name,
       regs->uregs[0], regs->uregs[1], regs->uregs[2], regs->uregs[3], regs->uregs[4], regs->uregs[5],
       regs->uregs[6], regs->uregs[7], regs->uregs[8], regs->uregs[9], regs->uregs[10], regs->uregs[11],
       regs->uregs[12], regs->uregs[13], regs->uregs[14], regs->uregs[15], regs->uregs[16], regs->uregs[17],
       regs->uregs[18], regs->uregs[19], regs->uregs[20], regs->uregs[21], regs->uregs[22], regs->uregs[23],
       regs->uregs[24], regs->uregs[25], regs->uregs[26], regs->uregs[27], regs->uregs[28], regs->uregs[29], regs->uregs[30],
       regs, /*((char*)regs + 0x110)*/((char*)regs + 0x310), regs->uregs[31], regs->uregs[32], regs->uregs[33], regs->pc,
       SP(0), SP(1), SP(2), SP(3), SP(4), SP(5)
    );

    LE("d0=%.15f, d1=%.15f, d2=%.15f, d3=%.15f, d4=%.15f, d5=%.15f, d6=%.15f, d7=%.15f, "
        "d8=%.15f, d9=%.15f, d10=%.15f, d11=%.15f, d12=%.15f, d13=%.15f, d14=%.15f, d15=%.15f, "
        "d16=%.15f, d17=%.15f, d18=%.15f, d19=%.15f, d20=%.15f, d21=%.15f, d22=%.15f, d23=%.15f, "
        "d24=%.15f, d25=%.15f, d26=%.15f, d27=%.15f, d28=%.15f, d29=%.15f, d30=%.15f, d31=%.15f;",
        DREGS(0), DREGS(1), DREGS(2), DREGS(3), DREGS(4), DREGS(5), DREGS(6), DREGS(7),
        DREGS(8), DREGS(9), DREGS(10), DREGS(11), DREGS(12), DREGS(13), DREGS(14), DREGS(15),
        DREGS(16), DREGS(17), DREGS(18), DREGS(19), DREGS(20), DREGS(21), DREGS(22), DREGS(23),
        DREGS(24), DREGS(25), DREGS(26), DREGS(27), DREGS(28), DREGS(29), DREGS(30), DREGS(31)
    );
    LE("s0=%.15f, s1=%.15f, s2=%.15f, s3=%.15f, s4=%.15f, s5=%.15f, s6=%.15f, s7=%.15f, "
        "s8=%.15f, s9=%.15f, s10=%.15f, s11=%.15f, s12=%.15f, s13=%.15f, s14=%.15f, s15=%.15f, "
        "s16=%.15f, s17=%.15f, s18=%.15f, s19=%.15f, s20=%.15f, s21=%.15f, s22=%.15f, s23=%.15f, "
        "s24=%.15f, s25=%.15f, s26=%.15f, s27=%.15f, s28=%.15f, s29=%.15f, s30=%.15f, s31=%.15f;",
        FREGS(0), FREGS(1), FREGS(2), FREGS(3), FREGS(4), FREGS(5), FREGS(6), FREGS(7),
        FREGS(8), FREGS(9), FREGS(10), FREGS(11), FREGS(12), FREGS(13), FREGS(14), FREGS(15),
        FREGS(16), FREGS(17), FREGS(18), FREGS(19), FREGS(20), FREGS(21), FREGS(22), FREGS(23),
        FREGS(24), FREGS(25), FREGS(26), FREGS(27), FREGS(28), FREGS(29), FREGS(30), FREGS(31)
    );
    LE("q0=%.15f, q1=%.15f, q2=%.15f, q3=%.15f, q4=%.15f, q5=%.15f, q6=%.15f, q7=%.15f, "
        "q8=%.15f, q9=%.15f, q10=%.15f, q11=%.15f, q12=%.15f, q13=%.15f, q14=%.15f, q15=%.15f, "
        "q16=%.15f, q17=%.15f, q18=%.15f, q19=%.15f, q20=%.15f, q21=%.15f, q22=%.15f, q23=%.15f, "
        "q24=%.15f, q25=%.15f, q26=%.15f, q27=%.15f, q28=%.15f, q29=%.15f, q30=%.15f, q31=%.15f;",
        QREGS(0), QREGS(1), QREGS(2), QREGS(3), QREGS(4), QREGS(5), QREGS(6), QREGS(7),
        QREGS(8), QREGS(9), QREGS(10), QREGS(11), QREGS(12), QREGS(13), QREGS(14), QREGS(15),
        QREGS(16), QREGS(17), QREGS(18), QREGS(19), QREGS(20), QREGS(21), QREGS(22), QREGS(23),
        QREGS(24), QREGS(25), QREGS(26), QREGS(27), QREGS(28), QREGS(29), QREGS(30), QREGS(31)
    );

#elif defined(__arm__)

    LE("tid=%d, onCallBack:%s, "
       "r0=0x%lx, r1=0x%lx, r2=0x%lx, r3=0x%lx, r4=0x%lx, r5=0x%lx, r6=0x%lx, r7=0x%lx, r8=0x%lx, r9=0x%lx, r10=0x%lx, r11=0x%lx, r12=0x%lx, "
       "cur_sp=%p, ori_sp=%p, ori_sp/13=0x%lx, lr=0x%lx, cur_pc=0x%lx, cpsr=0x%lx, "
       "arg4=0x%lx, arg5=0x%lx, arg4=0x%lx, arg5=0x%lx;"
    , gettid(), name,
       regs->uregs[0], regs->uregs[1], regs->uregs[2], regs->uregs[3], regs->uregs[4], regs->uregs[5],
       regs->uregs[6], regs->uregs[7], regs->uregs[8], regs->uregs[9], regs->uregs[10], regs->uregs[11],
       regs->uregs[12],
       regs, ((char*)regs + 0x44), regs->uregs[13], regs->uregs[14], regs->uregs[15], regs->uregs[16],
       regs->uregs[17], regs->uregs[18], SP(0), SP(1)
    );

#endif

    if (pInfo) {
        LE("onCallBack: HK_INFO=%p", pInfo);
        if (pInfo->pBeHookAddr == open && regs->uregs[0]) {
            LE("onCallBack: open ret: %ld", regs->uregs[0]);
        }
    }

}


static inline bool needs_mode(int flags) {
    return ((flags & O_CREAT) == O_CREAT) || ((flags & O_TMPFILE) == O_TMPFILE);
}


//可变参数的函数，需要自己按照被hook函数的逻辑，解析出参数再传递给原函数。
//因为并不清楚参数个数/类型，如果不改变参数的情况下还有方法不解析参数调用原函数。
//但是如果改变了参数，比如printf中的fmt，那么理论上后面的参数类型个数也应该改变，这种情况下应该是使用者
//已经清楚共用多少参数和类型，应该自己调用，而如果只改fmt，应该会出bug的。
//所以如果只是想打印明确类型的参数，不改变参数直接调用原函数的情况，应该实现下参数的解析重组/传递，待实现
int my_open(const char* pathname,int flags, .../*int mode*/){

//    const void* caller_addr = __builtin_return_address(0);
//    LE("caller_addr=%p", caller_addr);
//    LE("caller_addr_1=%p", __builtin_return_address(1));
    mode_t mode = 0;

    if (needs_mode(flags)) {
        va_list args;
        va_start(args, flags);
        mode = static_cast<mode_t>(va_arg(args, int));
        va_end(args);
    }

    LE("hk: open %s , %d, %d", pathname ? pathname : "is null", flags, mode);


    //测试解除hook
//    HK_INFO *pInfo = isHookedByHkFun((void *) (my_open));
//    unHook(pInfo);

    int fd = -1;
    if (!ori_open) {//理论上只有粗心没有给ori_open赋值。
        LE("ori_open null !");
        exit(1);
    }

    if (!(*ori_open)) {//理论上应该是hook取消了，但是如果是自己忘了给ori_open赋值或者赋值为NULL，那就是自己的锅了，太粗心了，会陷入死循环。
        LE("unhook open");
        old_open tmp = open;
        ori_open = &tmp;
    }
    fd = (*ori_open)(pathname, flags, mode);
    LE("ori: open %s , fd=%d", pathname ? pathname : "is null", fd);

//    caller_addr = __builtin_return_address(0);
//    LE("caller_addr=%p", caller_addr);
//    LE("caller_addr_1=%p", __builtin_return_address(1));
    return fd;

}


int my__open(const char* pathname,int flags, int mode){
    LE("hk: __open %s , %d, %d", pathname ? pathname : "is null", flags, mode);

    int fd = -1;
    //理论上有可能存在取消hook了，但是hook函数还要执行，所以应该可以再回调原函数了。
    //所以如果要绝对安全，那么hook和取消hook时暂停线程，检查函数调用栈是必须的。
    auto ori_open = (old__open)(getOriFunByHkFun((void *)(my__open)));
    if (!ori_open) {
        ori_open = (old__open)dlsym(RTLD_DEFAULT, "__open");
    }
    fd = ori_open(pathname, flags, mode);
    LE("ori: __open %s , fd=%d", pathname ? pathname : "is null", fd);
    return fd;
}

FILE* my_fopen(const char* pathname,const char* mode){
    LE("hk: fopen %s , %s", pathname ? pathname : "is null", mode);

    FILE* fd = NULL;
    //理论上有可能存在取消hook了，但是hook函数还要执行，所以应该可以再回调原函数了。
    //所以如果要绝对安全，那么hook和取消hook时暂停线程，检查函数调用栈是必须的。
    auto ori_open = (old_fopen)(getOriFunByHkFun((void *)(my_fopen)));
    if (!ori_open) {
        ori_open = (old_fopen)fopen;
    }
    fd = ori_open(pathname, mode);
    LE("ori: fopen %s , fd=%p", pathname ? pathname : "is null", fd);
    return fd;
}

int my__openat(int dirFd, const char* pathname,int flags, int mode){
    LE("hk: __openat %d, %s , %d, %d", dirFd, pathname ? pathname : "is null", flags, mode);

    int fd = -1;
    //理论上有可能存在取消hook了，但是hook函数还要执行，所以应该可以再回调原函数了。
    //所以如果要绝对安全，那么hook和取消hook时暂停线程，检查函数调用栈是必须的。
    auto ori_open = (old__openat)(getOriFunByHkFun((void *)(my__openat)));
    if (!ori_open) {
        ori_open = (old__openat)dlsym(RTLD_DEFAULT, "__openat");
    }
    fd = ori_open(dirFd, pathname, flags, mode);
    LE("ori: __openat %s , fd=%d", pathname ? pathname : "is null", fd);
    return fd;
}

//因为都是hook的同一个函数是互斥的，用于演示函数的用法
void test_dump(){
    LE("open=%p, callback=%p", open, onPreCallBack);
    if (dump((void *)(open), onPreCallBack, NULL, "open") != success) {
        LE("hook open error");
    }

    unsigned int* t = (unsigned int *)(open);
    for (int i = 0; i < 6; ++i) {
        LE("%d:0x%x", i, *(t+i));
    }

    int fd = open("/system/lib/libc.so", O_RDONLY);
    LE("open /system/lib/libc.so, fd=%d", fd);
}

void test_dump_with_ret(){
    LE("open=%p, callback=%p", open, onPreCallBack);
    if (dump((void *)(open), onPreCallBack, onCallBack, "open") != success) {
        LE("hook open error");
    }

    int fd = open("/system/lib/libc.so", O_RDONLY);
    LE("open /system/lib/libc.so, fd=%d", fd);
}

void test_dump_ret(){
    LE("open=%p, callback=%p", open, onPreCallBack);
    if (dump((void *)(open), NULL, onCallBack, "open") != success) {
        LE("hook open error");
    }

    int fd = open("/system/lib/libc.so", O_RDONLY);
    LE("open /system/lib/libc.so, fd=%d", fd);
}

void test_dump_just_ret(){
    LE("open=%p, callback=%p", open, onPreCallBack);
    if (dumpRet((void *)(open), onCallBack, "open") != success) {
        LE("hook open error");
    }

    int fd = open("/system/lib/libc.so", O_RDONLY);
    LE("open /system/lib/libc.so, fd=%d", fd);
}

void test_replace(){
    LE("open=%p, callback=%p, test_replace=%p", open, onPreCallBack, test_replace);
    const RetInfo info = dump_replace((void *) (open), (void *) (my_open), onPreCallBack,
                                       onCallBack, "open");
    if (info.status != success) {
        LE("hook open error");
    } else {
        //考虑到解除hook的问题,不能用getOriFun直接获取备份的函数，应该获取函数的指针
        //不然直接返回函数，free函数后无法知道。建议不用自己保存原函数，使用api获取，当然如果没有unhook的情况，就不需要考虑这些问题。
        ori_open = (old_open *) (getPoriFun(info.info));
    }

    int fd = open("/system/lib/libc.so", O_RDONLY);
    LE("open /system/lib/libc.so, fd=%d", fd);

    unHook(info.info);
}

//9.0上arm64未导出__open函数，而arm下实现为:
//.text:0006FEDC
//.text:0006FEDC                 EXPORT __open
//.text:0006FEDC __open                                  ; DATA XREF: LOAD:0000254C↑o
//.text:0006FEDC ; __unwind {
//.text:0006FEDC                 PUSH            {R7,LR}
//.text:0006FEDE                 BLX             j_abort
//.text:0006FEDE ; } // starts at 6FEDC
//.text:0006FEDE ; End of function __open

//open函数不再走__open，而是__openat,9.0上arm64未导出__openat函数(只是隐藏了符号，可以自己根据字符串解析出地址)，arm导出。
void test_justReplace(){
    LE("open=%p, callback=%p", open, onPreCallBack);
//    const RetInfo info = dump_replace(dlsym(RTLD_DEFAULT, "__openat"), (void *) (my__open), NULL,
//                                       NULL, "__open");
    //android高版本没有__open了,__openat也隐藏符号了(只有64位隐藏了)
//    const RetInfo info = dump_replace(dlsym(RTLD_DEFAULT, "__openat"), (void *) (my__openat), NULL,
//                                       NULL, "__openat");
    
    const RetInfo info = dump_replace((void*)fopen, (void *) (my_fopen), NULL,
                                      NULL, "fopen");
    if (info.status != success) {
        LE("hook fopen error=%d", info.status);
    }


    FILE *pFile = fopen("/system/lib/libc.so", "re");
    LE("fopen /system/lib/libc.so, fd=%p", pFile);
    unHook(info.info);
}

void nullCallBack(my_pt_regs *regs, HK_INFO *pInfo){

}

void test_args_for_cache(){
    //测试多参数传递情况
    dump((void *)(test_args_11), nullCallBack, /*onCallBack*/NULL);
//    dumpRet((void *) (test_args_11), onPreCallBack, "test_args_11");
    test_args_11(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    HK_INFO *pInfo = isHooked((void*)test_args_11);
//    unHook(pInfo);
}

//测试多参数传递情况
void test_args(){
    dump((void *)(test_args_11), onPreCallBack, onCallBack);
//    dumpRet((void *) (test_args_11), onPreCallBack, "test_args_11");
    test_args_11(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
    HK_INFO *pInfo = isHooked((void*)test_args_11);
    unHook(pInfo);
}

int (*ori__system_property_get)(const char *name, char *value);

//注意操作参数之前最好都检查参数是否为null，如果原函数也是crash处理还行，如果不是就改变了函数逻辑、进程中断等
//所以细节都要注意,参数和返回值也有对应关系
int my__system_property_get(const char *name, char *value){
    LE("hk: __system_property_get(%s, %p)", name ? name : "is null", value);
    ori__system_property_get = (int (*)(const char *, char *))(getOriFunByHkFun(
            (void *) (my__system_property_get)));
    if (!ori__system_property_get) {
        ori__system_property_get = __system_property_get;
    }
    int ret = ori__system_property_get(name, value);
    LE("ori: __system_property_get(%s, %s)=%d", name ? name : "is null", ret > 0 ? value : "is null", ret);
    if (name && !strncmp("ro.serialno", name, strlen("ro.serialno"))) {
        strcpy(value, "12345678");
        return (int)(strlen("12345678"));
    }
    return ret;
}

//测试既是参数也做返回值的函数
void test__system_property_get() {
    const RetInfo info = dump_replace((void*)__system_property_get, (void *) (my__system_property_get), NULL,
                                      NULL, "__system_property_get");
    if (info.status != success) {
        LE("hook __system_property_get error=%d", info.status);
    }

    char sn[256];
    int ret = __system_property_get("ro.serialno", sn);
    LE("serialno=%s", ret > 0 ? sn : "is null");

    //2019-12-23 17:10:14.810 22213-22213/? E/zhuo: hk: __system_property_get(ro.serialno, 0x7fd55a2088)
    //2019-12-23 17:10:14.810 22213-22213/? E/zhuo: ori: __system_property_get(ro.serialno, 8acd10de)=8
    //2019-12-23 17:10:14.810 22213-22213/? E/zhuo: lr aft_callback
    //2019-12-23 17:10:14.810 22213-22213/? E/zhuo: serialno=12345678

}

//拷贝回原指令后不刷新缓存。机器不同、cpu性能、缓存大小不同等，这个阈值需要自己测试。
//性能好的机器，循环100次基本都可以100%复现，差的机器200次也只有几次复现。
//通过测试：cacheflush无法刷新缓存，不过不确定是不是第三个参数的问题，ICACHE, DCACHE, or BCACHE，抽时间再看吧，
//不过gtoad的把地址转成无符号指针再取值肯定是不正确的。
//    cacheflush(*((uint32_t*)(info->pBeHookAddr)), info->backUpLength, 0);//测试也无法刷新缓存
//    cacheflush(PAGE_START((uintptr_t)info->pBeHookAddr), PAGE_SIZE, 0);//测试也无法刷新缓存
//    cacheflush(info->pBeHookAddr, info->backUpLength, 0);//测试也无法刷新缓存

//而确实有效的刷新缓存的是：可以直接传地址和结束地址，也可以刷新一页，效果来看都成功了。
//    if(TEST_BIT0((uint32_t)info->pBeHookAddr)){
//        __builtin___clear_cache((char*)info->pBeHookAddr - 1, (char*)info->pBeHookAddr - 1 + info->backUpLength);
//    } else {
//        __builtin___clear_cache((char *) info->pBeHookAddr,
//                                (char *) info->pBeHookAddr + info->backUpLength);
//    }
//   __builtin___clear_cache(PAGE_START((uintptr_t)info->pBeHookAddr), PAGE_END((uintptr_t)info->pBeHookAddr + info->backUpLength));


//后记：
//测试cacheflush可以，但是和int cacheflush(long __addr, long __nbytes, long __cache);不一致的是，从命名来看第二个参数应该是大小，不应该是
//结束地址，而且linux man中Some or all of the address range addr to (addr+nbytes-1) is not accessible。哎奇怪，抽时间再逆向看看吧。
/*if(TEST_BIT0((uint32_t)info->pBeHookAddr)){
    cacheflush((char*)info->pBeHookAddr - 1, (char*)info->pBeHookAddr - 1 + info->backUpLength, 0);
} else {
    cacheflush((char *) info->pBeHookAddr,
                            (char *) info->pBeHookAddr + info->backUpLength, 0);
}*/

//目前来看使用__builtin___clear_cache吧，应该是都实现了且确实可用，如果某些系统不生效在抠出系统调用吧，先使用函数吧。

void test_cache(){
    test_args_for_cache();
    LE("test_args_11=%p", test_args_11);
    for (int i = 0; i < 188; ++i) {
        if (i == 178) {
            HK_INFO *pInfo = isHooked((void *) test_args_11);
            LE("pBeHookAddr=%p, pBeHookAddr=0x%lx, pBeHookAddr=0x%lx", pInfo->pBeHookAddr, pInfo->pBeHookAddr, *((uint32_t*)(pInfo->pBeHookAddr)));
            unHook(pInfo);
        }
        test_args_11(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
        LE("i=%d", i);
//        usleep(1);
    }
    LE("end test cache");
}

typedef void* (*def_dlopen)(const char* __filename, int __flag);

typedef void* (*def__loader_dlopen)(const char* filename, int flags, const void* caller_addr);

void* my__loader_dlopen(const char* name, int flag, const void* caller_addr){
    LE("hk: __loader_dlopen(%s, %d, %p)", name ? name : "is null", flag, caller_addr);
    auto ori_dlopen = (def__loader_dlopen)(getOriFunByHkFun(
            (void *) (my__loader_dlopen)));
    if (!ori_dlopen) {
//        ori_dlopen = dlopen;
        void *__loader_dlopen = dlsym(RTLD_DEFAULT, "__loader_dlopen");//0x7d3ff19dd8
        LE("__loader_dlopen=%p", __loader_dlopen);

        if (!__loader_dlopen) {
            void *dl = dlopen("libdl.so", RTLD_LAZY);
            LE("libdl.so=%p", dl);
            __loader_dlopen = dlsym(dl, "__loader_dlopen");//0x7d3ff19dd8
            LE("__loader_dlopen=%p", __loader_dlopen);
        }

        if (!__loader_dlopen) {
            return NULL;
        }

        ori_dlopen = (def__loader_dlopen)(__loader_dlopen);
    }
    void* ret = ori_dlopen(name, flag, caller_addr);
    LE("ori: __loader_dlopen(%s, %d, %p)=%p", name ? name : "is null", flag, caller_addr, ret);
    return ret;
}

void* my_dlopen(const char* name, int flag){
    LE("hk: dlopen(%s, %d)", name ? name : "is null", flag);
    auto ori_dlopen = (def_dlopen)(getOriFunByHkFun(
            (void *) (my_dlopen)));
    if (!ori_dlopen) {
        ori_dlopen = dlopen;
    }
    void* ret = ori_dlopen(name, flag);
    LE("ori: dlopen(%s, %d)=%p", name ? name : "is null", flag, ret);
    return ret;
}

void test_log(){
    LE("dlopen ....");
}

//高版本尽量使用，没有详细看各个版本的Android，不过记忆中从Android8.0使用的libdl.so，所以应该8.0及以上
//或者一步到位hook所有dlopen(包括dlopen_ext)都调用的函数do_dlopen，这个函数在linker中，所以需要在linker中找到函数地址，并没有导出，但是没有去符号，可以自己解析
//    void *do_dlopen = dlsym(RTLD_DEFAULT, "__dl__Z9do_dlopenPKciPK17android_dlextinfoPKv");
//    LE("do_dlopen=%p", do_dlopen);
//    dump((void*)do_dlopen, onPreCallBack, onCallBack, "do_dlopen");
void test__loader_dlopen(){
    //因为__loader_dlopen只在libdl.so导入，真正的导出函数是在linker中
    void *__loader_dlopen = dlsym(RTLD_DEFAULT, "__loader_dlopen");//0x7d3ff19dd8
    LE("__loader_dlopen=%p", __loader_dlopen);
    if (!__loader_dlopen) {
        //通过libdl.so拿到的__loader_dlopen地址就是linker导出的函数地址。看源码的话这个__loader_dlopen函数只是调用dlopen_ext，应该因为
        //太短无法hook，不过就是因为太短被和dlopen_ext内联成一个函数了，dlopen_ext未导出。所以可以hook。
        void *dl = dlopen("libdl.so", RTLD_LAZY);
        LE("libdl.so=%p", dl);
        __loader_dlopen = dlsym(dl, "__loader_dlopen");//0x7d3ff19dd8
        LE("__loader_dlopen=%p", __loader_dlopen);
    }

    if (!__loader_dlopen) {
        return;
    }

    //    dump((void*)__loader_dlopen, onPreCallBack, onCallBack, "__loader_dlopen");
    const RetInfo info = dump_replace((void *) __loader_dlopen, (void *) (my__loader_dlopen), NULL,
                                       NULL, "__loader_dlopen");
    if (info.status != success) {
        LE("hook __loader_dlopen error=%d", info.status);
    }

}

//arm64待修复bl指令。已临时修复
void test_dlopen() {
    LE("dlopen=%p", dlopen);
    const RetInfo info = dump_replace((void*)dlopen, (void *) (my_dlopen), NULL,
                                      NULL, "dlopen");
    if (info.status != success) {
        LE("hook dlopen error=%d", info.status);
    }
//    dump((void*)dlopen, onPreCallBack, onCallBack, "dlopen");
//    dumpRet((void*)dlopen, onCallBack, "dlopen");



    HK_INFO *pInfo = isHooked((void *)(dlopen));
    LE("HK_INFO=%p", pInfo);
    if (pInfo) {
        void *addr = pInfo->pOriFuncAddr;
        LE("add=%p", addr);

//        for (int i = 0; i < 20 * 4; i += 4) {
//        LE("%d: 0x%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x", i/4, *((unsigned char*)addr + i), *((unsigned char*)addr + i + 1),
//            LE("%d: 0x%02x%02x%02x%02x", i / 4, *((unsigned char *) addr + i),
//               *((unsigned char *) addr + i + 1),
//               *((unsigned char *) addr + i + 2), *((unsigned char *) addr + i + 3)
//           , *((unsigned char*)addr + i + 4),
//           *((unsigned char*)addr + i + 5), *((unsigned char*)addr + i + 6), *((unsigned char*)addr + i + 7)
//            );

//            LE("%d: 0x%x", i / 4, *(unsigned int *) ((unsigned char *) addr + i));
//        }
    }

//    pInfo->pOriFuncAddr = (void *)(test_log);

    void *handler = NULL;
    if (sizeof(void *) == 8) {
//        dump((void*)dlopen, onPreCallBack, onCallBack, "dlopen");
//        dump((void*)dlopen, onPreCallBack, NULL, "dlopen");//如果在一个activity启动之前hook dlopen，E/libEGL: EGL_ANDROID_blob_cache advertised, but unable to get eglSetBlobCacheFuncsANDROID
        //dlopen(libEGL_adreno.so, 2)、dlopen(libGLESv2_adreno.so, 2)，但是onCallBack为空的话是可以加载的，不同的地方那就是如果为空lr寄存器
        //还是原来的，调用__loader_dlopen第三个参数是lr寄存器的值，除此之外其他都是一样的，应该就是lr寄存器的问题，如果是栈覆盖的问题，不可能dlopen其他没问题
        //抽时间再详细分析吧。
        void *(*def_loader_dlopen)(const char *__filename, int __flag, void *lr) = (void *(*)(
                const char *, int, void *)) 0x7d3ff19dd8;

//        dump((void*)0x74dd5dfdd8, onPreCallBack, onCallBack, "__loader_dlopen");

        //def_loader_dlopen("/system/lib64/libc.so", RTLD_NOW, (void *) (test_dlopen));
        LE("handler=%p", handler);


    }

//    handler = dlopen("/system/lib64/libc.so", RTLD_NOW);
//    LE("handler=%p", handler);

    handler = dlopen("libEGL_adreno.so", 2);
//    handler = dlopen("libGLESv2_adreno.so", 2);
    LE("handler=%p", handler);


}

//debug下测试，release会被优化，如需测试才加一些代码加长。
double retDou(double a, double b){
    return a + b;
}

float retFlo(float a, float b){
    return a + b;
}

float retFlo_(int i, float a, float b){
    return i+ a + b;
}

void test_dump_neon(){
    if (dump((void *)(retDou), onPreCallBack, NULL, "retDou") != success) {
        LE("hook retDou error");
    }

    LE("%.15f", retDou(0.123456789, 0.2938475643));

    if (dump((void *)(retFlo), onPreCallBack, NULL, "retFlo") != success) {
        LE("hook retFlo error");
    }
    LE("%.15f", retFlo(0.123456789, 0.2938475643));

}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved){
    LE("JNI_OnLoad=%p", JNI_OnLoad);

    test_dump_neon();

//    test_dump();
//    test_dump_with_ret();
//    test_dump_ret();
//    test_dump_just_ret();

//    test_replace();
//    test_justReplace();
//    test_args();

//    test_cache();

    //解除hook后再次hook
//    test_replace();
//    test_justReplace();
//    test_args();

//    test__system_property_get();

//    test_dlopen();

//    LE("open=%p, open64=%p", open, /*open64*/dlsym(RTLD_DEFAULT, "open64"));

    return JNI_VERSION_1_4;
}

extern "C" JNIEXPORT void JNICALL
Java_com_zhuotong_myihk_MainActivity_testdlopen(
        JNIEnv *env,
        jobject /* this */) {
    LE("%f", retDou(0.1, 0.2));
    test_dlopen();
}

int main(){
    LE("main");
    return 0;
}

