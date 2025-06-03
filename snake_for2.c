#include "snake.h" 
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#define GAME_WIDTH  40
#define GAME_HEIGHT 20
#define MAX_FOOD 2

// 双人模式数据结构
typedef struct {
    Snake snake;
    int alive;
    char name[20];
} Player;

// 全局变量定义
Player player1, player2;
Point foods[MAX_FOOD];
int food_count = 0;
int highscore = 0;
int speed_delay = 150000;

// 玩家1使用WASD，玩家2使用方向键
#define P1_UP    'w'
#define P1_DOWN  's'
#define P1_LEFT  'a'
#define P1_RIGHT 'd'

#define draw_block(y, x, pair) do { \
    wattron(gamewin, COLOR_PAIR(pair)); \
    mvwaddch(gamewin, y, (x) * 2, ' '); \
    mvwaddch(gamewin, y, (x) * 2 + 1, ' '); \
    wattroff(gamewin, COLOR_PAIR(pair)); \
} while(0)

void load_config() {
    FILE *fp = fopen("config/settings.conf", "r");
    if (fp) {
        char line[100];
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "speed=", 6) == 0) {
                int s = atoi(line + 6);
                switch (s) {
                    case 1: speed_delay = 300000; break;
                    case 2: speed_delay = 250000; break;
                    case 3: speed_delay = 200000; break;
                    case 4: speed_delay = 150000; break;
                    case 5: speed_delay = 100000; break;
                    default: speed_delay = 200000;
                }
            }
        }
        fclose(fp);
    }
}

void place_foods() {
    food_count = 0;
    
    // 生成1-2个食物
    int num_foods = (rand() % 2) + 1;
    
    for (int f = 0; f < num_foods; f++) {
        int attempts = 0;
        while (attempts < 100) {  // 防止无限循环
            int x = rand() % (GAME_WIDTH - 2) + 1;
            int y = rand() % (GAME_HEIGHT - 2) + 1;

            int collision = 0;
            
            // 检查是否与玩家1的蛇身重叠
            if (player1.alive) {
                for (int i = 0; i < player1.snake.length; i++) {
                    if (player1.snake.body[i].x == x && player1.snake.body[i].y == y) {
                        collision = 1;
                        break;
                    }
                }
            }
            
            // 检查是否与玩家2的蛇身重叠
            if (!collision && player2.alive) {
                for (int i = 0; i < player2.snake.length; i++) {
                    if (player2.snake.body[i].x == x && player2.snake.body[i].y == y) {
                        collision = 1;
                        break;
                    }
                }
            }
            
            // 检查是否与已存在的食物重叠
            if (!collision) {
                for (int i = 0; i < food_count; i++) {
                    if (foods[i].x == x && foods[i].y == y) {
                        collision = 1;
                        break;
                    }
                }
            }
            
            if (!collision) {
                foods[food_count].x = x;
                foods[food_count].y = y;
                food_count++;
                break;
            }
            attempts++;
        }
    }
}

// 检查碰撞 - 返回值: 0=无碰撞, 1=撞墙, 2=撞自己, 3=撞对方
int check_collision(Player *p, Player *other) {
    Point head = p->snake.body[0];
    
    // 检查撞墙
    if (head.x <= 0 || head.x >= GAME_WIDTH - 1 || head.y <= 0 || head.y >= GAME_HEIGHT - 1) {
        return 1;
    }
    
    // 检查撞自己
    for (int i = 1; i < p->snake.length; i++) {
        if (p->snake.body[i].x == head.x && p->snake.body[i].y == head.y) {
            return 2;
        }
    }
    
    // 检查撞对方
    if (other->alive) {
        for (int i = 0; i < other->snake.length; i++) {
            if (other->snake.body[i].x == head.x && other->snake.body[i].y == head.y) {
                return 3;
            }
        }
    }
    
    return 0;
}

void load_highscore() {
    FILE *fp = fopen("config/highscore.txt", "r");
    if (fp) {
        if (fscanf(fp, "%d", &highscore) != 1) {
            highscore = 0;
        }
        fclose(fp);
    } else {
        highscore = 0;
    }
}

void save_highscore() {
    int max_score = 0;
    if (player1.snake.length - 3 > max_score) max_score = player1.snake.length - 3;
    if (player2.snake.length - 3 > max_score) max_score = player2.snake.length - 3;
    
    if (max_score > highscore) {
        highscore = max_score;
        FILE *fp = fopen("config/highscore.txt", "w");
        if (fp) {
            fprintf(fp, "%d\n", highscore);
            fclose(fp);
        }
    }
}

WINDOW* gamewin;

void init_game() {
    initscr();
    cbreak();
    noecho();
    curs_set(FALSE);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        init_pair(2, COLOR_RED, COLOR_RED);     // 食物
        init_pair(3, COLOR_GREEN, COLOR_GREEN); // 玩家1蛇身
        init_pair(4, COLOR_WHITE, COLOR_WHITE); // 玩家1蛇头
        init_pair(5, COLOR_YELLOW, COLOR_YELLOW); // 玩家2蛇身（黄色）
	init_pair(6, COLOR_BLUE, COLOR_BLUE);     // 玩家2蛇头（蓝色）

    }

    int startx = (COLS - GAME_WIDTH * 2) / 2;
    int starty = (LINES - GAME_HEIGHT) / 2;

    gamewin = newwin(GAME_HEIGHT, GAME_WIDTH * 2, starty, startx);
    box(gamewin, 0, 0);

    // 初始化玩家1 (left side)
    strcpy(player1.name, "Player1");
    player1.alive = 1;
    player1.snake.length = 3;
    player1.snake.dir = RIGHT;
    for (int i = 0; i < player1.snake.length; i++) {
        player1.snake.body[i].x = 5 - i;
        player1.snake.body[i].y = 10;
    }

    // 初始化玩家2 (right side)
    strcpy(player2.name, "Player2");
    player2.alive = 1;
    player2.snake.length = 3;
    player2.snake.dir = LEFT;
    for (int i = 0; i < player2.snake.length; i++) {
        player2.snake.body[i].x = 35 + i;
        player2.snake.body[i].y = 10;
    }

    srand(time(NULL));
    place_foods();
    load_highscore();
    load_config();
}

void draw_game() {


    werase(gamewin);
    box(gamewin, 0, 0);

    // 绘制食物
    for (int i = 0; i < food_count; i++) {
        draw_block(foods[i].y, foods[i].x, 2);
    }

    // 绘制玩家1 (绿色)
    if (player1.alive) {
        for (int i = 0; i < player1.snake.length; i++) {
            int pair = (i == 0) ? 4 : 3;  // 头部白色，身体绿色
            draw_block(player1.snake.body[i].y, player1.snake.body[i].x, pair);
        }
    }

    // 绘制玩家2 (蓝色)
    if (player2.alive) {
        for (int i = 0; i < player2.snake.length; i++) {
            int pair = (i == 0) ? 6 : 5;  // 头部黄色，身体蓝色
            draw_block(player2.snake.body[i].y, player2.snake.body[i].x, pair);
        }
    }

    // 显示分数和状态
    mvprintw(GAME_HEIGHT + 2, 2, " P1(WASD): %d %s | P2(Arrow): %d %s | Foods: %d ",
             player1.snake.length - 3, player1.alive ? "ALIVE" : "DEAD",
             player2.snake.length - 3, player2.alive ? "ALIVE" : "DEAD",
             food_count);

    wrefresh(gamewin);
    refresh();
}

void update_snake(Player *p) {
    if (!p->alive) return;
    
    Point next = p->snake.body[0];
    switch (p->snake.dir) {
        case UP:    next.y--; break;
        case DOWN:  next.y++; break;
        case LEFT:  next.x--; break;
        case RIGHT: next.x++; break;
    }

    // 检查是否吃到食物
    int ate_food = 0;
    for (int i = 0; i < food_count; i++) {
        if (next.x == foods[i].x && next.y == foods[i].y) {
            ate_food = 1;
            // 移除被吃的食物
            for (int j = i; j < food_count - 1; j++) {
                foods[j] = foods[j + 1];
            }
            food_count--;
            break;
        }
    }

    if (ate_food) {
        if (p->snake.length < MAX_SNAKE_LENGTH) {
            p->snake.length++;
        }
        // 如果所有食物都被吃完了，重新生成
        if (food_count == 0) {
            place_foods();
        }
    }

    // 移动蛇身
    for (int i = p->snake.length - 1; i > 0; i--) {
        p->snake.body[i] = p->snake.body[i - 1];
    }
    p->snake.body[0] = next;
}
int key_buffer[512] = {0};  // 简单按键状态缓存（可以用 bool 或位图优化）
void read_all_keys() {
    int ch;
    while ((ch = getch()) != ERR) {
        key_buffer[ch] = 1;  // 标记按下
    }
}

void clear_key_buffer() {
    memset(key_buffer, 0, sizeof(key_buffer));
}

int main() {
    init_game();

    // 显示游戏开始信息
    nodelay(stdscr, FALSE);
    mvprintw(LINES / 2, (COLS - 40) / 2, "2-Player Snake Game");
mvprintw(LINES / 2 + 1, (COLS - 40) / 2, "Player1: WASD (Green)");
mvprintw(LINES / 2 + 2, (COLS - 40) / 2, "Player2: Arrow keys (Blue)");
mvprintw(LINES / 2 + 3, (COLS - 40) / 2, "Press any key to start...");

    refresh();
    getch();
    nodelay(stdscr, TRUE);

    while (player1.alive && player2.alive) {
        read_all_keys();

    // 玩家1（WASD）
    if (player1.alive) {
        if (key_buffer['w'] && player1.snake.dir != DOWN) player1.snake.dir = UP;
        else if (key_buffer['s'] && player1.snake.dir != UP) player1.snake.dir = DOWN;
        else if (key_buffer['a'] && player1.snake.dir != RIGHT) player1.snake.dir = LEFT;
        else if (key_buffer['d'] && player1.snake.dir != LEFT) player1.snake.dir = RIGHT;
    }

    // 玩家2（方向键）
    if (player2.alive) {
        if (key_buffer[KEY_UP] && player2.snake.dir != DOWN) player2.snake.dir = UP;
        else if (key_buffer[KEY_DOWN] && player2.snake.dir != UP) player2.snake.dir = DOWN;
        else if (key_buffer[KEY_LEFT] && player2.snake.dir != RIGHT) player2.snake.dir = LEFT;
        else if (key_buffer[KEY_RIGHT] && player2.snake.dir != LEFT) player2.snake.dir = RIGHT;
    }

    if (key_buffer['q']) break;

    clear_key_buffer();


        // 更新蛇的位置
        update_snake(&player1);
        update_snake(&player2);
	
	// 两蛇头坐标
	Point head1 = player1.snake.body[0];
	Point head2 = player2.snake.body[0];

// 判断蛇头是否重合
	if (head1.x == head2.x && head1.y == head2.y) {
    // 蛇头碰撞，平局
    	player1.alive = 0;
    	player2.alive = 0;

    // 跳出循环，游戏结束
    	break;
	}

        // 检查碰撞
        if (player1.alive) {
            int collision = check_collision(&player1, &player2);
            if (collision != 0) {
                player1.alive = 0;
                if (collision == 2) {
                    // 撞自己显示Python禁止
                    nodelay(stdscr, FALSE);
                    mvprintw(LINES / 2, (COLS - 25) / 2, "Player1: Anaconda is prohibited!");
                    mvprintw(LINES / 2 + 1, (COLS - 20) / 2, "Press any key...");
                    refresh();
                    getch();
                    nodelay(stdscr, TRUE);
                }
            }
        }
        
        if (player2.alive) {
            int collision = check_collision(&player2, &player1);
            if (collision != 0) {
                player2.alive = 0;
                if (collision == 2) {
                    // 撞自己显示Python禁止
                    nodelay(stdscr, FALSE);
                    mvprintw(LINES / 2, (COLS - 25) / 2, "Player2: Python is prohibited!");
                    mvprintw(LINES / 2 + 1, (COLS - 20) / 2, "Press any key...");
                    refresh();
                    getch();
                    nodelay(stdscr, TRUE);
                }
            }
        }

        draw_game();
        usleep(speed_delay);
    }

    save_highscore();
    
    // 游戏结束显示
    nodelay(stdscr, FALSE);
    mvprintw(LINES / 2, (COLS - 20) / 2, "	Game over!");
    
    if (player1.alive&& !player2.alive) {
        mvprintw(LINES / 2 + 1, (COLS - 20) / 2, "	Player 1 Wins!");
    } else if (player2.alive&& !player1.alive) {
        mvprintw(LINES / 2 + 1, (COLS - 20) / 2, "	Player 2 Wins!");
    } else {
        mvprintw(LINES / 2 + 1, (COLS - 20) / 2, "Draw!");
    }
    
    mvprintw(LINES / 2 + 2, (COLS - 30) / 2, "Player 1 Score: %d", player1.snake.length - 3);
    mvprintw(LINES / 2 + 3, (COLS - 30) / 2, "Player 2 Score: %d", player2.snake.length - 3);
    mvprintw(LINES / 2 + 4, (COLS - 20) / 2, "Press 'o' or 'O' to exit...");
    refresh();
char ch;
    while ((ch = getch()) != 'o' && ch != 'O') {
    // 等待按下 o 或 O
}

    endwin();
    printf("\n双人游戏结束！\n");
    printf("玩家1最终得分：%d\n", player1.snake.length - 3);
    printf("玩家2最终得分：%d\n", player2.snake.length - 3);
    return 0;
}
