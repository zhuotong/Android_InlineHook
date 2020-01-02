//
// Created by EDZ on 2019/12/23.
// 初期单独测试arm/thumb的，可用删除，已无用。
//

#include <jni.h>
#include <dlfcn.h>
#include "mhk.h"
#include "../arm/model/iModel.h"

#include <unistd.h>
#include <syscall.h>


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

//    LE("code=0x%lx", *(unsigned long*)(regs->uregs[15]));

    if (pInfo) {
        LE("onPreCallBack: HK_INFO=%p", pInfo);
        if ((pInfo->pBeHookAddr == open || pInfo->pBeHookAddr == dlsym(RTLD_DEFAULT, "__open")) && regs->uregs[0]) {
            const char* name = (const char *)(regs->uregs[0]);
            LE("onPreCallBack: open: %s , %o, %o", name, regs->uregs[1], (mode_t)regs->uregs[2]);
        }
    }

//    abort();
//    _exit(127);
//    syscall(__NR_tgkill, getpid(), gettid(), SIGABRT);

}

/**
 *  用户自定义的stub函数，嵌入在hook点中，可直接操作寄存器等
 * @param regs      寄存器结构，保存寄存器当前hook点的寄存器信息
 * @param pInfo     保存了被hook函数、hook函数等的结构体
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
//    LE("tid=%d onCallBack:%s", gettid(), name);

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

    if (pInfo) {
        LE("onCallBack: HK_INFO=%p", pInfo);
        if (pInfo->pBeHookAddr == open && regs->uregs[0]) {
            LE("onCallBack: open ret: %ld", regs->uregs[0]);
        }
    }

}

typedef FILE* (*old_fopen)(const char* __path, const char* __mode);
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


//因为都是hook的同一个函数是互斥的，用于演示函数的用法
void test_dump(){
    LE("open=%p, callback=%p", open, onPreCallBack);
    LE("__open=%p, callback=%p", dlsym(RTLD_DEFAULT, "__open"), onPreCallBack);
    if (dump((dlsym(RTLD_DEFAULT, "__open")), onPreCallBack, NULL, "__open") != success) {
        LE("hook __open error");
    } else {
        LE("hook success");
    }

    int (*my__open)(const char*, int, int) = (int (*)(const char*, int, int))dlsym(RTLD_DEFAULT, "__open");
    int fd = my__open("/system/lib/libc.so", O_RDONLY, 0);
    LE("__open /system/lib/libc.so, fd=%d", fd);
}

/*
void test_justReplace(){
    LE("fopen=%p, callback=%p", fopen, onPreCallBack);
    if (dump((void *)(fopen), onPreCallBack, NULL, "fopen") != success) {
        LE("hook fopen error");
    } else {
        LE("hook success");
    }

    FILE *pFile = fopen("/system/lib/libc.so", "re");
    LE("fopen /system/lib/libc.so, fd=%p", pFile);
}*/

void test_dump_just_ret(){
    LE("open=%p, callback=%p", open, onPreCallBack);
    if (dumpRet((void *)(open), onPreCallBack, "open") != success) {
        LE("hook open error");
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

void test_justReplace(){
    LE("fopen=%p, callback=%p", fopen, onPreCallBack);
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


JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved){
    LE("JNI_OnLoad");

//    test_dump();
    test_justReplace();
//    test_dump_just_ret();
    test_dump_with_ret();

    //    test_dump_with_ret();
    //    test_dump_ret();
    //    test_dump_just_ret();
//    test_replace();
//    test_justReplace();
//    test_args();

    //解除hook后再次hook
//    test_replace();
//    test_justReplace();
//    test_args();

//    test__system_property_get();


    return JNI_VERSION_1_4;
}

int main(){

    return 0;
}

