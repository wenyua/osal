/*
 * @file timer.c
 * @brief OSAL 硬件定时器实现(CH32V003): 使用内核 SysTick(STK) 提供滴答心跳
 *
 * STK 计数到 CMP 产生中断, 需要软件重装比较值(非自动重装)。
 * Tickless 一次性定时: 睡前把 CMP 推到 timeout 时刻并置 oneshot 标志,
 * 该次中断不调用 osal_update_timers(由 osal.c 主循环统一补偿), 避免双计。
 * @date  2026-09-11
 */
#include <stdint.h>

/* 注意: 芯片头(定义 ERROR/SUCCESS 枚举)需在 osal type.h 之前包含 */
#include "ch32v003_hw.h"

#include "timer.h"
#include "osal_timer.h"

/* 复位默认时钟为 HSI 24MHz; 若启用了 48MHz PLL 请同步修改 */
#define SYSTEM_CORE_CLOCK       24000000UL
#define STK_CLK_DIV             8UL             /* STK 时钟 = HCLK/8 */

#define HEARTBEAT_TICKS(sec_ms) ((uint32_t)((SYSTEM_CORE_CLOCK / STK_CLK_DIV) * (sec_ms) / 1000UL))

/* STK 寄存器 (CH32V003 内核外设) */
#define STK_CTLR                (*(volatile uint32_t *)0xE000F000UL)
#define STK_SR                  (*(volatile uint32_t *)0xE000F004UL)
#define STK_CNT                 (*(volatile uint32_t *)0xE000F008UL)
#define STK_CMP                 (*(volatile uint32_t *)0xE000F00CUL)

#define STK_STE                 (1UL << 0)      /* 使能计数        */
#define STK_STIE                (1UL << 1)      /* 比较中断使能    */
#define STK_STCLK               (1UL << 2)      /* 0=HCLK/8 时钟源 */
#define STK_CNTIF               (1UL << 0)      /* 比较中断标志    */

static uint32_t tick_period;            /* 一个周期 tick 的计数个数 */
static volatile uint8_t oneshot_armed;  /* 一次性定时期间不更新软件定时器 */

/*
 * @brief SysTick 中断服务函数: 重装比较值; 正常周期模式下更新软件定时器。
 */
void SysTick_Handler(void)
{
    STK_SR = 0;                                 /* 清中断标志 */

    if(oneshot_armed)
    {
        oneshot_armed = 0;                      /* Tickless 唤醒, 补偿由主循环完成 */
    }
    else
    {
        osal_update_timers();
    }
    STK_CMP = STK_CNT + tick_period;            /* 重装下一拍 */
}

/*
 * @brief 初始化 STK 心跳
 */
void OSAL_TIMER_TICKINIT(void)
{
    tick_period = HEARTBEAT_TICKS(TICK_PERIOD_MS);

    STK_CTLR = 0;
    STK_SR = 0;
    STK_CMP = STK_CNT + tick_period;
    STK_CTLR = STK_STE | STK_STIE | STK_STCLK;
}

/*
 * @brief 开启心跳(与 TICKINIT 同义, 接口兼容)
 */
void OSAL_TIMER_TICKSTART(void)
{
    STK_CTLR |= STK_STE | STK_STIE;
}

/*
 * @brief 停止心跳
 */
void OSAL_TIMER_TICKSTOP(void)
{
    STK_CTLR &= ~(STK_STE | STK_STIE);
}

/*
 * @brief Tickless 空闲: 把比较值推到 timeout_ms 毫秒后。
 *        该次中断不更新软件定时器, 由 osal.c 统一补偿。
 */
void OSAL_TIMER_ONESHOT(uint16 timeout_ms)
{
    uint32_t delta = HEARTBEAT_TICKS(1) * timeout_ms;

    if(delta == 0)
    {
        delta = 1;
    }
    oneshot_armed = 1;
    STK_CMP = STK_CNT + delta;
}

/*
 * @brief 恢复周期节拍
 */
void OSAL_TIMER_TICKRESTORE(void)
{
    oneshot_armed = 0;
    STK_CMP = STK_CNT + tick_period;
}
