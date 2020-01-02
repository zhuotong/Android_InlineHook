#ifndef _FIXOPCODE_H
#define _FIXOPCODE_H

#include <stdio.h>
#include "mhk.h"

#define ALIGN_PC(pc)	(pc & 0xFFFFFFFC)

bool isThumb32(uint16_t opcode);
bool isTargetAddrInBackup(uint32_t target_addr, uint32_t hook_addr, int backup_length);

int lengthFixThumb32(uint32_t opcode);
int lengthFixArm32(uint32_t opcode);
int lengthFixThumb16(uint16_t opcode);

static int getTypeInArm32(uint32_t instruction);
static int getTypeInThumb16(uint16_t instruction);
static int getTypeInThumb32(uint32_t instruction);


int fixPCOpcodeArm(void *fixOpcodes , HK_INFO* info);
int fixPCOpcodeThumb(void *fixOpcodes , HK_INFO* info);
int fixPCOpcodeArm32(uint32_t pc, uint32_t lr, uint32_t instruction, uint32_t *trampoline_instructions, HK_INFO* info);
int fixPCOpcodeThumb16(uint32_t pc, uint16_t instruction, uint16_t *trampoline_instructions, HK_INFO* info);
int fixPCOpcodeThumb32(uint32_t pc, uint16_t high_instruction, uint16_t low_instruction, uint16_t *trampoline_instructions, HK_INFO* info);

#endif