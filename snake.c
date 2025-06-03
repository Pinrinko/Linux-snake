#include "snake.h" 
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#define GAME_WIDTH  40
#define GAME_HEIGHT 20

// 全局变量定义
Snake snake;
Point food;
int highscore = 0;
int speed_delay = 150000;  // 默认值
int key_up    = 'w';       // 改为字符类型，默认wasd
int key_down  = 's';
int key_left  = 'a';
int key_right = 'd';
int key_pause = 'p';       // 默认 p 键暂停
int key_boost = 'j';       // 默认 j 键加速

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
            } else if (strncmp(line, "pause=", 6) == 0) {
                key_pause = line[6];
            } else if (strncmp(line, "boost=", 6) == 0) {
                key_boost = line[6];
            } else if (strncmp(line, "key_up=", 7) == 0) {
                key_up = line[7];
            } else if (strncmp(line, "key_down=", 9) == 0) {
                key_down = line[9];
            } else if (strncmp(line, "key_left=", 9) == 0) {
                key_left = line[9];
            } else if (strncmp(line, "key_right=", 10) == 0) {
                key_right = line[10];
            }
        }
        fclose(fp);
    }
}

void place_food() {
    while (1) {
        int x = rand() % (GAME_WIDTH - 2) + 1;
        int y = rand() % (GAME_HEIGHT - 2) + 1;

        int collision = 0;
        for (int i = 0; i < snake.length; i++) {
            if (snake.body[i].x == x && snake.body[i].y == y) {
                collision = 1;
                break;
            }
        }
        if (!collision) {
            food.x = x;
            food.y = y;
            break;
        }
    }
}

// 修改碰撞检测函数，返回不同的值表示不同的碰撞类型
// 返回值: 0=无碰撞, 1=撞墙, 2=撞自己
int check_collision() {
    Point head = snake.body[0];
    
    // 检查撞墙
    if (head.x <= 0 || head.x >= GAME_WIDTH - 1 || head.y <= 0 || head.y >= GAME_HEIGHT - 1) {
        return 1;  // 撞墙
    }
    
    // 检查撞自己
    for (int i = 1; i < snake.length; i++) {
        if (snake.body[i].x == head.x && snake.body[i].y == head.y) {
            return 2;  // 撞自己
        }
    }
    
    return 0;  // 无碰撞
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
    if (snake.length-3> highscore) {
        highscore = snake.length-3;
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
        init_pair(2, COLOR_RED, COLOR_RED);
        init_pair(3, COLOR_GREEN, COLOR_GREEN);
        init_pair(4, COLOR_WHITE, COLOR_WHITE);
    }

    int startx = (COLS - GAME_WIDTH * 2) / 2;
    int starty = (LINES - GAME_HEIGHT) / 2;

    gamewin = newwin(GAME_HEIGHT, GAME_WIDTH * 2, starty, startx);
    box(gamewin, 0, 0);

    snake.length = 3;
    snake.dir = RIGHT;
    for (int i = 0; i < snake.length; i++) {
        snake.body[i].x = 10 - i;
        snake.body[i].y = 10;
    }

    srand(time(NULL));
    place_food();
    load_highscore();
    load_config();
}

void draw_game() {
    werase(gamewin);
    box(gamewin, 0, 0);

    draw_block(food.y, food.x, 2);

    for (int i = 0; i < snake.length; i++) {
        int pair = (i == 0) ? 4 : 3;
        draw_block(snake.body[i].y, snake.body[i].x, pair);
    }

    mvprintw(GAME_HEIGHT + 2, 2, " Score: %d  Highscore: %d ", snake.length - 3, highscore);

    wrefresh(gamewin);
    refresh();
}

void update_snake() {
    Point next = snake.body[0];
    switch (snake.dir) {
        case UP:    next.y--; break;
        case DOWN:  next.y++; break;
        case LEFT:  next.x--; break;
        case RIGHT: next.x++; break;
    }

    int grow = (next.x == food.x && next.y == food.y);
    if (grow) {
        if (snake.length < MAX_SNAKE_LENGTH) {
            snake.length++;
        }
        place_food();
    }

    for (int i = snake.length - 1; i > 0; i--) {
        snake.body[i] = snake.body[i - 1];
    }
    snake.body[0] = next;
}

// 清空输入缓冲区，只保留最新的按键
int get_latest_key() {
    int ch = ERR;
    int latest_ch = ERR;
    
    // 读取所有缓存的按键，只保留最后一个
    while ((ch = getch()) != ERR) {
        latest_ch = ch;
    }
    
    return latest_ch;
}

int main() {
    init_game();
    int boost_active = 0;

    while (1) {
        int ch = get_latest_key();  // 使用新的函数获取最新按键
        
        // 如果没有按键输入，继续游戏循环
        if (ch == ERR) {
            boost_active = 0;  // 没有按键时取消加速
        } else {
            // boost 单独判定
            boost_active = (ch == key_boost);

            // 支持原生方向键和自定义字符键
            if ((ch == key_up || ch == KEY_UP) && snake.dir != DOWN && snake.dir != UP) {
                snake.dir = UP;
            } else if ((ch == key_down || ch == KEY_DOWN) && snake.dir != UP && snake.dir != DOWN) {
                snake.dir = DOWN;
            } else if ((ch == key_left || ch == KEY_LEFT) && snake.dir != RIGHT && snake.dir != LEFT) {
                snake.dir = LEFT;
            } else if ((ch == key_right || ch == KEY_RIGHT) && snake.dir != LEFT && snake.dir != RIGHT) {
                snake.dir = RIGHT;
            } else if (ch == key_pause) {
                nodelay(stdscr, FALSE);
                mvprintw(LINES / 2, (COLS - 20) / 2, "Paused. Press any key");
                getch();
                mvprintw(LINES / 2, (COLS - 20) / 2, "                    ");
                nodelay(stdscr, TRUE);
            } else if (ch == 'q') {
                save_highscore();
                endwin();
                printf("\n游戏结束！最终得分：%d\n", snake.length - 3);
                return 0;
            }
        }

        update_snake();

        int collision_type = check_collision();
        if (collision_type != 0) {
            save_highscore();
            
            // 根据碰撞类型显示不同的提示信息
            if (collision_type == 2) {
                // 撞自己时显示Python禁止提示
                nodelay(stdscr, FALSE);  // 暂停输入处理，等待用户按键
                mvprintw(LINES / 2, (COLS - 20) / 2, "Anaconda is prohibited!");
                mvprintw(LINES / 2 + 1, (COLS - 20) / 2, "Press any key...");
                refresh();
                getch();  // 等待用户按任意键
            }
            
            break;
        }

        draw_game();
        int delay = boost_active ? speed_delay / 2 : speed_delay;
        usleep(delay);
    }

    endwin();
    printf("\n游戏结束！最终得分：%d\n", snake.length - 3);
    return 0;
}
