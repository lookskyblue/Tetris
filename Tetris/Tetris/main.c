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
bool DetectFloor();

int board[BOARD_HEIGHT + 1][BOARD_WIDTH + 1] = { EMPTY, };
int newBlock[4][2];

int main()
{
	SetBoard();

	DrawBoard();
	do
	{
		CreateNewBlock();
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
			GoToXY(j, i);

			if (board[i][j] == EDGE)
			{
				printf("в╦");
			}

			else if (board[i][j] == NEW_BLOCK)
			{
				printf("бс");
			}

			else if (board[i][j] == FIXED_BLOCK)
			{
				printf("бр");
			}

			else if (board[i][j] == EMPTY)
			{
				printf("  ");
			}
		}
	}
}

void CreateNewBlock()
{
	int blockNumber;

	srand(time(NULL));
	blockNumber = rand() % 7; // There are seven blocks in block.h
	memcpy(newBlock, blocks[blockNumber], sizeof(blocks[blockNumber]));

	for (int i = 0; i < 4; i++)
		board[newBlock[i][0]][newBlock[i][1]] = NEW_BLOCK;

	DrawBoard();
}

void GetKeyInput()
{
	while (1)
	{
		if (_kbhit())
		{
			switch (_getch())
			{
			case UP:
				break;

			case LEFT:
				MoveBlock(-1, 0, 0);
				break;

			case RIGHT:
				MoveBlock(0, 1, 0);
				break;

			case DOWN:
			{
				MoveBlock(0, 0, 1);

				if (DetectFloor() == true)
				{
					FixBlock();
					return;
				}

				break;
			}

			case SPACE:
				DropBlock();
				FixBlock();
				return;
				
			default:
				break;
			}
		}
	}
}

void MoveBlock(int left, int right, int down)
{
	for (int i = 0; i < 4; i++)
	{
		if (board[newBlock[i][0] + down][newBlock[i][1] + left + right] == EDGE || board[newBlock[i][0] + down][newBlock[i][1] + left + right] == FIXED_BLOCK)
		{
			Beep(3000, 10);
			return;
		}
	}

	for (int i = 0; i < 4; i++)
		board[newBlock[i][0]][newBlock[i][1]] = EMPTY;

	for (int i = 0; i < 4; i++)
		board[newBlock[i][0] += down][newBlock[i][1] += left + right] = NEW_BLOCK;

	DrawBoard();
	Beep(300, 10);
}

bool DetectFloor()
{
	for (int i = 0; i < 4; i++)
	{
		if (board[newBlock[i][0] + 1][newBlock[i][1]] == EDGE || board[newBlock[i][0] + 1][newBlock[i][1]] == FIXED_BLOCK)
			return true;
	}

	return false;
}

void FixBlock()
{
	for (int i = 0; i < 4; i++)
		board[newBlock[i][0]][newBlock[i][1]] = FIXED_BLOCK;
}

void DropBlock()
{
	for (int i = 0; i < 4;i++)
		board[newBlock[i][0]][newBlock[i][1]] = EMPTY;

	while (DetectFloor() == false)
	{
			for (int i = 0; i < 4; i++)
				newBlock[i][0]++;
	}
}