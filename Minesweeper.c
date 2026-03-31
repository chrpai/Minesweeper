/*
	Minesweeper for CNet
    Copyright (C) 2026 Christopher Painter
	This file is part of the Minesweeper project.
	SPDX-License-Identifier: MIT
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <exec/types.h>
#include <cnet/cnet.h>
#include <time.h>
#include <dos/dos.h>

extern struct PortData *z;
extern struct MainPort *myp;
extern BOOL CNCL_DoorInit(int argc, char **argv);
extern void CNCL_DoorCleanup(void);
extern void  CPutText(char *text);
extern void  CPutA(void);
extern char  COneKey(void);
extern int   CEnterLine(UBYTE len, ULONG flags, char *prompt);
extern void  CEnterPassword(UBYTE len);
extern long  CCommonCommands(void);
extern UBYTE CReadFile(char *path, UBYTE flags);
extern void  CSetDoing(char *what);
extern void  CCallEditor(short max, short inlines);
extern UBYTE CReadGraphics(char *path, char flags);
extern UBYTE CReadAccount(short id, struct UserData *user);
extern UBYTE CSaveAccount(struct UserData *user, short id);
extern UBYTE CAddCharge(short n, short a);
extern UBYTE CCheckBalance(short n, short a);
extern int   CEnterText(char firstchar, short maxchars, short perline, short maxlines);
extern void  CCheckChanges(void);
extern long  CGetFree(char *s, UBYTE q);
extern void  CCheckFlowControl(void);
extern char  CWaitForInput(long mics);
extern UBYTE CPutQ(char *a);
extern void  CDoReturn(void);
extern UBYTE CCheckCarrier(void);
extern char  CCheckOLM(void);
extern void  CShutDown(char *spawn);

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

/* Minesweeper Globals		*/
int board[WIDTH][HEIGHT];
int revealed[WIDTH][HEIGHT];
int moves;

int mines = 10;

static void DoorCleanup(void)
{
    /* Your cleanup code here */
}


int main(int argc, char **argv)
{
    atexit(DoorCleanup);
    if (!CNCL_DoorInit(argc, argv)) {
        printf("This is a CNetC door -- run from CNet BBS.\n");
        return 0;
    }
    Minesweeper();
    return 0;
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
	CPutText("f1cf                               Minesweeper Helpn2");
	CPutText("cbObjective: caClear the minefield without detonating any mines.n2");
	CPutText("cbControls:n2");
	CPutText("cfARROW KEYSc6=ceMove Cursor cfSPACEc6=ceRevealc6 cfFc6=ceFlag/Unflag cfLc6=ceLeaderBoard cfHc6=ceHelp cfQc6=ceQuitn2");
	CPutText("cbGameplay:n2");
	CPutText("c7When you reveal a square, it will either be empty (no adjacent mines),\n");
	CPutText("contain a number (indicating how many adjacent mines there are), or\n");
	CPutText("be a mine (which ends the game).n2");
	CPutText("c27If you reveal an empty square, all adjacent empty squares will also ben1");
	CPutText("revealed automatically, along with any numbered squares bordering them.n2");
	CPutText("cePress any key to return to the game.");
	input = COneKey();
	if(input=='') 
	{
		COneKey();
		COneKey();
	}
}

void CreateDirIfNotExist(STRPTR dirName) 
{
    // BPTR lock = Lock(dirName, ACCESS_READ);
    
    // if (lock) {
    //     // Directory exists, unlock it
    //     UnLock(lock);
    //     printf("Directory '%s' already exists.\n", dirName);
    // } else {
    //     // Directory does not exist, create it
    //     lock = CreateDir(dirName);
    //     if (lock) {
    //         UnLock(lock);
    //         printf("Directory '%s' created.\n", dirName);
    //     } else {
    //         printf("Failed to create directory '%s'.\n", dirName);
    //     }
    // }
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
		CSetDoing(buffer);
		InitializeBoard();
		PaintBoard(0);
		ask = PlayGame();
		if (ask)
		{
			CPutText("ce Play again? (y/N)? ");
			input = COneKey();
			if(input=='') 
			{
				COneKey();
				COneKey();
			}
			if(input != 'y' && input != 'Y')
			{
				CPutText("cfNo");
				playmore = false;
			}
			else
			{
				CPutText("cfYes");
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

    // initialize random number generator based on current time as a seed value
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

	CPutText("f1cf Minesweeper! v0.2(beta)        c9by cbEnlightener!     c2Moves:       Mines:n1");
	sprintf(buffer,"f0n1ca%c%sP3%c%sP4%c%sP2%c%sq1[?25l[19;1HcfARROW KEYSc6=ceMove Cursor cfSPACEc6=ceRevealc6 cfFc6=ceFlag/Unflag cfLc6=ceLeaderBoard cfHc6=ceHelp cfQc6=ceQuit. ", UL, xBorder, UR, yBorder, LR, xBorder, LL, yBorder);
	CPutText(buffer);

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
		input = COneKey();
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
				CPutText("cePress any key to return to the game.");
				input = COneKey(); /* Should be '[' */
				if(input=='')
				{
				input = COneKey(); 
				input = COneKey(); 
				}
				PaintBoard(moves);
				break;
			case 'Q':
				CPutText("[21;1H");
				CPutText("cbAre you sure you wish to quit you yellow bellied coward? (y/N)");
				input = COneKey();
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
				input = COneKey(); /* Should be '[' */
				input = COneKey(); /* Actual key */
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
	CPutText("[21;1H");
	switch(reason)
	{
		case 0: /* Win */
			CPutText("cbCongratulations! caYou cleared the minefield!");
			UpdateLeaderBoard();
			if(mines<=100)
			{
				mines += 10;
			}
			break;
		case -1: /* Lose */
			CPutText("c9BOOM! cbYour body parts are everywhere.");
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
	CPutText(buffer);
}

void UpdateMines()
{ 	
	char buffer[1000]; /* Reusable buffer for PutText */
	sprintf(buffer, "[1;73Hce%d", mines );
	CPutText(buffer);
}

void ReturnCursor(void)
{
	CPutText("[19;62H");
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
	CPutText(buffer);
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


void DisplayLeaderBoard(void)
{
	CPutText("f1cbMinesweeper Leaderboardn2caFeature coming soon!n2");
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
