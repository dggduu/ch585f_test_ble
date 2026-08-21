* 企鹅交流群787475855
* [演示视频](https://www.bilibili.com/video/BV1YW8dzuEwL)
# 下载链接
* [最新上位机及其工具下载](https://github.com/KOUFU-DIY/WeGui_RGB/releases/tag/V0.5.8)
* [STM32F103C8最新V0.5.8下载](https://github.com/KOUFU-DIY/WeGui_RGB/releases/tag/V0.5.8) 
* [GD32E230C8最新V0.5.4下载](https://github.com/KOUFU-DIY/WeGui_RGB/releases/tag/V0.5.4)  
* [CW32L012C8最新V0.5.4下载](https://github.com/KOUFU-DIY/WeGui_RGB/releases/tag/V0.5.4)
* [教程文档](https://github.com/KOUFU-DIY/WeGui_RGB/tree/main/%E6%96%87%E6%A1%A3)


# 版本更新
## WeGui RGB V0.5.8
* 1.修复mlist在进入时无法执行菜单中的begin回调函数的问题
* 2.修复其他begin回调函数问题
* 3.修复显示utf8字库中不存在的字体时候会崩溃的问题
## WeGui RGB V0.5.7
* 1.修改移植接口,现接口更简易更简单,性能降低但可移植性提高
* 2.新增RLE压缩bitmap刷图驱动,gui支持使用RLE压缩图形,用法参考driver_demo();暂时使用LCD_mcuTool取模,后续集成到上位机上
* 3.修改了字体取模,后续支持外挂字体,旧版本字体失效,需要更新上位机重新取
* 4.优化了bitmap驱动,提升了帧率
* 5.新增OLED动态刷新细分
* 6.接口更变, 暂时取消灰度屏的支持
* <img width="1507" height="901" alt="e19dd96cf96ecd11f9eb9ba332850934" src="https://github.com/user-attachments/assets/8d35da06-3e9b-4f66-bf5c-85654a63c94c" />
* <img width="1507" height="841" alt="58240881daed64d228ad3defd6334cfe" src="https://github.com/user-attachments/assets/c0f4cf90-61c1-42be-ae87-93b6ada8abe7" />  
* <img width="1643" height="841" alt="04dc74c8123aed1d81da17bc6d68800c" src="https://github.com/user-attachments/assets/81a22fb5-54b7-4a82-8874-dcfc79173312" />  

