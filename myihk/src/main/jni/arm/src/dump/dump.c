#include "mhk.h"
#include "../fixPCOpcode.h"


extern unsigned long _dump_start;
extern unsigned long _hk_info;
extern unsigned long _oriFuc;
extern unsigned long _dump_end;

/**
 * 利用dump.s中的shellcode构造桩，跳转到pInfo->onPreCallBack函数后，回调老函数
 * @param  pInfo inlinehook信息
 * @return               inlinehook桩是否构造成功
 */
bool BuildStub_dump(HK_INFO* pInfo)
{
    bool bRet = false;
    LE("BuildStub_dump");
    while(1)
    {
        if(pInfo == NULL)
        {
            LOGI("pstInlineHook is null");
            break;
        }
        
        void *p_shellcode_start_s = &_dump_start;
        void *p_shellcode_end_s = &_dump_end;
        void *p_hk_info = &_hk_info;
        void *p_oriFuc = &_oriFuc;

        size_t sShellCodeLength = p_shellcode_end_s - p_shellcode_start_s;
        pInfo->shellCodeLength = sShellCodeLength;
        //malloc一段新的stub代码
        long pagesize = sysconf(_SC_PAGE_SIZE);
        void *pNewShellCode = NULL;
//        pNewShellCode = malloc(sShellCodeLength);
        int code = 0;
        code = posix_memalign(&pNewShellCode, pagesize, pagesize);

        LE("pNewShellCode=%p", pNewShellCode);
        if(code || pNewShellCode == NULL)
        {
            LOGI("shell code malloc fail.");
            break;
        }
        memcpy(pNewShellCode, p_shellcode_start_s, sShellCodeLength);
        //更改stub代码页属性，改成可读可写可执行
        if(ChangePageProperty(pNewShellCode, sShellCodeLength) == false)
        {
            LOGI("change shell code page property fail.");
            break;
        }


        pInfo->hk_infoAddr = pNewShellCode + (p_hk_info - p_shellcode_start_s);
        *(pInfo->hk_infoAddr) = pInfo;
            
        //填充shellcode地址到hookinfo中，用于构造hook点位置的跳转指令
        pInfo->pStubShellCodeAddr = pNewShellCode;

        pInfo->ppOriFuncAddr = pNewShellCode + (p_oriFuc - p_shellcode_start_s);
        

        bRet = true;
        break;
    }
    
    return bRet;
}






