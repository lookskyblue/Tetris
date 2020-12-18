#include <stdio.h>
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
#define UP 72
#define DOWN 80
#define HOLD 99
#define STOP 0
#define AGAIN 97
#define ESC 27
#define OFFSET_X 13
#define OFFSET_Y 3
#define FIRST_HOLD -1

void GoToXY(int x, int y);
void SetBoard();
void DrawBoard(); // 기준점 좌표 정리 할 것
void DrawGameOver();
void GetNextBlock();
void CreateNextBlock(); //크리, 겟, 홀드 삼박자 섞이는 구조 코드 너무 안좋아..
void GetKeyInput();
void MoveBlock(int left, int right, int down);
void FixBlock();
int  DropBlock();
void HideCursor();
int  RemoveLine();
int  GetUserAnswer();
void PullLine(int row);
void RotateBlock();
void Hold_Block();
void Unlock_Hold();
void AddGameScore(int score);
void ResetGame();
void SaveBestScore();
void LoadBestScoreFile();
void FixingBlockProcedure();
void LevelUpByRemovedLine();
int  BalancingLineScoreByLevel(int removedLine);

bool CheckGameOver();
bool RotateDetectCollision();
bool DetectCollision(int left, int right, int down);

typedef struct GameSetting
{
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

}GAMESETTING;

int board[BOARD_HEIGHT + 1][BOARD_WIDTH + 1] = { EMPTY, };
int boardCopy[BOARD_HEIGHT + 1][BOARD_WIDTH + 1] = { EMPTY, };
int holdBox[6][6] = { EMPTY, };
int holdBoxCopy[6][6] = { EMPTY, };
int nextBox[6][6] = { EMPTY, };
int nextBoxCopy[6][6] = { EMPTY, };
int newBlock[4][2];
int barAxisX;
int barAxisY;

GAMESETTING gs;

int main()
{
	HideCursor();
	system("color 3");                      //console color
	system("mode con: cols=75 lines=28");   //console size

	ResetGame();

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
				ResetGame();
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
	int i, j;

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
	GoToXY(OFFSET_X - 7, OFFSET_Y + 9);
	printf("NOW SCORE");

	GoToXY(OFFSET_X - 7, OFFSET_Y + 11);
	printf("%d", gs.nowScore);

	// Draw best score
	GoToXY(OFFSET_X - 7, OFFSET_Y + 13);
	printf("BEST SCORE");

	GoToXY(OFFSET_X - 7, OFFSET_Y + 15);
	printf("%d", gs.bestScore);

	// Draw level
	GoToXY(OFFSET_X - 7, OFFSET_Y + 17);
	printf("Level");

	GoToXY(OFFSET_X - 7, OFFSET_Y + 19);
	printf("%d", gs.gameLevel);

	// Draw lines
	GoToXY(OFFSET_X - 7, OFFSET_Y + 21);
	printf("Lines");

	GoToXY(OFFSET_X - 7, OFFSET_Y + 23);
	printf("%d", gs.removedLine);
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
			case UP:
			{
				if (RotateDetectCollision() == false)
				{
					RotateBlock();
					DrawBoard();
				}

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

			default:
				break;
			}
		}
	}
}

void MoveBlock(int left, int right, int down)
{

	for (int i = 0; i < 4; i++)
		board[newBlock[i][0]][newBlock[i][1]] = EMPTY;

	for (int i = 0; i < 4; i++)
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
	bool putFlag = false;

	for (int i = 0; i < 4; i++) // Return, if Fixed_Block over the ceiling
		if (newBlock[i][0] <= 1)
			return removeLineCount;

	for (int i = 0; i < 4; i++) // find x-coordinate with fulled FIXED_BLOCK 
	{
		putFlag = true;

		for (int j = 1; j < BOARD_WIDTH; j++)
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
	for (int i = 0; i < index - 1; i++)
	{
		for (int j = i + 1; j < index; j++)
		{
			if (fulledX[i] > fulledX[j]) // swap for ascending
			{
				int temp = fulledX[i];
				fulledX[i] = fulledX[j];
				fulledX[j] = temp;
			}

			else if (fulledX[i] == fulledX[j]) // put zero for deduplication
			{
				fulledX[j] = 0;
			}
		}
	}

	for (int i = 0; i < index; i++)
	{
		if (fulledX[i] == 0)
			continue;

		PullLine(fulledX[i] - 1);
		removeLineCount++;
	}

	return removeLineCount;
}

void PullLine(int row)
{
	bool ExitFlag;

	do
	{
		ExitFlag = true;

		if (row == 1)
		{
			for (int i = 1; i < BOARD_WIDTH; i++)
				board[row + 1][i] = CEILING;

			return;
		}

		else if (row == 2)
		{
			for (int i = 1; i < BOARD_WIDTH; i++)
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
			for (int i = 1; i < BOARD_WIDTH; i++)
			{
				board[row + 1][i] = board[row][i];
			}

			for (int i = 1; i < BOARD_WIDTH; i++)
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

	for (int i = 0; i < 4; i++)
		board[newBlock[i][0]][newBlock[i][1]] = EMPTY;

	for (int k = 0; k < 4; k++)
	{
		index = 0;

		for (int i = startX; i < startX + maxCell; i++)
		{
			for (int j = startY; j < startY + maxCell; j++)
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

	for (int i = 0; i < 4; i++)
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

	for (int k = 0; k < 4; k++)
	{
		index = 0;

		for (int i = startX; i < startX + maxCell; i++)
		{
			for (int j = startY; j < startY + maxCell; j++)
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
	//GoToXY(5, 3);
	//printf("DrawGameOver");
	//Sleep(300);
	//GoToXY(5, 3);
	//printf("             ");
	//Sleep(300);

	for (int i = 0; i < BOARD_HEIGHT; i++) // game over effect - paint all blocks  
	{
		for (int j = 1; j < BOARD_WIDTH; j++)
		{
			if (board[i][j] == FIXED_BLOCK)
				board[i][j] = NEW_BLOCK;
		}

		DrawBoard();
		Sleep(100);
	}

	for (int i = 0; i < 3; i++) // game over effect - blink string
	{
		GoToXY((OFFSET_X + (BOARD_WIDTH / 2)) - 1, OFFSET_Y - 1);
		printf("         ");
		Sleep(300);

		GoToXY((OFFSET_X + (BOARD_WIDTH / 2)) - 1, OFFSET_Y - 1);
		printf("GAME OVER");
		Sleep(300);
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

	for (int k = 0; k < 4; k++)
		board[newBlock[k][0]][newBlock[k][1]] = EMPTY;

	for (int i = 1; i < 5; i++) // Clear hold box
		for (int j = 1; j < 5; j++)
			holdBox[i][j] = EMPTY;

	for (int k = 0; k < 4; k++) // put now block to hold box
	{
		for (int i = 1; i < 5; i++)
		{
			for (int j = 1; j < 5; j++)
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

void ResetGame()
{
	system("cls");
	LoadBestScoreFile();
	memset(&board, 0, sizeof(board));
	memset(&boardCopy, 0, sizeof(boardCopy));
	memset(&holdBox, 0, sizeof(holdBox));
	memset(&holdBoxCopy, 0, sizeof(holdBoxCopy));
	memset(&nextBox, 0, sizeof(nextBox));
	memset(&nextBoxCopy, 0, sizeof(nextBoxCopy));
	memset(&newBlock, 0, sizeof(newBlock));

	SetBoard(); // Board && hold box && next box
	srand(clock());

	gs.gameLevel = 1;
	gs.nowScore = 0;
	gs.removedLine = 0;
	gs.autoDownPassedTime = 500;
	gs.nowBlockType;
	gs.nextBlockType;
	gs.heldBlockType = FIRST_HOLD;
	gs.hold_Lock = false;
	gs.isExecutedHold = false;
	gs.isNextBlockEmpty = true;
	gs.bestScore;
	gs.isLevelUp = false;
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
		gs.bestScore = -1;
		return;
	}

	fscanf_s(bestScoreFile, "%d", &gs.bestScore);
	fclose(bestScoreFile);
}

void FixingBlockProcedure() // autoDown, down, space, use this function.
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

	case2:
		return 300 * gs.gameLevel;

	case3:
		return 500 * gs.gameLevel;

	case4:
		return 800 * gs.gameLevel;

	default:
		return 0;
	}
}

void LevelUpByRemovedLine()
{
	int minimumPassedTime = 85;
	int coefficient = 10;
	int level = (gs.removedLine / 1) + 1;

	while (gs.gameLevel != level)
	{
		gs.gameLevel++;

		if (gs.autoDownPassedTime <= minimumPassedTime) // Minimum value 10 
		{
			gs.autoDownPassedTime = minimumPassedTime;
			break;
		}

		gs.autoDownPassedTime -= gs.autoDownPassedTime / coefficient;
	}
}
