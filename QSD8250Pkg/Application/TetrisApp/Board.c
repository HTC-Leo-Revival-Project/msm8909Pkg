#include "Board.h"

BOOLEAN isMovePossible( Board* this, int x, int y );
void markFieldsOccupiedByPiece( Board* this, UINT8 fieldState );
void drawFieldsOccupiedByPiece( Board* this, UINT8 color );
void redrawField( Board* this, int x, int y, UINT8 color );
void drawNextUp( Board* this );
void placePiece( Board* this );
BOOLEAN isGameOver( Board* this );
void checkAndRemoveLines( Board* this );
void loadNextPiece( Board* this );
void drawStaticElements( void );



void drawBoard( Board* this ) {
	drawNextUp( this );

	setTextColor( EFI_LIGHTGRAY );
	setCursorPos( NEXT_PIECE_FRAME_TOP_X + 8, NEXT_PIECE_FRAME_TOP_Y + NEXT_PIECE_FRAME_HEIGHT + 4 );
	Print( L"%d ", this->level );
	setCursorPos( NEXT_PIECE_FRAME_TOP_X + 8, NEXT_PIECE_FRAME_TOP_Y + NEXT_PIECE_FRAME_HEIGHT + 5 );
	Print( L"%d", this->lines );
	setCursorPos( NEXT_PIECE_FRAME_TOP_X + 8, NEXT_PIECE_FRAME_TOP_Y + NEXT_PIECE_FRAME_HEIGHT + 6 );
	Print( L"%d", this->score );
}



void rotatePiece( Board* this ) {
	markFieldsOccupiedByPiece( this, EMPTY_FIELD );

	this->activePiece->rotateCW( this->activePiece );
	if ( !isMovePossible( this, 0, 0 ) ) {
		this->activePiece->rotateCCW( this->activePiece );
		markFieldsOccupiedByPiece( this, this->activePiece->color );
	}
	else {
		this->activePiece->rotateCCW( this->activePiece );
		drawFieldsOccupiedByPiece( this, EFI_BLACK );
		this->activePiece->rotateCW( this->activePiece );
		drawFieldsOccupiedByPiece( this, this->activePiece->color );
	}
}



void movePieceLeft( Board* this ) {
	markFieldsOccupiedByPiece( this, EMPTY_FIELD );

	if ( isMovePossible( this, -1, 0 ) ) {
		drawFieldsOccupiedByPiece( this, EFI_BLACK );
		this->activePiece->pos.x--;
		drawFieldsOccupiedByPiece( this, this->activePiece->color );
	}

	markFieldsOccupiedByPiece( this, this->activePiece->color );
}



void movePieceRight( Board* this ) {
	markFieldsOccupiedByPiece( this, EMPTY_FIELD );

	if ( isMovePossible( this, 1, 0 ) ) {
		drawFieldsOccupiedByPiece( this, EFI_BLACK );
		this->activePiece->pos.x++;
		drawFieldsOccupiedByPiece( this, this->activePiece->color );
	}

	markFieldsOccupiedByPiece( this, this->activePiece->color );
}



BOOLEAN movePieceDown( Board* this ) {
	markFieldsOccupiedByPiece( this, EMPTY_FIELD );

	if ( isMovePossible( this, 0, 1 ) ) {
		drawFieldsOccupiedByPiece( this, EFI_BLACK );
		this->activePiece->pos.y++;
		drawFieldsOccupiedByPiece( this, this->activePiece->color );
		return TRUE;
	}
	else {
		markFieldsOccupiedByPiece( this, this->activePiece->color );
		placePiece( this );
		return FALSE;
	}
}



void dropPiece( Board* this ) {
	int height = 0;
	while ( movePieceDown( this ) ) {
		height++;
	};

	this->score += DROP_BONUS_SCORE_PER_LINE * height;
}



void ConstructBoard( Board** this ) {
	Board* board = AllocatePool( sizeof( Board ) );
	ZeroMem( board, sizeof( Board ) );

	board->blocked = FALSE;
	board->level = 1;
	board->lines = 0;
	board->score = 0;
	board->drawBoard = drawBoard;
	board->rotatePiece = rotatePiece;
	board->movePieceDown = movePieceDown;
	board->movePieceLeft = movePieceLeft;
	board->movePieceRight = movePieceRight;
	board->dropPiece = dropPiece;

	ConstructPiece( &board->nextPiece );
	ConstructPiece( &board->activePiece );

	loadNextPiece( board );

	drawStaticElements();

	*this = board;
}



void DestructBoard( Board* this ) {
	DestructPiece( this->nextPiece );
	DestructPiece( this->activePiece );
	FreePool( this );
}



BOOLEAN isMovePossible( Board* this, int x, int y ) {
	int i, fx, fy;

	for ( i = 0; i < 4; ++i ) {
		fx = ( *this->activePiece->body )[ i ].x + this->activePiece->pos.x + x;
		fy = ( *this->activePiece->body )[ i ].y + this->activePiece->pos.y + y;

		if ( fx < 0 || fx >= BOARD_WIDTH ||
			fy < 0 || fy >= BOARD_HEIGHT ||
			this->fields[ fy ][ fx ] != EMPTY_FIELD ) {
			return FALSE;
		}
	}

	return TRUE;
}



void markFieldsOccupiedByPiece( Board* this, UINT8 color ) {
	int i, x, y;

	for ( i = 0; i < 4; ++i ) {
		x = ( *this->activePiece->body )[ i ].x + this->activePiece->pos.x;
		y = ( *this->activePiece->body )[ i ].y + this->activePiece->pos.y;
		this->fields[ y ][ x ] = color;
	}
}



void drawFieldsOccupiedByPiece( Board* this, UINT8 color ) {
	int i, x, y;

	for ( i = 0; i < 4; ++i ) {
		x = ( *this->activePiece->body )[ i ].x + this->activePiece->pos.x;
		y = ( *this->activePiece->body )[ i ].y + this->activePiece->pos.y;
		redrawField( this, x, y, color );
	}
}



void redrawField( Board* this, int x, int y, UINT8 color ) {
	ASSERT( x >= 0 && x < BOARD_WIDTH );
	ASSERT( y >= 0 && y < BOARD_HEIGHT );

	if ( y >= INVISIBLE_ROWS_COUNT ) {
		this->fields[ y ][ x ] = color;
		setTextColor( color );
		putcharx( BOARD_TOP_X + 1 + 2 * x, BOARD_TOP_Y - INVISIBLE_ROWS_COUNT + y, BLOCKELEMENT_FULL_BLOCK );
		putcharx( BOARD_TOP_X + 1 + 2 * x + 1, BOARD_TOP_Y - INVISIBLE_ROWS_COUNT + y, BLOCKELEMENT_FULL_BLOCK );
	}
}



void drawNextUp( Board* this ) {
	int i, j, margin;
	static UINT8 previousPieceColor = 0;

	if ( this->nextPiece->color != previousPieceColor ) {
		previousPieceColor = this->nextPiece->color;

		setTextColor( EFI_BLACK );
		for ( i = 1; i <= NEXT_PIECE_FRAME_WIDTH; ++i ) {
			for ( j = 1; j <= NEXT_PIECE_FRAME_HEIGHT; ++j ) {
				putcharx( NEXT_PIECE_FRAME_TOP_X + i, NEXT_PIECE_FRAME_TOP_Y + j, L' ' );
			}
		}

		setTextColor( this->nextPiece->color );

		if ( this->nextPiece->color == TETRIMINO_I_COLOR || this->nextPiece->color == TETRIMINO_O_COLOR ) { // color may be treated as piece's unique identifier
			margin = 1;
		}
		else {
			margin = 2;
		}

		for ( i = 0; i < 4; ++i ) {
			putcharx( NEXT_PIECE_FRAME_TOP_X + margin + 2 * ( *this->nextPiece->body )[ i ].x, NEXT_PIECE_FRAME_TOP_Y + 1 + ( *this->nextPiece->body )[ i ].y, BLOCKELEMENT_FULL_BLOCK );
			putcharx( NEXT_PIECE_FRAME_TOP_X + margin + 2 * ( *this->nextPiece->body )[ i ].x + 1, NEXT_PIECE_FRAME_TOP_Y + 1 + ( *this->nextPiece->body )[ i ].y, BLOCKELEMENT_FULL_BLOCK );
		}
	}
}



void placePiece( Board* this ) {
	if ( isGameOver( this ) ) {
		this->blocked = TRUE;
	}
	else {
		checkAndRemoveLines( this );
		loadNextPiece( this );
	}
}



BOOLEAN isGameOver( Board* this ) {
	int i;
	for ( i = 0; i < 4; ++i ) {
		if ( ( *this->activePiece->body )[ i ].y + this->activePiece->pos.y < INVISIBLE_ROWS_COUNT ) {
			return TRUE;
		}
	}
	return FALSE;
}



void checkAndRemoveLines( Board* this ) {
	int i, j, k;
	int counter = 0;
	BOOLEAN lineWasFull;

	for ( i = BOARD_HEIGHT - 1; i >= INVISIBLE_ROWS_COUNT; --i ) {
		lineWasFull = TRUE;

		for ( j = 0; j < BOARD_WIDTH; ++j ) {
			if ( this->fields[ i ][ j ] == EMPTY_FIELD ) {
				lineWasFull = FALSE;
				break;
			}
		}

		if ( lineWasFull ) {
			for ( j = 0; j < BOARD_WIDTH; ++j ) {
				redrawField( this, j, i, EFI_BLACK );
			}

			for ( j = i; j > 0; --j ) {
				for ( k = 0; k < BOARD_WIDTH; ++k ) {
					redrawField( this, k, j, this->fields[ j - 1 ][ k ] );
				}
			}

			i++;
			counter++;
		}
	}

	this->lines += counter;

	if ( counter != 0 ) {
		this->score += LINE_CLEAR_SCORE << ( counter - 1 );
		this->score += LINE_CLEAR_LEVEL_BONUS_SCORE * this->level;
	}
}



void loadNextPiece( Board* this ) {
	Point startingPos = PIECE_STARTING_POS;

	Piece* tmp = this->activePiece;
	this->activePiece = this->nextPiece;
	this->nextPiece = tmp;

	this->nextPiece->randomize( this->nextPiece );
	this->activePiece->pos = startingPos;
}



void drawStaticElements( void ) {
	int i;

	setTextColor( EFI_LIGHTGRAY );
	setCursorPos( 0, 8 );
	Print( STATIC_TEXT );

	setCursorPos( NEXT_PIECE_FRAME_TOP_X + NEXT_PIECE_FRAME_WIDTH / 2 - 3, NEXT_PIECE_FRAME_TOP_Y + NEXT_PIECE_FRAME_HEIGHT + 2 );
	Print( L"Next Up" );
	setCursorPos( NEXT_PIECE_FRAME_TOP_X, NEXT_PIECE_FRAME_TOP_Y + NEXT_PIECE_FRAME_HEIGHT + 4 );
	Print( L"Level :" );
	setCursorPos( NEXT_PIECE_FRAME_TOP_X, NEXT_PIECE_FRAME_TOP_Y + NEXT_PIECE_FRAME_HEIGHT + 5 );
	Print( L"Lines : " );
	setCursorPos( NEXT_PIECE_FRAME_TOP_X, NEXT_PIECE_FRAME_TOP_Y + NEXT_PIECE_FRAME_HEIGHT + 6 );
	Print( L"Score : " );

	setTextColor( EFI_RED );

	// next piece frame
	for ( i = 1; i <= NEXT_PIECE_FRAME_WIDTH; i++ ) {
		putcharx( NEXT_PIECE_FRAME_TOP_X + i, NEXT_PIECE_FRAME_TOP_Y, BOXDRAW_DOUBLE_HORIZONTAL );
		putcharx( NEXT_PIECE_FRAME_TOP_X + i, NEXT_PIECE_FRAME_TOP_Y + NEXT_PIECE_FRAME_HEIGHT + 1, BOXDRAW_DOUBLE_HORIZONTAL );
	}

	for ( i = 1; i <= NEXT_PIECE_FRAME_HEIGHT; i++ ) {
		putcharx( NEXT_PIECE_FRAME_TOP_X, NEXT_PIECE_FRAME_TOP_Y + i, BOXDRAW_DOUBLE_VERTICAL );
		putcharx( NEXT_PIECE_FRAME_TOP_X + NEXT_PIECE_FRAME_WIDTH + 1, NEXT_PIECE_FRAME_TOP_Y + i, BOXDRAW_DOUBLE_VERTICAL );
	}

	putcharx( NEXT_PIECE_FRAME_TOP_X, NEXT_PIECE_FRAME_TOP_Y, BOXDRAW_DOUBLE_DOWN_RIGHT );
	putcharx( NEXT_PIECE_FRAME_TOP_X + NEXT_PIECE_FRAME_WIDTH + 1, NEXT_PIECE_FRAME_TOP_Y, BOXDRAW_DOUBLE_DOWN_LEFT );
	putcharx( NEXT_PIECE_FRAME_TOP_X + NEXT_PIECE_FRAME_WIDTH + 1, NEXT_PIECE_FRAME_TOP_Y + NEXT_PIECE_FRAME_HEIGHT + 1, BOXDRAW_DOUBLE_UP_LEFT );
	putcharx( NEXT_PIECE_FRAME_TOP_X, NEXT_PIECE_FRAME_TOP_Y + NEXT_PIECE_FRAME_HEIGHT + 1, BOXDRAW_DOUBLE_UP_RIGHT );



	// board frame
	for ( i = 0; i < 2 * BOARD_WIDTH + 2; i++ ) {
		putcharx( BOARD_TOP_X + i, BOARD_TOP_Y - 1, BOXDRAW_DOUBLE_HORIZONTAL );
		putcharx( BOARD_TOP_X + i, BOARD_TOP_Y + BOARD_HEIGHT - INVISIBLE_ROWS_COUNT, BOXDRAW_DOUBLE_HORIZONTAL );
	}

	for ( i = 0; i < BOARD_HEIGHT - INVISIBLE_ROWS_COUNT; i++ ) {
		putcharx( BOARD_TOP_X, BOARD_TOP_Y + i, BOXDRAW_DOUBLE_VERTICAL );
		putcharx( BOARD_TOP_X + 2 * BOARD_WIDTH + 1, BOARD_TOP_Y + i, BOXDRAW_DOUBLE_VERTICAL );
	}

	putcharx( BOARD_TOP_X, BOARD_TOP_Y - 1, BOXDRAW_DOUBLE_DOWN_RIGHT );
	putcharx( BOARD_TOP_X + 2 * BOARD_WIDTH + 1, BOARD_TOP_Y - 1, BOXDRAW_DOUBLE_DOWN_LEFT );
	putcharx( BOARD_TOP_X, BOARD_TOP_Y + BOARD_HEIGHT - INVISIBLE_ROWS_COUNT, BOXDRAW_DOUBLE_UP_RIGHT );
	putcharx( BOARD_TOP_X + 2 * BOARD_WIDTH + 1, BOARD_TOP_Y + BOARD_HEIGHT - INVISIBLE_ROWS_COUNT, BOXDRAW_DOUBLE_UP_LEFT );
}