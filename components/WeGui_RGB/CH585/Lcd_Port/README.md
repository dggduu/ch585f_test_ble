# CH585 WeGui_RGB 移植说明

本目录是 [WeGui_RGB](https://github.com/KOUFU-DIY/WeGui_RGB) (KOUFU-DIY, Apache-2.0) 在
WCH CH585F (RISC-V, 128KB SRAM / 448KB Flash) 上的硬件移植层。

## 硬件与配置

| 项目 | 配置 | 说明 |
|------|------|------|
| 屏幕 | ST7789V2 (BOE154IPS) 240x240 RGB565 | 初始化时序与内置 `_ST7789V3` 驱动一致 (`lcd_driver_config.h`) |
| 接口 | 硬件 SPI0 (PA13=SCK, PA14=MOSI, Mode0) | `bsp_spi_send_bulk` 阻塞发送 |
| 控制脚 | CS/DC/RST/BLC 走 PCA9539 扩展 IO | `bsp_pin_defs.h` 的 `SCREEN_CS/DC/RST/BLC` 宏 |
| 色彩深度 | 3 位色 (8 色/像素) | `LCD_COLOUR_BIT=3`, 主题色表见 `lcd_wegui_config.h` |
| 刷屏模式 | 页缓存动态刷新 `_PAGE_BUFF_DYNA_UPDATE` | 3 位色全屏缓存需 172.8KB > 128KB SRAM, 页缓存仅 720B/页 |
| 按键 | `WEGUI_PORT=_NO_GUI_PORT` | 板载无按键, 菜单仅展示; 加按键后改为键值端口 |

## 文件

- `lcd_driver_config.h` — 屏幕分辨率 / 刷屏模式 / IC 型号 / 字体 / 外挂 Flash 配置
- `lcd_port_ch585.h` / `lcd_port_ch585.c` — 端口实现 (SPI 收发、RGB565 转换、CRC16)
- `lcd_wegui_config.h` — wegui 主题色、菜单深度、MCU 型号字符串

## 移植要点

1. `lcd_port_init()` 必须先调用 `lcd_ic_init()` (在 `lcd_driver_init()` 内部调用)
2. `lcd_send_nCmd` 按 TFT 约定: 数组[0] 按命令、其余按数据 (DC 切换)
3. `lcd_rgb565_port` 逐行转 RGB565 (大端) 进缓冲后批量 SPI 发送, 避免逐字节发送
4. `lcd_gram_crc_port` 为软件 CRC16 (0x1021), 仅用于显存变化检测, 不要求与硬件一致
5. `lcd_is_busy` 恒返 0: SPI0_MasterTrans 为阻塞发送, 无需忙检测

## 页缓存动态刷新的语义 (重要)

`GRAM_YPAGE_NUM=1` 时仅 1 页显存 (720B) 滚动复用, crc 表按全屏 30 页存储:

- `LCD_Refresh()` 每次只处理**当前刷新页** (`lcd_refresh_ypage`), 扫 8 个 chunk
  (30px 一个) 计算 CRC, 与 `crc[页][块]` 比对, 变化才重发对应块
- **绘制只允许落在当前刷新页内** (`gram_draw_one_byte` 会丢弃页外写入):
  UI 渲染应先对准刷新页再画; wegui 内置渲染流程 (画当前页 → 刷新 → 推进) 天然满足
- `rgb_set_driver_colour` 修改颜色表后会自动 `lcd_reset_crc()` 强制全刷, 属正常设计
- 窗口 `x1` 为**排他上界** (`x_end = x_start + 30px`), 仅末块截断到 SCREEN_WIDTH-1

## 固件集成 (src/Main.c)

```c
lcd_driver_init();        // 屏幕驱动初始化 (SPI + 复位 + 背光 + IC + 全屏刷)
lcd_wegui_init();         // wegui 图形库初始化
wegui.menu = &m_main;     // 开机初始菜单

TMR0_TimerInit(FREQ_SYS / 1000);   // 1ms 软件计时
TMR0_ITCfg(ENABLE, RB_TMR_IE_CYC_END);
PFIC_EnableIRQ(TMR0_IRQn);

while (1) { wegui_loop_func(); }   // 菜单/渲染/动态刷新

// TMR0_IRQHandler 中调用 wegui_1ms_stick()
```

## 后续升级

- **加按键**: `WEGUI_PORT` 改为按键端口实现 (模拟键值 / 矩阵), 参考库内
  `STM32F103` 与 `mcu_demo_project` 示例
- **换更大屏幕**: 改 `SCREEN_WIDTH/SCREEN_HIGH` 与偏移, 或改用全屏缓存动态刷新
  (`_FULL_BUFF_DYNA_UPDATE`, 需 RAM ≥ 屏幕全缓存大小)
- **外挂 Flash**: 实现 `flash_port_*` 并设 `FLASH_PORT/_FLASH_W25Qxx`, 用于存储
  `mcu_fonts_utf8_*` 等大资源

## 验证

移植层与库的 `lcd_driver.c` 可脱离硬件 host 编译验证 (CRC 一致性、页刷新循环、
单像素触发单块重发、RGB565 色彩映射), 测试骨架见 `/tmp/wegui_host_test/`。
