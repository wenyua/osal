#include "osal.h"
#include "osal_event.h"
#include "osal_memory.h"
#include "osal_timer.h"

#include <string.h>

#define OSAL_IDLE_SLEEP_MIN_TICKS   2   /* 最近到期小于该值时不值得进入睡眠 */

osal_msg_q_t osal_qHead;

/*********************************************************************
 * @fn osal_init_system
 *
 * @brief
 *
 *   This function initializes the "task" system by creating the
 *   tasks defined in the task table (OSAL_Tasks.h).
 *
 * @param   void
 *
 * @return  ZSUCCESS
 */
uint8 osal_init_system(void)
{
    // Initialize the Memory Allocation System
    osal_mem_init();

    // Initialize the message queue
    osal_qHead = NULL;

#if defined( OSAL_TOTAL_MEM )
    osal_msg_cnt = 0;
#endif

    osalTimerInit();
    osal_init_TaskHead();

    return (ZSUCCESS);
}

/*********************************************************************
 * @fn osal_start_system
 *
 * @brief
 *
 *   This function is the main loop function of the task system.  It
 *   will look through all task events and call the task_event_processor()
 *   function for the task with the event.  If there are no events (for
 *   all tasks), this function puts the processor into Sleep.
 *   This Function doesn't return.
 *
 * @param   void
 *
 * @return  none
 */
void osal_start_system(void)
{
    uint16 events;
    uint16 retEvents;
    uint16 sleepTicks;

    while(1)
    {
        TaskActive = osalNextActiveTask();
        if(TaskActive == NULL)
        {
            /****************************************************
             * 空闲: Tickless 低功耗
             * - 最近定时器尚远: 重配 SysTick 为一次性定时,
             *   睡到下个定时器到期, 醒后一次性补偿软件定时器;
             * - 否则: 普通 WFI, 睡一个周期 tick。
             * 任意中断(按键EXTI/串口等)也可唤醒并走正常调度。
             ****************************************************/
            sleepTicks = osal_next_timeout();

            HAL_ENTER_CRITICAL_SECTION();
            if(sleepTicks > OSAL_IDLE_SLEEP_MIN_TICKS)
            {
                OSAL_TIMER_ONESHOT(sleepTicks * TICK_PERIOD_MS);
                HAL_EXIT_CRITICAL_SECTION();

                OSAL_IDLE_SLEEP();   /* 睡眠, 由一次性 tick 中断或其他中断唤醒 */

                /* 醒来后一次性补偿睡掉的 tick, 触发到期定时器 */
                HAL_ENTER_CRITICAL_SECTION();
                osalTimerUpdate(sleepTicks);
                OSAL_TIMER_TICKRESTORE();
                HAL_EXIT_CRITICAL_SECTION();
            }
            else
            {
                HAL_EXIT_CRITICAL_SECTION();
                OSAL_IDLE_SLEEP();
            }
        }
        else
        {
            HAL_ENTER_CRITICAL_SECTION();
            events = TaskActive->events;
            // Clear the Events for this task
            TaskActive->events = 0;
            HAL_EXIT_CRITICAL_SECTION();

            if(events != 0)
            {
                // Call the task to process the event(s)
                if(TaskActive->pfnEventProcessor)
                {
                    retEvents = (TaskActive->pfnEventProcessor)(TaskActive->taskID, events);

                    // Add back unprocessed events to the current task
                    HAL_ENTER_CRITICAL_SECTION();
                    TaskActive->events |= retEvents;
                    HAL_EXIT_CRITICAL_SECTION();
                }
            }
        }
    }
}

/*********************************************************************
 * @fn osal_strlen
 *
 * @brief
 *
 *   Calculates the length of a string.  The string must be null
 *   terminated.
 *
 * @param   char *pString - pointer to text string
 *
 * @return  int - number of characters
 */
int osal_strlen(char *pString)
{
    return (int)(strlen(pString));
}

/*********************************************************************
 * @fn osal_memcpy
 *
 * @brief
 *
 *   Generic memory copy.
 *
 *   Note: This function differs from the standard memcpy(), since
 *         it returns the pointer to the next destination uint8. The
 *         standard memcpy() returns the original destination address.
 *
 * @param   dst - destination address
 * @param   src - source address
 * @param   len - number of bytes to copy
 *
 * @return  pointer to end of destination buffer
 */
void *osal_memcpy(void *dst, const void *src, unsigned int len)
{
    uint8 *pDst;
    const uint8 *pSrc;

    pSrc = src;
    pDst = dst;

    while(len--)
        *pDst++ = *pSrc++;

    return (pDst);
}

/*********************************************************************
 * @fn osal_revmemcpy
 *
 * @brief   Generic reverse memory copy.  Starts at the end of the
 *   source buffer, by taking the source address pointer and moving
 *   pointer ahead "len" bytes, then decrementing the pointer.
 *
 *   Note: This function differs from the standard memcpy(), since
 *         it returns the pointer to the next destination uint8. The
 *         standard memcpy() returns the original destination address.
 *
 * @param   dst - destination address
 * @param   src - source address
 * @param   len - number of bytes to copy
 *
 * @return  pointer to end of destination buffer
 */
void *osal_revmemcpy(void *dst, const void *src, unsigned int len)
{
    uint8 *pDst;
    const uint8 *pSrc;

    pSrc = src;
    pSrc += (len - 1);
    pDst = dst;

    while(len--)
        *pDst++ = *pSrc--;

    return (pDst);
}

/*********************************************************************
 * @fn osal_memdup
 *
 * @brief   Allocates a buffer [with osal_mem_alloc()] and copies
 *          the src buffer into the newly allocated space.
 *
 * @param   src - source address
 * @param   len - number of bytes to copy
 *
 * @return  pointer to the new allocated buffer, or NULL if
 *          allocation problem.
 */
void *osal_memdup(const void *src, unsigned int len)
{
    uint8 *pDst;

    pDst = osal_mem_alloc(len);
    if(pDst)
    {
        osal_memcpy(pDst, src, len);
    }

    return ((void *)pDst);
}

/*********************************************************************
 * @fn osal_memcmp
 *
 * @brief
 *
 *   Generic memory compare.
 *
 * @param   src1 - source 1 addrexx
 * @param   src2 - source 2 address
 * @param   len - number of bytes to compare
 *
 * @return  TRUE - same, FALSE - different
 */
uint8 osal_memcmp(const void *src1, const void *src2, unsigned int len)
{
    const uint8 *pSrc1;
    const uint8 *pSrc2;

    pSrc1 = src1;
    pSrc2 = src2;

    while(len--)
    {
        if(*pSrc1++ != *pSrc2++)
            return FALSE;
    }
    return TRUE;
}

/*********************************************************************
 * @fn osal_memset
 *
 * @brief
 *
 *   Set memory buffer to value.
 *
 * @param   dest - pointer to buffer
 * @param   value - what to set each uint8 of the message
 * @param   size - how big
 *
 * @return  pointer to destination buffer
 */
void *osal_memset(void *dest, uint8 value, int len)
{
    return memset(dest, value, len);
}
