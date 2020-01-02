#include <unistd.h>
#include "mhk.h"
#include "fixPCOpcode.h"

#define ALIGN_PC(pc)	(pc & 0xFFFFFFFC)

void clear_cache_addr(void* addr, int size){

    __builtin___clear_cache((char *) addr,
                            (char *) addr + size);
}

/**
 * 修改页属性，改成可读可写可执行
 * @param   pAddress   需要修改属性起始地址
 * @param   size       需要修改页属性的长度，byte为单位
 * @return  bool       修改是否成功
 */
bool ChangePageProperty(void *pAddress, size_t size)
{
    bool bRet = false;
    
    if(pAddress == NULL)
    {
        LOGI("change page property error.");
        return bRet;
    }
    
    //计算包含的页数、对齐起始地址
    unsigned long ulPageSize = sysconf(_SC_PAGESIZE); //得到页的大小
    int iProtect = PROT_READ | PROT_WRITE | PROT_EXEC;

    if (true) {
        uintptr_t start = MY_PAGE_START((uintptr_t)pAddress, ulPageSize);
        uintptr_t end = MY_PAGE_END((uintptr_t) pAddress + size, ulPageSize);
        LE("start=%p, end=%p, size=%p", start, end, end-start);
        //高版本mprotect即使第二个参数size为0，也是改变一页的内存
        int code = mprotect((void *) (start), end - start, iProtect);
        if(code)
        {
            LE("mprotect error:%s", strerror(errno));
            return bRet;
        }
        return true;
    }
    
    unsigned long ulNewPageStartAddress = (unsigned long)(pAddress) & ~(ulPageSize - 1); //pAddress & 0x1111 0000 0000 0000
    long lPageCount = (size / ulPageSize) + 1;
    LE("ulPageSize=%lu, lPageCount=%ld, size=%ld", ulPageSize, lPageCount, size);
    LE("pAddress=%p, ulNewPageStartAddress=%p, %d, %d", pAddress, ulNewPageStartAddress, (unsigned long)pAddress%4, ulNewPageStartAddress%4)
    
    long l = 0;
    while(l < lPageCount)
    {
//        LE("mprotect");
        //利用mprotect改页属性
        int iRet = mprotect((void *)(ulNewPageStartAddress), ulPageSize, iProtect);
        if(-1 == iRet)
        {
            LE("mprotect error:%s", strerror(errno));
            return bRet;
        }
        l++; 
    }
    
    return true;
}

/**
 * 通过/proc/$pid/maps，获取模块基址
 * @param   pid                 模块所在进程pid，如果访问自身进程，可填小余0的值，如-1
 * @param   pszModuleName       模块名字
 * @return  void*               模块基址，错误则返回0
 */
void * GetModuleBaseAddr(pid_t pid, char* pszModuleName)
{
    FILE *pFileMaps = NULL;
    unsigned long ulBaseValue = 0;
    char szMapFilePath[256] = {0};
    char szFileLineBuffer[1024] = {0};
    LOGI("first fork(): I'am father pid=%d", getpid());

    LOGI("Pid is %d\n",pid);

    //pid判断，确定maps文件
    if (pid < 0)
    {
        snprintf(szMapFilePath, sizeof(szMapFilePath), "/proc/self/maps");
    }
    else
    {
        snprintf(szMapFilePath, sizeof(szMapFilePath),  "/proc/%d/maps", pid);
    }

    pFileMaps = fopen(szMapFilePath, "r");
    if (NULL == pFileMaps)
    {
        return (void *)ulBaseValue;
    }
    LOGI("%d",pFileMaps);

    LOGI("Get map.\n");

    //循环遍历maps文件，找到相应模块，截取地址信息
    while (fgets(szFileLineBuffer, sizeof(szFileLineBuffer), pFileMaps) != NULL)
    {
        //LOGI("%s\n",szFileLineBuffer);
        //LOGI("%s\n",pszModuleName);
        if (strstr(szFileLineBuffer, pszModuleName))
        {
            LOGI("%s\n",szFileLineBuffer);
            char *pszModuleAddress = strtok(szFileLineBuffer, "-");
            if (pszModuleAddress)
            {
                ulBaseValue = strtoul(pszModuleAddress, NULL, 16);

                if (ulBaseValue == 0x8000)
                    ulBaseValue = 0;

                break;
            }
        }
    }
    fclose(pFileMaps);
    return (void *)ulBaseValue;
}

/**
 * arm下inline hook基础信息备份（备份原先的opcodes）
 * @param  pInfo inlinehook信息
 * @return               初始化信息是否成功
 */
bool InitArmHookInfo(HK_INFO* pInfo)
{
    bool bRet = false;
    uint32_t *currentOpcode = pInfo->pBeHookAddr;

    for(int i=0;i<BACKUP_CODE_NUM_MAX;i++){
        pInfo->backUpFixLengthList[i] = -1;
    }
    LOGI("pstInlineHook->szbyBackupOpcodes is at %p",&pInfo->szbyBackupOpcodes);


    if(!pInfo)
    {
        LOGI("pstInlineHook is null");
        return bRet;
    }

    pInfo->backUpLength = 24;
    
    memcpy(pInfo->szbyBackupOpcodes, pInfo->pBeHookAddr, pInfo->backUpLength);

    for(int i=0;i<6;i++){
        //currentOpcode += i; //GToad BUG
        LOGI("Arm64 Opcode to fix %d : %x",i,*currentOpcode);
        LOGI("Fix length : %d",lengthFixArm32(*currentOpcode));
        pInfo->backUpFixLengthList[i] = lengthFixArm64(*currentOpcode);
        currentOpcode += 1; //GToad BUG
    }
    
    return true;
}



void clear_cache(HK_INFO* info)
{
//    PAGE_MASK

//    __builtin___clear_cache((void *)PAGE_START(addr), (void *)PAGE_END(addr));
    __builtin___clear_cache((char*)info->pBeHookAddr, (char*)info->pBeHookAddr + info->backUpLength);
}

bool unHk(HK_INFO* info) {
    if (!ChangePageProperty(info->pBeHookAddr, info->backUpLength)) {
        LOGI("change page property error.");
        return false;
    }
    memcpy(info->pBeHookAddr, info->szbyBackupOpcodes, info->backUpLength);
    clear_cache(info);

    free(info->pStubShellCodeAddr);
//    free(info->pOriFuncAddr);//如果和pStubShellCodeAddr共用内存页面，不能free
    info->pStubShellCodeAddr = NULL;
    info->pOriFuncAddr = NULL;

    return true;
}


/**
 * 构造并填充ARM下32的跳转指令，需要外部保证可读可写，且pCurAddress至少8个bytes大小
 * @param  pCurAddress      当前地址，要构造跳转指令的位置
 * @param  pJumpAddress     目的地址，要从当前位置跳过去的地址
 * @return                  跳转指令是否构造成功
 */
bool BuildArmJumpCode(void *pCurAddress , void *pJumpAddress, HK_INFO* info)
{
    LOGI("LIVE4.3.1");
    bool bRet = false;
    while(1)
    {
        LOGI("LIVE4.3.2");
        if(pCurAddress == NULL)
        {
            LE("pCurAddress address null.");
            break;
        } else if (pJumpAddress == NULL) {
            LE("pJumpAddress address null.");
            break;
        }
        LOGI("LIVE4.3.3");    
        //LDR PC, [PC, #-4]
        //addr
        //LDR PC, [PC, #-4]对应的机器码为：0xE51FF004
        //addr为要跳转的地址。该跳转指令范围为32位，对于32位系统来说即为全地址跳转。
        //缓存构造好的跳转指令（ARM下32位，两条指令只需要8个bytes）
        //BYTE szLdrPCOpcodes[8] = {0x04, 0xF0, 0x1F, 0xE5};

        //
        BYTE szLdrPCOpcodes[24] = {0xe1, 0x03, 0x3f, 0xa9, 0x40, 0x00, 0x00, 0x58, 0x00, 0x00, 0x1f, 0xd6};
        //将目的地址拷贝到跳转指令缓存位置
        memcpy(szLdrPCOpcodes + 12, &pJumpAddress, 8);
        szLdrPCOpcodes[20] = 0xE0;
        szLdrPCOpcodes[21] = 0x83;
        szLdrPCOpcodes[22] = 0x5F;
        szLdrPCOpcodes[23] = 0xF8;
        LOGI("LIVE4.3.4");
        
        //将构造好的跳转指令刷进去
        memcpy(pCurAddress, szLdrPCOpcodes, 24);
        LOGI("LIVE4.3.5");
        //__flush_cache(*((uint32_t*)pCurAddress), 20);
        //__builtin___clear_cache (*((uint64_t*)pCurAddress), *((uint64_t*)(pCurAddress+20)));
//        cacheflush(((char*)pCurAddress), 24, 0);
        clear_cache_addr(pCurAddress, 24);
        LOGI("LIVE4.3.6");
        bRet = true;
        break;
    }
    LOGI("LIVE4.3.7");
    return bRet;
}



    
/**
 * 在要HOOK的位置，构造跳转，跳转到shellcode stub中
 * @param  pInfo inlinehook信息
 * @return               原地跳转指令是否构造成功
 */
bool RebuildHookTarget(HK_INFO* pInfo)
{
    bool bRet = false;
    
    while(1)
    {
        LOGI("LIVE4.1");
        if(pInfo == NULL)
        {
            LOGI("pstInlineHook is null");
            break;
        }
        LOGI("LIVE4.2");
        //修改原位置的页属性，保证可写
        if(ChangePageProperty(pInfo->pBeHookAddr, 24) == false)
        {
            LOGI("change page property error.");
            break;
        }
        LOGI("LIVE4.3");
        //填充跳转指令
        if(BuildArmJumpCode(pInfo->pBeHookAddr, pInfo->pStubShellCodeAddr, pInfo) == false)
        {
            LOGI("build jump opcodes for new entry fail.");
            break;
        }
        LOGI("LIVE4.4");
        bRet = true;
        break;
    }
    LOGI("LIVE4.5");
    
    return bRet;
}


extern bool BuildStub_dump(HK_INFO* pInfo);
extern bool BuildStub_dump_with_ret(HK_INFO* pInfo);
extern bool BuildStub_replace(HK_INFO* pInfo);
extern bool BuildStub_dump_just_ret(HK_INFO* pInfo);


/**
 * ARM下的inlinehook
 * @param  pInfo inlinehook信息
 * @return               inlinehook是否设置成功
 */
bool HookArm(HK_INFO* pInfo)
{
    bool bRet = false;
    LOGI("HookArm()");

    
    while(1)
    {
        //LOGI("pstInlineHook is null 1.");
        if(pInfo == NULL)
        {
            LOGI("pstInlineHook is null.");
            break;
        }
        LOGI("LIVE1");

        //LOGI("Init Arm HookInfo fail 1.");
        //第零步，设置ARM下inline hook的基础信息
        if(InitArmHookInfo(pInfo) == false)
        {
            LOGI("Init Arm HookInfo fail.");
            break;
        }
        LOGI("LIVE2");
        
        //LOGI("BuildStub fail 1.");
        //第二步，构造stub，功能是保存寄存器状态，同时跳转到目标函数，然后跳转回原函数
        //需要目标地址，返回stub地址，同时还有old指针给后续填充 
        if (pInfo->pHkFunAddr) {
            if(BuildStub_replace(pInfo) == false)
            {
                LOGI("BuildStub_replace fail.");
                break;
            }
        } else if(pInfo->onCallBack)
        {
            if (!BuildStub_dump_with_ret(pInfo)) {
                LOGI("BuildStub_dump_with_ret fail.");
                break;
            }
        } else if (pInfo->onPreCallBack) {
            unsigned long addr = (unsigned long)(pInfo->onPreCallBack);
            unsigned long mask = 0xff00000000000000;
            if (addr > mask) {
                pInfo->onPreCallBack = (void (*)(struct my_pt_regs *, struct STR_HK_INFO *)) (addr &
                                                                                              0x00ffffffffffffff);
                if (!BuildStub_dump_just_ret(pInfo)) {
                    LOGI("BuildStub_dump_just_ret fail.");
                    break;
                }
            } else if (!BuildStub_dump(pInfo)) {
                LOGI("BuildStub_dump fail.");
                break;
            }
        } else {
            LE("what do you want to do ?");
            return bRet;
        }
        LOGI("LIVE3");
        
        //LOGI("BuildOldFunction fail 1.");
        //第四步，负责重构原函数头，功能是修复指令，构造跳转回到原地址下
        //需要原函数地址
        
        if(BuildOldFunction(pInfo) == false)
        {
            LOGI("BuildOldFunction fail.");
            break;
        }
        LOGI("LIVE4");
        
        //LOGI("RebuildHookAddress fail 1.");
        //第一步，负责重写原函数头，功能是实现inline hook的最后一步，改写跳转
        //需要cacheflush，防止崩溃
        if(RebuildHookTarget(pInfo) == false)
        {
            LOGI("RebuildHookAddress fail.");
            break;
        }
        LOGI("LIVE5");
        
        bRet = true;
        break;
    }
    LOGI("LIVE6");

    return bRet;
}


/**
 * 构造被inline hook的函数头，还原原函数头+增加跳转
 * 仅是拷贝跳转即可，同时填充stub shellcode中的oldfunction地址及hookinfo里面的old函数地址
 * 这个实现没有指令修复功能，即是HOOK的位置指令不能涉及PC等需要重定向指令
 * @param  pInfo inlinehook信息
 * @return               原函数构造是否成功
 */
bool BuildOldFunction(HK_INFO* pInfo)
{
    bool bRet = false;

    void *fixOpcodes;
    int fixLength;
    LOGI("LIVE3.1");

    fixOpcodes = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
    LOGI("LIVE3.2");
    while(1)
    {
        if(pInfo == NULL)
        {
            LOGI("pstInlineHook is null");
            break;
        }
        LOGI("LIVE3.3");

        //8个bytes存放原来的opcodes，另外8个bytes存放跳转回hook点下面的跳转指令
//        void * pNewEntryForOldFunction = malloc(200);
//        long pagesize = sysconf(_SC_PAGE_SIZE);
        void *pNewEntryForOldFunction = NULL;
//        pNewEntryForOldFunction = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
//        posix_memalign(&pNewEntryForOldFunction,pagesize, pagesize);

        //因为确定最大的shellcode才432，分配了4096的空间，应该是足够的，当然可以再加上容错处理，不够的话分配新的内存。
        pNewEntryForOldFunction = (char*)(pInfo->pStubShellCodeAddr) + pInfo->shellCodeLength;

        if(pNewEntryForOldFunction == NULL)
        {
            LOGI("new entry for old function malloc fail.");
            break;
        }
        LOGI("LIVE3.4");

        pInfo->pNewEntryForOriFuncAddr = pNewEntryForOldFunction;
        LOGI("%p",pNewEntryForOldFunction);

        fixLength = fixPCOpcodeArm(fixOpcodes, pInfo); //把第三部分的起始地址传过去

        //可以省略了，如果使用已分配好的内存
        if(ChangePageProperty(pNewEntryForOldFunction, fixLength) == false)
        {
            LOGI("change new entry page property fail.");
            break;
        }
        LOGI("LIVE3.5");

        memcpy(pNewEntryForOldFunction, fixOpcodes, fixLength);
        LOGI("LIVE3.6");
        //memcpy(pNewEntryForOldFunction, pstInlineHook->szbyBackupOpcodes, 8);
        //填充跳转指令
        if(BuildArmJumpCode(pNewEntryForOldFunction + fixLength, pInfo->pBeHookAddr + pInfo->backUpLength - 4, pInfo) == false)
        {
            LOGI("build jump opcodes for new entry fail.");
            break;
        }
        LOGI("LIVE3.7");
        //填充shellcode里stub的回调地址
//        *(pInfo->ppOriFuncAddr) = pNewEntryForOldFunction;
        pInfo->pOriFuncAddr = pNewEntryForOldFunction;
        pInfo->ppOriFuncAddr = &(pInfo->pOriFuncAddr);
//        pInfo->ppStubShellCodeAddr = &(pInfo->pStubShellCodeAddr);
        if (pInfo->pHkFunAddr) {
//            *(pInfo->ppHkFunAddr) = pInfo->pHkFunAddr;
        }
        LOGI("LIVE3.8");

        bRet = true;
        break;
    }
    LOGI("LIVE3.9");
    if (fixOpcodes) {
        munmap(fixOpcodes, PAGE_SIZE);
    }

    return bRet;
}



