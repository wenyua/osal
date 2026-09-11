# OSAL 精简移植 — CH32V003 (青稞 V2A, 16KB Flash / 2KB RAM)

面向资源最紧张芯片的 OSAL 移植示例: 无 WCH 官方库依赖、无 printf、
内存堆裁剪到 512B, Tickless 空闲睡眠保留。

## 目录结构

```
ch32v003/
├── osal/        OSAL 核心层(MAXMEMHEAP=512B)
├── hal/
│   ├── osal_hal.h   RISC-V 全局中断(mstatus.MIE) + WFI(与 GD32VF103 通用)
│   └── timer.c/h    心跳源: 内核 SysTick(STK), 支持 Tickless 一次性定时
├── osal_app/
│   ├── led_task         PD6 LED 每 500ms 翻转
│   ├── print_task       每 2s 打印堆内存/系统时钟(轻量串口, 无 printf)
│   ├── console.c        USART1(PD5-TX) putc/puts/putint
│   ├── osal_main.c      任务注册(led=1, print=2)
│   └── main.c           入口
├── wch/
│   ├── ch32v003_hw.h    最小寄存器定义(仅本例程所需)
│   ├── startup_ch32v003.S  最小启动: 栈/mtvec/向量表/bss/data
│   └── ch32v003.ld      链接脚本(16K Flash @0x08000000, 2K RAM @0x20000000)
└── Makefile         rv32ec/ilp32e 工具链配置
```

## 资源预算(2KB RAM)

| 项目 | 大小 |
|---|---|
| OSAL 内存堆 | 512 B |
| 栈 | 512 B |
| OSAL 全局变量 | ~64 B |
| 剩余给业务 | ~950 B |

## 硬件假设(按板子修改)

| 项 | 默认值 | 修改位置 |
|---|---|---|
| LED | PD6, 低电平点亮 | `led_task.c` 宏 |
| 串口 | USART1, PD5-TX, 115200 | `console.c` |
| 主频 | 复位默认 HSI 24MHz | `hal/timer.c` / `console.c` 的 `CORE_CLOCK` |
| 向量表 | 复位 0x08000000, 直接模式 mtvec | `wch/` |

## 构建

```bat
make          # 生成 osal-ch32v003.elf/.bin/.map
make clean
```

工具链: WCH `riscv-none-embed`(MounRiver) 或 xpack, 注意架构为
`-march=rv32ec -mabi=ilp32e`。

## 与 GD32VF103 移植的差异

| 项 | GD32VF103 | CH32V003 |
|---|---|---|
| 架构 | rv32imac / ilp32 | **rv32ec / ilp32e**(16 寄存器) |
| Tick 源 | CLINT mtime + ECLIC | 内核 **STK** + 直接模式向量表 |
| 中断控制器 | ECLIC | PFIC(本例只用了 STK, 无需配置) |
| printf | 固件库 stubs | **禁用**, 轻量 console |
| 内存堆 | 6KB | **512B** |
| Tickless | 保留 | 保留(STK 比较值推迟 + 补偿标志防双计) |

## 注意事项

1. `rv32ec` 无硬件除法: 避免运行时除法, 分频值都用编译期常量
   (`hal/timer.c` 的 `HEARTBEAT_TICKS` 为宏常量折叠)。
2. Tickless 补偿标志(`oneshot_armed`)保证睡眠唤醒的中断不会与
   主循环的 `osalTimerUpdate` 双重计数——其他移植(GD32E230/GD32VF103)
   建议同步应用此修正。
3. 正式项目建议替换 `wch/ch32v003_hw.h` 为 WCH EVT 包的 `ch32v003.h`。
