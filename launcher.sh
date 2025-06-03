#!/bin/bash

CONFIG_DIR="config"
CONFIG_FILE="$CONFIG_DIR/settings.conf"
HIGHSCORE_FILE="$CONFIG_DIR/highscore.txt"

mkdir -p "$CONFIG_DIR"
touch "$CONFIG_FILE"
touch "$HIGHSCORE_FILE"

# 默认值
speed=3
pause_key="p"
boost_key="j"
key_up="w"
key_down="s"
key_left="a"
key_right="d"

function read_config() {
    if [[ -f "$CONFIG_FILE" ]]; then
        speed=$(grep "^speed=" "$CONFIG_FILE" | cut -d '=' -f2)
        pause_key=$(grep "^pause=" "$CONFIG_FILE" | cut -d '=' -f2)
        boost_key=$(grep "^boost=" "$CONFIG_FILE" | cut -d '=' -f2)
        key_up=$(grep "^key_up=" "$CONFIG_FILE" | cut -d '=' -f2)
        key_down=$(grep "^key_down=" "$CONFIG_FILE" | cut -d '=' -f2)
        key_left=$(grep "^key_left=" "$CONFIG_FILE" | cut -d '=' -f2)
        key_right=$(grep "^key_right=" "$CONFIG_FILE" | cut -d '=' -f2)

        [[ -z "$speed" ]] && speed=3
        [[ -z "$pause_key" ]] && pause_key="p"
        [[ -z "$boost_key" ]] && boost_key="j"
        [[ -z "$key_up" ]] && key_up="w"
        [[ -z "$key_down" ]] && key_down="s"
        [[ -z "$key_left" ]] && key_left="a"
        [[ -z "$key_right" ]] && key_right="d"
    fi
}

function write_config() {
    echo "speed=$speed" > "$CONFIG_FILE"
    echo "pause=$pause_key" >> "$CONFIG_FILE"
    echo "boost=$boost_key" >> "$CONFIG_FILE"
    echo "key_up=$key_up" >> "$CONFIG_FILE"
    echo "key_down=$key_down" >> "$CONFIG_FILE"
    echo "key_left=$key_left" >> "$CONFIG_FILE"
    echo "key_right=$key_right" >> "$CONFIG_FILE"
}

function set_speed() {
    SPEED_CHOICE=$(dialog --clear --backtitle "设置游戏速度" \
        --title "设置速度" \
        --menu "请选择游戏速度，数值越大越快" 15 50 5 \
        1 "速度 1（最慢）" \
        2 "速度 2" \
        3 "速度 3（默认）" \
        4 "速度 4" \
        5 "速度 5（最快）" \
        3>&1 1>&2 2>&3)

    if [[ "$SPEED_CHOICE" =~ ^[1-5]$ ]]; then
        speed=$SPEED_CHOICE
        write_config
        dialog --msgbox "速度设置成功！当前速度：$speed" 6 40
    else
        dialog --msgbox "无效选择，保持原速度：$speed" 6 40
    fi
}

function validate_key() {
    local key="$1"
    if [[ ${#key} -eq 1 ]]; then
        return 0
    else
        return 1
    fi
}

function set_keys() {
    local temp_pause temp_boost temp_up temp_down temp_left temp_right
    
    temp_pause=$(dialog --inputbox "请输入暂停键（单个字符，如 p）\n注意：暂停键仅在单人模式有效" 10 50 "$pause_key" 3>&1 1>&2 2>&3)
    if [[ $? -ne 0 ]]; then return; fi
    
    temp_boost=$(dialog --inputbox "请输入加速键（单个字符，如 j）\n注意：加速键仅在单人模式有效" 10 50 "$boost_key" 3>&1 1>&2 2>&3)
    if [[ $? -ne 0 ]]; then return; fi

    temp_up=$(dialog --inputbox "请输入 ↑ 上键（单个字符，如 w）\n注意：仅影响单人模式，双人模式固定WASD和方向键" 12 60 "$key_up" 3>&1 1>&2 2>&3)
    if [[ $? -ne 0 ]]; then return; fi
    
    temp_down=$(dialog --inputbox "请输入 ↓ 下键（单个字符，如 s）" 8 50 "$key_down" 3>&1 1>&2 2>&3)
    if [[ $? -ne 0 ]]; then return; fi
    
    temp_left=$(dialog --inputbox "请输入 ← 左键（单个字符，如 a）" 8 50 "$key_left" 3>&1 1>&2 2>&3)
    if [[ $? -ne 0 ]]; then return; fi
    
    temp_right=$(dialog --inputbox "请输入 → 右键（单个字符，如 d）" 8 50 "$key_right" 3>&1 1>&2 2>&3)
    if [[ $? -ne 0 ]]; then return; fi

    # 验证所有按键
    local error_msg=""
    if ! validate_key "$temp_pause"; then error_msg+="暂停键无效\n"; fi
    if ! validate_key "$temp_boost"; then error_msg+="加速键无效\n"; fi
    if ! validate_key "$temp_up"; then error_msg+="上键无效\n"; fi
    if ! validate_key "$temp_down"; then error_msg+="下键无效\n"; fi
    if ! validate_key "$temp_left"; then error_msg+="左键无效\n"; fi
    if ! validate_key "$temp_right"; then error_msg+="右键无效\n"; fi

    # 检查按键冲突
    local keys=("$temp_pause" "$temp_boost" "$temp_up" "$temp_down" "$temp_left" "$temp_right")
    local unique_keys=($(printf "%s\n" "${keys[@]}" | sort -u))
    if [[ ${#keys[@]} -ne ${#unique_keys[@]} ]]; then
        error_msg+="存在重复按键\n"
    fi

    if [[ -n "$error_msg" ]]; then
        dialog --msgbox "设置失败：\n$error_msg请重新设置。" 10 40
        return
    fi

    # 所有验证通过，保存设置
    pause_key="$temp_pause"
    boost_key="$temp_boost"
    key_up="$temp_up"
    key_down="$temp_down"
    key_left="$temp_left"
    key_right="$temp_right"
    
    write_config
    dialog --msgbox "按键设置成功！\n单人模式方向键：$key_up(上)/$key_down(下)/$key_left(左)/$key_right(右)\n暂停：$pause_key，加速：$boost_key\n\n双人模式固定按键：\n玩家1: WASD\n玩家2: 方向键" 14 60
}

function show_highscore() {
    if [[ -f "$HIGHSCORE_FILE" ]]; then
        highscore=$(head -n 1 "$HIGHSCORE_FILE")
    else
        highscore=0
    fi
    dialog --msgbox "当前最高分：$highscore\n（单人和双人模式共享最高分记录）" 8 50
}

function show_current_settings() {
    dialog --msgbox "当前设置：\n\n游戏速度：$speed\n\n单人模式按键：\n  方向键：$key_up(上)/$key_down(下)/$key_left(左)/$key_right(右)\n  暂停键：$pause_key\n  加速键：$boost_key\n\n双人模式按键：\n  玩家1：WASD\n  玩家2：方向键\n\n注意：单人模式支持方向键和自定义键" 18 60
}

function show_2player_help() {
    dialog --msgbox "双人模式说明：\n\n🎮 控制方式：\n  玩家1：WASD键控制（绿色蛇）\n  玩家2：方向键控制（蓝色蛇）\n\n🎯 游戏规则：\n  • 场上会随机出现1-2个食物\n  • 吃完所有食物后会重新生成\n  • 撞墙、撞自己或撞对方即死亡\n  • 任一玩家死亡游戏结束\n  • 存活玩家获胜\n\n⌨️  游戏中按 'q' 键退出" 16 60
}

function show_menu() {
    CHOICE=$(dialog --clear --backtitle "贪吃蛇游戏菜单" \
        --title "主菜单" \
        --menu "请选择一个选项" 22 70 10 \
        1 "单人游戏" \
        2 "双人游戏" \
        3 "双人游戏说明" \
        4 "设置速度 (当前: $speed)" \
        5 "设置按键" \
        6 "查看当前设置" \
        7 "查看最高分" \
        8 "退出" \
        3>&1 1>&2 2>&3)

    case $CHOICE in
        1)
            clear
            echo "启动单人游戏，当前速度：$speed"
            echo "按键设置 - 上:$key_up 下:$key_down 左:$key_left 右:$key_right 暂停:$pause_key 加速:$boost_key"
            echo "注意：游戏中方向键和自定义键都可以使用"
            echo "按任意键开始游戏..."
            read -n 1
            write_config
            if [[ -f "./snake" ]]; then
                ./snake
            else
                echo "错误：找不到单人游戏程序 'snake'"
                echo "请确保已编译 snake.c"
                read -n 1
            fi
            ;;
        2)
            clear
            echo "启动双人游戏，当前速度：$speed"
            echo "玩家1: WASD键控制（绿色蛇）"
            echo "玩家2: 方向键控制（蓝色蛇）"
            echo "按任意键开始游戏..."
            read -n 1
            write_config
            if [[ -f "./snake_for2" ]]; then
                ./snake_for2
            else
                echo "错误：找不到双人游戏程序 'snake_for2'"
                echo "请确保已编译 snake_for2.c"
                read -n 1
            fi
            ;;
        3)	show_2player_help   ;;
	4)
            set_speed
            ;;
        5)
            set_keys
            ;;
        6)
            show_current_settings
            ;;
        7)
            show_highscore
            ;;
        8)
            clear
            echo "退出游戏菜单"
            exit 0
            ;;
        *)
            clear
            echo "无效选项，退出。"
            exit 1
            ;;
    esac
}

read_config
while true; do
    show_menu
done
