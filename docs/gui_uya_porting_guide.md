# UyaGUI 移植指南

## 平台抽象面

移植时主要看三个模块：

- [disp.uya](/home/winger/uya/gui-uya/src/gui/platform/disp.uya:1): framebuffer 与 display profile
- [indev.uya](/home/winger/uya/gui-uya/src/gui/platform/indev.uya:1): 触摸/鼠标/按键/编码器
- [tick.uya](/home/winger/uya/gui-uya/src/gui/platform/tick.uya:1): 毫秒/微秒计时与 sleep

文件系统和资源层如果目标板需要落地，也要看：

- [fs.uya](/home/winger/uya/gui-uya/src/gui/res/fs.uya:1)
- [cache.uya](/home/winger/uya/gui-uya/src/gui/res/cache.uya:1)

## 推荐移植顺序

1. 先打通 `tick`
2. 再打通 `DisplayCtx` / framebuffer
3. 确认 `RenderCtx` 能在离屏 buffer 正常绘制
4. 再接入输入设备
5. 最后再考虑 DMA / 硬件 GPU 优化

## 显示侧检查项

- 分辨率
- 像素格式
- stride 是否正确
- 双缓冲还是单缓冲
- 刷新区域是否支持局部传输

## 输入侧检查项

- 触摸坐标范围
- 校准样本与滤波窗口
- 是否需要 hover / pinch / encoder

## 当前状态

仓库内已经提供 Linux/宿主环境下的验证基线，但这些目标板项仍需要真实硬件完成：

- STM32F4xx / STM32H7xx
- ESP32-S3
- RP2040
- RISC-V GD32V
