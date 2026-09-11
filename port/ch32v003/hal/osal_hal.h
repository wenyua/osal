/**
 * @file osal_hal.h
 * @brief OSAL 移植层接口实现 (CH32V003 / 青稞 V2A RISC-V 核)
 *
 * 全局中断控制(mstatus.MIE)与睡眠指令与 GD32VF103 完全通用,
 * 差异仅在心跳定时器(见 hal/timer.c 的 STK 实现)。
 * @date  2026-09-11
 */
#ifndef OSAL_HAL_H
#define OSAL_HAL_H

/* mstatus.MIE: 全局中断使能位 */
#define OSAL_MSTATUS_MIE        0x8

static __inline void __attribute__((always_inline)) OSAL_CLI(void)
{
    __asm volatile ("csrc csr_mstatus, %0" :: "r"(OSAL_MSTATUS_MIE) : "memory");
}
static __inline void __attribute__((always_inline)) OSAL_SEI(void)
{
    __asm volatile ("csrs csr_mstatus, %0" :: "r"(OSAL_MSTATUS_MIE) : "memory");
}

#define HAL_DISABLE_INTERRUPTS()        OSAL_CLI()      /* 关全局中断      */
#define HAL_ENABLE_INTERRUPTS()         OSAL_SEI()      /* 开全局中断      */

#define HAL_ENTER_CRITICAL_SECTION()    OSAL_CLI()      /* 进入临界区      */
#define HAL_EXIT_CRITICAL_SECTION()     OSAL_SEI()      /* 退出临界区      */

/* 空闲低功耗: RISC-V wfi, 任意中断可唤醒 */
#define OSAL_IDLE_SLEEP()               __asm volatile ("wfi" ::: "memory")

#endif /* OSAL_HAL_H */
