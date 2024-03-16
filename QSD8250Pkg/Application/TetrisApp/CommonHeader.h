#pragma once

#include <Uefi.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>


typedef struct {
	int x, y;
} Point;

void putcharx( IN int x, IN int y, IN CHAR16 ch );
BOOLEAN isAlphanumeric( IN CHAR16 ch );
void setCursorPos( int x, int y );
void setTextColor( UINT8 color );