#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "block.h"
#include "rotate_Info.h"
#include "wall_kick_convention.h"

#define COUNTER_CLOCK_ROTATE 122 // ascii 122 'z'
#define UPPER_COUNTER_CLOCK_ROTATE 90 // ascii 90 'Z'
#define CHANGE_BLOCK_COLOR 9     // tab
#define SOUND_TOGGLE 109         // alp 'm' 
#define CLOCK_ROTATE 72          // ascii up key.
#define BOARD_HEIGHT 23
#define SIDE_BOX_SIZE 6
#define BOARD_WIDTH 11
#define FIXED_BLOCK 2
#define FIRST_HOLD -1
#define NEW_BLOCK 1
#define CEILING -2
#define OFFSET_X 13
#define OFFSET_Y 3
#define RIGHT 77
#define AGAIN 97                 // alp 'a'
#define SPACE 32
#define ENTER 13
#define EDGE -1
#define EMPTY 0
#define LEFT 75
#define DOWN 80
#define HOLD 99                  // alp 'c'
#define UPPER_HOLD 67		     // alp 'C'
#define STOP 0
#define ESC 27

int  BalancingLineScoreByLevel(int removedLine);
int  GetUserAnswer();
int  RemoveLine();
int  DropBlock();

bool DetectCollision(int RightOrLeft, int down, int(*block)[2]);
bool DetectRotateCollision(int direction);
bool WallKick(int direction);
bool IsGameOver();

void MoveRightOrLeft(int direction);
void ActiveRemoveLineEffect(const int* const x, int index);
void ChangeRotateState(int direction, int* const state);
void RotateBlock(int direction, int(*block)[2]);
void MoveBlock(int RightOrLeft, int down);
void RotationByDirection(int direction);
void AddComma(char* result, int score);
void AddGameScore(int score);
void CheckForegroundWindow();
void FixingBlockProcedure();
void LevelUpByRemovedLine();
void ProcessNextBlockStep();
void StartGameOverProcess();
void AutomaticBlockDown();
void GoToXY(int x, int y);
void LoadBestScoreFile();
void ChangeBlockColor();
void CreateNextBlock();
void PullLine(int row);
void InitGameSetting();
void DrawFirstScreen();
void EGKey(int EGNum);
void DrawHowToPlay();
void OverDrawPauseText(const char str[]);
void SaveBestScore();
int  CheckSevenBag();
void StartMainGame(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime);
void DrawGameOver();
void GetNextBlock();
void HandleKeyInput();
void Unlock_Hold();
void SoundToggle();
int GetKeyInput();
void ResetTimer();
void HideCursor();
void Hold_Block();
void DrawBoard();
void SetBoard();
void FixBlock();
void EG1();
void EG2();

typedef struct GameSetting
{
	int  rotateState;
	int  blockColorNum;
	int  gameLevel;
	int  removedLine;
	int  nowScore;
	int  nextBlockType;
	int  nowBlockType;
	int  heldBlockType; // Init FIRST_HOLD
	double  autoDownPassedTime;
	double  autoFixPassedTime;
	int  bestScore;
	int  sevenBagIndex;
	int  sevenBag[7];
	bool isLevelUp;
	bool hold_Lock; // Init false
	bool isExecutedHold; // Init false
	bool isNextBlockEmpty; // Init true
	bool soundOn;
}GAMESETTING;

int boardCopy[BOARD_HEIGHT + 1][BOARD_WIDTH + 1] = { EMPTY, };
int board[BOARD_HEIGHT + 1][BOARD_WIDTH + 1] = { EMPTY, };
int holdBoxCopy[SIDE_BOX_SIZE][SIDE_BOX_SIZE] = { EMPTY, };
int holdBox[SIDE_BOX_SIZE][SIDE_BOX_SIZE] = { EMPTY, };
int nextBoxCopy[SIDE_BOX_SIZE][SIDE_BOX_SIZE] = { EMPTY, };
int nextBox[SIDE_BOX_SIZE][SIDE_BOX_SIZE] = { EMPTY, };
int newBlock[4][2];
int wallKickCount;
int barAxisX;
int barAxisY;
bool isWallKickLock;
bool is_pause_game = false;
clock_t wallKickLockTime;
clock_t autoFixBaseTime;
clock_t autoDownBaseTime;
GAMESETTING gs;

HWND foreground_window_name;

int main()
{
	foreground_window_name = GetForegroundWindow();

	system("mode con: cols=75 lines=28");
	HideCursor();
	
	DrawFirstScreen();

	InitGameSetting();
	SetTimer(NULL, 1, 10, StartMainGame);

	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void StartMainGame(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime)
{
	CheckForegroundWindow();

	if (is_pause_game == true)
	{
		// 애석하게도 clock 경과에 따른 block fix처리하는 함수까지는 손을 못봄.. 블록이 땅에 닿았을 경우 pause 일정 시간 뒤 resume 되면 block 바로 fix됨. 

		if (GetKeyInput() == ESC)
		{
			is_pause_game = false;
			OverDrawPauseText("        ");
		}
	}
	else
	{
		AutomaticBlockDown();
		HandleKeyInput();
		DrawBoard();

		if (IsGameOver() == true)
		{
			StartGameOverProcess();
		}
	}
}

void ResetTimer()
{
	autoFixBaseTime = clock();
	autoDownBaseTime = clock();
	gs.autoFixPassedTime = 1000;
}

void StartGameOverProcess()
{
	SaveBestScore();
	DrawGameOver();

	if (GetUserAnswer() == AGAIN) // whether to again or exit
	{
		InitGameSetting();
	}
	else
	{
		exit(EXIT_SUCCESS);
	}
}

void CheckForegroundWindow()
{
	if (foreground_window_name != GetForegroundWindow())
	{
		is_pause_game = true;
		OverDrawPauseText("PAUSE...");
	}
}

int GetKeyInput()
{
	int keyValue = -1;

	if (_kbhit())
	{
		keyValue = _getch();

		if (keyValue == 227)
		{
			keyValue = _getch();
		}
	}

	return keyValue;
}

void ProcessNextBlockStep()
{
	ResetTimer();
	GetNextBlock();
	LevelUpByRemovedLine();
}

void HandleKeyInput()
{
	int keyValue = GetKeyInput();

	isWallKickLock = false;
	wallKickCount = 0;

	switch (keyValue)
	{
	case CLOCK_ROTATE: // key up
	{
		RotationByDirection(CLOCK_ROTATE);

		break;
	}

	case COUNTER_CLOCK_ROTATE: // key 'z'
	case UPPER_COUNTER_CLOCK_ROTATE:
	{
		RotationByDirection(COUNTER_CLOCK_ROTATE);

		break;
	}

	case LEFT:
	{
		MoveRightOrLeft(LEFT);

		break;
	}

	case RIGHT:
	{
		MoveRightOrLeft(RIGHT);

		break;
	}

	case DOWN:
	{
		if (DetectCollision(0, 1, newBlock) == false)
		{
			MoveBlock(0, 1);
			AddGameScore(1);
			autoFixBaseTime = clock();
		}

		break;
	}

	case HOLD:
	case UPPER_HOLD:
	{
		if (gs.hold_Lock == false)
		{
			Hold_Block();
			gs.hold_Lock = true;
			ProcessNextBlockStep();

		}

		break;
	}

	case SPACE:
	{
		AddGameScore(DropBlock() * 2);
		FixingBlockProcedure();
		ProcessNextBlockStep();

		break;
	}

	case SOUND_TOGGLE: // mute
	{
		SoundToggle();

		break;
	}

	case CHANGE_BLOCK_COLOR:
	{
		ChangeBlockColor();

		break;
	}

	case 'b':
	case 'B':
	{
		EG2();

		break;
	}

	// added a function. check pause resume. 2021-10-22
	case ESC:
	{
		is_pause_game = true;
		OverDrawPauseText("PAUSE...");

		break;
	}

	default:
	{
		break;
	}
	}
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
	for (i = 0; i < SIDE_BOX_SIZE; i++)
	{
		holdBox[i][0] = FIXED_BLOCK;
		holdBox[i][SIDE_BOX_SIZE - 1] = FIXED_BLOCK;
		holdBox[0][i] = FIXED_BLOCK;
		holdBox[SIDE_BOX_SIZE - 1][i] = FIXED_BLOCK;

		nextBox[i][0] = FIXED_BLOCK;
		nextBox[i][SIDE_BOX_SIZE - 1] = FIXED_BLOCK;
		nextBox[0][i] = FIXED_BLOCK;
		nextBox[SIDE_BOX_SIZE - 1][i] = FIXED_BLOCK;
	}
}

void DrawBoard()
{
	char scoreAddComma[20];
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

	// Draw side box
	for (int i = 0; i < SIDE_BOX_SIZE; i++)
	{
		for (int j = 0; j < SIDE_BOX_SIZE; j++)
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
	AddComma(scoreAddComma, gs.nowScore);
	printf("%s", scoreAddComma);

	// Draw level
	GoToXY(OFFSET_X - 8, OFFSET_Y + 13);
	printf("LEVEL");

	GoToXY(OFFSET_X - 8, OFFSET_Y + 15);
	printf("%d", gs.gameLevel);

	// Draw lines
	GoToXY(OFFSET_X - 8, OFFSET_Y + 17);
	printf("LINES");

	GoToXY(OFFSET_X - 8, OFFSET_Y + 19);
	printf("%d", gs.removedLine);

	GoToXY(OFFSET_X + 14, OFFSET_Y + 17);
	printf("SOUND ON/OFF: M");

	GoToXY(OFFSET_X + 14, OFFSET_Y + 19);
	printf("COLOR: TAB");
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

	gs.rotateState = 0;
}

void CreateNextBlock()
{
	int nextBlockPrototype[4][2];

	srand(clock());
	gs.nextBlockType = CheckSevenBag();
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

void OverDrawPauseText(const char str[])
{
	GoToXY(15 + OFFSET_X, 12 + OFFSET_Y);
	printf("%s", str);
}

void MoveBlock(int RightOrLeft, int down)
{
	int i;

	for (i = 0; i < 4; i++)
		board[newBlock[i][0]][newBlock[i][1]] = EMPTY;

	for (i = 0; i < 4; i++)
		board[newBlock[i][0] += down][newBlock[i][1] += RightOrLeft] = NEW_BLOCK;

	if (gs.nowBlockType == 6)
	{
		barAxisX += down;
		barAxisY += RightOrLeft;
	}
}

bool DetectCollision(int RightOrLeft, int down, int(*block)[2])
{
	int movedBlockPiece;

	for (int i = 0; i < 4; i++)
	{
		movedBlockPiece = board[block[i][0] + down][block[i][1] + RightOrLeft];

		if (movedBlockPiece == EDGE || movedBlockPiece == FIXED_BLOCK)
			return true;
	}

	return false;
}

void FixBlock()
{
	if (gs.soundOn == true)
		Beep(200, 10);

	for (int i = 0; i < 4; i++)
		board[newBlock[i][0]][newBlock[i][1]] = FIXED_BLOCK;
}

int DropBlock()
{
	int dropCount = 0;
	int i = 0;

	for (i = 0; i < 4;i++)
		board[newBlock[i][0]][newBlock[i][1]] = EMPTY;

	for (dropCount = 1; DetectCollision(0, dropCount, newBlock) == false; dropCount++);

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

	for (k = 0; k < 2; k++)
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
		Beep(100, 100);
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

void RotateBlock(int direction, int(*block)[2])
{
	// square block is already returned 
	int(*rotate_Info)[2] = NULL;
	int maxCell;
	int startX;
	int startY;
	int index;
	int i, j, k;
	int x = 0, y = 1, mul = 1;

	if (direction == COUNTER_CLOCK_ROTATE)
	{
		x = 1;
		y = 0;
		mul = -1;
	}

	if (gs.nowBlockType == 6) // bar block
	{
		rotate_Info = c_Rotate_Info_Bar;
		startX = barAxisX;
		startY = barAxisY;
		maxCell = 4;
	}

	else // the others
	{
		rotate_Info = c_Rotate_Info_Others;
		startX = block[2][0] - 1;
		startY = block[2][1] - 1;
		maxCell = 3;
	}

	for (k = 0; k < 4; k++)
	{
		index = 0;

		for (i = startX; i < startX + maxCell; i++)
		{
			for (j = startY; j < startY + maxCell; j++)
			{
				if (block[k][0] == i && block[k][1] == j)
				{
					block[k][0] += rotate_Info[index][x];
					block[k][1] += rotate_Info[index][y] * mul;

					goto GET_NEW_K; // escape for getting k++
				}

				index++;
			}
		}

	GET_NEW_K:;
	}
}

bool DetectRotateCollision(int direction)
{
	// square block is already returned 
	int(*rotate_Info)[2] = NULL;
	int maxCell;
	int startX;
	int startY;
	int index = 0;
	int i, j, k;
	int x = 0, y = 1, mul = 1;

	if (direction == COUNTER_CLOCK_ROTATE) // set revers turn value
	{
		x = 1;
		y = 0;
		mul = -1;
	}

	if (gs.nowBlockType == 6) // block - bar
	{
		startX = barAxisX;
		startY = barAxisY;
		maxCell = 4;
		rotate_Info = c_Rotate_Info_Bar;
	}

	else // block - the others
	{
		startX = newBlock[2][0] - 1;
		startY = newBlock[2][1] - 1;
		maxCell = 3;
		rotate_Info = c_Rotate_Info_Others;
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
					int movePieceByX = rotate_Info[index][x];
					int movePieceByY = rotate_Info[index][y] * mul;
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

bool IsGameOver()
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
	char scoreAddComma[20];

	LoadBestScoreFile();
	AddComma(scoreAddComma, gs.bestScore);

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
		GoToXY(OFFSET_X + 3, OFFSET_Y - 2);
		printf("          ");

		GoToXY(OFFSET_X + 3, OFFSET_Y - 1);
		printf("           ");
		Sleep(250);

		GoToXY(OFFSET_X + 3, OFFSET_Y - 2);
		printf("BEST SCORE");

		GoToXY(OFFSET_X + 3, OFFSET_Y - 1);
		printf("%s", scoreAddComma);
		Sleep(250);
	}

	DrawBoard();

	for (i = 0; i < 5; i++)
	{
		for (j = 1; j < BOARD_WIDTH; j++)
		{
			board[8 + i][j] = EMPTY;
		}
	}


	for (i = 0; i < 2; i++)
	{
		GoToXY(OFFSET_X + 3, OFFSET_Y + 9);
		printf("          ");

		GoToXY(OFFSET_X + 3, OFFSET_Y + 11);
		printf("         ");
		Sleep(350);

		GoToXY(OFFSET_X + 3, OFFSET_Y + 9);
		printf("AGAIN: A");

		GoToXY(OFFSET_X + 3, OFFSET_Y + 11);
		printf("STOP: ESC");
		Sleep(350);
	}
}

int GetUserAnswer()
{
	char userKeyInput = 0;

	while (1)
	{
		userKeyInput = GetKeyInput();

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

	if (gs.nowScore == 777 || gs.nowScore == 7777 || gs.nowScore == 77777 || gs.nowScore == 777777)
		EG1();
}

void InitGameSetting()
{
	system("cls");
	system("color 0");
	HideCursor();
	memset(&board, 0, sizeof(board));
	memset(&boardCopy, 0, sizeof(boardCopy));
	memset(&holdBox, 0, sizeof(holdBox));
	memset(&holdBoxCopy, 0, sizeof(holdBoxCopy));
	memset(&nextBox, 0, sizeof(nextBox));
	memset(&nextBoxCopy, 0, sizeof(nextBoxCopy));
	memset(&newBlock, 0, sizeof(newBlock));
	memset(&gs.sevenBag, -1, sizeof(gs.sevenBag));
	LoadBestScoreFile();
	SetBoard(); // Board && hold box && next box

	gs.gameLevel = 1;
	gs.sevenBagIndex = 0;
	gs.nowScore = 0;
	gs.removedLine = 0;
	gs.autoDownPassedTime = 1000;
	gs.autoFixPassedTime = 1000;
	gs.heldBlockType = FIRST_HOLD;
	gs.hold_Lock = false;
	gs.isExecutedHold = false;
	gs.isNextBlockEmpty = true;
	gs.isLevelUp = false;
	gs.soundOn = true;
	gs.blockColorNum = 7;

	ResetTimer();
	GetNextBlock();
}

void SaveBestScore()
{
	if (gs.bestScore >= gs.nowScore)
	{
		return;
	}

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
	double speedUpRatio = 0.1; // I think Init value 0.1 ~ 0.3 is good 
	int levelUp_Per_Lines = 2; // 4~6 is good. 10 is so long game..
	int minimumPassedTime = 10;
	int level = (gs.removedLine / levelUp_Per_Lines) + 1;

	if (680 < gs.autoDownPassedTime && gs.autoDownPassedTime < 1000)
		speedUpRatio = 0.09;

	else if (382 < gs.autoDownPassedTime && gs.autoDownPassedTime < 680)
		speedUpRatio = 0.07;

	else if (100 < gs.autoDownPassedTime && gs.autoDownPassedTime < 382)
		speedUpRatio = 0.05;

	else if (50 < gs.autoDownPassedTime && gs.autoDownPassedTime < 100)
		speedUpRatio = 0.04;

	else
		speedUpRatio = 0.03;

	while (gs.gameLevel < level)
	{
		gs.gameLevel++;

		if (gs.autoDownPassedTime <= minimumPassedTime)
		{
			gs.autoDownPassedTime = minimumPassedTime;
			break;
		}

		gs.autoDownPassedTime -= gs.autoDownPassedTime * speedUpRatio;
	}
}

void SoundToggle()
{
	gs.soundOn = !gs.soundOn;
}

void DrawHowToPlay()
{
	puts("\n\n\n\n\n\n");
	puts("                            ■■■■■■■■■■■■■■");
	puts("                            ■                        ■");
	puts("                            ■   SOUND  ON/OFF: M     ■");
	puts("                            ■   CHANGE COLOR: TAB    ■");
	puts("                            ■   LEFT:          ←    ■");
	puts("                            ■   RIGHT:         →    ■");
	puts("                            ■   HARD DROP:   SPACE   ■");
	puts(" EXIT: ESC                  ■                        ■");
	puts("                            ■   SOFT DROP:     ↓    ■");
	puts("                            ■   ROTATE RIGHT:  ↑    ■");
	puts("                            ■   ROTATE LEFT:    Z    ■");
	puts("                            ■   HOLD:           C    ■");
	puts("                            ■   PAUSE OR RESUME: ESC ■");
	puts("                            ■                        ■");
	puts("                            ■■■■■■■■■■■■■■ \n");

	while (1)
	{
		if (GetKeyInput() == ESC)
		{
			return;
		}
	}
}

void DrawFirstScreen()
{
	const int START = 0;
	const int METHOD = 1;
	int choice = 0;
	int offSetX = 15;
	int offSetY = 23;

	while (1)
	{
		GoToXY(offSetX, offSetY - 10);
		puts("[ TETRIS GAME ]");

		if (choice > METHOD)
			choice = START;

		else if (choice < START)
			choice = METHOD;

		switch (GetKeyInput())
		{
		case CLOCK_ROTATE:
			choice--;
			break;

		case DOWN:
			choice++;
			break;

		case ENTER:
		{
			if (choice == START)
				return;

			else if (choice == METHOD)
			{
				system("cls");
				DrawHowToPlay();
				system("cls");
			}

			break;
		}

		default:
			break;
		}

		if (choice == START)
		{
			GoToXY(offSetX, offSetY);
			puts(" ▶  START  ◀ ");
			GoToXY(offSetX - 1, offSetY + 2);
			puts("    HOW TO PLAY    ");
		}

		else
		{
			GoToXY(offSetX, offSetY);
			puts("     START     ");
			GoToXY(offSetX - 1, offSetY + 2);
			puts("▶  HOW TO PLAY  ◀");
		}
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

void ChangeRotateState(int direction, int* const state)
{
	if (direction == CLOCK_ROTATE)
	{
		if (++(*state) >= 4)
			*state = 0;
	}

	else
	{
		if (--(*state) <= -1)
			*state = 3;
	}
}

void RotationByDirection(int direction)
{
	if (gs.nowBlockType == 5)
		return;

	if (DetectRotateCollision(direction) == false)
	{
		for (int i = 0; i < 4; i++)
			board[newBlock[i][0]][newBlock[i][1]] = EMPTY;

		RotateBlock(direction, newBlock);

		for (int i = 0; i < 4; i++)
			board[newBlock[i][0]][newBlock[i][1]] = NEW_BLOCK;

		ChangeRotateState(direction, &gs.rotateState);
	}

	else // if detect rotate collision, active wall kick function
	{
		if (wallKickCount == 5)
		{
			isWallKickLock = true;
			wallKickCount = 0;
			wallKickLockTime = clock();
		}

		if (isWallKickLock == true)
		{
			if (clock() > wallKickLockTime + 1000)
				isWallKickLock = false;
		}

		else if (isWallKickLock == false)
		{
			if (WallKick(direction) == true)
			{
				ChangeRotateState(direction, &gs.rotateState);
				wallKickCount++;
				gs.autoFixPassedTime -= 100;
			}
		}
	}
}

bool WallKick(int direction)
{
	int newBlockCopy[4][2];
	int nextState = gs.rotateState;
	int(*testCase)[4][2] = NULL;
	int conventionValues;
	int i, j, x, y;

	memcpy(newBlockCopy, newBlock, sizeof(newBlock));
	RotateBlock(direction, newBlockCopy);
	ChangeRotateState(direction, &nextState);
	conventionValues = convention[gs.rotateState][nextState];

	if (gs.nowBlockType == 6)
		testCase = barTestCase;

	else
		testCase = othersTestCase;

	for (i = 0; i < 4; i++)
	{
		x = testCase[conventionValues][i][0];
		y = testCase[conventionValues][i][1];

		if (DetectCollision(y, x, newBlockCopy) == false)
		{
			for (j = 0; j < 4; j++)
				board[newBlock[j][0]][newBlock[j][1]] = EMPTY;

			memcpy(newBlock, newBlockCopy, sizeof(newBlock));

			for (j = 0; j < 4; j++)
				board[newBlock[j][0] += x][newBlock[j][1] += y] = NEW_BLOCK;

			if (gs.nowBlockType == 6)
			{
				barAxisX += x;
				barAxisY += y;
			}

			return true;
		}
	}

	return false;
}

int CheckSevenBag()
{
	int blockType;
	bool putFlag;

	do
	{
		putFlag = true;
		srand(clock());
		blockType = rand() % 7;

		for (int i = 0; i < gs.sevenBagIndex; i++)
		{
			if (gs.sevenBag[i] == blockType)
			{
				putFlag = false;
				break;
			}
		}

		if (putFlag == true)
		{
			gs.sevenBag[gs.sevenBagIndex++] = blockType;

			if (gs.sevenBagIndex == 7)
			{
				memset(&gs.sevenBag, -1, sizeof(gs.sevenBag));
				gs.sevenBagIndex = 0;
			}

			return blockType;
		}

	} while (1);
}

void AutomaticBlockDown()
{
	if ((DetectCollision(0, 1, newBlock) == true)) // If it dectct collision && over autoFixBaseTime, active FixBlock() function
	{
		if (clock() > autoFixBaseTime + gs.autoFixPassedTime)
		{
			FixingBlockProcedure();

			ProcessNextBlockStep();
		}
	}

	else if (clock() > autoDownBaseTime + gs.autoDownPassedTime) // If it dosen't collide yet, down a block automatically  
	{
		autoDownBaseTime = clock();
		autoFixBaseTime = clock();
		MoveBlock(0, 1);
	}
}

void AddComma(char* scoreAddComma, int score)
{
	char str[20];
	char* p = str;
	int maxStr = 0;
	int scoreCopy = score;

	for (maxStr++; (scoreCopy /= 10) != 0; maxStr++);

	for (int i = 0; i < maxStr; i++)
	{
		if (i && (i % 3) == 0)
			*p++ = ',';

		*p++ = score % 10 + '0';
		score /= 10;
	}

	p--;

	while (p >= str)
		*scoreAddComma++ = *p--;

	*scoreAddComma = 0;
}

void EG1()
{
	char msg[5][50] = {
		"You got exactly ",
		" points. Congratulations!",
		"\nI'll remove all fixed block.",
		"\nPlease press 'Q'",
		"\nHave a nice game :)"
	};

	system("cls");

	for (int i = 0; i < 5; i++)
	{
		int length = strlen(msg[i]);

		if (i == 1)
			printf("%d", gs.nowScore);

		for (int j = 0; j < length; j++)
		{
			printf("%c", msg[i][j]);
			Sleep(85);
		}
	}

	EGKey(1);
}

void EG2()
{
	char msg[4][40] = {
		"Great! You found a hidden key.",
		"\nBut it has no function.",
		"\nPlease press 'Q'",
		"\nHave a nice game :)"
	};

	system("cls");

	for (int i = 0; i < 4; i++)
	{
		int length = strlen(msg[i]);

		for (int j = 0; j < length; j++)
		{
			printf("%c", msg[i][j]);
			Sleep(100);
		}
	}

	EGKey(2);
}

void EGKey(int EGNum)
{
	while (1)
	{
		if (tolower(GetKeyInput()) == 'q')
		{
			system("cls");

			if (EGNum == 1)
			{
				memset(&board, 0, sizeof(board));
				SetBoard();
			}

			memset(&boardCopy, 0, sizeof(boardCopy));
			memset(&holdBoxCopy, 0, sizeof(holdBoxCopy));
			memset(&nextBoxCopy, 0, sizeof(nextBoxCopy));
			DrawBoard();

			return;
		}
	}
}

void MoveRightOrLeft(int direction)
{
	int dir = 1;

	if (direction == LEFT)
		dir = -1;

	if (DetectCollision(dir, 0, newBlock) == false)
	{
		MoveBlock(dir, 0);
	}
}
