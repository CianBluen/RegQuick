# STM32F103C8T6 寄存器与 SPL 速查

站点提供 STM32F103C8T6 片上外设速查。每页先列库函数与寄存器索引，正文按函数、寄存器顺序排列。题库支持按外设或模块、题目来源和题型筛选。站内「芯片覆盖清单」列出封装适用性和各模块入口。

## 本地启动

使用 Node.js 22.12 或更新版本。从项目根目录运行。

```powershell
npm ci
npm run build
npm run preview
```

打开终端显示的本地地址。全站搜索使用生产构建生成的 Pagefind 索引。

## 资料与固件

中文手册位于 `public/manuals/`。时钟计算器和手册框图可从相关速查页进入。

`firmware/` 保留原有 Keil 工程、源码和 ARMCC 构建脚本。从项目根目录运行相应命令。

```powershell
.\firmware\rcc-gpio\build.ps1 -CompilerBin 'D:\keil\ARM\ARMCC\bin'
.\firmware\rcc-gpio\build.ps1 -CompilerBin 'D:\keil\ARM\ARMCC\bin' -ClockExti
.\firmware\rcc-gpio\build.ps1 -CompilerBin 'D:\keil\ARM\ARMCC\bin' -TimUsart
```

速查中的扩展 SPL 示例按 STM32F10x SPL V3.5.0 编译。工作区自带的驱动在 `firmware/rcc-gpio/vendor`，其余模块核对本机 `G:\study\stm32\工程模板\Library`。USB 示例使用本机 `USB-JoyStickMouse` 示例内的 ST USB-FS-Device V3.2.1 驱动和配置头。此次编译检查的对象文件位于 `output/reference-check`，片段需要按条目所列前提纳入实际工程。
