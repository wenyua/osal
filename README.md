# OSAL

OSAL(operating system abstraction layer)，操作系统抽象层，是一种以实现多任务为核心的系统资源管理机制，实现了类似操作系统的某些功能，但并不能称之为真正意义上的操作系统。本OSAL仓库源码来源于TI CC2530的zigbee协议栈Z-Stack中，剥离了其应用相关及不通用的功能模块，提取其最核心的事件驱动型多任务内核。OSAL的实现源码非常精简高效，总共约1100多行，全部纯C语言实现，最小资源占用要求为RAM约512Byte，ROM约2KB。理论上可以移植至全部支持C语言的芯片平台。

## OSAL移植的接口

| OSAL接口 | 说明 |
| -- | -- |
| Message Management API | 消息管理 |
| Task Synchronization API | 任务同步 |
| Timer Management API | 定时器管理 |
| Memory Management API | 内存管理 |

## 移植说明

1. 完成hal\timer.c文件，为系统提供滴答时钟，建议滴答心跳的周期为1～10ms，并对应修改hal\timer.h中的宏定义TICK_PERIOD_MS为相应心跳毫秒值；
2. 修改osal\type.h文件中的全局中断开关宏定义（可为空），根据需要修改数据类型的宏定义，根据实际芯片字长修改“halDataAlign_t”类型；
3. 根据需要修改osal\osal_memory.h文件中的内存池大小定义，默认最大为32768字节，osal\osal_memory.c中osalMemHdr_t类型需要确保长度为16bit或以上，非8位单片机需要设定内存池的字节对齐；
4. 添加任务函数中的任务优先级数值大的任务则优先级高；
5. 根据需要修改osal\osal_memory.h文件中的OSALMEM_METRICS定义，有效则开启内存统计功能；

各API的使用可参考doc下的官方API手册《OSAL_API.pdf》。

## 动态内存管理拓展说明

OSAL中默认使用15位的数据标识管理内存，最大能管理32768字节，需要增加管理更多的动态内存可按照以下方式拓展：

1. 注释掉osal_memory.c中的内存大小编译限制；
2. 替换osal_memory.c中的全部uint16为osalMemHdr_t；
3. 修改osal_memory.h中的osalMemHdr_t类型宏为halDataAlign_t，确保芯片字长halDataAlign_t为32bit；
4. 修改osal_memory.c中的宏定义OSALMEM_IN_USE为0x80000000；

## 编译运行

本仓库在linux下可以直接编译运行基础例程，例程定义了两个任务，任务一使用定时器API进行定时触发打印事件，并累计打印次数，每累计5次就会向任务二发送统计事件，任务二接收任务一发送的统计事件后进行统计结果的打印输出。

编译：

```shell
wat@wat:~$ make
building ./app/main.c
building ./app/osal_main.c
building ./app/print_task.c
building ./app/statistics_task.c
building ./hal/timer.c
building ./osal/osal_msg.c
building ./osal/osal_event.c
building ./osal/osal_timer.c
building ./osal/osal_memory.c
building ./osal/osal.c
linking object to linux-osal-example.elf

real    0m0.585s
user    0m0.332s
sys     0m0.242s
```

运行：

```shell
wat@wat:~$ ./linux-osal-example.elf
Init hal timer ok !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Statistics task receive print task printf count : 5
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Statistics task receive print task printf count : 10
Print task printing, total memory : 6144 byte, used memory : 92 byte !
Print task printing, total memory : 6144 byte, used memory : 92 byte !
......
```

## GD32E230 适配说明（本分支）

本示例默认在 Linux 上通过线程模拟定时器运行。为将 OSAL 移植到
GD32E230（Cortex-M23 内核）单片机，已按官方的移植点完成以下适配：

### 1. 硬件定时器（hal/timer.c）

- 使用 Cortex-M23 内核 SysTick 提供系统滴答心跳，替代 Linux 线程模拟。
- `OSAL_TIMER_TICKINIT()` 配置 SysTick 重装载值，使其按 `TICK_PERIOD_MS`(10ms) 触发中断。
- `SysTick_Handler()` 中调用 `osal_update_timers()` 更新软件定时器链表。
- `OSAL_TIMER_TICKSTART()/TICKSTOP()` 控制 SysTick 使能。
- 系统主频取自固件库符号 `SystemCoreClock`；若工程未定义，可自行 `#define SystemCoreClock`（如 72MHz）。

### 2. 类型与临界区（osal/type.h）

- `uint32/int32U` 固定为 32 位；`halDataAlign_t` 保持 32 位以便内存管理按 4 字节对齐。
- 临界区宏改用 Cortex-M23 的 `PRIMASK`（`cpsid i` / `cpsie i`）实现全局中断开关，替代 Linux 空实现。

### 3. 程序入口（app/main.c）

- 删除 `argc`/`argv`，改为嵌入式 `int main(void)`，由启动文件调用。

### 4. GD32E230 编译目标（Makefile）

新增目标 `make gd32`，使用 `arm-none-eabi-gcc` 编译 Cortex-M23 固件：

```shell
make gd32            # 生成 gd32e230-osal-example.elf / .bin
make rebuild-gd32     # 清理后重新编译
make clean-gd32       # 清理 GD32E230 产物
```

`gd32e230/` 目录下提供：
- `gd32e230.ld`：GD32E230 链接脚本（Flash 64KB / SRAM 8KB，可按实际容量修改）。
- `startup_gd32e230.s`：启动文件（中断向量表 + 数据段/BSS 初始化 + 调用 main）。

### 5. 注意事项

- **内存池大小**：示例 `MAXMEMHEAP=6144`，OSAL 内部使用静态数组作为堆（约 6KB），GD32E230 8KB SRAM 下需按实际芯片调低 `osal/osal_memory.h` 中的 `MAXMEMHEAP`；6KB SRAM 的型号需相应调小。
- **printf 重定向**：打印任务使用 `printf()`，在 GD32E230 上运行前需先初始化串口（如 USART0）并将 `fputc` 重定向到串口，否则无法看到输出。
- **下载烧录**：编译生成的 `.bin` 可用 J-Link 等烧录器写入，从 0x08000000 片内启动。
