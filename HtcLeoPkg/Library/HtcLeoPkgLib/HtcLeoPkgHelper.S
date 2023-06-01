#
#  Copyright (c) 2018, Linaro Limited. All rights reserved.
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#

#include <AsmMacroIoLib.h>
#include <Library/ArmLib.h>

#.text
#.align 3
#
#ASM_FUNC(ArmPlatformPeiBootAction)
#  // check if we're located at expected location
#  adr r4, .
#  ldr r5, =ArmPlatformPeiBootAction
#  cmp r4, r5
#  bne docopy
#  bx lr
#docopy:
#  // find our start address by getting our expected offset, then subtracting it from our actual address
#  ldr r6, =0xBE000000
#  sub r5, r5, r6 // r5 now holds offset of ArmPlatformPeiBootAction from start of FD base
#  sub r4, r4, r5 // r4 now holds address of actual FD base
#
#  // tweak the return address
#  sub r14, r14, r4
#  add r14, r14, r6
#  ldr r5, =FixedPcdGet64 (PcdFdSize)
#
#  // crap memcpy
#loop:
#  ldr r3, [r4], #16       //Load FD Address into r3 probably with a 16bit offset?
#  str r3, [r6], #16       //store into r3 the value from r6, memcpy most likely lol
#  subs r5, r5, #16         //substract 16bits from r5?
#  bne  loop                //branch greater
#
#
#ASM_FUNC(ArmPlatformIsPrimaryCore)
#  MOV32  (r1, FixedPcdGet32 (PcdArmPrimaryCoreMask))
#  and   r0, r0, r1
#  MOV32  (r1, FixedPcdGet32 (PcdArmPrimaryCore))
#  cmp   r0, r1
#  moveq r0, #1
#  movne r0, #0
#  bx    lr
#  
#ASM_FUNC(ArmPlatformGetPrimaryCoreMpId)
#  MOV32  (r0, FixedPcdGet32 (PcdArmPrimaryCore))
#  bx    lr
#
#ASM_FUNC(ArmPlatformGetCorePosition)
#  and   r1, r0, #ARM_CORE_MASK
#  and   r0, r0, #ARM_CLUSTER_MASK
#  add   r0, r1, r0, LSR #7
#  bx    lr

ASM_FUNC(ArmPlatformPeiBootAction)
  bx    lr

//UINTN
//ArmPlatformGetCorePosition (
//  IN UINTN MpId
//  );
ASM_FUNC(ArmPlatformGetCorePosition)
  and   r1, r0, #ARM_CORE_MASK
  and   r0, r0, #ARM_CLUSTER_MASK
  add   r0, r1, r0, LSR #7
  bx    lr

//UINTN
//ArmPlatformGetPrimaryCoreMpId (
//  VOID
//  );
ASM_FUNC(ArmPlatformGetPrimaryCoreMpId)
  MOV32  (r0, FixedPcdGet32 (PcdArmPrimaryCore))
  bx    lr

//UINTN
//ArmPlatformIsPrimaryCore (
//  IN UINTN MpId
//  );
ASM_FUNC(ArmPlatformIsPrimaryCore)
  MOV32  (r1, FixedPcdGet32 (PcdArmPrimaryCoreMask))
  and   r0, r0, r1
  MOV32  (r1, FixedPcdGet32 (PcdArmPrimaryCore))
  cmp   r0, r1
  moveq r0, #1
  movne r0, #0
  bx    lr