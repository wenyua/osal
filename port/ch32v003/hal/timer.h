/**
 * @file timer.h
 * @brief OSAL 硬件定时器抽象 (CH32V003 / 青稞 V2A)
 * @date  2026-09-11
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
