/** @file

  Copyright (c) 2011 - 2013, ARM Limited. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "PrePi.h"

static void SetVectorBase(unsigned long addr)
{
	__asm__ volatile("mcr	p15, 0, %0, c12, c0, 0" :: "r" (addr));
}

VOID
ArchEarlyInit()
{
  /* turn off the cache */
	ArchDisableCache(UCACHE);

  /* set the vector base to our exception vectors so we dont need to double map at 0 */
	SetVectorBase(MEMBASE);

  /* turn the cache back on */
  ArchEnableCache(UCACHE);
}

VOID
ArchInitialize (
  VOID
  )
{
  // Enable program flow prediction, if supported.
  ArmEnableBranchPrediction ();

  if (FixedPcdGet32 (PcdVFPEnabled)) {
    ArmEnableVFP ();
  }
}
