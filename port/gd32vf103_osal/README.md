# OSAL 移植例程 — GD32VF103 (RISC-V Bumblebee 核)

将 OSAL 操作系统移植到 GD32VF103, 演示任务调度、软件定时器、消息机制
以及 **Tickless 空闲低功耗**(机器定时器一次性唤醒)。

## 目录结构

```
gd32vf103_osal/
├── osal/        OSAL 核心层(平台无关, 与根目录 osal/ 保持一致)
├── hal/
│   ├── osal_hal.h   移植契约实现: RISC-V 全局中断(mstatus.MIE) + WFI 睡眠
│   ├── timer.c/h    心跳源: CLINT mtime/mtimer + ECLIC (eclic_mtip_handler)
│   ├── console.c    USART0 控制台初始化(printf 经固件库 stubs 重定向)
│   └── gd32vf103_libopt.h  固件库副本缺失的库配置头(按需包含外设头)
├── osal_app/    应用任务
│   ├── led_task         每 500ms 翻转 PC13 LED 并打印系统时钟
│   ├── print_task       每 1s 打印堆内存使用, 每 5 次向统计任务发消息
│   ├── statistics_task  接收统计消息并打印
│   ├── osal_main.c      任务注册(led=1, print=2, statistics=3)
│   └── main.c           入口: 初始化串口/LED 后启动 OSAL
└── Makefile     使用本地 Firmware/ 官方固件库
```

## 硬件资源

| 资源 | 用途 |
|---|---|
| USART0 (PA9/PA10) | printf 输出, 115200 |
| PC13 | 板载 LED(低电平点亮), 500ms 闪烁 |
| CLINT mtime/mtimer | OSAL 系统滴答(TICK_PERIOD_MS = 10ms), Tickless 唤醒 |
| ECLIC IRQ_M_TIMER | 机器定时器中断 → `eclic_mtip_handler` |

## 构建与烧录

```bat
riscv-none-embed-gcc 工具链路径由 RISCV_DIR 指定(默认 xpack 安装位置)
make          # 生成 osal-gd32vf103.elf/.bin
make clean
```

板型宏默认 `GD32VF103V_EVAL`(HXTAL 8MHz), 换板时请同步修改 Makefile 中
`CFLAGS` 与 `system_gd32vf103.c` 的时钟宏。

烧录方式与 ../gd32vf103 例程相同(DFU/SPL/OpenOCD 均可)。

## 关键移植点(移植到其他 RISC-V MCU 时参考)

1. `hal/osal_hal.h` — 提供全局中断开关(操作 `mstatus.MIE`)与
   `OSAL_IDLE_SLEEP()`(`wfi` 指令)。
2. `hal/timer.c` — 提供周期 tick 与 tickless 一次性定时:
   - `OSAL_TIMER_ONESHOT(ms)`: 把 mtimercmp 设为 `mtime + N`, 空闲时睡到点;
   - `OSAL_TIMER_TICKRESTORE()`: 唤醒后恢复周期节拍;
   - 定时器时间补偿由 OSAL 核心的 `osalTimerUpdate(sleepTicks)` 完成。
3. 时钟频率: `TIMER_FREQ = SystemCoreClock / 4`(CLINT 定时器频率)。
