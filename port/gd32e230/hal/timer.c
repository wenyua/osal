/*
 * @file timer.c
 * @brief OSAL 硬件定时器实现(GD32E230): SysTick 提供滴答心跳
 * @date  2026-09-04
 */
#include <stdint.h>

#include "timer.h"
#include "osal_timer.h"
#include "gd32e23x.h"

#define HAL_HEARTBEAT_HZ   ((uint32_t)(1000UL / TICK_PERIOD_MS))

static volatile uint8_t oneshot_armed;  /* 一次性定时期间不更新软件定时器 */

/*
 * @brief SysTick 中断服务函数, 作为 OSAL 心跳源。
 *        每个心跳周期调用 osal_update_timers() 更新软件定时器。
 *        Tickless 一次性定时期间的唤醒中断不更新(由主循环统一补偿)。
 */
void SysTick_Handler(void)
{
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
 * @brief 初始化 SysTick: 使用 CMSIS SysTick_Config 配置重装载值,
 *        使能中断, 内核时钟源。
 */
void OSAL_TIMER_TICKINIT(void)
{
    (void)SysTick_Config(SystemCoreClock / HAL_HEARTBEAT_HZ);
}

/*
 * @brief Tickless 空闲: 将 SysTick 重配置为一次性定时,
 *        timeout_ms 毫秒后产生一次中断唤醒 CPU。
 */
void OSAL_TIMER_ONESHOT(uint16 timeout_ms)
{
    (void)SysTick_Config(SystemCoreClock / 1000UL * timeout_ms);
    oneshot_armed = 1;
}

/*
 * @brief 从一次性定时恢复为周期 tick (与 TICKINIT 相同)。
 */
void OSAL_TIMER_TICKRESTORE(void)
{
    oneshot_armed = 0;
    OSAL_TIMER_TICKINIT();
}

/*
 * @brief 开启硬件定时器。
 */
void OSAL_TIMER_TICKSTART(void)
{
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

/*
 * @brief 停止硬件定时器。
 */
void OSAL_TIMER_TICKSTOP(void)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}