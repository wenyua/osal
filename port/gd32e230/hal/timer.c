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

/*
 * @brief SysTick 中断服务函数, 作为 OSAL 心跳源。
 *        每个心跳周期调用 osal_update_timers() 更新软件定时器。
 */
void SysTick_Handler(void)
{
    osal_update_timers();
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