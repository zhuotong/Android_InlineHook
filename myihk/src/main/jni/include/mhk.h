#ifndef _MHK_H
#define _MHK_H

#include <stdio.h>
#include <Android/log.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <stdbool.h>
//#include <cacheflush.h>


#ifndef BYTE
#define BYTE unsigned char
#endif

#define OPCODEMAXLEN 24      //inline hook所需要的opcodes最大长度,arm64为20
#define BACKUP_CODE_NUM_MAX 6  //尽管备份原程序6条arm64指令。

#define LOG_TAG "zhuo"
#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args);
#define LE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##args);

#define PAGE_START(addr)	(~(PAGE_SIZE - 1) & (addr))
#define PAGE_END(addr)   (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))

#define MY_PAGE_START(addr, page_size)	(~(page_size - 1) & (addr))
#define MY_PAGE_END(addr, page_size)   (((addr) + page_size - 1) & ~(page_size - 1))

#define SET_BIT0(addr)		(addr | 1)
#define CLEAR_BIT0(addr)	(addr & 0xFFFFFFFE)
#define TEST_BIT0(addr)		(addr & 1)

#define ACTION_ENABLE	0
#define ACTION_DISABLE	1

//#define __flush_cache(c, n)        __builtin___clear_cache(reinterpret_cast<char *>(c), reinterpret_cast<char *>(c) + n)




#if defined(__aarch64__)
struct my_pt_regs {
    __u64 uregs[31];
    __u64 sp;
    __u64 pstate;       //有时间应该修复，pc在前，但是涉及到栈和生成shellcode都要改，先这么用吧，和系统结构体有这点不同
    __u64 pc;
};

#define ARM_lr uregs[30]
#define ARM_sp sp
//#define SP (__u64*)sp
//#define SP_32(i) *(__u32*)((__u64)(regs->sp) + i*4)
//#define SP_32(i) *((__u32*)regs->sp+i)
//#define SP_32(i) *((__u64*)regs->sp+i)
#define SP(i) *((__u64*)regs->sp+i)


#elif defined(__arm__)

struct my_pt_regs {
  long uregs[18];
};

#ifndef __ASSEMBLY__

#define ARM_cpsr uregs[16]
#define ARM_pc uregs[15]
#define ARM_lr uregs[14]
#define ARM_sp uregs[13]
#define ARM_ip uregs[12]
#define ARM_fp uregs[11]
#define ARM_r10 uregs[10]
#define ARM_r9 uregs[9]
#define ARM_r8 uregs[8]
#define ARM_r7 uregs[7]
#define ARM_r6 uregs[6]
#define ARM_r5 uregs[5]
#define ARM_r4 uregs[4]
#define ARM_r3 uregs[3]
#define ARM_r2 uregs[2]
#define ARM_r1 uregs[1]
#define ARM_r0 uregs[0]
#define ARM_ORIG_r0 uregs[17]
#define ARM_VFPREGS_SIZE (32 * 8 + 4)

#endif

#define SP(i) *((__u32*)regs->ARM_sp+i)

#endif


//hook信息
typedef struct STR_HK_INFO{
    void (*onPreCallBack)(struct my_pt_regs *, struct STR_HK_INFO *pInfo);       //回调函数，在执行原函数之前获取参数/寄存器的函数
    void * pOriFuncAddr;             //存放备份/修复后原函数的地址
    void (*pre_callback)(struct my_pt_regs *, struct STR_HK_INFO *pInfo);      //pre_callback,内部做保存lr的操作，之后回调onPreCallBack，不能被用户操作
    void (*onCallBack)(struct my_pt_regs *, struct STR_HK_INFO *pInfo);            //回调函数，执行原函数之后获取返回值/寄存器的函数
    void (*aft_callback)(struct my_pt_regs *, struct STR_HK_INFO *pInfo);       //aft_callback，内部做恢复lr的操作，之后回调onCallBack，不能被用户操作
    void *pHkFunAddr;                 //hook函数，即自定义按照被hook的函数原型构造，处理参数/返回值的函数
    //以上为在shellcode中通过偏移直接或间接用到的，所以如果有变动，相应的shellcode也要跟着变动

    void **ppHkFunAddr;                     //上面pHkFunAddr函数在shellcode中的地址,已废弃;
    void **hk_infoAddr;                     //shellcode中HK_INFO的地址
    void *pBeHookAddr;                      //被hook的地址/函数
    void *pStubShellCodeAddr;               //跳过去的shellcode stub的地址
    size_t shellCodeLength;                 //上面pStubShellCodeAddr的字节数
    void ** ppOriFuncAddr;                  //shellcode 中存放备份/修复后原函数的地址，已废弃，待去除；改成上面pHkFunAddr函数的指针，应用于解除hook
    void *pNewEntryForOriFuncAddr;          //和pOriFuncAddr一致
    BYTE szbyBackupOpcodes[OPCODEMAXLEN];   //原来的opcodes
    int backUpLength;                       //备份代码的长度，arm64模式下为24
    int backUpFixLengthList[BACKUP_CODE_NUM_MAX]; //保存
    const char* methodName;
} HK_INFO;

#ifdef __cplusplus
extern "C" {
#endif

bool ChangePageProperty(void *pAddress, size_t size);

extern void * GetModuleBaseAddr(pid_t pid, char* pszModuleName);

bool InitArmHookInfo(HK_INFO* info);

bool BuildStub(HK_INFO* pInfo);

bool BuildArmJumpCode(void *pCurAddress , void *pJumpAddress, HK_INFO* info);

bool BuildOldFunction(HK_INFO* info);

bool RebuildHookTarget(HK_INFO* info);

extern bool HookArm(HK_INFO* info);

extern bool HookThumb(HK_INFO* info);

bool unHk(HK_INFO* info);

#ifdef __cplusplus
}
#endif

#endif

