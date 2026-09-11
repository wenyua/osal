/*
 * @file timer.c
 * @brief OSAL 硬件定时器实现(GD32VF103): 使用 Bumblebee 核内 CLINT 的
 *        mtime/mtimer(机器定时器) + ECLIC 提供系统滴答心跳
 * @date  2026-09-11
 */
#include <stdint.h>

/* 注意: 芯片头文件需在 type.h(定义 ERROR/SUCCESS 宏)之前包含 */
#include "gd32vf103.h"
#include "riscv_encoding.h"   /* IRQ_M_TIMER */

#include "timer.h"
#include "osal_timer.h"
#include "n200_timer.h"
#include "n200_func.h"
#include "n200_eclic.h"

#define HAL_HEARTBEAT_HZ   ((uint32_t)(1000UL / TICK_PERIOD_MS))

/* mtimercmp 为 64 位寄存器, 拆成高低 32 位访问(先写高再写低, 防止误触发) */
#define MTIMECMP_HI        (*(volatile uint32_t *)(TIMER_CTRL_ADDR + TIMER_MTIMECMP + 4))
#define MTIMECMP_LO        (*(volatile uint32_t *)(TIMER_CTRL_ADDR + TIMER_MTIMECMP))

/*
 * @brief 设定下一次机器定时器比较值: 当前 mtime + delta。
 * @param delta  mtime tick 数(频率 TIMER_FREQ = SystemCoreClock/4)
 */
static void timer_schedule(uint64_t delta)
{
    uint64_t next = get_timer_value() + delta;

    MTIMECMP_HI = 0xFFFFFFFFUL;
    MTIMECMP_LO = (uint32_t)(next & 0xFFFFFFFFUL);
    MTIMECMP_HI = (uint32_t)(next >> 32);
}

static volatile uint8_t oneshot_armed;  /* 一次性定时期间不更新软件定时器 */

/*
 * @brief 机器定时器中断服务函数(由 entry.S 中断入口调用),
 *        作为 OSAL 心跳源, 每个心跳周期更新软件定时器。
 */
void eclic_mtip_handler(void)
{
    /* 立即安排下一个周期 tick, 保证节拍连续 */
    timer_schedule((uint64_t)TIMER_FREQ / HAL_HEARTBEAT_HZ);

    /* Tickless 一次性定时期间, 补偿由 osal.c 主循环统一完成 */
    if(oneshot_armed)
    {
        oneshot_armed = 0;
    }
    else
    {
        osal_update_timers();
    }
}

/*
 * @brief 初始化机器定时器心跳: 配置 ECLIC 并装载第一个比较值。
 */
void OSAL_TIMER_TICKINIT(void)
{
    /* ECLIC: 机器定时器中断(IRQ_M_TIMER), 电平触发, 低等级 */
    eclic_set_intattr(IRQ_M_TIMER, 0);
    eclic_set_irq_lvl(IRQ_M_TIMER, 1);
    eclic_enable_interrupt(IRQ_M_TIMER);

    /* 装载首个周期 tick 比较值(中断生效还需全局 MIE 打开) */
    timer_schedule((uint64_t)TIMER_FREQ / HAL_HEARTBEAT_HZ);
}

/*
 * @brief 开启机器定时器中断(保持与全局中断分离, 空实现保留接口)。
 */
void OSAL_TIMER_TICKSTART(void)
{
    eclic_enable_interrupt(IRQ_M_TIMER);
}

/*
 * @brief 停止机器定时器中断。
 */
void OSAL_TIMER_TICKSTOP(void)
{
    eclic_disable_interrupt(IRQ_M_TIMER);
}

/*
 * @brief Tickless 空闲: 把下一次比较值设为 timeout_ms 毫秒后,
 *        唤醒后由 osal_timer 补偿时间。
 */
void OSAL_TIMER_ONESHOT(uint16 timeout_ms)
{
    uint64_t delta = ((uint64_t)TIMER_FREQ * timeout_ms) / 1000UL;

    /* 最少 1 个 mtime tick, 防止立即中断 */
    if(delta == 0)
    {
        delta = 1;
    }
    oneshot_armed = 1;
    timer_schedule(delta);
}

/*
 * @brief 从一次性定时恢复为周期 tick (与 TICKINIT 的装载相同)。
 */
void OSAL_TIMER_TICKRESTORE(void)
{
    oneshot_armed = 0;
    timer_schedule((uint64_t)TIMER_FREQ / HAL_HEARTBEAT_HZ);
}
