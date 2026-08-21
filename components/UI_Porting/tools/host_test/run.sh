#!/bin/sh
# 编译并运行 u8g2 porting 主机端测试（无需 MCU 工具链）
set -e
cd "$(dirname "$0")"

gcc -Wall -Wextra -I ../.. \
    -DU8G2_PORTING_SCREEN_W=128 -DU8G2_PORTING_SCREEN_H=64 \
    test_u8g2.c ../../u8g2.c ../../u8g2_fonts.c -o test_u8g2

./test_u8g2
