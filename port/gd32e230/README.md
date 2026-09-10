# OSAL 移植到 GD32E230 (Cortex-M23)

本目录是 OSAL 操作系统抽象层在国产 32 位 MCU **GD32E230**（Arm Cortex-M23 内核，GD32E23 系列）上的独立完整移植工程，不依赖厂商固件库，直接操作 Cortex-M23 内核寄存器与 GD32E230 外设寄存器，方便嵌入任意工程环境。

## 目录结构

```
port/gd32e230/
├── Makefile                   # GNU 工具链构建脚本(arm-none-eabi-gcc)
├── osal/                      # OSAL 内核源码(取自仓库 osal/, 已做的移植调整)
│   ├── osal.c / osal.h
│   ├── osal_event.c/.h        # 任务调度与事件
│   ├── osal_timer.c/.h        # 软件定时器
│   ├── osal_msg.c/.h          # 消息队列
│   ├── osal_memory.c/.h       # 内存池管理
│   └── type.h                 # 类型定义 + 临界区(PRIMASK)
├── hal/
│   └── timer.c/.h             # SysTick 滴答时钟, 调用 osal_update_timers()
├── board/
│   ├── gd32e230_regs.h        # GD32E230 寄存器最小映射
│   ├── system_gd32e230.c      # 系统主频符号(默认 72MHz)
│   ├── board.c/.h             # 时钟 + USART0 初始化
│   ├── retarget.c             # printf -> USART0 重定向 (_write)
│   ├── startup_gd32e230.s     # 启动文件(向量表 + 数据/BSS 初始化)
│   └── gd32e230.ld            # 链接脚本(Flash 64KB / SRAM 8KB)
└── osal_app/                  # 示例应用
    ├── main.c
    ├── osal_main.c            # OSAL 启动, 注册任务
    ├── task_event.h
    ├── print_task.c           # 打印任务 + 向统计任务发消息
    ├── statistics_task.c      # 统计任务
    ├── led_task.c/.h          # LED 闪烁任务(重复定时器 + GPIO)
```

## 移植接口对照

| OSAL 接口 | GD32E230 实现 |
| -- | -- |
| Timer Management | `hal/timer.c`：SysTick (Cortex-M23) 提供 10ms 滴答，中断调 `osal_update_timers()` |
| Critical Section | `osal/type.h`：`CLI()/SEI()` 用 `cpsid i / cpsie i` (PRIMASK) |
| Memory Alignment | `osal/type.h`：`halDataAlign_t = uint32`，4 字节对齐 |
| Heap | `osal/osal_memory.h`：`MAXMEMHEAP=2048`（8KB SRAM），`osalMemHdr_t=halDataAlign_t` |
| `SystemCoreClock` | `board/gd32e230_regs.h` 默认 72MHz |

## 编译

依赖 ARM GNU 工具链：

```shell
make            # 生成 gd32e230-osal-example.elf / .bin
make clean      # 清理
make list       # 查看参与编译的源文件
```

## 烧录与运行

- 编译产物为 `gd32e230-osal-example.bin`，起始地址 `0x08000000`，可用 J-Link / GD-Link 等写入片内 Flash。
- 串口：**USART0，PA9(TX)，波特率 115200，8N1**。
- 上电后每秒输出一次打印任务信息，每打印 5 次统计任务输出计数：

```text
Init hal timer ok !
Print task printing, total memory : 2048 byte, used memory : ... byte !
...
Statistics task receive print task printf count : 5
```

## 说明

- 由于不同 batch GD32E230 的 PLL 倍频编码不同，`board/board.c` 中的 `PLL_CFG_VALUE` 请按《GD32E230 用户手册》RCU_CFG0.PLLMF 调整。
- 若工程已有官方 `system_gd32e230.c`，可删除 `board/system_gd32e230.c`。
- 内存池 8KB SRAM 下默认 2048 字节，实际按芯片调 `osal/osal_memory.h` 的 `MAXMEMHEAP`。
- 板载 LED 引脚默认 **PC13(低电平点亮)**：若与实际开发板不同，修改 `board/gd32e230_regs.h` 的 `BOARD_LED_PORT` / `BOARD_LED_PIN` 即可。

## 运行效果示例

上电后每秒打印一次统计信息，同时板载 LED 每 500ms 翻转一次：

```text
LED blink, system clock : 500 ms
Print task printing, total memory : 2048 byte, used memory : ... byte !
LED blink, system clock : 1000 ms
...
Statistics task receive print task printf count : 5
```