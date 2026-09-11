/**
 * @file osal_hal.h
 * @brief OSAL 移植层接口实现 (GD32VF103 / Nuclei Bumblebee RISC-V 核)
 *
 * OSAL 核心层(type.h)只依赖本文件提供的接口, 移植到其他 MCU 时
 * 只需在新目标的 hal/ 目录下提供一份同名的 osal_hal.h:
 *   - HAL_DISABLE_INTERRUPTS() / HAL_ENABLE_INTERRUPTS()   全局中断开关
 *   - HAL_ENTER_CRITICAL_SECTION() / HAL_EXIT_CRITICAL_SECTION() 临界区
 *   - OSAL_IDLE_SLEEP()                                    空闲低功耗(WFI)
 *
 * RISC-V 实现: mstatus.MIE(bit3) 控制全局中断; wfi 进入睡眠。
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
