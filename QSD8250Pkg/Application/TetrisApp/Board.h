#pragma once

#include "Piece.h"

#define BOARD_HEIGHT 22
#define INVISIBLE_ROWS_COUNT 2
#define BOARD_WIDTH  10
#define BOARD_TOP_X  25
#define BOARD_TOP_Y   3

#define NEXT_PIECE_FRAME_HEIGHT 2
#define NEXT_PIECE_FRAME_WIDTH 8
#define NEXT_PIECE_FRAME_TOP_X BOARD_TOP_X + 2 * BOARD_WIDTH + 4
#define NEXT_PIECE_FRAME_TOP_Y BOARD_TOP_Y + 1

#define PIECE_STARTING_POS { 4, 0 };
#define EMPTY_FIELD EFI_BLACK

#define DROP_BONUS_SCORE_PER_LINE 2
#define LINE_CLEAR_SCORE 100
#define LINE_CLEAR_LEVEL_BONUS_SCORE 50

#define STATIC_TEXT L"\
   CONTROLS\r\n\
   Vol Up - Move left\r\n\
   Vol Down - Move right\r\n\
   Windows - Rotate piece\r\n\
   Home - Drop piece\r\n\
   Dial - Move down\r\n\
   Back - Pause game\r\n\
   Power on - Exit game\r\n"

typedef struct _Board Board;

typedef void( *DRAW_BOARD )( IN Board* this );
typedef void( *ROTATE_ACTIVE_PIECE )( IN Board* this );
typedef void( *MOVE_PIECE_LEFT ) ( IN Board* this );
typedef void( *MOVE_PIECE_RIGHT ) ( IN Board* this );
typedef BOOLEAN( *MOVE_PIECE_DOWN ) ( IN Board* this );
typedef void( *DROP_PIECE ) ( IN Board* this );

struct _Board {
	Piece* nextPiece;
	Piece* activePiece;
	UINT8 fields[ BOARD_HEIGHT ][ BOARD_WIDTH ];
	BOOLEAN blocked;
	int level;
	int lines;
	int score;

	DRAW_BOARD drawBoard;
	ROTATE_ACTIVE_PIECE rotatePiece;
	MOVE_PIECE_LEFT movePieceLeft;
	MOVE_PIECE_RIGHT movePieceRight;
	MOVE_PIECE_DOWN movePieceDown;
	DROP_PIECE dropPiece;
};

void ConstructBoard( OUT Board** this );
void DestructBoard( IN Board* this );