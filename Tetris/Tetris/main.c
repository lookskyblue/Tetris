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

void GoToXY(int x, int y);
void SetBoard();
void DrawBoard();
void DrawGameOver();
void CreateNewBlock();
void GetKeyInput();
void MoveBlock(int left, int right, int down);
void FixBlock();
void DropBlock();
void HideCursor();
void RemoveLine(); // 어쩌다 블럭 안 지워지는 문제 있음. 고친 후 최적화 해볼 것.
void PullLine(int firstRow);
void RotateBlock();
void HoldBlock();
bool CheckGameOver();
bool RotateDetectCollision();
bool DetectCollision(int left, int right, int down);
int  GetUserAnswer();

int board[BOARD_HEIGHT + 1][BOARD_WIDTH + 1] = { EMPTY, };
int boardCopy[BOARD_HEIGHT + 1][BOARD_WIDTH + 1] = { EMPTY, };
int holdBox[6][6] = { EMPTY, };
int holdBoxCopy[6][6] = { EMPTY, };
int nextBox[6][6] = { EMPTY, };
int nextBoxCopy[6][6] = { EMPTY, };
int newBlock[4][2];
int nowBlockType;
int autoDownSpeed = 20;
int barAxisX;
int barAxisY;
int autoDownPassedTime = 500;
int heldBlockType = 0;

bool isAlreadyHeld = false;
bool isBlockChanged = false;

int main()
{
	SetBoard();
	HideCursor();
	system("color 3");                      //console color
	system("mode con: cols=75 lines=28");   //console size

	do
	{
		CreateNewBlock();
		DrawBoard();
		GetKeyInput();

		if (CheckGameOver() == true)
		{
			int userAnswer;

			DrawGameOver();
			userAnswer = GetUserAnswer(); // whether to again or stop

			if (userAnswer == AGAIN)
			{
				//system("cls");
				memset(&board, 0, sizeof(board));
				SetBoard();

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

	// Set hold box
	for (i = 0; i < 6; i++)
	{
		holdBox[i][0] = FIXED_BLOCK;
		holdBox[i][5] = FIXED_BLOCK;
		holdBox[0][i] = FIXED_BLOCK;
		holdBox[5][i] = FIXED_BLOCK;
	}

	// Set next box
}

void DrawBoard()
{
	/*GoToXY(5, 3);
	printf("Draw Board");
	Sleep(500);
	GoToXY(5, 3);
	printf("             ");
	Sleep(500);*/

	// Draw main game board.
	for (int i = 0; i <= BOARD_HEIGHT; i++)
	{
		for (int j = 0; j <= BOARD_WIDTH; j++)
		{
			if (i == 2) // draw the ceiling is erased by newBlock 
			{
				if (board[i][j] == EMPTY)
					board[i][j] = CEILING;
			}

			if (board[i][j] != boardCopy[i][j])
			{
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

	// Draw hold box
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			GoToXY(j + OFFSET_X - 8, i + OFFSET_Y + 3);

			if (holdBox[i][j] != holdBoxCopy[i][j])
			{
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

	// Draw next box

	// Draw score

	// Draw best score
}

void CreateNewBlock()
{
	//if (changeHeldBlock == true && isAlreadyHeld == true)
	//{
	//	memcpy(newBlock, blocks[realBlockType], sizeof(blocks[realBlockType]));
	//	changeHeldBlock = false;
	//	//isAlreadyHeld = false;
	//}

	//else
	{
		if (isBlockChanged == true)
		{
			isBlockChanged = false;
		}

		else if (isBlockChanged == false)
		{
			srand(clock());
			nowBlockType = rand() % 7; // There are seven blocks in block.h
		}

		memcpy(newBlock, blocks[nowBlockType], sizeof(blocks[nowBlockType]));
	}

	for (int i = 0; i < 4; i++)
	{
		board[newBlock[i][0]][newBlock[i][1]] = NEW_BLOCK;
	}

	if (nowBlockType == 6)
	{
		barAxisX = 0;
		barAxisY = 4;
	}
}

void GetKeyInput()
{
	clock_t autoDownBaseTime = clock();
	clock_t autoFixBaseTime = clock();

	while (1)
	{
		if ((DetectCollision(0, 0, 1) == true)) // If it dectct collision and over autoFixBaseTime, active FixBlock() function
		{


			if (clock() > autoFixBaseTime + 1000)
			{
				FixBlock();
				RemoveLine();
				return;
			}
		}

		else if (clock() > autoDownBaseTime + autoDownPassedTime) // If it dosen't collide yet, down a block automatically  
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
					FixBlock();
					RemoveLine();

					return;
				}

				MoveBlock(0, 0, 1);
				DrawBoard();
				autoFixBaseTime = clock();

				break;
			}

			case HOLD:
			{
				if (isAlreadyHeld == false)
				{
					HoldBlock();
					isAlreadyHeld = true;

					return;
				}

				/*else if (isAlreadyHeld == false && changeHeldBlock == true)
				{
					HoldBlock();
					return;
				}*/

				else
					break;
			}

			case SPACE:
			{
				DropBlock();
				FixBlock();
				RemoveLine();

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

	if (nowBlockType == 6)
	{
		barAxisX += down;
		barAxisY += left + right;
	}

}

bool DetectCollision(int left, int right, int down)
{


	for (int i = 0; i < 4; i++)
	{
		if (board[newBlock[i][0] + down][newBlock[i][1] + left + right] == EDGE || board[newBlock[i][0] + down][newBlock[i][1] + left + right] == FIXED_BLOCK)
			return true;
	}

	return false;
}

void FixBlock()
{
	Beep(300, 10);

	for (int i = 0; i < 4; i++)
		board[newBlock[i][0]][newBlock[i][1]] = FIXED_BLOCK;

	if (isAlreadyHeld == true)
		isAlreadyHeld = false;
}

void DropBlock()
{
	for (int i = 0; i < 4;i++)
		board[newBlock[i][0]][newBlock[i][1]] = EMPTY;

	while (DetectCollision(0, 0, 1) == false)
	{
		for (int i = 0; i < 4; i++)
			newBlock[i][0]++;
	}
}

void HideCursor()
{
	CONSOLE_CURSOR_INFO CurInfo;
	CurInfo.dwSize = 1;
	CurInfo.bVisible = FALSE;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &CurInfo);
}

void RemoveLine()
{
	bool removeFlag;
	//int previousRow;
	//int x = 15;
	//int y = 21;

	for (int i = 0; i < 4; i++)
	{
		removeFlag = true;
		//previousRow = newBlock[i][0];

		//if (i >= 1)
		//{
		//	if (previousRow == newBlock[i][0]) // Skip overlap row
		//		continue;

		//	else
		//		previousRow = newBlock[i][0];
		//}

		/*GoToXY(15, 20);
		printf("블럭 조각 하나의 x좌표: %d", newBlock[i][0]);
		for (int j = 1; j < BOARD_WIDTH; j++)
		{
			GoToXY(x++, y);
			printf("%d", board[newBlock[i][0]][j]);
			Sleep(500);
		}
		GoToXY(x++, y++);
		printf(" a Line DONE !");
		Sleep(1000);
		x = 15;*/

		/*GoToXY(5, 3);
		printf("Remove line");
		Sleep(300);
		GoToXY(5, 3);
		printf("           ");
		Sleep(300);*/

		for (int j = 1; j < BOARD_WIDTH; j++)
		{
			//if (board[newBlock[i][0]][j] == EMPTY || board[newBlock[i][0]][j] == CEILING ) // If a line dosen't have any EMPTY, replaced a line with one line up.
			//{
			//	removeFlag = false;
			//	break;
			//}

			if (board[newBlock[i][0]][j] != FIXED_BLOCK)
			{
				removeFlag = false;
				break;
			}
		}

		if (removeFlag == true)
		{
			PullLine(newBlock[i][0] - 1);
		}
	}
}

void PullLine(int firstRow)
{
	/*GoToXY(5, 3);
	printf("PULL line");
	Sleep(300);
	GoToXY(5, 3);
	printf("           ");
	Sleep(300);*/

	bool pullFlag;

	for (; firstRow != 3; firstRow--)
	{
		//pullFlag = false;

		/*for (int i = 1; i < BOARD_WIDTH; i++)
		{
			if(board[firstRow][i] == )
		}*/

		for (int i = 1; i < BOARD_WIDTH; i++)
		{
			board[firstRow + 1][i] = board[firstRow][i];

			//if (board[firstRow][i] != EMPTY)
				//pullFlag = true;
		}

		/*if (pullFlag == false)
			return;*/
	}
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

	if (nowBlockType == 6) // bar block
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
	if (nowBlockType == 5) // square block dosen't rotate.
		return true;

	int rotate_Info_Bar[16][2] = { {0,3}, {1,2}, {2,1}, {3,0}, {-1,2},{0, 1},{1,0},{2, -1}, {-2,1},{-1,0},{0,-1},{1,-2},{-3,0},{-2,-1},{-1,-2},{0,-3} };
	int rotate_Info_Others[9][2] = { {0, 2},{1, 1},{2, 0},{-1, 1}, {0, 0}, {1, -1},{-2, 0},{-1, -1},{0, -2} };
	int(*rotate_Info)[2] = NULL;
	int maxCell;
	int startX;
	int startY;
	int index = 0;

	if (nowBlockType == 6) // block - bar
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
	/*GoToXY(5, 3);
	printf("CheckGameOver");
	Sleep(300);
	GoToXY(5, 3);
	printf("             ");
	Sleep(300);*/

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
		Sleep(150);
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

void HoldBlock()
{
	int nowBlockPrototype[4][2];

	// Change block.
	if (isBlockChanged == false)
	{
		int temp = nowBlockType;
		nowBlockType = heldBlockType;
		heldBlockType = temp;
		isBlockChanged = true;
	}

	memcpy(nowBlockPrototype, blocks[heldBlockType], sizeof(blocks[heldBlockType])); // get prototype block in block.h

	for (int k = 0; k < 4; k++)
		board[newBlock[k][0]][newBlock[k][1]] = EMPTY;

	for (int i = 1; i < 5; i++)
		for (int j = 1; j < 5; j++)
			holdBox[i][j] = EMPTY;

	for (int k = 0; k < 4; k++) // Each piece in holdBox will be written as a NEW_BLOCK, if it's position correspond with nowBlock position
	{
		for (int i = 1; i < 5; i++)
		{
			for (int j = 1; j < 5; j++)
			{
				if (nowBlockPrototype[k][0] + 2 == i && nowBlockPrototype[k][1] - 3 == j) // Adding number for adjust the position of nowBlock
				{
					holdBox[i][j] = NEW_BLOCK;
					goto GET_NEW_K; // escape for getting k++
				}
			}
		}

	GET_NEW_K:;
	}
}
