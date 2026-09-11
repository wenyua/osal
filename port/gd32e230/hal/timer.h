/*
 * @file timer.h
 * @brief OSAL 硬件定时器抽象(GD32E230 / Cortex-M23)
 * @date  2026-09-04
 */
#ifndef TIMER_H
#define TIMER_H

#include "type.h"

#define TICK_PERIOD_MS      10              /* 系统滴答周期 */

extern void OSAL_TIMER_TICKINIT(void);
extern void OSAL_TIMER_TICKSTART(void);
extern void OSAL_TIMER_TICKSTOP(void);
extern void OSAL_TIMER_ONESHOT(uint16 timeout_ms);
extern void OSAL_TIMER_TICKRESTORE(void);

#endif /* TIMER_H */