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
void InitializeLeaderboard(void);
void UpdateLeaderBoard(void);
void DisplayLeaderBoard(void);
void PaintBoard(int moves);
bool PlayGame(void); 
void UpdateRow(int y);
void UpdateMoves(int moves);
void UpdateMines(void);
void UpdateSquare(int x, int y, char color, unsigned char symbol);
void GameOver(char color, unsigned char symbol);
void ClearSquare(int x, int y);
bool OutOfBounds(int x, int y);
int CountAdjacentMines(int x, int y);
bool ClearAdjacentSquares(int x, int y);
void UpdateRow(int y);
void ReturnCursor(void);
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

/* Minesweeper Globals		*/
int board[WIDTH][HEIGHT];
int revealed[WIDTH][HEIGHT];
int moves;

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
	InitializeLeaderboard();
	PlayLoop();
	DisplayLeaderBoard();
}	

void Help(void)
{
	char input;
	PutText("f1cf                               Minesweeper Helpn2");
	PutText("cbObjective: caClear the minefield without detonating any mines.n2");
	PutText("cbControls:n2");
	PutText("cfARROW KEYSc6=ceMove Cursor cfSPACEc6=ceRevealc6 cfFc6=ceFlag/Unflag cfLc6=ceLeaderBoard cfHc6=ceHelp cfQc6=ceQuitn2");
	PutText("cbGameplay:n2");
	PutText("c7When you reveal a square, it will either be empty (no adjacent mines),\n");
	PutText("contain a number (indicating how many adjacent mines there are), or\n");
	PutText("be a mine (which ends the game).n2");
	PutText("c27If you reveal an empty square, all adjacent empty squares will also ben1");
	PutText("revealed automatically, along with any numbered squares bordering them.n2");
	PutText("cePress any key to return to the game.");
	input = OneKey();
	if(input=='') 
	{
		OneKey();
		OneKey();
	}
}

void CreateDirIfNotExist(STRPTR dirName) 
{
    BPTR lock = Lock(dirName, ACCESS_READ);
    if (lock) {
        // Directory exists, unlock it
        UnLock(lock);
        printf("Directory '%s' already exists.\n", dirName);
    } else {
        // Directory does not exist, create it
        lock = CreateDir(dirName);
        if (lock) {
            UnLock(lock);
            printf("Directory '%s' created.\n", dirName);
        } else {
            printf("Failed to create directory '%s'.\n", dirName);
        }
    }
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
		PaintBoard(0);
		ask = PlayGame();
		if (ask)
		{
			PutText("ce Play again? (y/N)? ");
			input = OneKey();
			if(input=='') 
			{
				OneKey();
				OneKey();
			}
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

void PaintBoard(int moves)
{
    char buffer[1000]; 
	unsigned char UL = ULS;
	unsigned char UR = URS;
	unsigned char LL = LLS;
	unsigned char LR = LRS;	
	unsigned char HL = HLS;
	unsigned char VL = VLS;
	char xBorder[200];
	char yBorder[200];
	int y;

	memset(xBorder, 0, sizeof(xBorder));
	memset(yBorder, 0, sizeof(yBorder));
	memset(xBorder, HL, (WIDTH)*2+1);
	memset(yBorder, VL, HEIGHT);

	PutText("f1cf Minesweeper! v0.2(beta)        c9by cbEnlightener!     c2Moves:       Mines:n1");
	sprintf(buffer,"f0n1ca%c%sP3%c%sP4%c%sP2%c%sq1[?25l[19;1HcfARROW KEYSc6=ceMove Cursor cfSPACEc6=ceRevealc6 cfFc6=ceFlag/Unflag cfLc6=ceLeaderBoard cfHc6=ceHelp cfQc6=ceQuit. ", UL, xBorder, UR, yBorder, LR, xBorder, LL, yBorder);
	PutText(buffer);

	/* Initialize board arrays */
	for (y = 0; y < HEIGHT; y++) 
	{
		UpdateRow(y);
	}
	UpdateMoves(moves);
	UpdateMines();
}


bool PlayGame(void)
{
	int x,xx, y ,yy, unrevealedcount, val, reason, moves;
	char input;
	bool leave;
	bool ask;

	ask = true;
	x=0;
	y=0;
	moves =0;
	leave=false;
	reason = 0;
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
			UpdateSquare(x,y,'f', SQS);
		}
		else if(revealed[x][y]==70)
		{
			UpdateSquare(x,y,'9','?');
		}
		input = OneKey();
		switch(input)
		{
			case 'F':
				switch (revealed[x][y])
				{
					case 0:
						revealed[x][y]=70; // Flag
						break;
					case 70:
						revealed[x][y]=0; // Unflag
						break;
					default:
					break;
				}
				break;
			case 'H':
				Help();
				PaintBoard(moves);
				break;
			case 'L':
				DisplayLeaderBoard();
				PutText("cePress any key to return to the game.");
				input = OneKey(); /* Should be '[' */
				if(input=='')
				{
				input = OneKey(); 
				input = OneKey(); 
				}
				PaintBoard(moves);
				break;
			case 'Q':
				PutText("[21;1H");
				PutText("cbAre you sure you wish to quit you yellow bellied coward? (y/N)");
				input = OneKey();
				if(input=='y' || input=='Y')
				{
					leave = true;
					reason = 1;
				}
				else
				{
					PaintBoard(moves);
				}
				break;
			case ' ':
				if(revealed[x][y]==70)
				{	
					break;	
				}
				moves++;
				UpdateMoves(moves);
				if(board[x][y]==9)
				{
					GameOver('9','*');
					leave=true;
					reason=-1;
				}
				else
				{
						ClearSquare(x,y);
						val = board[x][y];
						if(val ==0) val = ' ';
						else val = (char)(val + '0');
						UpdateSquare(x,y,'f', val);
				}
				break;

				case '': /* Escape sequence for arrow keys */
				input = OneKey(); /* Should be '[' */
				input = OneKey(); /* Actual key */
				switch (revealed[x][y])
				{
					case 0:
						UpdateSquare(x,y,'6', SQS);
						break;
					case 70:
						UpdateSquare(x,y,'1','F');
						break;
					default:
						val = board[x][y];
						if(val ==0) val = ' ';
						else val = (char)(val + '0');
						UpdateSquare(x,y,'f', val);
						break;
				}
				switch(input)
				{
					case 'A': /* Up */
						y--;
						break;
					case 'B': /* Down */
						y++;
						break;
					case 'C': /* Right */
						x++;
						break;
					case 'D': /* Left */
						x--;
						break;
				}
				if(x<0) x=WIDTH-1;
				if(x>=WIDTH) x=0;
				if(y<0) y=HEIGHT-1;
				if(y>=HEIGHT) y=0;
				UpdateSquare(x,y,'f', SQS);
				ReturnCursor();
				break;	
		}
	}	
	/* Game over */
	/* reason*/
	PutText("[21;1H");
	switch(reason)
	{
		case 0: /* Win */
			PutText("cbCongratulations! caYou cleared the minefield!");
			UpdateLeaderBoard();
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

void UpdateMoves(int moves)
{ 	
	char buffer[1000]; /* Reusable buffer for PutText */
	sprintf(buffer, "[1;60Hce%d", moves );
	PutText(buffer);
}

void UpdateMines()
{ 	
	char buffer[1000]; /* Reusable buffer for PutText */
	sprintf(buffer, "[1;73Hce%d", mines );
	PutText(buffer);
}

void ReturnCursor(void)
{
	PutText("[19;62H");
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
	sprintf(buffer, "[%d;%dHc%c%c", y+3, (x*2)+3, color, symbol);
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

void DisplayLeaderBoard(void)
{
	char input;
	PutText("f1cbMinesweeper Leaderboardn2caFeature coming soon!n2");
}

void InitializeLeaderboard(void)
{
	FILE *leaderfile;
	const char *filename = "PFiles:MinesweeperData/leaderboard.txt";
    CreateDirIfNotExist("PFiles:MinesweeperData");
	
	leaderfile = fopen(filename, "a");
	if (leaderfile == NULL)	
	{
		return;
	}
	else
	{
		fclose(leaderfile);
	}
}

void UpdateLeaderBoard(void)
{
	FILE *leaderfile;
	const char *filename = "PFiles:MinesweeperData/leaderboard.txt";

	leaderfile = fopen(filename, "a");
	if (leaderfile == NULL)	
	{
		return;
	}
	else
	{
		fprintf(leaderfile, "Mines: %d Moves: %d\n", mines, moves);
		fclose(leaderfile);
	}
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
