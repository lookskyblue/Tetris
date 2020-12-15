#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "block.h"

#define BOARD_HEIGHT 21
#define BOARD_WIDTH 11
#define EDGE -1
#define EMPTY 0
#define NEW_BLOCK 1
#define FIXED_BLOCK 2
#define SPACE 32
#define LEFT 75
#define RIGHT 77
#define UP 72
#define DOWN 80

void GoToXY(int x, int y);
void SetBoard();
void DrawBoard();
void CreateNewBlock();
void GetKeyInput();
void MoveBlock(int left, int right, int down);
void FixBlock();
void DropBlock();
void HideCursor();
void RemoveLine(); // 어쩌다 블럭 안 지워지는 문제 있음. 고친 후 최적화 해볼 것.
void PullLine(int firstRow);
bool RotateDetectCollision();
void RotateBlock();
bool DetectCollision(int left, int right, int down);

int board[BOARD_HEIGHT + 1][BOARD_WIDTH + 1] = { EMPTY, };
int boardCopy[BOARD_HEIGHT + 1][BOARD_WIDTH + 1] = { EMPTY, };
int newBlock[4][2];
int blockType;
int speed = 20;

int main()
{
	SetBoard();
	HideCursor();
	system("color 3");                      //console color
	system("mode con: cols=30 lines=25");   //console size

	do
	{
		CreateNewBlock();
		DrawBoard();
		GetKeyInput();
	} while (1);
}

void GoToXY(int x, int y)
{
	COORD pos = { x * 2, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void SetBoard()
{
	for (int i = 0; i <= BOARD_HEIGHT; i++)
	{
		board[i][0] = EDGE;
		board[i][BOARD_WIDTH] = EDGE;
	}

	for (int i = 0; i <= BOARD_WIDTH; i++)
	{
		board[0][i] = EDGE;
		board[BOARD_HEIGHT][i] = EDGE;
	}
}

void DrawBoard()
{
	for (int i = 0; i <= BOARD_HEIGHT; i++)
	{
		for (int j = 0; j <= BOARD_WIDTH; j++)
		{
			if (board[i][j] != boardCopy[i][j])
			{
				GoToXY(j, i);

				switch (board[i][j])
				{
				case EDGE:
					printf("▧");
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
				}

				boardCopy[i][j] = board[i][j];
			}
		}
	}
}

void CreateNewBlock()
{
	srand(time(NULL));
	blockType = rand() % 7; // There are seven blocks in block.h
	memcpy(newBlock, blocks[blockType], sizeof(blocks[blockType]));

	for (int i = 0; i < 4; i++)
	{
		board[newBlock[i][0]][newBlock[i][1]] = NEW_BLOCK;
	}
}

void GetKeyInput()
{
	while (1)
	{
		for (int i = 0; i < 50; i++)
		{
			if (_kbhit())
			{
				switch (_getch())
				{
				case UP:
				{
					if(RotateDetectCollision() == false)
						RotateBlock();

					break;
				}

				case LEFT:
				{
					if (DetectCollision(-1, 0, 0) == false)
						MoveBlock(-1, 0, 0);

					break;
				}

				case RIGHT:
				{
					if (DetectCollision(0, 1, 0) == false)
						MoveBlock(0, 1, 0);

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

			Sleep(10);
		}

		if (DetectCollision(0, 0, 1) == true)
		{
			FixBlock();

			return;
		}

		MoveBlock(0, 0, 1);
	}
}

void MoveBlock(int left, int right, int down)
{
	for (int i = 0; i < 4; i++)
		board[newBlock[i][0]][newBlock[i][1]] = EMPTY;

	for (int i = 0; i < 4; i++)
		board[newBlock[i][0] += down][newBlock[i][1] += (left + right)] = NEW_BLOCK;

	DrawBoard();
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
	int previousRow;
	int x = 15;
	int y = 21;

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

		for (int j = 1; j < BOARD_WIDTH; j++)
		{
			if (board[newBlock[i][0]][j] == EMPTY) // If a line dosen't have any EMPTY, replaced a line with one line up.
			{
				removeFlag = false;
				break;
			}
		}

		if (removeFlag == true)
		{
			DrawBoard();
			PullLine(newBlock[i][0] - 1);
		}
	}
}

void PullLine(int firstRow)
{
	/*GoToXY(20, 20);
	printf("PULL LINE");
	Sleep(1000);
	GoToXY(20, 20);
	printf("             ");*/

	bool pullFlag;

	for (; firstRow != 1; firstRow--)
	{
		pullFlag = false;

		for (int i = 1; i < BOARD_WIDTH; i++)
		{
			board[firstRow + 1][i] = board[firstRow][i];

			if (board[firstRow][i] != EMPTY)
				pullFlag = true;
		}

		/*if (pullFlag == false)
			return;*/
	}
}

void RotateBlock()
{
	int shiftCell[9][2] = { {0, 2},{1, 1},{2, 0},{-1, 1}, {0, 0}, {1, -1},{-2, 0},{-1, -1},{0, -2} };
	int startX = newBlock[2][0] - 1;
	int startY = newBlock[2][1] - 1;
	int index = 0;

	for (int i = 0; i < 4; i++)
		board[newBlock[i][0]][newBlock[i][1]] = EMPTY;

	if (blockType >= 0 && blockType <= 4)
	{
		for (int k = 0; k < 4; k++)
		{
			index = 0;

			for (int i = startX; i < startX + 3; i++)
			{
				for (int j = startY; j < startY + 3; j++)
				{
					if (newBlock[k][0] == i && newBlock[k][1] == j)
					{
						newBlock[k][0] += shiftCell[index][0];
						newBlock[k][1] += shiftCell[index][1];

						goto GET_NEW_K; // escape for getting k++
					}

					index++;
				}
			}

		GET_NEW_K:;
		}
	}

	else if (blockType == 6)
	{
		int k0_startX = newBlock[0][0]; // k0 X
		int k0_startY = newBlock[0][1]; // k0 Y
		int k1_startX = newBlock[1][0]; // k1 X
		int k1_startY = newBlock[1][1]; // k1 Y
		int shiftCell[4][4][2] = {
			{{2, 2}, {1, 1}, {0, 0}, {-1, -1}},
			{{-1, 1}, {0, 0}, {1, -1}, {2, -2}},
			{{-2, -2}, {-1, -1}, {0, 0}, {1, 1}},
			{{1, -1}, {0, 0}, {-1, 1}, {-2, 2}}
		};

		if (k0_startY < k1_startY)
		{
			for (int i = 0; i < 4; i++)
			{
				newBlock[i][0] += shiftCell[0][i][0];
				newBlock[i][1] += shiftCell[0][i][1];
			}
		}

		else if (k0_startX > k1_startX)
		{
			for (int i = 0; i < 4; i++)
			{
				newBlock[i][0] += shiftCell[1][i][0];
				newBlock[i][1] += shiftCell[1][i][1];
			}
		}

		else if (k0_startY > k1_startY)
		{
			for (int i = 0; i < 4; i++)
			{
				newBlock[i][0] += shiftCell[2][i][0];
				newBlock[i][1] += shiftCell[2][i][1];
			}
		}

		else if (k0_startX < k1_startX)
		{
			for (int i = 0; i < 4; i++)
			{
				newBlock[i][0] += shiftCell[3][i][0];
				newBlock[i][1] += shiftCell[3][i][1];
			}
		}
	}

	for (int i = 0; i < 4; i++)
		board[newBlock[i][0]][newBlock[i][1]] = NEW_BLOCK;

	DrawBoard();
}

bool RotateDetectCollision()
{
	int shiftCell[9][2] = { {0, 2},{1, 1},{2, 0},{-1, 1}, {0, 0}, {1, -1},{-2, 0},{-1, -1},{0, -2} };
	int startX = newBlock[2][0] - 1;
	int startY = newBlock[2][1] - 1;
	int index = 0;

	if (blockType >= 0 && blockType <= 4)
	{
		for (int k = 0; k < 4; k++)
		{
			index = 0;

			for (int i = startX; i < startX + 3; i++)
			{
				for (int j = startY; j < startY + 3; j++)
				{
					if (newBlock[k][0] == i && newBlock[k][1] == j)
					{
						if (board[newBlock[k][0] + shiftCell[index][0]][newBlock[k][1] + shiftCell[index][1]] == EDGE
							||
							board[newBlock[k][0] + shiftCell[index][0]][newBlock[k][1] + shiftCell[index][1]] == FIXED_BLOCK)
							return true;
						
						goto GET_NEW_K; // escape for getting k++
					}

					index++;
				}
			}

		GET_NEW_K:;
		}
	}

	else if (blockType == 6)
	{
		int k0_startX = newBlock[0][0]; // k0 X
		int k0_startY = newBlock[0][1]; // k0 Y
		int k1_startX = newBlock[1][0]; // k1 X
		int k1_startY = newBlock[1][1]; // k1 Y
		int shiftCell[4][4][2] = {
			{{2, 2}, {1, 1}, {0, 0}, {-1, -1}},
			{{-1, 1}, {0, 0}, {1, -1}, {2, -2}},
			{{-2, -2}, {-1, -1}, {0, 0}, {1, 1}},
			{{1, -1}, {0, 0}, {-1, 1}, {-2, 2}}
		};

		if (k0_startY < k1_startY)
		{
			for (int i = 0; i < 4; i++)
			{
				if (board[newBlock[i][0] + shiftCell[0][i][0]][newBlock[i][1] + shiftCell[0][i][1]] == EDGE
					||
					board[newBlock[i][0] + shiftCell[0][i][0]][newBlock[i][1] + shiftCell[0][i][1]] == FIXED_BLOCK)
					return true;
			}
		}

		else if (k0_startX > k1_startX)
		{
			for (int i = 0; i < 4; i++)
			{
				if (board[newBlock[i][0] + shiftCell[1][i][0]][newBlock[i][1] + shiftCell[1][i][1]] == EDGE
					||
					board[newBlock[i][0] + shiftCell[1][i][0]][newBlock[i][1] + shiftCell[1][i][1]] == FIXED_BLOCK)
					return true;
			}
		}

		else if (k0_startY > k1_startY)
		{
			for (int i = 0; i < 4; i++)
			{
				if (board[newBlock[i][0] + shiftCell[2][i][0]][newBlock[i][1] + shiftCell[2][i][1]] == EDGE
					||
					board[newBlock[i][0] + shiftCell[2][i][0]][newBlock[i][1] + shiftCell[2][i][1]] == FIXED_BLOCK)
					return true;
			}
		}

		else if (k0_startX < k1_startX)
		{
			for (int i = 0; i < 4; i++)
			{
				if (board[newBlock[i][0] + shiftCell[3][i][0]][newBlock[i][1] + shiftCell[3][i][1]] == EDGE
					||
					board[newBlock[i][0] + shiftCell[3][i][0]][newBlock[i][1] + shiftCell[3][i][1]] == FIXED_BLOCK)
					return true;
			}
		}
	}

	return false;
}
