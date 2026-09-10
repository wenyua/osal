# OSAL 框架使用说明

OSAL（Operating System Abstraction Layer，操作系统抽象层）是一套以**事件驱动的多任务**为核心的轻量级任务调度框架（纯 C 实现，源于 TI CC2530 的 Z-Stack）。它通过任务事件、消息队列、软件定时器和内存池，实现类操作系统功能，最小资源占用约为 RAM 几百字节到几 KB、ROM 几 KB。

> 本文档针对当前仓库源码（`app/`、`osal/`、`hal/`）编写。官方 API 细节可参见 `doc/OSAL_API.pdf`。

## 目录

- [1. 整体结构](#1-整体结构)
- [2. 启动流程](#2-启动流程)
- [3. 任务编程模型](#3-任务编程模型)
- [4. 任务 API](#4-任务-api)
- [5. 事件 API](#5-事件-api)
- [6. 消息 API](#6-消息-api)
- [7. 软件定时器 API](#7-软件定时器-api)
- [8. 内存管理 API](#8-内存管理-api)
- [9. 全局中断与临界区](#9-全局中断与临界区)
- [10. 典型编程模式与注意事项](#10-典型编程模式与注意事项)

---

## 1. 整体结构

| 目录/文件 | 说明 |
| -- | -- |
| `osal/` | OSAL 内核源码，跨平台（纯 C），一般无需修改 |
| `hal/` | 硬件抽象层，移植时只需重点修改 `timer.c/.h` |
| `app/` | 应用任务示例，展示如何新增任务 |

调用关系（启动时的初始化顺序）：

```
main()                       /* app/main.c */
 └─ osal_main()              /* app/osal_main.c */
     ├─ osal_init_system()   任务链表、定时器、内存初始化
     ├─ osal_add_Task()      逐个注册任务（init 函数 + 事件处理函数 + 优先级）
     ├─ osal_Task_init()     依次调用每个任务的 init
     ├─ osal_mem_kick()      内存标示激活
     └─ osal_start_system()  主循环，永不返回
```

主循环每一次迭代：

```
找最高优先级且有事件的已就绪任务（osalNextActiveTask）
  → 读出并清掉该任务的 events
  → 调用它的事件处理函数 pfnEventProcessor(taskID, events)
  → 把未处理的位按返回值还回去
```

定时心跳（本仓库 GD32E230 移植）：

```
SysTick 每 TICK_PERIOD_MS(10ms) 中断一次
  → SysTick_Handler()
      → osal_update_timers()   // hal/timer.c 中由硬件触发
          → osalTimerUpdate(TIMER_DECR_TIME)
              → 递减所有软件定时器，超时则触发对应任务事件
```

## 2. 启动流程

以 `app/osal_main.c` 为标准模板，新建工程时把 `osal_main()` 改成自己的初始化：

```c
void osal_main(void)
{
    // 1. 系统硬件、外设初始化（时钟、串口等），可在 HAL_DISABLE_INTERRUPTS 前完成

    HAL_DISABLE_INTERRUPTS();          // 关闭全局中断

    osal_init_system();                // 初始化内存、消息队列、定时器、任务链表

    // 2. 添加任务（按优先级大小，数值越大优先级越高）
    osal_add_task(my_task_init, my_task_event_process, 1);

    osal_Task_init();                   // 依次调用已注册任务的 init 函数
    osal_mem_kick();                    // 内存管理激活（喊活）

    HAL_ENABLE_INTERRUPTS();            // 开放全局中断

    osal_start_system();                // 启动主循环，永不返回
}
```

> 注意：`osal_add_Task` 会为任务结构体（持久）分配内存，`taskID` 按注册先后从 0 递增。

## 3. 任务编程模型

一个任务由两部分组成：

- **初始化函数**：`void xxx_init(uint8 task_id)`，在 `osal_Task_init()` 中被调用一次，通常：
  - 保存系统分配的 `task_id`；
  - 用 `osal_start_reload_timer` / `osal_set_event` 等设定首次触发方式。
- **事件处理函数**：`uint16 xxx_event_processor(uint8 task_id, uint16 task_event)`，被主循环反复调用。它要做的是：
  - 按 `task_event` 的各个位判断是哪种事件；
  - 处理该事件；
  - `return` 中清理已处理的事件位，把尚未处理的事件回报给框架。

### 事件位约定

`events` 是一个 `uint16` 位标志：

| 事件 | 值 | 说明 |
| -- | -- | -- |
| `SYS_EVENT_MSG` | `0x8000` | 消息到达通知（框架内部使用，任务一般必须响应并收消息） |
| 业务事件位 | `0x0001 ~ 0x4000` | 用户自定义，一个 bit 一个事件 |

`SYS_EVENT_MSG` 是框架专用位，务必不要用于普通业务事件。

## 4. 任务 API

定义在 `osal/osal_event.h`。

| 函数 | 说明 |
| -- | -- |
| `uint8 osal_init_system(void)` | 初始化 OSAL 系统（内存、消息队列、定时器、任务链表） |
| `void osal_add_Task(pTaskInitFn pfnInit, pTaskEventHandlerFn pfnEventProcessor, uint8 taskPriority)` | 注册一个任务 |
| `void osal_Task_init(void)` | 依次调用每个任务的 init 函数 |
| `void osal_start_system(void)` | 启动事件主循环，不返回 |
| `OsalTadkREC_t *osalFindTask(uint8 taskID)` | 按任务 ID 找任务节点 |
| `uint8 tasksCnt` | 已注册任务数（全局变量） |

`pTaskInitFn`：`void (*)(uint8 task_id)`
`pTaskEventHandlerFn`：`uint16 (*)(uint8 task_id, uint16 task_event)`

### 优先级

`taskPriority` 数值越大，优先级越高；注入任务链表时按优先级从大到小排序。
主循环总是先让当前就绪的最高优先级任务执行，天然实现"优先级抢占式"的合作调度
（注意：非抢占式任务都在主循环中，一旦某任务长时间不返回，会阻塞其它任务）。

## 5. 事件 API

定义在 `osal/osal_event.h`。

| 函数 | 说明 |
| -- | -- |
| `uint8 osal_set_event(byte task_id, uint16 event_flag)` | 给某任务置一个或多个事件位（可以是任意源：中断、其它任务、人为调用） |
| `uint8 osal_clear_event(uint8 task_id, uint16 event_flag)` | 清除某任务的事件位 |
| `uint8 osal_set_event` 返回 `ZSUCCESS` 或 `INVALID_TASK` | — |

> 在临界安全的角度，`osal_set_event` / `clear_event` 均已在临界区保护下修改 `events`，可从任意上下文（包括中断）调用。

## 6. 消息 API

定义在 `osal/osal_msg.h`。消息是在任务之间传递数据的手段，底层由内存池 + 队列实现。

### 发送消息

```c
// 1. 分配消息缓冲区（传入的是"消息体实际数据长度"，框架会自动多分配消息头）
general_msg_data_t *msg_pkt;
msg_pkt = (general_msg_data_t *)osal_msg_allocate(sizeof(general_msg_data_t) + sizeof(int));
if(msg_pkt != NULL)
{
    // 2. 填充消息：hdr.event 用于接收方区分消息类型
    msg_pkt->hdr.event = MY_EVENT;
    msg_pkt->hdr.status = 0;
    msg_pkt->data = (unsigned char *)(msg_pkt + 1);   // 数据指针指向缓冲区数据区
    *((int *)msg_pkt->data) = value;

    // 3. 发给目标任务（框架会入队并给目标置 SYS_EVENT_MSG 事件）
    osal_msg_send(dest_task_id, (uint8 *)msg_pkt);
}
```

### 接收消息（通常在 `SYS_EVENT_MSG` 事件分支内）

```c
if(task_event & SYS_EVENT_MSG)
{
    osal_sys_msg_t *msg_pkt = (osal_sys_msg_t*)osal_msg_receive(task_id);
    while(msg_pkt)
    {
        switch(msg_pkt->hdr.event)
        { case ...: 处理 ...; break; }
        osal_msg_deallocate((uint8*)msg_pkt);      // 处理完必须释放
        msg_pkt = osal_msg_receive(task_id);      // 继续取下一行
    }
    return (task_event ^ SYS_EVENT_MSG);          // 返回未处理的部分
}
```

| 函数 | 说明 |
| -- | -- |
| `uint8 *osal_msg_allocate(uint16 len)` | 分配消息缓冲区，返回指向消息体的指针（头已填充） |
| `uint8 osal_msg_deallocate(uint8 *msg_ptr)` | 释放已处理完的消息缓冲区 |
| `uint8 osal_msg_send(uint8 dest_task, uint8 *msg_ptr)` | 发送消息，同时给目标任务置 SYS_EVENT_MSG |
| `uint8 *osal_msg_receive(uint8 task_id)` | 从该任务的队列前端取出一条，NULL 表示无 |
| `osal_event_hdr_t *osal_msg_find(uint8 task_id, uint8 event)` | 查找某一队内消息 |
| 队列低级操作 | `osal_msg_enqueue`/`dequeue`/`push`/`extract`/`enqueue_max`，一般无需直接使用 |

注意：一道消息只能 `msg_send` 一次（发送后头被标记占用，二次 send 会失败并释放）；发送后所有权转给目标任务，接收方处理完毕必须 `deallocate`，否则泄漏内存。

## 7. 软件定时器 API

定义在 `osal/osal_timer.h`。

| 函数 | 说明 |
| -- | -- |
| `uint8 osal_start_timerEx(uint8 task_id, uint16 event_id, uint16 timeout)` | 启动一次性定时器，timeout 单位 = 心跳周期（TICK） |
| `uint8 osal_start_reload_timer(uint8 task_id, uint16 event_id, uint16 timeout)` | 启动周期/循环定时器 |
| `uint8 osal_stop_timerEx(uint8 task_id, uint16 event_id)` | 停止定时器 |
| `uint16 osal_get_timeoutEx(uint8 task_id, uint16 event_id)` | 查询剩余 TICK 数 |
| `uint8 osal_timer_num_active(void)` | 当前激活中的定时器个数 |
| `uint32 osal_GetSystemClock(void)` | 开机以来的 ticking 时钟（单位心跳，非真实毫秒） |
| `void osal_update_timers(void)` | 每个心跳调用，递减定时器（由 HAL 定时器中断调用） |

> 注意：`timeout` 与 `osal_GetSystemClock` 的单位都是「心跳周期」。若心跳为 10ms，则 `1000ms / 10 = 100` 表示 1 秒。此仓库 `print_task` 使用 `1000 / TICK_PERIOD_MS` 正由此而来。

示例（周期 1 秒定时，用完 event 必须清）：

```c
void my_task_init(uint8 task_id)
{
    my_task_id = task_id;
    osal_start_reload_timer(my_task_id, MY_PERIOD_EVT, 1000 / TICK_PERIOD_MS);
}
```

## 8. 内存管理 API

定义在 `osal/osal_memory.h`。

| 宏/函数 | 说明 |
| -- | -- |
| `MAXMEMHEAP` | 内存池大小（字节），如 `1024*6`；必须在 `#if MAXMEMHEAP >= 32768` 之内 |
| `OSALMEM_METRICS` | 置 1 开启内存统计，0 关闭（省资源） |
| `void osal_mem_init(void)` | 初始化内存池（`osal_init_system` 内部调用） |
| `void osal_mem_kick(void)` | 激活内存池 / 第一块分配 |
| `void *osal_mem_alloc(uint16 size)` | 从池中分配，返回的已符合对齐（HDR 之后） |
| `void osal_mem_free(void *ptr)` | 释放 |
| `uint16 osal_heap_mem_used(void)` 等 | 统计接口（ISALMEM_METRICS 时才可见） |

注意事项：
- `MAXMEMHEAP` 设得过大（>= 32768）会 `#error`，需按上面所述的方式扩展为 32 位管理（见 README 内存扩展）。
- 默认使用静态数组 `_theHeap` 作为池，位于 `.bss`，RAM 初始化即占用相应大小。
- 内存池结构是「定长小块 + 可变块」桶缓存方案，不是标准的 `malloc`，分配的最小单位为 `HDRSZ`（多为 4 字节对齐）。

## 9. 全局中断与临界区

定义在 `osal/type.h`，由其中的 `CLI()/SEI()` 宏实现，OSAL 内核内部大量使用。

| 宏 | 作用 |
| -- | -- |
| `HAL_ENTER_CRITICAL_SECTION()` / `HAL_EXIT_CRITICAL_SECTION()` | 进入/退出临界区 |
| `HAL_DISABLE_INTERRUPTS()` / `HAL_ENABLE_INTERRUPTS()` | 禁止/开放全局中断 |

在 GD32E230 移植中这些最终映射到 `PRIMASK`：`cpsid i`（关中断）与 `cpsie i`（开中断）。所有内核数据结构（任务链表、消息队列、定时器）的访问都应在临界区内进行，防止中断打断。

## 10. 典型编程模式与注意事项

1. **事件处理返回**：永远把已经处理的事件位从返回值中清掉，把未处理的部分 return 回去，否则事件会残留导致 `无限回放`。
   ```c
   return (task_event ^ SYS_EVENT_MSG);   // 消息处理完，交回剩余位并清掉 SYS_EVENT_MSG
   ```
2. **消息内存所有权**：发送后接收方必须 `deallocate`；分配后如需发送失败要自己 `deallocate` 或发送失败由框架释放。
3. **临界区不是锁**：临界区只禁用中断仍不阻塞任务，所有互斥都是通过"一次只能有一个任务跑"来隐含保证（非抢占式协程风格）。
4. **任务处理尽量短**：不要在事件处理里做长阻塞（如忙等、长时间延时），会阻塞低优先级任务。长延时用软件定时器而非 `delay`。
5. **消息头大小**：`osal_msg_allocate(len)` 的 `len` 是你的实际数据长（下会加上 `sizeof(osal_msg_hdr_t)`）。需保证 `len + sizeof(hdr)` 不超过内存池可用。
6. **多任务共享全局**：由于非抢占，任务上下文内的共享数据在一些情况下是天然的原子；但 API 仍保留临界区。
7. **适配平台**：新平台只需按 README 改 `hal/timer.c`（心跳）、`osal/type.h`（类型与中断开关）、`osal/osal_memory.h`（内存池大小）即可。

## 附：快速自查清单（新增任务步骤）

1. 在 `task_event.h`（或新头文件）中 `extern uint8 my_task_id;`
2. 编写 `my_task_init(uint8 id)` 与 `my_task_event(uint8 id, uint16 e)`；
3. 在 `osal_main()` 中添加 `osal_add_Task(my_task_init, my_task_event, 优先级);`
4. 用 `osal_set_event`/`osal_msg_send`/定时器把事件送进去；
5. 处理完记得在 return 中把事件位清回。

欲知更多函数边界返回值和详细行为，请参考 `doc/OSAL_API.pdf` 与各 `.c` 文件源码注释。


