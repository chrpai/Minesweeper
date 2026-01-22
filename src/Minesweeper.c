/*
	Minesweeper for CNet
	by Enlightener!
*/
#define bool int
#define false 0
#define true 1
#define WIDTH  38
#define HEIGHT 15

void InitializeBoard(int);
void PlayGame(int c);
void UpdateMoves(int moves);
void UpdateSquare(int x, int y, char color, char symbol);
void GameOver(char color, char symbol);
void ClearSquare(int x, int y);
bool OutOfBounds(int x, int y);
int CountAdjacentMines(int x, int y);
bool ClearAdjacentSquares(int x, int y);
void UpdateRow(int y);
void CallHost( UBYTE c );
void ShutDown( char *spawn );
ULONG Timer();

struct MsgPort  *replyp;	/* Some commnunication details ...	*/
struct CPort    *cport;
struct CMessage  cmess;

struct	MainPort  *myp;		/* Pointer to CNet port--ALL info!	*/
struct	PortData  *z;

char	**bm;
struct	Library *CNetBase = NULL;
struct	SignalSemaphore *SEM;

				/* put your other GLOBALS here		*/
int mines = (WIDTH * HEIGHT) / 5; /* Number of mines on the board */
int board[WIDTH][HEIGHT];
int revealed[WIDTH][HEIGHT];
char buffer[1000]; /* Reusable buffer for PutText */

void main( int argc, char **argv ){
	bool playmore;
	char input;
	playmore = true;

	if( argc<2 || !(cport = (struct CPort *)FindPort( argv[1] )) ) {
		printf("This is a CNet C program.\n");
		exit(0);
	}

	if( !(replyp = CreatePort( 0,0 )))
		exit(0);

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
		goto err0;

	while(playmore){	
		InitializeBoard(1);
		PlayGame(1);
		PutText("ce Play again? (y/N)? ");
		input = OneKey();
		if(input != 'y' && input != 'Y'){
			PutText("cfNo");
			playmore = false;
		}
		else{
			PutText("cfYes");
		}
	}

	err0:	ShutDown( NULL );

	err:	DeletePort( replyp );

	if( CNetBase ) CloseLibrary( CNetBase );

	exit(0);
}

void InitializeBoard(int c){
	char *xBorder;
 	char *yBorder; 
	int mines_placed, mineplaced, rand_x, rand_y, x, y;


	xBorder = "อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ\0";
	yBorder = "บบบบบบบบบบบบบบบ\0";
	srand((unsigned)time(NULL));
	PutText("f1cf Minesweeper! v0.1beta         c9by cbEnlightener!       c2Moves:n1");
	sprintf(buffer,"f0n1caษ%sP3ป%sP4ผ%sP2ศ%sq1n1", xBorder, yBorder, xBorder, yBorder);
	PutText(buffer);


	/* Initialize board arrays */
	for (y = 0; y < HEIGHT; y++) 
	{
		for (x = 0; x < WIDTH; x++) 
		{
			board[x][y] = 10; /* Unrevealed */
			revealed[x][y] = 0;	/* Not revealed */
		}
		PutText(">2þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ þ\n\0");
	}

	UpdateMoves(0);

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


void PlayGame(int c){
	int x,xx, y ,yy, unrevealedcount, val, reason, moves;
	char input;
	bool leave;

	x=0;
	y=0;
	moves =0;
	UpdateSquare(x,y,'f','.');
	PutText("[?25l[19;1Hc6Use the c9ARROW KEYSc6 to move the cursor. SPACE=Reveal, Q=Quit.");

	leave=false;
	reason = 0;
	while(!leave){
		unrevealedcount =0;
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

		input = OneKey();
		switch(input){
			case 'q':
			case 'Q':
				leave=true;
				reason=1;
				break;
			case ' ':
				moves++;
				UpdateMoves(moves);
				if(board[x][y]==9){ /* Hit a mine */
					GameOver('9','*');
					leave=true;
					reason=-1;
				}
				else{
						ClearSquare(x,y);
				}
				break;
			case '': /* Escape sequence for arrow keys */
				input = OneKey(); /* Should be '[' */
				input = OneKey(); /* Actual key */
				if(revealed[x][y]==0) UpdateSquare(x,y,'6','.');
				else
				{
					val = board[x][y];
					if(val ==0) val = ' ';
					else val = (char)(val + '0');
					UpdateSquare(x,y,'f', val);
				}
				switch(input){
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
				UpdateSquare(x,y,'f','.');
				break;	
		}
	}	
	/* Game over */
	/* reason*/
	PutText("[21;1H");
	switch(reason){
		case 0: /* Win */
			PutText("cbCongratulations! caYou cleared the minefield!");
			break;
		case -1: /* Lose */
			PutText("c9BOOM! c3Your body parts are everywhere.");
			break;
		case 1: /* Quit */
			PutText("cbOff with you then you coward!");
			break;
	}

}


void ClearSquare(int x, int y){
int count;
char character;
	if(OutOfBounds(x,y)) return;
	if(revealed[x][y]==1) return;
	count = CountAdjacentMines(x,y);
	if(count>0){
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

	if(count==0){
		// clear adjacent squares
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

bool OutOfBounds(int x, int y){
	if(x<0 || x>=WIDTH || y<0 || y>=HEIGHT){
		return true;
	}
	return false;
}	
void UpdateMoves(int moves){ 	
	sprintf(buffer, "[1;61Hc0   [1;61Hce%d", moves );
	PutText(buffer);
}

void GameOver(char color, char symbol){
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
void UpdateSquare(int x, int y, char color, char symbol){
	if(symbol == '.')
	{
		symbol = 'þ';
	}
	sprintf(buffer, "[%d;%dHc%c%c", y+3, (x*2)+3, color, symbol);
	PutText(buffer);
}

int CountAdjacentMines(int x, int y){
	int count, ymin, ymax, xmin, xmax, dy, dx;
	count =0;

	ymin = y-1;
	ymax = y+1;
	xmin = x-1;
	xmax = x+1;

	for(dy=ymin; dy<=ymax; dy++)
	{
 		for(dx=xmin; dx<=xmax; dx++){
			if(dx==x && dy==y) continue;
			if(dx>=0 && dx<WIDTH && dy>=0 && dy<HEIGHT){
				if(board[dx][dy]==9){
					count++;
				}
			}
		}
	}
	return count;
}

/*
	C-Net communication functions
*/


void UpdateRow(int y){
	int x;
	for(x=0; x<WIDTH; x++){
		if(revealed[x][y]==0){
			UpdateSquare(x,y,'6','.');
		}
		else{
			int val = board[x][y];
			char character;
			if(val ==0) character = ' ';
			else character = (char)(val + '0');
			UpdateSquare(x,y,'f', character);
		}
	}
}	

void ShutDown( char *spawn ){
	if( spawn )
		strcpy( z->CSpawn, spawn );

	CallHost( 0 );
}

void CallHost( UBYTE c ){
	cmess.command = c;
	PutMsg  ( (struct MsgPort *)cport, (struct Message *)&cmess );
	WaitPort( replyp );
	GetMsg  ( replyp );
}

void PutText( char *text ){
	cmess.arg1 = (ULONG)text;	/* text to print		*/
	CallHost( 1 );
}

char OneKey( void ){
	CallHost( 3 );
	return( (char)cmess.result );
}
