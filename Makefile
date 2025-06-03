# Makefile for Snake Game
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LIBS = -lncurses

# 源文件
SINGLE_SRC = snake.c
DOUBLE_SRC = snake_for2.c
HEADER = snake.h

# 目标文件
SINGLE_TARGET = snake
DOUBLE_TARGET = snake_for2

# 默认目标：编译所有版本
all: $(SINGLE_TARGET) $(DOUBLE_TARGET)

# 编译单人版本
$(SINGLE_TARGET): $(SINGLE_SRC) $(HEADER)
	$(CC) $(CFLAGS) -o $(SINGLE_TARGET) $(SINGLE_SRC) $(LIBS)

# 编译双人版本
$(DOUBLE_TARGET): $(DOUBLE_SRC) $(HEADER)
	$(CC) $(CFLAGS) -o $(DOUBLE_TARGET) $(DOUBLE_SRC) $(LIBS)

# 只编译单人版本
single: $(SINGLE_TARGET)

# 只编译双人版本
double: $(DOUBLE_TARGET)

# 清理编译文件
clean:
	rm -f $(SINGLE_TARGET) $(DOUBLE_TARGET)

# 安装（可选）
install: all
	cp $(SINGLE_TARGET) /usr/local/bin/
	cp $(DOUBLE_TARGET) /usr/local/bin/
	chmod +x launcher.sh
	cp launcher.sh /usr/local/bin/snake-launcher

# 卸载（可选）
uninstall:
	rm -f /usr/local/bin/$(SINGLE_TARGET)
	rm -f /usr/local/bin/$(DOUBLE_TARGET)
	rm -f /usr/local/bin/snake-launcher

# 运行单人游戏
run-single: $(SINGLE_TARGET)
	./$(SINGLE_TARGET)

# 运行双人游戏
run-double: $(DOUBLE_TARGET)
	./$(DOUBLE_TARGET)

# 启动游戏菜单
run-launcher: all
	./launcher.sh

# 显示帮助
help:
	@echo "Snake Game Makefile"
	@echo "==================="
	@echo "Available targets:"
	@echo "  all          - 编译所有版本（默认）"
	@echo "  single       - 只编译单人版本"
	@echo "  double       - 只编译双人版本"
	@echo "  clean        - 清理编译文件"
	@echo "  install      - 安装到系统"
	@echo "  uninstall    - 从系统卸载"
	@echo "  run-single   - 编译并运行单人游戏"
	@echo "  run-double   - 编译并运行双人游戏"
	@echo "  run-launcher - 编译并启动游戏菜单"
	@echo "  help         - 显示此帮助信息"

.PHONY: all single double clean install uninstall run-single run-double run-launcher help
