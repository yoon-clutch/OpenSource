#include "pch.h"
#include <iostream>
#include<stdio.h>
#include<windows.h>
#include<conio.h>
#include<stdlib.h>
#include<time.h>
#include<assert.h>
#pragma warning(disable:4996)
//키값을 지정
#define LEFT 75
#define RIGHT 77
#define UP 72
#define DOWN 80
#define PAUSE 112
#define ESC 27
#define STATUS 115


#define MAP_ADJ_X 3// 실행창의 (0,0)로부터 떨어진 거리 
#define MAP_ADJ_Y 2
#define MAP_X 30//맵의 가로길이
#define MAP_Y 20//맵의 세로길이   

int x[100], y[100];
int food_x, food_y; //food 위치좌표
int swamp_x, swamp_y; //늪 위치좌표
int length; //뱀의 길이
int speed; //뱀의 속도
int score; //게임 점수
int best_score = 0; //게임내의 최대 점수
int last_score = 0; //현재 게임의 점수
int dir; //방향값을 저장하는 변수
int key; //입력받는 key값을 저장하는 변수
int status_on = 0; //?

//좌표값으로 가서 문자?를 출력해준다.
void gotoxy(int x, int y, const char* s) {
	COORD pos = { 2 * x,y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
	printf("%s", s);
}


void title(void); //최초 화면
void start_game(void); //최초 설정
void draw_snake(void);//뱀 그리기
void draw_map(void); //벽 그리기
void move(int dir); //게임 규칙(food에 닿았을때,swmap에 닿았을때,자신의 몸통에 닿았을때)
void pause(void); //게임 멈춤
void game_over(void); //게임종료
void food(void); //food의 위치 설정
void swamp(void); //swamp의 위치 설정
void status(void); //관지자용 정보
void wallcrash(void); //벽면에 닿을시 함수
void detect_collision(void); //food에 닿을시,자신의 몸과 닿을시,swamp에 닿을시 함수
void print_manual(void); //게임을 어떻게 하는지 가르쳐주는 메뉴얼
void check_overlap(void); //먹이랑 함정이랑 같은 좌표에서 출력될 경우 assert 함수를 통해 디버그오류를 냄
//방향키와 p=일시정지, ESC=종료, s=관리자용 정보
int main() {
	title(); //타이틀 화면 출력
	while (1)
	{
		if (kbhit())
		{
			do { key = getch(); } while (key == 224);
		}
		while (_kbhit()) getch();
		Sleep(speed);
		switch (key) {
		case LEFT:
		case RIGHT:
		case UP:
		case DOWN:
			//180 needed to prevent rotation movement
			if ((dir == LEFT && key != RIGHT) ||
				(dir == RIGHT && key != LEFT) ||
				(dir == UP && key != DOWN) ||
				(dir == DOWN && key != UP))
			{
				dir = key;
			}


			key = 0;
			break;
		case PAUSE:
			pause();
			break;
		case STATUS:
			if (status_on == 0) status_on = 1;
			else status_on = 0;
			key = 0;
			break;
		case ESC:
			exit(0);
		default:
			break;
		}

		move(dir);
		if (status_on == 1) status();
	}
}

//처음 타이틀화면 출력하는 함수
void title(void) {
	int i, j;
	draw_map(); // 겉테두리를 그려준다
	//버튼을 입력받을 때까지, 아래의 화면창을 계속 출력해준다.?
	while (kbhit()) getch();
	for (i = MAP_ADJ_Y + 1; i < MAP_ADJ_Y + MAP_Y - 1; i++) {
		for (j = MAP_ADJ_X + 1; j < MAP_ADJ_X + MAP_X - 1; j++) gotoxy(j, i, "  ");
	}

	gotoxy(MAP_ADJ_X + (MAP_X / 2) - 7, MAP_ADJ_Y + 5, "+--------------------------+");
	gotoxy(MAP_ADJ_X + (MAP_X / 2) - 7, MAP_ADJ_Y + 6, "|        S N A K E         |");
	gotoxy(MAP_ADJ_X + (MAP_X / 2) - 7, MAP_ADJ_Y + 7, "+--------------------------+");

	gotoxy(MAP_ADJ_X + (MAP_X / 2) - 7, MAP_ADJ_Y + 9, " < PRESS ANY KEY TO START > ");

	gotoxy(MAP_ADJ_X + (MAP_X / 2) - 7, MAP_ADJ_Y + 11, "   ◇ ←,→,↑,↓ : Move    ");
	gotoxy(MAP_ADJ_X + (MAP_X / 2) - 7, MAP_ADJ_Y + 12, "   ◇ P : Pause             ");
	gotoxy(MAP_ADJ_X + (MAP_X / 2) - 7, MAP_ADJ_Y + 13, "   ◇ ESC : Quit              ");


	while (1) {
		if (kbhit()) {
			key = getch();
			if (key == ESC) exit(0);
			else if (key == LEFT) key = RIGHT;
			else break;
		}
		gotoxy(MAP_ADJ_X + (MAP_X / 2) - 7, MAP_ADJ_Y + 9, " < PRESS ANY KEY TO START > ");
		Sleep(400);
		gotoxy(MAP_ADJ_X + (MAP_X / 2) - 7, MAP_ADJ_Y + 9, "                            ");
		Sleep(400);

	}
	while (_kbhit()) getch();
	print_manual();
}

//게임을 시작하게하면 실행되는 함수.
//최초설정 속도:100, 몸통길이:4 스코어: 0
void start_game(void) {
	int i;
	system("cls"); //화면을 초기화 시켜준다
	draw_map(); //테두리를 그려준다
	while (kbhit()) getch();
	//초기 뱀의 위치랑 방향 설정
	dir = LEFT;
	speed = 100;
	length = 5;
	score = 0;
	draw_snake();
	food(); //음식을 출력하게 해주는 함수
	swamp();// 먹으면 죽는 늪을 출력하게 해주는 함수
	check_overlap();

}

//뱀을 그려주는 함수
void draw_snake(void) {
	int i;
	//뱀의 몸통을 출력하게 해주는 반복문
	for (i = 0; i < length; i++) {
		x[i] = MAP_X / 2 + i;
		y[i] = MAP_Y / 2;
		gotoxy(MAP_ADJ_X + x[i], MAP_ADJ_Y + y[i], "○");
	}
	// 뱀의 머리 출력
	gotoxy(MAP_ADJ_X + x[0], MAP_ADJ_Y + y[0], "♠");
}

//Functions to draw map borders
//맵 겉테두리를 그려주는 함수
void draw_map(void) {

	int i, j;

	for (i = 0; i < MAP_X; i++) { // 사각형의 가로 윗변
		gotoxy(MAP_ADJ_X + i, MAP_ADJ_Y, "■");
	}
	for (i = MAP_ADJ_Y + 1; i < MAP_ADJ_Y + MAP_Y - 1; i++) {//사각형의 왼쪽 세로변
		gotoxy(MAP_ADJ_X, i, "■");
		gotoxy(MAP_ADJ_X + MAP_X - 1, i, "■");
	}
	for (i = 0; i < MAP_X; i++) {//사각형의 가로 아랫변
		gotoxy(MAP_ADJ_X + i, MAP_ADJ_Y + MAP_Y - 1, "■");
	}
}



// food와 닿았을때, 자기 몸과 닿았을때, 몸이 움직일때
// 삭제 : 벽에 닿을때 게임종료
// 변경 : 벽에 닿았을때 반대쪽 벽면으로 이동(wallcrash()문)
void move(int dir) {
	int i;
	detect_collision();
	wallcrash();
	gotoxy(MAP_ADJ_X + x[length - 1], MAP_ADJ_Y + y[length - 1], "  ");
	for (i = length - 1; i > 0; i--) {
		x[i] = x[i - 1];
		y[i] = y[i - 1];
	}
	gotoxy(MAP_ADJ_X + x[0], MAP_ADJ_Y + y[0], "○");
	if (dir == LEFT) --x[0];
	if (dir == RIGHT) ++x[0];
	if (dir == UP) --y[0];
	if (dir == DOWN) ++y[0];
	gotoxy(MAP_ADJ_X + x[i], MAP_ADJ_Y + y[i], "♠");
}
void detect_collision(void)
{
	int i;
	//뱀이 음식을 먹었을 때 
	if (x[0] == food_x && y[0] == food_y)
	{
		score += 10;
		food();
		length++;
		x[length - 1] = x[length - 2];
		y[length - 1] = y[length - 2];
	}
	//뱀의 머리가 뱀의 몸통에 닿았는지 확인해주는 반목문 닿았을 경우 게임오버
	for (i = 1; i < length; i++) {
		if (x[0] == x[i] && y[0] == y[i])
		{
			game_over();
		}
	}
	//뱀의 머리가 늪에 닿았는지 확인하는 반복문 닿았을 경우엔 게임오버
	if (swamp_x == x[0] && swamp_y == y[0])
	{
		game_over();
	}
}
//음식과 늪의 좌표가 겹칠경우 디버그 해주는 함수
void check_overlap(void)
{
	assert(food_x != swamp_x && food_y != swamp_y);
}
//벽을 지나칠경우 반대쪽벽에서 나오게 출력해주는 함수
void wallcrash(void)
{
	if (x[0] == 0)
	{

		x[0] = MAP_X - 2;
		gotoxy(MAP_ADJ_X, y[0] + 2, "■");
	}
	else if (x[0] == MAP_X - 1)
	{
		x[0] = 1;
		gotoxy(MAP_ADJ_X + 29, MAP_ADJ_Y + y[0], "■");
	}
	else if (y[0] == 0)
	{

		y[0] = MAP_Y - 2;
		gotoxy(x[0] + MAP_ADJ_X, MAP_ADJ_Y, "■");
	}
	else if (y[0] == MAP_Y - 1)
	{
		y[0] = 1;
		gotoxy(MAP_ADJ_X + x[0], MAP_ADJ_Y + MAP_Y - 1, "■");
	}
}
// Pause the game if you press p
//p를 눌렀을때, 게임을 멈춰주는 함수
void pause(void) {
	while (1) {
		if (key == PAUSE)
		{
			gotoxy(MAP_ADJ_X + (MAP_X / 2) - 9, MAP_ADJ_Y, "< PAUSE : PRESS ANY KEY TO RESUME > ");
			Sleep(400);
			gotoxy(MAP_ADJ_X + (MAP_X / 2) - 9, MAP_ADJ_Y, "                                    ");
			Sleep(400);
		}
		else {
			draw_map();
			return;
		}
		if (kbhit()) {

			do {
				key = getch();
			} while (key == 224);
		}

	}
}
//Function used to print Game_over screen
//게임오버 화면을 출력하는 함수
void game_over(void) {
	gotoxy(MAP_ADJ_X + (MAP_X / 2) - 6, MAP_ADJ_Y + 5, "+----------------------+");
	gotoxy(MAP_ADJ_X + (MAP_X / 2) - 6, MAP_ADJ_Y + 6, "|      GAME OVER..     |");
	gotoxy(MAP_ADJ_X + (MAP_X / 2) - 6, MAP_ADJ_Y + 7, "+----------------------+");
	gotoxy(MAP_ADJ_X + (MAP_X / 2) - 6, MAP_ADJ_Y + 8, " YOUR SCORE : ");
	printf("%d", last_score = score);

	gotoxy(MAP_ADJ_X + (MAP_X / 2) - 7, MAP_ADJ_Y + 12, " Press any keys to restart.. ");
	//베스트 스코어인지 아닌지 판단하고 맞으면 그 값을 저정하고 축하해주는 조건문
	if (score > best_score) {
		best_score = score;
		gotoxy(MAP_ADJ_X + (MAP_X / 2) - 4, MAP_ADJ_Y + 10, "☆ BEST SCORE ☆");
	}
	Sleep(500);
	while (kbhit()) getch();
	key = getch();
	title();
}
//먹이는 뱀의 위치를 제외한 나머지 칸중 랜덤으로 등장
void food(void) {
	int i;

	int food_crush_on = 0;
	int r = 0;
	gotoxy(MAP_ADJ_X, MAP_ADJ_Y + MAP_Y, " YOUR SCORE: ");
	printf("%3d, LAST SCORE: %3d, BEST SCORE: %3d", score, last_score, best_score);

	while (1) {
		food_crush_on = 0;
		srand((unsigned)time(NULL) + r);
		food_x = (rand() % (MAP_X - 2)) + 1;
		food_y = (rand() % (MAP_Y - 2)) + 1;

		for (i = 0; i < length; i++) {
			if (food_x == x[i] && food_y == y[i]) {
				food_crush_on = 1;
				r++;
				break;
			}
		}
		if (food_crush_on == 1) continue;
		gotoxy(MAP_ADJ_X + food_x, MAP_ADJ_Y + food_y, "♪");
		speed -= 3;
		break;

	}
}

//늪을 만들어 뱀의 위치와 food의 위치를 제외한 나머지칸에 생성
void swamp(void) {
	int swamp_crush_on = 0;
	int r = 1;
	srand((unsigned)time(NULL) + r);
	swamp_x = (rand() % (MAP_X - 2)) + 1;
	swamp_y = (rand() % (MAP_Y - 2)) + 1;
	gotoxy(MAP_ADJ_X + swamp_x, MAP_ADJ_Y + swamp_y, "★");
}

//Functions that allow you to see various states
void status(void) {
	gotoxy(MAP_ADJ_X + MAP_X + 1, MAP_ADJ_Y, "head= ");
	printf("%2d,%2d", x[0], y[0]);
	gotoxy(MAP_ADJ_X + MAP_X + 1, MAP_ADJ_Y + 1, "food= ");
	printf("%2d,%2d", food_x, food_y);
	gotoxy(MAP_ADJ_X + MAP_X + 1, MAP_ADJ_Y + 2, "leng= ");
	printf("%2d", length);
	gotoxy(MAP_ADJ_X + MAP_X + 1, MAP_ADJ_Y + 3, "key= ");
	printf("%3d", key);
	gotoxy(MAP_ADJ_X + MAP_X + 1, MAP_ADJ_Y + 4, "spd= ");
	printf("%3d", speed);
	gotoxy(MAP_ADJ_X + MAP_X + 1, MAP_ADJ_Y + 6, "score= ");
	printf("%3d", score);
}
//겜에 대한 설명을 출력해주는 함수
void print_manual(void)
{
	system("cls");
	draw_map();
	gotoxy(MAP_ADJ_X + 12, MAP_ADJ_Y + 1, "■규칙 설명■");
	gotoxy(MAP_ADJ_X + 1, MAP_ADJ_Y + 3, "뱀은 상하좌우로 움직이며 ♪(먹이)를 먹는다.");
	gotoxy(MAP_ADJ_X + 1, MAP_ADJ_Y + 5, "♪를 먹을 시 꼬리가 한칸 씩 늘어나며 점수가 10점 오르며");
	gotoxy(MAP_ADJ_X + 1, MAP_ADJ_Y + 6, "속도가 빨라진다.");
	gotoxy(MAP_ADJ_X + 1, MAP_ADJ_Y + 8, "움직이며 자신의 몸에 닿거나 ★(늪)에 닿을 시");
	gotoxy(MAP_ADJ_X + 1, MAP_ADJ_Y + 9, "즉시 종료되며 점수가 저장된다.");
	gotoxy(MAP_ADJ_X + 1, MAP_ADJ_Y + 11, "게임 실해중에 가장 높은 점수는 best score에 저장되어");
	gotoxy(MAP_ADJ_X + 1, MAP_ADJ_Y + 12, "게임종료 시 화면상 표시된다.");
	gotoxy(MAP_ADJ_X + 1, MAP_ADJ_Y + 14, "벽면에 닿을 시 닿은 벽면에 대칭되는 곳으로 이동되며 ");
	gotoxy(MAP_ADJ_X + 1, MAP_ADJ_Y + 15, "게임은 계속 진행된다.");
	while (1) {
		if (kbhit()) {
			key = getch();
			if (key == ESC) exit(0);
			else break;
		}
		gotoxy(MAP_ADJ_X + (MAP_X / 2) - 7, MAP_ADJ_Y + 18, " < PRESS ANY KEY TO START > ");
		Sleep(400);
		gotoxy(MAP_ADJ_X + (MAP_X / 2) - 7, MAP_ADJ_Y + 18, "                            ");
		Sleep(400);

	}
	while (_kbhit()) getch();
	start_game();
}