#ifndef TYPE_H
#define TYPE_H

#include <stdint.h>

#define ZSUCCESS                  1
#define INVALID_TASK              2
#define INVALID_MSG_POINTER       3
#define INVALID_EVENT_ID          4
#define NO_TIMER_AVAIL            5
#define TASK_NO_TASK              6
#define MSG_BUFFER_NOT_AVAIL      7

typedef unsigned char       BOOL;

//芯片硬件字长, 由移植层按目标平台定义(典型 32 位 MCU 为 4 字节对齐)
typedef unsigned int        halDataAlign_t;

// Unsigned numbers
typedef uint8_t             uint8;
typedef uint8_t             byte;
typedef uint16_t            uint16;
typedef uint16_t            int16U;
typedef uint32_t            uint32;
typedef uint32_t            int32U;

// Signed numbers
typedef int8_t              int8;
typedef int16_t             int16;
typedef int32_t             int32;

#ifndef FALSE
#define FALSE       0
#endif

#ifndef TRUE
#define TRUE        1
#endif

#ifndef ARRAY_NULL
#define ARRAY_NULL  '\0'
#endif

#ifndef OPEN
#define OPEN        1
#endif

#ifndef CLOSE
#define CLOSE       0
#endif

#ifndef NULL
#define NULL        ((void*) 0)
#endif

#ifndef HIGH
#define HIGH        1
#endif

#ifndef LOW
#define LOW         0
#endif

#ifndef SUCCESS
#define SUCCESS     1
#endif

#ifndef ERROR
#define ERROR       0
#endif

/*
 * 本文件只定义 OSAL 核心层使用的类型与错误码, 不包含任何 MCU/编译器相关代码。
 * 与硬件平台相关的接口(中断控制/临界区/空闲低功耗)由移植层提供的
 * osal_hal.h 实现, 见各目标目录下的 hal/osal_hal.h:
 *   - HAL_DISABLE_INTERRUPTS() / HAL_ENABLE_INTERRUPTS()
 *   - HAL_ENTER_CRITICAL_SECTION() / HAL_EXIT_CRITICAL_SECTION()
 *   - OSAL_IDLE_SLEEP()          (空闲低功耗, osal.c 使用)
 */
#include "osal_hal.h"

#endif
