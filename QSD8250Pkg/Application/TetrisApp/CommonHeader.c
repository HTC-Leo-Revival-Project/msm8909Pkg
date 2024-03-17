#include "CommonHeader.h"

void putcharx( int x, int y, CHAR16 ch ) {
	CHAR16 chartab[ 2 ] = { 0 };
	chartab[ 0 ] = ch;

	setCursorPos( x, y );
	Print( chartab );
}



BOOLEAN isAlphanumeric( CHAR16 ch ) {
	return (ch >= 'a' && ch <= 'z') ||
		(ch >= 'A' && ch <= 'Z') ||
		(ch >= '0' && ch <= '9');
}



void setCursorPos( int x, int y ) {
	gST->ConOut->SetCursorPosition( gST->ConOut, x, y );
}



void setTextColor( UINT8 color ) {
	gST->ConOut->SetAttribute( gST->ConOut, EFI_TEXT_ATTR( color, EFI_BLACK ) );
}