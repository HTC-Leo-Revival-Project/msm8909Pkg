#include <Library/UefiRuntimeServicesTableLib.h>
#include "Piece.h"

unsigned rand( void );



void randomize( Piece* this ) {
	static Bodies tetrimino_I = TETRIMINO_I_BODY;
	static Bodies tetrimino_J = TETRIMINO_J_BODY;
	static Bodies tetrimino_L = TETRIMINO_L_BODY;
	static Bodies tetrimino_O = TETRIMINO_O_BODY;
	static Bodies tetrimino_S = TETRIMINO_S_BODY;
	static Bodies tetrimino_T = TETRIMINO_T_BODY;
	static Bodies tetrimino_Z = TETRIMINO_Z_BODY;
	Bodies* target = NULL;

	switch ( rand() % 7 ) {
		case 0:
			target = &tetrimino_I;
			this->color = TETRIMINO_I_COLOR;
			break;
		case 1:
			target = &tetrimino_J;
			this->color = TETRIMINO_J_COLOR;
			break;
		case 2:
			target = &tetrimino_L;
			this->color = TETRIMINO_L_COLOR;
			break;
		case 3:
			target = &tetrimino_O;
			this->color = TETRIMINO_O_COLOR;
			break;
		case 4:
			target = &tetrimino_S;
			this->color = TETRIMINO_S_COLOR;
			break;
		case 5:
			target = &tetrimino_T;
			this->color = TETRIMINO_T_COLOR;
			break;
		case 6:
			target = &tetrimino_Z;
			this->color = TETRIMINO_Z_COLOR;
			break;
		default:
			ASSERT( FALSE );
	}

	CopyMem( &this->bodies, target, sizeof( Bodies ) );
	this->rotation = 0;
	this->body = &this->bodies[ 0 ];
}



void rotateCW( Piece* this ) {
	this->rotation = ( this->rotation + 1 ) % 4;
	this->body = &this->bodies[ this->rotation ];
}



void rotateCCW( Piece* this ) {
	if ( this->rotation == 0 ) { // modulus operator for negative values is implementation dependant
		this->rotation = 3;      // that's why we don't use it here
	}
	else {
		this->rotation--;
	}
	this->body = &this->bodies[ this->rotation ];

}



void ConstructPiece( Piece** this ) {
	Piece* piece = AllocatePool( sizeof( Piece ) );
	ZeroMem( piece, sizeof( Piece ) );

	// seed before first actual call so we don't always get the same piece at the beggining
	// dunno why it works this way
	rand();

	piece->randomize = randomize;
	piece->rotateCW = rotateCW;
	piece->rotateCCW = rotateCCW;
	piece->pos.x = 0;
	piece->pos.y = 0;
	piece->body = &piece->bodies[ 0 ];
	randomize( piece );

	*this = piece;
}



void DestructPiece( Piece* this ) {
	FreePool( this );
}



unsigned rand( void ) {
	static int a = 69069;
	static int c = 1;
	static int M = 1 << 31;
	static unsigned previousValue;
	static BOOLEAN initialized = FALSE;

	if ( initialized == FALSE ) {
		EFI_TIME time;
		gRT->GetTime( &time, NULL );
		previousValue = time.Second;
		initialized = TRUE;
	}

	return previousValue = ( a*previousValue + c ) % M;
}