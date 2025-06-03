#ifndef SNAKE_H
#define SNAKE_H
#include <unistd.h>

#include <ncurses.h>

#define MAX_SNAKE_LENGTH 100

typedef struct {
    int x, y;
} Point;

typedef enum {
    UP, DOWN, LEFT, RIGHT
} Direction;

typedef struct {
    Point body[MAX_SNAKE_LENGTH];
    int length;
    Direction dir;
} Snake;

// 全局变量声明（定义放在snake.c）
extern Snake snake;
extern Point food;
extern int highscore;

// 按键宏定义（方便替换）
// 替换原先的宏定义（改为变量）
extern int key_up;
extern int key_down;
extern int key_left;
extern int key_right;
extern int key_pause;
extern int key_boost;


// 函数声明
void init_game();
void draw_game();
void update_snake();
void place_food();
int check_collision();
void load_highscore();
void save_highscore();

#endif

