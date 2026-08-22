#!/usr/bin/env python3
"""
extract_u8g2_fonts.py — 从 u8g2 字体源码中提取指定字体表，生成本框架的字体文件。

u8g2 的字体表是 "const uint8_t NAME[size] U8G2_FONT_SECTION("...") = "\ooo\ooo...""
形式的 C 字符串字面量（八进制转义，可跨行拼接）。本脚本原样提取字节数组，
去掉 U8G2_FONT_SECTION 属性（本框架无需 Flash 段属性），重新排版输出，
保证与 u8g2 原始数据完全一致（字形 RLE 解码器在 u8g2.c 中实现，数据格式不变）。

用法:
    python3 extract_u8g2_fonts.py <备份的u8g2_fonts.c> [字体名...]

默认字体名与输出文件:
    u8g2_fonts.c / u8g2_fonts.h 生成在脚本同级的上级目录（components/UI_Porting）
"""

import re
import sys
import os

OUT_DIR = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

DEFAULT_FONTS = [
    "u8g2_font_5x7_tf",
    "u8g2_font_6x10_tf",
    "u8g2_font_8x13_tr",
    "u8g2_font_logisoso20_tn",
    "u8g2_font_open_iconic_all_4x_t",
]

# 简单转义映射
SIMPLE_ESCAPES = {
    "n": 10, "t": 9, "r": 13, "\\": 92, '"': 34, "'": 39, "0": 0, "a": 7,
    "b": 8, "f": 12, "v": 11,
}


def parse_c_string(s: str) -> bytes:
    """解析一个 C 字符串字面量（仅处理八进制/十六进制/简单转义），返回字节。"""
    out = bytearray()
    i = 0
    n = len(s)
    while i < n:
        c = s[i]
        if c != "\\":
            out.append(ord(c))
            i += 1
            continue
        i += 1
        if i >= n:
            break
        c2 = s[i]
        if c2 == "x":
            i += 1
            hexd = ""
            while i < n and len(hexd) < 2 and s[i] in "0123456789abcdefABCDEF":
                hexd += s[i]
                i += 1
            out.append(int(hexd, 16) if hexd else 0)
        elif c2 in "01234567":
            octd = ""
            while i < n and len(octd) < 3 and s[i] in "01234567":
                octd += s[i]
                i += 1
            out.append(int(octd, 8))
        else:
            out.append(SIMPLE_ESCAPES.get(c2, ord(c2)))
            i += 1
    return bytes(out)


def extract_fonts(source_path: str, font_names):
    with open(source_path, "r", encoding="utf-8", errors="replace") as f:
        src = f.read()
    # 注意：不要在这里清除块注释——字形二进制数据中可能恰好包含
    # "/*" 字节序列，正则会把它们误当注释吞掉。声明间的注释不影响
    # 下面的字符串字面量匹配。

    result = {}
    for name in font_names:
        # 数据为多个相邻的字符串字面量（分号是语句终止符，数据内 0x3B 的
        # 字形字节被编码为八进制转义，不会裸奔出来干扰匹配）
        pat = (
            r"const\s+uint8_t\s+" + re.escape(name) +
            r"(?:\[[0-9]+\])?\s+U8G2_FONT_SECTION\([^)]*\)\s*=\s*"
            r'((?:"(?:[^"\\]|\\.)*"\s*)+);'
        )
        m = re.search(pat, src, flags=re.S)
        if not m:
            print(f"ERROR: 未找到字体 {name}", file=sys.stderr)
            sys.exit(1)
        body = m.group(1)
        # 收集所有相邻的字符串字面量
        literals = re.findall(r'"((?:[^"\\]|\\.)*)"', body)
        if not literals:
            print(f"ERROR: 字体 {name} 声明中未找到字符串数据", file=sys.stderr)
            sys.exit(1)
        data = b"".join(parse_c_string(s) for s in literals)
        result[name] = data
        print(f"{name}: {len(data)} bytes")
    return result


def emit_c(fonts, c_path, h_path):
    PER_LINE = 12
    with open(c_path, "w") as f:
        f.write("/*\n")
        f.write(" * u8g2_fonts.c — 内置字体表（u8g2 原版数据，请勿手改）\n")
        f.write(" *\n")
        f.write(" * 由 tools/extract_u8g2_fonts.py 从 u8g2 字体源码生成。\n")
        f.write(" * 数据格式为 u8g2 字体协议（23 字节头 + 跳转表 + RLE 字形），\n")
        f.write(" * 解码器见 u8g2.c。需要更多字体时，把 u8g2_fonts.c 中对应\n")
        f.write(" * 数组拷入并重新生成即可。\n")
        f.write(" */\n\n")
        f.write('#include "u8g2.h"\n\n')
        for name, data in fonts.items():
            f.write(f"const uint8_t {name}[] = {{\n")
            for i in range(0, len(data), PER_LINE):
                row = ", ".join(f"0x{b:02X}" for b in data[i:i + PER_LINE])
                f.write(f"    {row},\n")
            f.write("};\n\n")

    with open(h_path, "w") as f:
        f.write("/* u8g2_fonts.h — 字体表声明（生成文件，见 tools/） */\n")
        f.write("#ifndef __U8G2_FONTS_H__\n#define __U8G2_FONTS_H__\n\n")
        f.write('/* 声明已并入 u8g2.h，本文件保留以便直接包含 */\n')
        f.write("#endif\n")
    print(f"已生成: {c_path}")
    print(f"已生成: {h_path}")


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)
    source = sys.argv[1]
    names = sys.argv[2:] or DEFAULT_FONTS
    fonts = extract_fonts(source, names)
    emit_c(fonts, os.path.join(OUT_DIR, "u8g2_fonts.c"),
           os.path.join(OUT_DIR, "u8g2_fonts.h"))


if __name__ == "__main__":
    main()
