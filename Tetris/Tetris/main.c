﻿#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "block.h"

#define BOARD_HEIGHT 23
#define BOARD_WIDTH 11
#define CEILING -2
#define EDGE -1
#define EMPTY 0
#define NEW_BLOCK 1
#define FIXED_BLOCK 2
#define SPACE 32
#define LEFT 75
#define RIGHT 77
#define COUNTER_CLOCK_ROTATE 72 // ascii up key.  // 대문자 H도 아스키 72라서 Up 키로 인식됨.. 키입력은 getch 두번 받아서 해
#define CLOCK_ROTATE 122 // ascii 'z'
#define DOWN 80
#define HOLD 99 // alp 'c'
#define STOP 0
#define AGAIN 97 // alp 'a'
#define ESC 27
#define OFFSET_X 13
#define OFFSET_Y 3
#define FIRST_HOLD -1
#define SOUND_TOGGLE 109 // alp 'm' 
#define CHANGE_BLOCK_COLOR 9 // tab

int  BalancingLineScoreByLevel(int removedLine);
int  GetUserAnswer();
int  RemoveLine();
int  DropBlock();

bool DetectCollision(int left, int right, int down);
bool RotateDetectCollision();
bool CheckGameOver();

void ActiveRemoveLineEffect(const int* const x, int index);
void MoveBlock(int left, int right, int down);
void AddGameScore(int score);
void FixingBlockProcedure();
void LevelUpByRemovedLine();
void GoToXY(int x, int y);
void LoadBestScoreFile();
void ChangeBlockColor();
void CreateNextBlock();
void PullLine(int row);
void InitGameSetting();
void DrawInstruction();
void SaveBestScore();
void DrawGameOver();
void GetNextBlock();
void RotateBlock();
void GetKeyInput();
void Unlock_Hold();
void SoundToggle();
void HideCursor();
void Hold_Block();
void DrawBoard();
void SetBoard();
void FixBlock();

typedef struct GameSetting
{
	int  blockColorNum;
	int  gameLevel;
	int  removedLine;
	int  nowScore;
	int  nextBlockType;
	int  nowBlockType;
	int  heldBlockType; // FIRST_HOLD
	int  autoDownPassedTime; // 500
	int  bestScore;
	bool isLevelUp;
	bool hold_Lock; // false
	bool isExecutedHold; // false
	bool isNextBlockEmpty; // true
	bool soundOn;
}GAMESETTING;

int boardCopy[BOARD_HEIGHT + 1][BOARD_WIDTH + 1] = { EMPTY, };
int board[BOARD_HEIGHT + 1][BOARD_WIDTH + 1] = { EMPTY, };
int holdBoxCopy[6][6] = { EMPTY, };
int holdBox[6][6] = { EMPTY, };
int nextBoxCopy[6][6] = { EMPTY, };
int nextBox[6][6] = { EMPTY, };
int newBlock[4][2];
int barAxisX;
int barAxisY;

GAMESETTING gs;

int main()
{
	system("mode con: cols=75 lines=28");
	HideCursor();
	DrawInstruction();
	InitGameSetting();

	do
	{
		GetNextBlock();
		DrawBoard();
		GetKeyInput();
		LevelUpByRemovedLine();

		if (CheckGameOver() == true)
		{
			int userAnswer;

			if (gs.bestScore < gs.nowScore)
			{
				SaveBestScore();
			}

			DrawGameOver();
			userAnswer = GetUserAnswer(); // whether to again or stop

			if (userAnswer == AGAIN)
			{
				InitGameSetting();
				continue;
			}

			else if (userAnswer == ESC)
			{
				return;
			}
		}

	} while (1);
}

void GoToXY(int x, int y)
{
	COORD pos = { x * 2, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void SetBoard()
{
	int i;

	// Set main game board
	for (i = 0; i <= BOARD_HEIGHT; i++) // Draw side edge
	{
		board[i][0] = EDGE;
		board[i][BOARD_WIDTH] = EDGE;
	}

	for (i = 0; i <= BOARD_WIDTH; i++) // Draw up down edge
	{
		board[2][i] = CEILING;
		board[BOARD_HEIGHT][i] = EDGE;
	}

	board[2][0] = EDGE;
	board[2][BOARD_WIDTH] = EDGE;

	// Set hold box and next box
	for (i = 0; i < 6; i++)
	{
		holdBox[i][0] = FIXED_BLOCK;
		holdBox[i][5] = FIXED_BLOCK;
		holdBox[0][i] = FIXED_BLOCK;
		holdBox[5][i] = FIXED_BLOCK;

		nextBox[i][0] = FIXED_BLOCK;
		nextBox[i][5] = FIXED_BLOCK;
		nextBox[0][i] = FIXED_BLOCK;
		nextBox[5][i] = FIXED_BLOCK;
	}
}

void DrawBoard()
{
	// Draw main game board
	for (int i = 0; i <= BOARD_HEIGHT; i++)
	{
		for (int j = 0; j <= BOARD_WIDTH; j++)
		{
			if (board[i][j] != boardCopy[i][j])
			{
				if (i == 2) // draw the ceiling is erased by newBlock 
				{
					if (board[i][j] == EMPTY)
						board[i][j] = CEILING;
				}

				GoToXY(j + OFFSET_X, i + OFFSET_Y);

				switch (board[i][j])
				{
				case EDGE:
					printf("▨");
					break;

				case NEW_BLOCK:
					printf("■");
					break;

				case FIXED_BLOCK:
					printf("□");
					break;

				case EMPTY:
					printf("  ");
					break;

				case CEILING:
					printf("￣");
					break;

				default:
					break;
				}

				boardCopy[i][j] = board[i][j];
			}
		}
	}

	GoToXY(OFFSET_X - 7, OFFSET_Y + 0);
	printf("H O L D");

	// Draw next box
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 6; j++)
		{   
			// Draw next box
			if (nextBox[i][j] != nextBoxCopy[i][j])
			{
				GoToXY(j + OFFSET_X + 14, i + OFFSET_Y + 2);

				switch (nextBox[i][j])
				{
				case NEW_BLOCK:
					printf("■");
					break;

				case FIXED_BLOCK:
					printf("□");
					break;

				case EMPTY:
					printf("  ");
					break;

				default:
					break;
				}

				nextBoxCopy[i][j] = nextBox[i][j];
			}

			// Draw hold box
			if (holdBox[i][j] != holdBoxCopy[i][j])
			{
				GoToXY(j + OFFSET_X - 8, i + OFFSET_Y + 2);

				switch (holdBox[i][j])
				{
				case FIXED_BLOCK:
					printf("□");
					break;

				case NEW_BLOCK:
					printf("■");
					break;

				case EMPTY:
					printf("  ");
					break;

				default:
					break;
				}

				holdBoxCopy[i][j] = holdBox[i][j];
			}
		}
	}

	GoToXY(OFFSET_X + 15, OFFSET_Y + 0);
	printf("N E X T");

	// Draw score
	GoToXY(OFFSET_X - 8, OFFSET_Y + 9);
	printf("NOW SCORE");

	GoToXY(OFFSET_X - 8, OFFSET_Y + 11);
	printf("%d", gs.nowScore);

	// Draw level
	GoToXY(OFFSET_X - 8, OFFSET_Y + 13);
	printf("Level");

	GoToXY(OFFSET_X - 8, OFFSET_Y + 15);
	printf("%d", gs.gameLevel);

	// Draw lines
	GoToXY(OFFSET_X - 8, OFFSET_Y + 17);
	printf("Lines");

	GoToXY(OFFSET_X - 8, OFFSET_Y + 19);
	printf("%d", gs.removedLine);

	GoToXY(OFFSET_X + 14, OFFSET_Y + 17);
	printf("Sound On/Off: M");

	GoToXY(OFFSET_X + 14, OFFSET_Y + 19);
	printf("Change Color: TAB");
}

void GetNextBlock()
{
	if (gs.isNextBlockEmpty != true && gs.isExecutedHold != true) // General Progress
	{
		gs.nowBlockType = gs.nextBlockType;
		CreateNextBlock();
	}

	else if (gs.isNextBlockEmpty == true) // First game active this logic 
	{
		CreateNextBlock();
		gs.nowBlockType = gs.nextBlockType;
		gs.isNextBlockEmpty = false;
		Sleep(10); // Delay to prevent same seed in srand(clock());
		CreateNextBlock();
	}

	else if (gs.isExecutedHold == true) // Every time a user active Hold_Block function.
	{
		srand(clock());
		gs.isExecutedHold = false;

		if (gs.nowBlockType == FIRST_HOLD) // if user active a Hold_Block function for the first time
		{
			gs.nowBlockType = gs.nextBlockType;
			CreateNextBlock();
		}
	}

	memcpy(newBlock, blocks[gs.nowBlockType], sizeof(blocks[gs.nowBlockType]));

	for (int i = 0; i < 4; i++)
	{
		board[newBlock[i][0]][newBlock[i][1]] = NEW_BLOCK;
	}

	if (gs.nowBlockType == 6) // bar block
	{
		barAxisX = 0;
		barAxisY = 4;
	}
}

void CreateNextBlock()
{
	int nextBlockPrototype[4][2];

	srand(clock());
	gs.nextBlockType = rand() % 7;
	memcpy(nextBlockPrototype, blocks[gs.nextBlockType], sizeof(blocks[gs.nextBlockType]));

	for (int i = 1; i < 5; i++)
	{
		for (int j = 1; j < 5; j++)
		{
			nextBox[i][j] = EMPTY;
		}
	}

	for (int k = 0; k < 4; k++)
	{
		for (int i = 1; i < 5; i++)
		{
			for (int j = 1; j < 5; j++)
			{
				if (nextBlockPrototype[k][0] + 2 == i && nextBlockPrototype[k][1] - 3 == j)
				{
					nextBox[i][j] = NEW_BLOCK;
					goto GET_NEW_K;
				}
			}
		}

	GET_NEW_K:;
	}
}

void GetKeyInput()
{
	clock_t autoDownBaseTime = clock();
	clock_t autoFixBaseTime = clock();

	while (1)
	{
		if ((DetectCollision(0, 0, 1) == true)) // If it dectct collision && over autoFixBaseTime, active FixBlock() function
		{
			if (clock() > autoFixBaseTime + 1000)
			{
				FixingBlockProcedure();
				return;
			}
		}

		else if (clock() > autoDownBaseTime + gs.autoDownPassedTime) // If it dosen't collide yet, down a block automatically  
		{
			autoDownBaseTime = clock();
			autoFixBaseTime = clock();
			MoveBlock(0, 0, 1);
			DrawBoard();
		}

		if (_kbhit())
		{
			switch (_getch())
			{
			case COUNTER_CLOCK_ROTATE:
			{
				if (RotateDetectCollision() == false)
				{
					RotateBlock();
					DrawBoard();
				}

				break;
			}

			case CLOCK_ROTATE:
			{
				break;

			}

			case LEFT:
			{
				if (DetectCollision(-1, 0, 0) == false)
				{
					MoveBlock(-1, 0, 0);
					DrawBoard();
				}

				break;
			}

			case RIGHT:
			{
				if (DetectCollision(0, 1, 0) == false)
				{
					MoveBlock(0, 1, 0);
					DrawBoard();
				}

				break;
			}

			case DOWN:
			{
				if (DetectCollision(0, 0, 1) == true)
				{
					FixingBlockProcedure();
					return;
				}

				MoveBlock(0, 0, 1);
				AddGameScore(1);
				DrawBoard();
				autoFixBaseTime = clock();

				break;
			}

			case HOLD:
			{
				if (gs.hold_Lock == false)
				{
					Hold_Block();
					gs.hold_Lock = true;

					return;
				}

				else
					break;
			}

			case SPACE:
			{
				AddGameScore(DropBlock() * 2);
				FixingBlockProcedure();
				return;
			}

			case SOUND_TOGGLE: // mute
				SoundToggle();
				break;

			case CHANGE_BLOCK_COLOR:
				ChangeBlockColor();
				break;

			default:
				break;
			}
		}
	}
}

void MoveBlock(int left, int right, int down)
{
	int i;

	for (i = 0; i < 4; i++)
		board[newBlock[i][0]][newBlock[i][1]] = EMPTY;

	for (i = 0; i < 4; i++)
		board[newBlock[i][0] += down][newBlock[i][1] += (left + right)] = NEW_BLOCK;

	if (gs.nowBlockType == 6)
	{
		barAxisX += down;
		barAxisY += left + right;
	}
}

bool DetectCollision(int left, int right, int down)
{
	int movedBlockPiece;

	for (int i = 0; i < 4; i++)
	{
		movedBlockPiece = board[newBlock[i][0] + down][newBlock[i][1] + left + right];

		if (movedBlockPiece == EDGE || movedBlockPiece == FIXED_BLOCK)
			return true;
	}

	return false;
}

void FixBlock()
{
	if (gs.soundOn == true)
		Beep(300, 10);

	for (int i = 0; i < 4; i++)
		board[newBlock[i][0]][newBlock[i][1]] = FIXED_BLOCK;
}

int DropBlock()
{
	int dropCount = 0;
	int i = 0;

	for (i = 0; i < 4;i++)
		board[newBlock[i][0]][newBlock[i][1]] = EMPTY;

	for (dropCount = 1; DetectCollision(0, 0, dropCount) == false; dropCount++);

	for (i = 0, dropCount--; i < 4; i++)
		newBlock[i][0] += dropCount;

	return dropCount;
}

void HideCursor()
{
	CONSOLE_CURSOR_INFO CurInfo;
	CurInfo.dwSize = 1;
	CurInfo.bVisible = FALSE;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &CurInfo);
}

int RemoveLine()
{
	int  removeLineCount = 0;
	int  fulledX[4] = { 0, };
	int  index = 0;
	int  i, j, temp;
	bool putFlag = false;

	for (i = 0; i < 4; i++) // Return, if Fixed_Block over the ceiling
		if (newBlock[i][0] <= 1)
			return removeLineCount;

	for (i = 0; i < 4; i++) // find x-coordinate with fulled FIXED_BLOCK 
	{
		putFlag = true;

		for (j = 1; j < BOARD_WIDTH; j++)
		{
			if (board[newBlock[i][0]][j] != FIXED_BLOCK)
			{
				putFlag = false;
				break;
			}

		}

		if (putFlag == true)
			fulledX[index++] = newBlock[i][0];
	}

	if (index == 0) // Pass if it dosen't have any fulled line
		return removeLineCount;

	// Sort x-coordinates in ascending order && Deduplication
	for (i = 0; i < index - 1; i++)
	{
		for (j = i + 1; j < index; j++)
		{
			if (fulledX[i] > fulledX[j]) // swap for ascending
			{
				temp = fulledX[i];
				fulledX[i] = fulledX[j];
				fulledX[j] = temp;
			}

			else if (fulledX[i] == fulledX[j]) // put zero for deduplication
			{
				fulledX[j] = 0;
			}
		}
	}

	ActiveRemoveLineEffect(fulledX, index);

	for (i = 0; i < index; i++)
	{
		if (fulledX[i] == 0)
			continue;
		
		PullLine(fulledX[i] - 1);
		removeLineCount++;
	}

	return removeLineCount;
}

void ActiveRemoveLineEffect(const int* const x, int index)
{
	int i, j, k;

	for (k = 0; k < 3; k++)
	{
		for (i = 0; i < index; i++)
		{
			if (x[i] == 0)
				continue;

			for (j = 1; j < BOARD_WIDTH; j++)
			{
				board[x[i]][j] = NEW_BLOCK;
			}
		}

		DrawBoard();
		Sleep(50);

		for (i = 0; i < index; i++)
		{
			if (x[i] == 0)
				continue;

			for (j = 1; j < BOARD_WIDTH; j++)
			{
				board[x[i]][j] = FIXED_BLOCK;
			}
		}

		DrawBoard();
		Sleep(50);
	}
	
	if (gs.soundOn == true)
		Beep(100, 100); // 100, 100 GOOD
}

void PullLine(int row)
{
	int  i;
	bool ExitFlag;

	do
	{
		ExitFlag = true;

		if (row == 1)
		{
			for (i = 1; i < BOARD_WIDTH; i++)
				board[row + 1][i] = CEILING;

			return;
		}

		else if (row == 2)
		{
			for (i = 1; i < BOARD_WIDTH; i++)
			{
				if (board[row][i] == FIXED_BLOCK)
				{
					board[row + 1][i] = FIXED_BLOCK;
					board[row][i] = CEILING;
				}

				else if (board[row][i] == CEILING)
					board[row + 1][i] = EMPTY;
			}

			return;
		}

		else
		{
			for (i = 1; i < BOARD_WIDTH; i++)
			{
				board[row + 1][i] = board[row][i];
			}

			for (i = 1; i < BOARD_WIDTH; i++)
			{
				if (board[row][i] != EMPTY)
				{
					ExitFlag = false;
					break;
				}
			}
		}

		row--;
	} while (ExitFlag == false);
}

void RotateBlock()
{
	// square block is already returned in RotateDetectCollision function
	int rotate_Info_Bar[16][2] = { {0,3}, {1,2}, {2,1}, {3,0}, {-1,2},{0, 1},{1,0},{2, -1}, {-2,1},{-1,0},{0,-1},{1,-2},{-3,0},{-2,-1},{-1,-2},{0,-3} };
	int rotate_Info_Others[9][2] = { {0, 2},{1, 1},{2, 0},{-1, 1}, {0, 0}, {1, -1},{-2, 0},{-1, -1},{0, -2} };
	int(*rotate_Info)[2] = NULL;
	int maxCell;
	int startX;
	int startY;
	int index;
	int i, j, k;

	if (gs.nowBlockType == 6) // bar block
	{
		rotate_Info = rotate_Info_Bar;
		startX = barAxisX;
		startY = barAxisY;
		maxCell = 4;
	}

	else // the others
	{
		rotate_Info = rotate_Info_Others;
		startX = newBlock[2][0] - 1;
		startY = newBlock[2][1] - 1;
		maxCell = 3;
	}

	for (i = 0; i < 4; i++)
		board[newBlock[i][0]][newBlock[i][1]] = EMPTY;

	for (k = 0; k < 4; k++)
	{
		index = 0;

		for (i = startX; i < startX + maxCell; i++)
		{
			for (j = startY; j < startY + maxCell; j++)
			{
				if (newBlock[k][0] == i && newBlock[k][1] == j)
				{
					newBlock[k][0] += rotate_Info[index][0];
					newBlock[k][1] += rotate_Info[index][1];

					goto GET_NEW_K; // escape for getting k++
				}

				index++;
			}
		}

	GET_NEW_K:;
	}

	for (i = 0; i < 4; i++)
		board[newBlock[i][0]][newBlock[i][1]] = NEW_BLOCK;
}

bool RotateDetectCollision()
{
	if (gs.nowBlockType == 5) // square block dosen't rotate.
		return true;

	int rotate_Info_Bar[16][2] = { {0,3}, {1,2}, {2,1}, {3,0}, {-1,2},{0, 1},{1,0},{2, -1}, {-2,1},{-1,0},{0,-1},{1,-2},{-3,0},{-2,-1},{-1,-2},{0,-3} };
	int rotate_Info_Others[9][2] = { {0, 2},{1, 1},{2, 0},{-1, 1}, {0, 0}, {1, -1},{-2, 0},{-1, -1},{0, -2} };
	int(*rotate_Info)[2] = NULL;
	int maxCell;
	int startX;
	int startY;
	int index = 0;
	int i, j, k;

	if (gs.nowBlockType == 6) // block - bar
	{
		startX = barAxisX;
		startY = barAxisY;
		maxCell = 4;
		rotate_Info = rotate_Info_Bar;
	}

	else // block - the others
	{
		startX = newBlock[2][0] - 1;
		startY = newBlock[2][1] - 1;
		maxCell = 3;
		rotate_Info = rotate_Info_Others;
	}

	for (k = 0; k < 4; k++)
	{
		index = 0;

		for (i = startX; i < startX + maxCell; i++)
		{
			for (j = startY; j < startY + maxCell; j++)
			{
				if (newBlock[k][0] == i && newBlock[k][1] == j)
				{
					int blockPieceX = newBlock[k][0];
					int blockPieceY = newBlock[k][1];
					int movePieceByX = rotate_Info[index][0];
					int movePieceByY = rotate_Info[index][1];
					int movedPosition = board[blockPieceX + movePieceByX][blockPieceY + movePieceByY];

					if (movedPosition == EDGE || movedPosition == FIXED_BLOCK)
						return true;

					goto GET_NEW_K; // escape for getting k++
				}

				index++;
			}
		}

	GET_NEW_K:;
	}

	return false;
}

bool CheckGameOver()
{
	for (int i = 1; i < BOARD_WIDTH; i++)
	{
		if (board[1][i] == FIXED_BLOCK)
			return true;
	}

	return false;
}

void DrawGameOver()
{   
	int i, j;
	LoadBestScoreFile();

	// game over effect - paint all blocks
	for (i = 0; i < BOARD_HEIGHT; i++)   
	{
		for (int j = 1; j < BOARD_WIDTH; j++)
		{
			if (board[i][j] == FIXED_BLOCK)
				board[i][j] = NEW_BLOCK;
		}

		DrawBoard();
		Sleep(110);
	}

	// game over effect - blink string
	for (i = 0; i < 2; i++) 
	{
		GoToXY((OFFSET_X + (BOARD_WIDTH / 2)) - 1, OFFSET_Y - 2);
		printf("          ");

		GoToXY((OFFSET_X + (BOARD_WIDTH / 2)), OFFSET_Y - 1);
		printf("           ");
		Sleep(250);

		GoToXY((OFFSET_X + (BOARD_WIDTH / 2)) - 1, OFFSET_Y - 2);
		printf("BEST SCORE");

		GoToXY((OFFSET_X + (BOARD_WIDTH / 2)), OFFSET_Y - 1);
		printf("%d", gs.bestScore);
		Sleep(250);
	}

	for (i = 0; i < 5; i++)
	{
		for (j = 1; j < BOARD_WIDTH; j++)
		{
			board[8+i][j] = EMPTY;
		}
	}

	DrawBoard();

	for (i = 0; i < 2; i++)
	{
		GoToXY(OFFSET_X + 3, OFFSET_Y + 9);
		printf("          ");

		GoToXY(OFFSET_X + 3, OFFSET_Y + 11);
		printf("         ");
		Sleep(350);

		GoToXY(OFFSET_X + 3, OFFSET_Y + 9);
		printf("A  : AGAIN");

		GoToXY(OFFSET_X + 3, OFFSET_Y + 11);
		printf("ESC: STOP");
		Sleep(350);
	}
}

int GetUserAnswer()
{
	char userKeyInput = 0;

	while (1)
	{
		userKeyInput = tolower(_getch());

		if (userKeyInput == ESC)
			return ESC;

		else if (userKeyInput == AGAIN) // AGAIN ascii = 'a'
			return AGAIN;
	}
}

void Hold_Block()
{
	int nowBlockPrototype[4][2];
	int i, j, k;

	// Change block.
	if (gs.isExecutedHold == false)
	{
		//printf("%d", gs.heldBlockType);
		int temp = gs.nowBlockType;
		gs.nowBlockType = gs.heldBlockType;
		gs.heldBlockType = temp;
		gs.isExecutedHold = true;
	}

	memcpy(nowBlockPrototype, blocks[gs.heldBlockType], sizeof(blocks[gs.heldBlockType])); // get prototype block in block.h

	for (k = 0; k < 4; k++)
		board[newBlock[k][0]][newBlock[k][1]] = EMPTY;

	for (i = 1; i < 5; i++) // Clear hold box
		for (j = 1; j < 5; j++)
			holdBox[i][j] = EMPTY;

	for (k = 0; k < 4; k++) // put now block to hold box
	{
		for (i = 1; i < 5; i++)
		{
			for (j = 1; j < 5; j++)
			{
				if (nowBlockPrototype[k][0] + 2 == i && nowBlockPrototype[k][1] - 3 == j) // Adding number for adjust the position of nowBlockPrototype
				{
					holdBox[i][j] = NEW_BLOCK;
					goto GET_NEW_K; // for getting k++

				}
			}
		}

	GET_NEW_K:;
	}
}

void Unlock_Hold()
{
	if (gs.hold_Lock == true)
		gs.hold_Lock = false;
}

void AddGameScore(int score)
{
	gs.nowScore += score;
}

void InitGameSetting()
{
	system("cls");
	HideCursor();
	memset(&board, 0, sizeof(board));
	memset(&boardCopy, 0, sizeof(boardCopy));
	memset(&holdBox, 0, sizeof(holdBox));
	memset(&holdBoxCopy, 0, sizeof(holdBoxCopy));
	memset(&nextBox, 0, sizeof(nextBox));
	memset(&nextBoxCopy, 0, sizeof(nextBoxCopy));
	memset(&newBlock, 0, sizeof(newBlock));
	LoadBestScoreFile();
	SetBoard(); // Board && hold box && next box

	gs.gameLevel = 1;
	gs.nowScore = 0;
	gs.removedLine = 0;
	gs.autoDownPassedTime = 1000;
	gs.nowBlockType;
	gs.nextBlockType;
	gs.heldBlockType = FIRST_HOLD;
	gs.hold_Lock = false;
	gs.isExecutedHold = false;
	gs.isNextBlockEmpty = true;
	gs.bestScore;
	gs.isLevelUp = false;
	gs.soundOn = true;
	gs.blockColorNum = 7;

	system("color 0");
}

void SaveBestScore()
{
	FILE* bestScoreFile;

	fopen_s(&bestScoreFile, "bestScore.txt", "w");

	if (bestScoreFile == NULL)
	{
		return;
	}

	fprintf(bestScoreFile, "%d", gs.nowScore);
	fclose(bestScoreFile);
}

void LoadBestScoreFile()
{
	FILE* bestScoreFile;

	fopen_s(&bestScoreFile, "bestScore.txt", "r");

	if (bestScoreFile == NULL)
	{
		gs.bestScore = 0;
		return;
	}

	fscanf_s(bestScoreFile, "%d", &gs.bestScore);
	fclose(bestScoreFile);
}

void FixingBlockProcedure() // autoDown, down, space logic, use this function.
{
	int removedLine = 0;
	int score = 0;
	
	FixBlock();
	Unlock_Hold();
	removedLine = RemoveLine();

	if (removedLine != 0)
	{
		gs.removedLine += removedLine;
		score = BalancingLineScoreByLevel(removedLine);
		AddGameScore(score);
	}
}

int BalancingLineScoreByLevel(int removedLine)
{
	switch (removedLine)
	{
	case 1:
		return 100 * gs.gameLevel;

	case 2:
		return 300 * gs.gameLevel;

	case 3:
		return 500 * gs.gameLevel;

	case 4:
		return 800 * gs.gameLevel;

	default:
		return 0;
	}
}

void LevelUpByRemovedLine()
{
	int minimumPassedTime = 50;
	int speedUpRatio = 2; // I think 1~3 is good 
	int levelUp_Per_FiveLines = 5; // 3~5 is good. 10 is so long game..
	int level = (gs.removedLine / levelUp_Per_FiveLines) + 1;

	while (gs.gameLevel != level)
	{
		gs.gameLevel++;

		if (gs.autoDownPassedTime <= minimumPassedTime) // Minimum value 50 
		{
			gs.autoDownPassedTime = minimumPassedTime;
			break;
		}

		gs.autoDownPassedTime -= (gs.autoDownPassedTime / 10) * speedUpRatio;
	}
}

void SoundToggle()
{
	gs.soundOn = !gs.soundOn;
}

void DrawInstruction()
{
	puts("\n\n\n\n\n  [ TETRIS   GAME ]");
	puts("\n   ▼  KEY  ▼ \n");
	puts("   Sound  On/Off: M");
	puts("   Change Color: TAB\n");
	puts("   Left:   ←");
	puts("   Right:  →");
	puts("   Down:   ↓");
	puts("   Rotate: ↑");
	puts("   Hold:    C");
	puts("   Drop:  SPACE");
	puts("\n   ▲  KEY  ▲ \n\n\n\n\n");
	puts("   Git: github.com/lookskyblue/Tetris");

	while (1)
	{
		if (_kbhit())
			if (tolower(_getch()) == 'a')
				return;

		GoToXY(0, 21);
		printf("   Game Start: Press A");
		Sleep(350);

		GoToXY(0, 21);
		printf("                      ");
		Sleep(350);
	}
}

void ChangeBlockColor()
{
	gs.blockColorNum++;

	if (15 <= gs.blockColorNum)
		gs.blockColorNum = 1;

	switch (gs.blockColorNum)
	{
	case 1:
		system("color 1");
		break;

	case 2:
		system("color 2");
		break;

	case 3:
		system("color 3");
		break;

	case 4:
		system("color 4");
		break;

	case 5:
		system("color 5");
		break;

	case 6:
		system("color 6");
		break;

	case 7:
		system("color 7");
		break;

	case 8:
		system("color 8");
		break;

	case 9:
		system("color 9");
		break;

	case 10:
		system("color A");
		break;

	case 11:
		system("color B");
		break;

	case 12:
		system("color C");
		break;

	case 13:
		system("color D");
		break;

	case 14:
		system("color E");
		break;

	default:
		break;
	}
}
