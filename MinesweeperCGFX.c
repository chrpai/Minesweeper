	/*
	Minesweeper for CNet
    Copyright (C) 2026 Christopher Painter
	This file is part of the Minesweeper project.
	SPDX-License-Identifier: MIT
*/

#define bool int
#define false 0
#define true 1
#define WIDTH 38
#define HEIGHT 15
#define ULS 0xC9
#define URS 0xBB
#define LLS 0xC8
#define LRS 0xBC
#define HLS 0xCD
#define VLS 0xBA
#define SQS 0xFE

void Minesweeper(void);
void Help(void);
void PlayLoop(void);
void InitializeBoard(void);
void PaintBoard(void);
bool PlayGame(void); 
void UpdateRow(int y);
void UpdateSquare(int x, int y, char color, unsigned char symbol);
void GameOver(char color, unsigned char symbol);
void ClearSquare(int x, int y);
bool OutOfBounds(int x, int y);
int CountAdjacentMines(int x, int y);
bool ClearAdjacentSquares(int x, int y);
void UpdateRow(int y);
//void ReturnCursor(void);
void CallHost( UBYTE c );
void ShutDown( char *spawn );
 
/* C-Net communication GLOBALS		*/
struct MsgPort  *replyp;
struct CPort    *cport;
struct CMessage  cmess;
struct	MainPort  *myp;
struct	PortData  *z;
char	**bm;
struct	Library *CNetBase = NULL;
struct	SignalSemaphore *SEM;
char *RexxGetUser( char *arg );

/* Minesweeper Globals		*/
int board[WIDTH][HEIGHT];
int revealed[WIDTH][HEIGHT];

int mines = 10;

void main( int argc, char **argv )
{

	if( argc<2 || !(cport = (struct CPort *)FindPort( argv[1] )) ) 
	{
		printf("This is a CNet C program.\n");
		exit(0);
	}

	if( !(replyp = CreatePort( 0,0 )))
	{
		exit(0);
	}

	cmess.cn_Message.mn_ReplyPort   = replyp;
	cmess.cn_Message.mn_Length      = sizeof( struct CMessage );
	cmess.cn_Message.mn_Node.ln_Name= "cstuff";

	if( cport->ack != 30 ) {	/* right CNet version running? */
	    cport->ack = 1;
	    goto err;
	}

	cport->ack = 0;
	z		=	cport->zp;
	myp		=	cport->myp;
	SEM		=	myp->SEM;
	bm		=	z->bm;

	if( !(CNetBase = OpenLibrary( "cnet.library",0 )) )
	{
		goto err0;
	}

	Minesweeper();

	err0:	ShutDown( NULL );
	err:	DeletePort( replyp );

	if( CNetBase )
	{
		CloseLibrary( CNetBase );
	}
	exit(0);
}

void Minesweeper(void)
{
	PlayLoop();
}	

void Help(void)
{
	char input;
	PutText("f1cf            Minesweeper Helpn2");
	PutText("cbObjective: caClear the minefield without detonating any mines.n2");
	PutText("cbControls:n2");
	PutText("cfARROW KEYSc6=ceMove Cursor cfSPACEc6=ceRevealc6 cfHc6=ceHelp cfQc6=ceQuitn2");
	PutText("cbGameplay:n2");
	PutText("c7When you reveal a square, it will either be empty (no adjacent mines),\n");
	PutText("contain a number (indicating how many adjacent mines there are), or\n");
	PutText("be a mine (which ends the game).n2");
	PutText("c2If you reveal an empty square, all adjacent empty squares will also ben1");
	PutText("revealed automatically, along with any numbered squares bordering them.n2");
	PutText("cePress any key to return to the game.");
	input = OneKey();
}

void PlayLoop(void)
{
	bool playmore;
	char input;
	bool ask;

	playmore = true;
	while(playmore)
	{	
	    char buffer[16]; 
		sprintf(buffer, "Mines Lvl %d", mines );
		SetDoing(buffer);
		InitializeBoard();
		playmore = false;
		ask = PlayGame();
		if (ask)
		{
			PutText("ce Play again? (y/N)? ");
			input = OneKey();
			if(input != 'y' && input != 'Y')
			{
				PutText("cfNo");
				playmore = false;
			}
			else
			{
				PutText("cfYes");
			}
		}
		else
		{
			playmore = false;
		}
	}
}

void InitializeBoard()
{
	int mines_placed, mineplaced, rand_x, rand_y, x, y;
	srand((unsigned)time(NULL));

	/* Initialize board arrays */
	for (y = 0; y < HEIGHT; y++) 
	{
		for (x = 0; x < WIDTH; x++) 
		{
			board[x][y] = 0; /* Unrevealed */
			revealed[x][y] = 0;	/* Not revealed */
		}
	}

	/* Place mines randomly */
	mines_placed = 0;
	while (mines_placed < mines) 
	{
		mineplaced = 0;
		while(mineplaced == 0)
		{
			rand_x = rand() % WIDTH ;
			rand_y = rand() % HEIGHT ;
			if (board[rand_x][rand_y] != 9) 
			{
				board[rand_x][rand_y] = 9; /* Place mine */
				mineplaced = 1;
			}
		}
		mines_placed++;
	}
}

void PaintBoard(void)
{
	 PutText("*64 PFILES:C/gameboard.seq}f0}!2}>1}cf");
}

//	/* Initialize board arrays */
//	for (y = 0; y < HEIGHT; y++) 
//	{
//		UpdateRow(y);
//	}
//}


bool PlayGame(void)
{
	int x,xx, y ,yy, unrevealedcount, reason;
	char input;
	bool leave;
	bool ask;

	ask = true;
	x=0;
	y=0;
	leave=false;
	reason = 0;

	PaintBoard();

	while(!leave)
	{
		unrevealedcount=0;
		/* Check Win Condition */
		for (yy = 0; yy < HEIGHT; yy++)
		{
			for (xx = 0; xx < WIDTH; xx++)
			{
				if(revealed[xx][yy]==0)
				{
					unrevealedcount++;
				}
			}
		}
		if (unrevealedcount <= mines)
		{
			GameOver('2','*');
			leave=true;
			reason=0;
			break;
		}

		if(revealed[x][y]==0)
		{
			//UpdateSquare(x,y,'f', SQS);
		}
		else if(revealed[x][y]==70)
		{
			UpdateSquare(x,y,'9','?');
		}
		
		input = OneKey();
		switch(input)
		{
			case 72:
				Help();
				PaintBoard();
				break;
			case 81:
				PutText("f1cbAre you sure you wish to quit you yellow bellied coward? (y/N)");
				input = OneKey();
				if(input=='y' || input=='Y')
				{
					leave = true;
					reason = 1;
				}
				else
				{
					PaintBoard();
				}
				break;
			default:
				break;	
		}	
	}	
	/* Game over */
	/* reason*/
	switch(reason)
	{
		case 0: /* Win */
			PutText("cbCongratulations! caYou cleared the minefield!");
			if(mines<=100)
			{
				mines += 10;
			}
			break;
		case -1: /* Lose */
			PutText("c9BOOM! cbYour body parts are everywhere.");
			if(mines>20)
			{
				mines -= 10;
			}
			break;
		case 1: /* Quit */
			ask = false;
			break;
	}
	return ask;
}


void ClearSquare(int x, int y)
{
	int count;
	char character;
	
	if(OutOfBounds(x,y)) return;
	if(revealed[x][y]==1) return;
	count = CountAdjacentMines(x,y);
	if(count>0)
	{
		// convert the count to an ascii character
		character = (char)(count + '0');
	}
	else
	{
		character = ' ';
	}
	board[x][y] = count;
	revealed[x][y]=1;
	UpdateSquare(x,y,'f',character);

	if(count==0)
	{
		ClearSquare(x-1,y-1);
		ClearSquare(x,y-1);
		ClearSquare(x+1,y-1);
		ClearSquare(x-1,y);
		ClearSquare(x+1,y);
		ClearSquare(x-1,y+1);
		ClearSquare(x,y+1);
		ClearSquare(x+1,y+1);
	}	
}

bool OutOfBounds(int x, int y)
{
	return (x<0 || x>=WIDTH || y<0 || y>=HEIGHT);
}

void GameOver(char color, unsigned char symbol)
{
	// reveal all mines
	int x,y;
	for (y = 0; y < HEIGHT; y++) 
	{
		for (x = 0; x < WIDTH; x++) 
		{
			if(board[x][y]==9)
			{
				UpdateSquare(x,y,color,symbol);
			}
		}
	}
}

/* Update a single square on the board */
void UpdateSquare(int x, int y, char color, unsigned char symbol)
{
	char buffer[16]; /* Reusable buffer for PutText */
	sprintf(buffer, "c%c%c", y+3, (x*2)+3, color, symbol);
	PutText(buffer);
}

int CountAdjacentMines(int x, int y)
{
	int count, ymin, ymax, xmin, xmax, dy, dx;
	count =0;

	ymin = y-1;
	ymax = y+1;
	xmin = x-1;
	xmax = x+1;

	for(dy=ymin; dy<=ymax; dy++)
	{
 		for(dx=xmin; dx<=xmax; dx++)
		{
			if(dx==x && dy==y) 
			{
				continue;
			}
			if(dx>=0 && dx<WIDTH && dy>=0 && dy<HEIGHT)
			{
				if(board[dx][dy]==9)
				{
					count++;
				}
			}
		}
	}
	return count;
}



void UpdateRow(int y)
{
	int x, val;
	char character;
		
	for(x=0; x<WIDTH; x++)
	{
		switch (revealed[x][y])
		{
			case 0:
				UpdateSquare(x,y,'6',SQS);
				break;
			case 70:
				UpdateSquare(x,y,'1','F');
				break;
			default:
			val = board[x][y];
			if(val ==0) character = ' ';
			else character = (char)(val + '0');
			UpdateSquare(x,y,'f', character);
			break;
		}
	}
}	

/*
	C-Net communication functions
*/

void ShutDown( char *spawn )
{
	if( spawn )
	{
		strcpy( z->CSpawn, spawn );
	}
	CallHost( 0 );
}


void CallHost( UBYTE c )
{
	cmess.command = c;
	PutMsg  ( (struct MsgPort *)cport, (struct Message *)&cmess );
	WaitPort( replyp );
	GetMsg  ( replyp );
}

void SetDoing( char *what )
{
	cmess.arg1 = (ULONG)what;
	CallHost( 7 );
}

void PutText( char *text )
{
	cmess.arg1 = (ULONG)text;
	CallHost( 1 );
}

char OneKey( void )
{
	CallHost( 3 );
	return  (char)cmess.result;
}

