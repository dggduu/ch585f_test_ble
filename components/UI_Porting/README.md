# UI_Porting — u8g2 风格显示框架（对接 bsp_lcd_hw）

为 CH585F + ST7789V（240x240，SPI，RGB565）彩色屏幕提供一套 **u8g2 兼容 API** 的轻量显示框架，
让 `components/my-u8g2-ui-toolkit` 的 UI 代码（HList、VList、page_stack、splash_log、
splash_screen、brick_break、portal_component、ui_toolkit）**不经修改**直接编译运行。

彩色屏幕的像素位深通过宏配置，默认 **3 bit/像素（8 色调色板）**，
240x240 全屏帧缓冲仅 21600 字节（CH585F SRAM 128KB，可承受）。

## 目录结构

```
components/UI_Porting/
├── u8g2_porting.h        # 配置层：屏幕尺寸 / 位深 / 调色板 / LCD_WritePixels 声明
├── u8g2.h                # u8g2 兼容 API 头（类型、结构体、函数原型、颜色常量）
├── u8g2.c                # 实现：3bpp 帧缓冲、图元、字形 RLE 解码器、文本、逐行刷新
├── u8g2_fonts.c/.h       # 内置字体表（u8g2 原版数据，生成文件）
└── tools/
    ├── extract_u8g2_fonts.py    # 从 u8g2 字体源码提取字体的脚本
    └── host_test/
        ├── test_u8g2.c          # 主机端单元测试（stub 掉 LCD_WritePixels）
        └── run.sh               # 编译并运行测试
```

## 配置宏（u8g2_porting.h）

| 宏 | 默认值 | 说明 |
|---|---|---|
| `U8G2_PORTING_SCREEN_W/H` | 240 / 240 | 屏幕宽高，与 `bsp_lcd_hw.h` 的 `LCD_W/LCD_H` 保持一致 |
| `U8G2_PORTING_BPP` | 3 | 像素位深，支持 1/2/3/4/8。3 = 8 色调色板 |
| `U8G2_PORTING_DEFAULT_PALETTE` | 8 色 RGB565 | 默认调色板（黑/白/红/绿/蓝/黄/青/洋红），可整体覆盖 |
| `U8G2_PIX_BUF_BYTES` | 由上式算出 | 帧缓冲字节数 = W*H*BPP/8 |
| `U8G2_NUM_COLORS` | `1<<BPP` | 调色板条目数 |

> BPP=3 时默认调色板与 `bsp_lcd_hw.h` 的 `LCD_WHITE/BLACK/RED/...` 一一对应。
> 其他位深需自行覆盖默认调色板宏，条目数必须等于 `U8G2_NUM_COLORS`。

## 与 toolkit 的 API 对照（统计自 my-u8g2-ui-toolkit）

toolkit 实际调用的 u8g2 API 均已在本框架实现，签名一致：

**初始化/缓冲**：`u8g2_porting_init`（本框架新增，替代 `u8g2_Setup_*`）、
`u8g2_InitDisplay`（空操作）、`u8g2_SetPowerSave`（空操作）、
`u8g2_ClearBuffer`、`u8g2_SendBuffer`

**颜色/字体**：`u8g2_SetDrawColor`、`u8g2_SetFontMode`、`u8g2_SetFont`、
`u8g2_SetFontPosBaseline`（空操作，锚点固定基线）

**裁剪**：`u8g2_SetClipWindow`、`u8g2_SetMaxClipWindow`

**图元**：`u8g2_DrawPixel`、`u8g2_DrawHLine`、`u8g2_DrawVLine`、
`u8g2_DrawBox`、`u8g2_DrawFrame`、`u8g2_DrawRBox`、`u8g2_DrawXBM`

**文本**：`u8g2_DrawStr`、`u8g2_DrawUTF8`、`u8g2_DrawGlyph`、
`u8g2_GetStrWidth`、`u8g2_GetGlyphWidth`

**内置字体**：`u8g2_font_5x7_tf`、`u8g2_font_6x10_tf`、`u8g2_font_8x13_tr`、
`u8g2_font_logisoso20_tn`、`u8g2_font_open_iconic_all_4x_t`

**补充提供**（未在 toolkit 使用，但属 u8g2 常用）：`u8g2_DrawLine`、`u8g2_DrawRFrame`、
`u8g2_DrawCircle`、`u8g2_DrawDisc`、`u8g2_DrawTriangle`、`u8g2_DrawXBMP`、
`u8g2_GetUTF8Width`、`u8g2_GetFontAscent/Descent/Height`、`u8g2_get_buffer`、
`u8g2_porting_set_palette`

**颜色常量**（BPP>=3 时可用）：`U8G2_COLOR_BLACK/WHITE/RED/GREEN/BLUE/YELLOW/CYAN/MAGENTA`

> 注意：`u8g2_Setup_ssd1306_i2c_128x64_noname_f` 这类带 u8x8 硬件层的初始化函数
> 不在本框架内 —— STM32 demo 的 `main.c/ui.c`（含 `stm32f10x.h`）已从构建中排除，
> CH585 的初始化流程见下文。

## 使用流程

```c
#include "u8g2.h"

void app_main(void)
{
    Lcd_Init();                        // 1. bsp_lcd_hw 初始化屏幕（必须先调用）
    u8g2_porting_init(&u8g2);          // 2. 绑定帧缓冲/调色板，复位裁剪与颜色

    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
    u8g2_SetDrawColor(&u8g2, U8G2_COLOR_WHITE);
    u8g2_ClearBuffer(&u8g2);
    u8g2_DrawStr(&u8g2, 10, 30, "Hello CH585!");
    u8g2_DrawBox(&u8g2, 0, 40, 40, 20);          // 画红色框改 SetDrawColor(U8G2_COLOR_RED)
    u8g2_SendBuffer(&u8g2);              // 3. 逐行刷新到 LCD（每行调用一次 LCD_WritePixels）
}
```

要点：

- `u8g2` 是全局显示对象（`extern u8g2_t u8g2;`），toolkit 的
  `portal_component.c` 等直接引用它，无需自建实例。
- 绘制色 = 调色板索引：`u8g2_SetDrawColor(0)` 即背景色（黑色，可作橡皮擦）。
- 3bpp 帧缓冲是**全缓冲**：先 `ClearBuffer` 清屏，画完一次 `SendBuffer` 整帧刷新。
  刷新按行执行（每行 240 像素 = 480 字节），不需要分页模式。

## 底层刷新接口

`u8g2_SendBuffer` 每行调用一次：

```c
void LCD_WritePixels(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                     const uint16_t *pixels, uint32_t count);
```

实现在 `src/bsp/bsp_lcd_hw.c`（设置窗口后把 RGB565 像素分块经 `bsp_spi_send_bulk` 发送，
480 字节静态缓冲一次发完一整行）。`u8g2_porting.h` 中仅有前置声明，主机测试可替换为 stub。

## 主机端测试

不依赖 MCU 外设，用 stub 的 `LCD_WritePixels` 验证帧缓冲打包、字形解码、
图元、裁剪与逐行刷新：

```sh
cd components/UI_Porting/tools/host_test
./run.sh
```

覆盖：3bpp 打包/解包往返（含跨字节边界）、字形形状与步进、GetStrWidth、
裁剪窗口、Box/Frame/Line/Circle/Disc/Triangle/XBM、颜色索引、RGB565 调色板映射、
自定义调色板、SendBuffer 逐行输出。

## 字体

内置 5 个字体的数据与 u8g2 完全一致（23 字节头 + 跳转表 + RLE 字形），
解码器按 u8g2 原版算法实现（`u8g2_font_decode_glyph` 等）。

新增字体：从 u8g2 字体源码（如 `backup/u8g2_fonts.c`）中提取——

```sh
python3 tools/extract_u8g2_fonts.py <u8g2_fonts.c> u8g2_font_xxx
```

提取脚本只做原样字节搬运（不去注释、不解码），保证数据零改动；
提取后在 `u8g2.h` 中补 `extern` 声明即可使用。

## 与标准 u8g2 的差异

- 帧缓冲为全缓冲，位深由宏配置（默认 3bpp），无 FirstPage/NextPage 分页模式；
- 无 u8x8 硬件抽象层（由 `bsp_lcd_hw` 承担），无 `u8g2_Setup_*` 初始化函数；
- 颜色索引 0 = 背景（黑色），`SetDrawColor(0/1)` 语义与 1bpp u8g2 一致；
- 文本锚点固定在基线（`SetFontPosBaseline` 为空操作，与 u8g2 默认一致）；
- `u8g2_GetGlyphWidth` 返回 `int8_t`（u8g2 的 delta_x 为有符号值）。
