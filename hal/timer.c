/**
 * @file timer.c
 * @brief 硬件定时器实现（GD32E230 适配）：使用 Cortex-M23 内核 SysTick
 *        提供系统滴答时钟，SysTick 中断中调用 osal_update_timers()
 * @version 0.1
 * @date 2026-09-02
 */

#include <stdint.h>

#include "timer.h"
#include "osal_timer.h"

/*
 * SysTick 寄存器（Cortex-M23 地址 0xE000E010）。
 * 本移植直接操作标准内核寄存器，不依赖厂商固件库，
 * 可方便地嵌入 GD32E230 的任意工程环境。
 */
typedef struct
{
    volatile uint32_t CSR;      /* 0xE000E010 控制和状态寄存器 */
    volatile uint32_t RVR;      /* 0xE000E014 重装载值寄存器      */
    volatile uint32_t CVR;      /* 0xE000E018 当前值寄存器      */
    volatile uint32_t CALIB;    /* 0xE000E01C 校准值寄存器      */
} systick_reg_t;

#define HAL_SYSTICK_BASE         (0xE000E010UL)
#define HAL_SYSTICK              ((systick_reg_t *)HAL_SYSTICK_BASE)

/* SYSTICK->CSR 控制位 */
#define SYSTICK_CSR_COUNTFLAG    ((uint32_t)0x00010000UL) /* 读取后清零计数标志 */
#define SYSTICK_CSR_CLKSOURCE   ((uint32_t)0x00000004UL) /* 时钟源：内核时钟       */
#define SYSTICK_CSR_TICKINT     ((uint32_t)0x00000002UL) /* SysTick 异常使能      */
#define SYSTICK_CSR_ENABLE      ((uint32_t)0x00000001UL) /* SysTick 使能          */

/*
 * 系统主频通常由 GD32E230 的 system 文件初始化，该符号由固件库提供。
 * 若工程中未定义，可在此直接指定实际主频，例如 72MHz：#define SystemCoreClock 72000000UL
 */
extern uint32_t SystemCoreClock;

/* 心跳频率：HAL_HEARTBEAT_HZ 次/秒，与 hal/timer.h 中 TICK_PERIOD_MS 保持一致 */
#define HAL_HEARTBEAT_HZ   ((uint32_t)(1000UL / TICK_PERIOD_MS))

/**
 * @brief SysTick 中断服务函数，作为 OSAL 的心跳时钟源。
 *        每个心跳周期调用一次，更新 OSAL 软件定时器。
 */
void SysTick_Handler(void)
{
    osal_update_timers();
}

/**
 * @brief 硬件定时器初始化。配置 SysTick 重装载值，
 *        使能中断以便在每个 TICK_PERIOD_MS 周期触发一次心跳。
 */
void OSAL_TIMER_TICKINIT(void)
{
    uint32_t reload;

    reload = SystemCoreClock / HAL_HEARTBEAT_HZ - 1UL;

    HAL_SYSTICK->RVR = reload;
    HAL_SYSTICK->CVR = 0u;

    /* 使用内核时钟源，使能 SysTick 异常并使能定时器 */
    HAL_SYSTICK->CSR = SYSTICK_CSR_ENABLE | SYSTICK_CSR_TICKINT | SYSTICK_CSR_CLKSOURCE;
}

/**
 * @brief 开启硬件定时器。OSAL 有定时任务在运行时才需要开启，
 *        为简单实现此处保持打开。
 */
void OSAL_TIMER_TICKSTART(void)
{
    HAL_SYSTICK->CSR |= SYSTICK_CSR_ENABLE;
}

/**
 * @brief 停止硬件定时器。为空则一直不停止。
 */
void OSAL_TIMER_TICKSTOP(void)
{
    HAL_SYSTICK->CSR &= ~SYSTICK_CSR_ENABLE;
}
