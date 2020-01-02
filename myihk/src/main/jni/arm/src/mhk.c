#include "mhk.h"
#include "fixPCOpcode.h"

#define ALIGN_PC(pc)	(pc & 0xFFFFFFFC)



void clear_cache_addr(void* addr, int size){
    if(TEST_BIT0((uint32_t)addr)){
        __builtin___clear_cache((char*)addr - 1, (char*)addr - 1 + size);
    } else {
        __builtin___clear_cache((char *) addr,
                                (char *) addr + size);
    }
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
 * @param  info inlinehook信息
 * @return               初始化信息是否成功
 */
bool InitArmHookInfo(HK_INFO* info)
{
    bool bRet = false;
    uint32_t *currentOpcode = info->pBeHookAddr;

    for(int i=0;i<BACKUP_CODE_NUM_MAX;i++){
        info->backUpFixLengthList[i] = -1;
    }
    
    if(info == NULL)
    {
        LOGI("HK_INFO is null");
        return bRet;
    }

    info->backUpLength = 8;
    
    memcpy(info->szbyBackupOpcodes, info->pBeHookAddr, info->backUpLength);

    for(int i=0;i<2;i++){
        //currentOpcode += i;
        LOGI("Arm32 Opcode to fix %d : %x",i,*currentOpcode);
        LOGI("Fix length : %d",lengthFixArm32(*currentOpcode));
        info->backUpFixLengthList[i] = lengthFixArm32(*currentOpcode);
        currentOpcode += 1; //GToad BUG
    }
    
    return true;
}

bool InitThumbHookInfo(HK_INFO* info)
{
    bool bRet = false;
    int backUpPos = 0;
    uint16_t *currentOpcode = info->pBeHookAddr - 1;
    int cnt = 0;
    int is_thumb32_count=0;

    for(int i=0;i<BACKUP_CODE_NUM_MAX;i++){
        info->backUpFixLengthList[i] = -1;
    }
    
    if(info == NULL)
    {
        LOGI("HK_INFO is null");
        return bRet;
    }

    uint16_t *p11; 
    
    //判断最后由(pHookAddr-1)[10:11]组成的thumb命令是不是thumb32，
    //如果是的话就需要备份14byte或者10byte才能使得汇编指令不被截断。由于跳转指令在补nop的情况下也只需要10byte，
    //所以就取pstInlineHook->backUpLength为10

    for (int k=5;k>=0;k--){
        p11 = info->pBeHookAddr-1+k*2;
        LOGI("P11 : %x",*p11);
        if(isThumb32(*p11)){
            is_thumb32_count += 1;
        }else{
            break;
        }
    }

    LOGI("is_thumb32_count : %d",is_thumb32_count);
    
    if(is_thumb32_count%2==1)
    {
        LOGI("The last ins is thumb32. Length will be 10.");
        info->backUpLength = 10;
    }
    else{
        LOGI("The last ins is not thumb32. Length will be 12.");
        info->backUpLength = 12;
    }

    //修正：否则szbyBackupOpcodes会向后偏差1 byte
    memcpy(info->szbyBackupOpcodes, info->pBeHookAddr-1, info->backUpLength);

    while(1)
    {
        LOGI("Hook Info Init");
        //int cnt=0;
        if(isThumb32(*currentOpcode))
        {
            LOGI("cnt %d thumb32",cnt);
            uint16_t *currentThumb32high = currentOpcode;
            uint16_t *currentThumb32low = currentOpcode+1;
            uint32_t instruction;
            int fixLength;

            instruction = (*currentThumb32high<<16) | *currentThumb32low;
            fixLength = lengthFixThumb32(instruction);
            LOGI("fixLength : %d",fixLength);
            info->backUpFixLengthList[cnt++] = 1; //说明是个thumb32
            info->backUpFixLengthList[cnt++] = fixLength - 1;
            backUpPos += 4;
        }
        else{
            LOGI("cnt %d thumb16",cnt);
            uint16_t instruction = *currentOpcode;
            int fixLength;
            fixLength = lengthFixThumb16(instruction);
            LOGI("fixLength : %d",fixLength);
            info->backUpFixLengthList[cnt++] = fixLength;
            backUpPos += 2;
        }

        if (backUpPos < info->backUpLength)
        {
            currentOpcode = info->pBeHookAddr -1 + sizeof(uint8_t)*backUpPos;
            LOGI("backUpPos : %d", backUpPos);
        }
        else{
            return true;
        }
    }
    
    return false;
}


/**
 * 构造并填充ARM下32的跳转指令，需要外部保证可读可写，且pCurAddress至少8个bytes大小
 * @param  pCurAddress      当前地址，要构造跳转指令的位置
 * @param  pJumpAddress     目的地址，要从当前位置跳过去的地址
 * @return                  跳转指令是否构造成功
 */
bool BuildArmJumpCode(void *pCurAddress , void *pJumpAddress, HK_INFO* info)
{
    bool bRet = false;
    while(1)
    {
        if(pCurAddress == NULL || pJumpAddress == NULL)
        {
            LOGI("address null.");
            break;
        }        
        //LDR PC, [PC, #-4]
        //addr
        //LDR PC, [PC, #-4]对应的机器码为：0xE51FF004
        //addr为要跳转的地址。该跳转指令范围为32位，对于32位系统来说即为全地址跳转。
        //缓存构造好的跳转指令（ARM下32位，两条指令只需要8个bytes）
        BYTE szLdrPCOpcodes[8] = {0x04, 0xF0, 0x1F, 0xE5};
        //将目的地址拷贝到跳转指令缓存位置
        memcpy(szLdrPCOpcodes + 4, &pJumpAddress, 4);
        
        //将构造好的跳转指令刷进去
        memcpy(pCurAddress, szLdrPCOpcodes, 8);
//        cacheflush(*((uint32_t*)pCurAddress), 8, 0);
        clear_cache_addr(pCurAddress, 8);
        
        bRet = true;
        break;
    }
    return bRet;
}

bool BuildThumbJumpCode(void *pCurAddress , void *pJumpAddress)
{
    bool bRet = false;
    while(1)
    {
        if(pCurAddress == NULL || pJumpAddress == NULL)
        {
            LOGI("address null.");
            break;
        }        
        //LDR PC, [PC, #0]
        //addr
        //LDR PC, [PC, #0]对应的thumb机器码为：0xf004f85f//arm下LDR PC, [PC, #-4]为0xE51FF004
        //addr为要跳转的地址。该跳转指令范围为32位，对于32位系统来说即为全地址跳转。
        //缓存构造好的跳转指令（ARM下32位，两条指令只需要8个bytes）
        //对于目标代码是thumb-2指令集来说，使用固定的8或者12byte备份是肯定有问题的！因为thumb16（2byte）和thumb32（4byte）是混合使用的
        //因此，当备份12byte时，如果目标是2+2+2+2+2+4，那就会把最后的那个thumb32截断。
        //当备份8byte时，如果目标是2+4+4，也会把最后的thumb32截断
        if (CLEAR_BIT0((uint32_t)pCurAddress) % 4 != 0) {
			//((uint16_t *) CLEAR_BIT0(pCurAddress))[i++] = 0xBF00;  // NOP
            BYTE szLdrPCOpcodes[12] = {0x00, 0xBF, 0xdF, 0xF8, 0x00, 0xF0};
            memcpy(szLdrPCOpcodes + 6, &pJumpAddress, 4);
            memcpy(pCurAddress, szLdrPCOpcodes, 10);
//            cacheflush(*((uint32_t*)pCurAddress), 10, 0);
            clear_cache_addr(pCurAddress + 1, 10);
		}
        else{
            BYTE szLdrPCOpcodes[8] = {0xdF, 0xF8, 0x00, 0xF0};
            //将目的地址拷贝到跳转指令缓存位置
            memcpy(szLdrPCOpcodes + 4, &pJumpAddress, 4);
            memcpy(pCurAddress, szLdrPCOpcodes, 8);
//            cacheflush(*((uint32_t*)pCurAddress), 8, 0);
            clear_cache_addr(pCurAddress + 1, 8);
        }

        
        
        //将构造好的跳转指令刷进去
        //memcpy(pCurAddress, szLdrPCOpcodes, 8); //这边需要参考ele7enxxh的代码，补上nop之类的
        //cacheflush(*((uint32_t*)pCurAddress), 8, 0);
        
        bRet = true;
        break;
    }
    return bRet;
}

/**
 * 构造被inline hook的函数头，还原原函数头+增加跳转
 * 仅是拷贝跳转即可，同时填充stub shellcode中的oldfunction地址及hookinfo里面的old函数地址
 * 这个实现没有指令修复功能，即是HOOK的位置指令不能涉及PC等需要重定向指令
 * @param  info inlinehook信息
 * @return               原函数构造是否成功
 */
bool BuildOldFunction(HK_INFO* info)
{
    bool bRet = false;

    void *fixOpcodes;
    int fixLength;

    fixOpcodes = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
    
    while(1)
    {
        if(info == NULL)
        {
            LOGI("pstInlineHook is null");
            break;
        }
        
        //8个bytes存放原来的opcodes，另外8个bytes存放跳转回hook点下面的跳转指令
//        void * pNewEntryForOldFunction = malloc(200);

        long pagesize = sysconf(_SC_PAGE_SIZE);
        void *pNewEntryForOldFunction = NULL;
//        pNewEntryForOldFunction = malloc(200);
        int code = 0;
//        code = posix_memalign(&pNewEntryForOldFunction, pagesize, pagesize);
        pNewEntryForOldFunction = (char*)(info->pStubShellCodeAddr) + info->shellCodeLength;

        if(code || pNewEntryForOldFunction == NULL)
        {
            LOGI("new entry for old function malloc fail.");
            break;
        }

        info->pNewEntryForOriFuncAddr = pNewEntryForOldFunction;
        
        fixLength = fixPCOpcodeArm(fixOpcodes, info); //把第三部分的起始地址传过去

        if(ChangePageProperty(pNewEntryForOldFunction, fixLength) == false)
        {
            LOGI("change new entry page property fail.");
            break;
        }

        memcpy(pNewEntryForOldFunction, fixOpcodes, fixLength);
        //memcpy(pNewEntryForOldFunction, pstInlineHook->szbyBackupOpcodes, 8);
        //填充跳转指令
        if(BuildArmJumpCode(pNewEntryForOldFunction + fixLength, info->pBeHookAddr + info->backUpLength, info) == false)
        {
            LOGI("build jump opcodes for new entry fail.");
            break;
        }


        //填充shellcode里stub的回调地址
        if (info->ppOriFuncAddr){
            *(info->ppOriFuncAddr) = pNewEntryForOldFunction;
        } else {
            info->pOriFuncAddr = pNewEntryForOldFunction;
            info->ppOriFuncAddr = &(info->pOriFuncAddr);
        }
        LE("填充shellcode里stub的回调地址");

//        *(pstInlineHook->ppHkFun) = pstInlineHook->pHkFun;

        bRet = true;
        break;
    }
    if (fixOpcodes) {
        munmap(fixOpcodes, PAGE_SIZE);
    }

    return bRet;
}











bool BuildOldFunctionThumb(HK_INFO* info)
{
    bool bRet = false;

    void *fixOpcodes;
    int fixLength;

    fixOpcodes = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
    
    while(1)
    {
        if(info == NULL)
        {
            LOGI("pstInlineHook is null");
            break;
        }
        
        //12个bytes存放原来的thumb opcodes，另外8个bytes存放跳转回hook点下面的跳转指令
//        void * pNewEntryForOldFunction = malloc(200);

        long pagesize = sysconf(_SC_PAGE_SIZE);
        void *pNewEntryForOldFunction = NULL;
//        pNewEntryForOldFunction = malloc(200);
        int code = 0;
//        code = posix_memalign(&pNewEntryForOldFunction, pagesize, pagesize);
        pNewEntryForOldFunction = (char*)(info->pStubShellCodeAddr) + info->shellCodeLength;

        if(code || pNewEntryForOldFunction == NULL)
        {
            LOGI("new entry for old function malloc fail.");
            break;
        }
        info->pNewEntryForOriFuncAddr = pNewEntryForOldFunction;
        
        fixLength = fixPCOpcodeThumb(fixOpcodes, info); //修复PC相关指令

        if(ChangePageProperty(info->pNewEntryForOriFuncAddr, fixLength) == false)
        {
            LOGI("change new entry page property fail.");
            break;
        }

        //返回修复后opcode的指令长度，修复后的指令保存在fixOpcode中
        memcpy(pNewEntryForOldFunction, fixOpcodes, fixLength);
        //memcpy(pNewEntryForOldFunction, pstInlineHook->szbyBackupOpcodes, pstInlineHook->backUpLength);
        LOGI("pBeHookAddr : %x",info->pBeHookAddr);
        LOGI("backupLength : %x",info->backUpLength);
        //填充跳转指令
        if(BuildThumbJumpCode(pNewEntryForOldFunction + fixLength, info->pBeHookAddr + info->backUpLength) == false)
        {
            LOGI("build jump opcodes for new entry fail.");
            break;
        }
        //填充shellcode里stub的回调地址
        if (info->ppOriFuncAddr) {
            *(info->ppOriFuncAddr) = pNewEntryForOldFunction + 1;
        } else {
            info->pOriFuncAddr = pNewEntryForOldFunction + 1;
            info->ppOriFuncAddr = &(info->pOriFuncAddr);
        }


        if (info->pHkFunAddr) {
//            *(pstInlineHook->ppOldFuncAddr) = (char*)pNewEntryForOldFunction + 1;//为了安全还是转成char*
        } else {
//            *(pstInlineHook->ppOldFuncAddr) = pNewEntryForOldFunction;
        }

//        *(pstInlineHook->ppHkFun) = pstInlineHook->pHkFun;

        bRet = true;
        break;
    }

    if (fixOpcodes) {
        munmap(fixOpcodes, PAGE_SIZE);
    }
    
    return bRet;
}
    
/**
 * 在要HOOK的位置，构造跳转，跳转到shellcode stub中
 * @param  info inlinehook信息
 * @return               原地跳转指令是否构造成功
 */
bool RebuildHookTarget(HK_INFO* info)
{
    bool bRet = false;
    
    while(1)
    {
        if(info == NULL)
        {
            LOGI("pstInlineHook is null");
            break;
        }
        //修改原位置的页属性，保证可写
        if(ChangePageProperty(info->pBeHookAddr, 12) == false)
        {
            LOGI("change page property error.");
            break;
        }
        //填充跳转指令
        if(BuildArmJumpCode(info->pBeHookAddr, info->pStubShellCodeAddr, info) == false)
        {
            LOGI("build jump opcodes for new entry fail.");
            break;
        }
        bRet = true;
        break;
    }
    
    return bRet;
}

bool RebuildHookTargetThumb(HK_INFO* pstInlineHook)
{
    bool bRet = false;
    
    while(1)
    {
        if(pstInlineHook == NULL)
        {
            LOGI("pstInlineHook is null");
            break;
        }
        //修改原位置的页属性，保证可写
        if(ChangePageProperty(pstInlineHook->pBeHookAddr, 8) == false)
        {
            LOGI("change page property error.");
            break;
        }
        //填充跳转指令
        if(BuildThumbJumpCode(pstInlineHook->pBeHookAddr-1, pstInlineHook->pStubShellCodeAddr) == false)
        {
            LOGI("build jump opcodes for new entry fail.");
            break;
        }
        bRet = true;
        break;
    }
    
    return bRet;
}

extern bool BuildStub_dump(HK_INFO* pInfo);
extern bool BuildStub_dump_just_ret(HK_INFO* pInfo);
extern bool BuildStub_dump_with_ret(HK_INFO* pInfo);
extern bool BuildStub_replace(HK_INFO* pInfo);

/**
 * ARM下的inlinehook
 * @param  info inlinehook信息
 * @return               inlinehook是否设置成功
 */
bool HookArm(HK_INFO* info)
{
    bool bRet = false;
    LOGI("HookArm()");
    
    while(1)
    {
        //LOGI("HK_INFO is null 1.");
        if(info == NULL)
        {
            LOGI("HK_INFO is null.");
            break;
        }

        //LOGI("Init Arm HookInfo fail 1.");
        //设置ARM下inline hook的基础信息
        if(InitArmHookInfo(info) == false)
        {
            LOGI("Init Arm HookInfo fail.");
            break;
        }
        
        //LOGI("BuildStub fail 1.");
        //构造stub，功能是保存寄存器状态，同时跳转到目标函数，然后跳转回原函数
        //需要目标地址，返回stub地址，同时还有old指针给后续填充
        if (info->pHkFunAddr) {
            if(BuildStub_replace(info) == false)
            {
                LOGI("BuildStub_replace fail.");
                break;
            }
        } else if(info->onCallBack)
        {
            if (info->onCallBack == 0xffffffff) {
                if (info->onPreCallBack) {
                    if (!BuildStub_dump_just_ret(info)) {
                        LOGI("BuildStub_dump_just_ret fail.");
                        break;
                    }
                } else {
                    LE("what do you want to do ?");
                    return bRet;
                }
            } else if (!BuildStub_dump_with_ret(info)) {
                LOGI("BuildStub_dump_with_ret fail.");
                break;
            }
        } else if (info->onPreCallBack) {
            if (!BuildStub_dump(info)) {
                LOGI("BuildStub_dump fail.");
                break;
            }
        } else {
            LE("what do you want to do ?");
            return bRet;
        }
        
        //LOGI("BuildOldFunction fail 1.");
        //负责重构原函数头，功能是修复指令，构造跳转回到原地址下
        //需要原函数地址
        if(BuildOldFunction(info) == false)
        {
            LOGI("BuildOldFunction fail.");
            break;
        }

        LE("RebuildHookTarget");
        //LOGI("RebuildHookAddress fail 1.");
        //负责重写原函数头，功能是实现inline hook的最后一步，改写跳转
        //需要cacheflush，防止崩溃
        if(RebuildHookTarget(info) == false)
        {
            LOGI("RebuildHookAddress fail.");
            break;
        }
        LE("RebuildHookTarget end");
        bRet = true;
        break;
    }

    return bRet;
}

/**
 * Thumb16 Thumb32下的inlinehook
 * @param  info inlinehook信息
 * @return               inlinehook是否设置成功
 */
bool HookThumb(HK_INFO* info)
{
    bool bRet = false;
    LOGI("HookThumb()");
    
    while(1)
    {
        //LOGI("HK_INFO is null 1.");
        if(info == NULL)
        {
            LOGI("HK_INFO is null.");
            break;
        }

        //LOGI("Init Thumb HookInfo fail 1.");
        //设置ARM下inline hook的基础信息
        if(InitThumbHookInfo(info) == false)
        {
            LOGI("Init Thumb HookInfo fail.");
            break;
        }
        
        //LOGI("BuildStub fail 1.");
        //构造stub，功能是保存寄存器状态，同时跳转到目标函数，然后跳转回原函数
        //需要目标地址，返回stub地址，同时还有old指针给后续填充
        if (info->pHkFunAddr) {
            if(BuildStub_replace(info) == false)
            {
                LOGI("BuildStub_replace fail.");
                break;
            }
        } else if(info->onCallBack)
        {
            if (info->onCallBack == 0xffffffff) {
                if (info->onPreCallBack) {
                    if (!BuildStub_dump_just_ret(info)) {
                        LOGI("BuildStub_dump_just_ret fail.");
                        break;
                    }
                } else {
                    LE("what do you want to do ?");
                    return bRet;
                }
            } else if (!BuildStub_dump_with_ret(info)) {
                LOGI("BuildStub_dump_with_ret fail.");
                break;
            }
        } else if (info->onPreCallBack) {
            if (!BuildStub_dump(info)) {
                LOGI("BuildStub_dump fail.");
                break;
            }
        } else {
            LE("what do you want to do ?");
            return bRet;
        }
        
        //LOGI("BuildOldFunction fail 1.");
        //负责重构原函数头，功能是修复指令，构造跳转回到原地址下
        //需要原函数地址
        if(BuildOldFunctionThumb(info) == false)
        {
            LOGI("BuildOldFunction fail.");
            break;
        }
        
        //LOGI("RebuildHookAddress fail 1.");
        //负责重写原函数头，功能是实现inline hook的最后一步，改写跳转
        //需要cacheflush，防止崩溃
        if(RebuildHookTargetThumb(info) == false)
        {
            LOGI("RebuildHookAddress fail.");
            break;
        }
        bRet = true;
        break;
    }

    return bRet;
}



void clear_cache(HK_INFO* info)
{
//    PAGE_MASK

//    __builtin___clear_cache((void *)PAGE_START(addr), (void *)PAGE_END(addr));

    //测试这两个可以，即一页或绝对地址和大小都可以
    if(TEST_BIT0((uint32_t)info->pBeHookAddr)){
        __builtin___clear_cache((char*)info->pBeHookAddr - 1, (char*)info->pBeHookAddr - 1 + info->backUpLength);
    } else {
        __builtin___clear_cache((char *) info->pBeHookAddr,
                                (char *) info->pBeHookAddr + info->backUpLength);
    }
//    __builtin___clear_cache(PAGE_START((uintptr_t)info->pBeHookAddr), PAGE_END((uintptr_t)info->pBeHookAddr + info->backUpLength));

    //测试可以，但是和int cacheflush(long __addr, long __nbytes, long __cache);不一致的是，从命名来看第二个参数应该是大小，不应该是
    //结束地址，哎奇怪，抽时间再逆向看看吧。
    /*if(TEST_BIT0((uint32_t)info->pBeHookAddr)){
        cacheflush((char*)info->pBeHookAddr - 1, (char*)info->pBeHookAddr - 1 + info->backUpLength, 0);
    } else {
        cacheflush((char *) info->pBeHookAddr,
                                (char *) info->pBeHookAddr + info->backUpLength, 0);
    }*/

//    cacheflush(*((uint32_t*)(info->pBeHookAddr)), info->backUpLength, 0);//测试也无法刷新缓存
//    cacheflush(PAGE_START((uintptr_t)info->pBeHookAddr), PAGE_SIZE, 0);//测试也无法刷新缓存
//    cacheflush(info->pBeHookAddr, info->backUpLength, 0);//错误
}
bool unHk(HK_INFO* info) {
    if (!ChangePageProperty(info->pBeHookAddr, info->backUpLength)) {
        LOGI("change page property error.");
        return false;
    }
    if(TEST_BIT0((uint32_t)info->pBeHookAddr)){
        memcpy(info->pBeHookAddr - 1, info->szbyBackupOpcodes, info->backUpLength);
    } else {
        memcpy(info->pBeHookAddr, info->szbyBackupOpcodes, info->backUpLength);
    }
    clear_cache(info);

    free(info->pStubShellCodeAddr);
//    free(info->pOriFuncAddr);//如果和pStubShellCodeAddr共用内存页面，不能free
    info->pStubShellCodeAddr = NULL;
    info->pOriFuncAddr = NULL;

    return true;
}