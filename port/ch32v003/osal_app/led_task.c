/*
 * @file led_task.c
 * @brief LED 闪烁任务 (CH32V003, 板载 LED 假设位于 PD6/低电平点亮,
 *        请按实际板子修改 LED_PORT/LED_PIN 宏)
 * @date  2026-09-11
 */
#include "ch32v003_hw.h"
#include "task_event.h"

#define LED_BLINK_TIMEOUT       (500U / TICK_PERIOD_MS)   /* 500ms 翻转一次 */

#define LED_PORT                GPIOD_BASE
#define LED_PIN                 6
#define LED_RCC_EN              RCC_IOPDEN

/* LED 引脚配置为通用推挽输出: CFGR 中 MODE=11(输出50MHz), CNF=00(推挽) */
#define PIN_CFG(pin, cfg)       do { GPIO_CFGR(LED_PORT) = \
    (GPIO_CFGR(LED_PORT) & ~(0xFUL << ((pin) * 4))) | ((cfg) << ((pin) * 4)); } while (0)

uint8 led_task_id;

void board_led_init(void)
{
    RCC_APB2PCENR |= LED_RCC_EN;
    PIN_CFG(LED_PIN, 0x3);                      /* 推挽输出 */
    GPIO_BSR(LED_PORT) = 1UL << LED_PIN;        /* 低电平点亮, 先置高熄灭 */
}

void board_led_toggle(void)
{
    if(GPIO_OUTDR(LED_PORT) & (1UL << LED_PIN))
    {
        GPIO_BCR(LED_PORT) = 1UL << LED_PIN;    /* 拉低: 点亮 */
    }
    else
    {
        GPIO_BSR(LED_PORT) = 1UL << LED_PIN;    /* 拉高: 熄灭 */
    }
}

void led_task_init(uint8 task_id)
{
    led_task_id = task_id;
    osal_start_reload_timer(led_task_id, LED_BLINK_EVENT, LED_BLINK_TIMEOUT);
}

uint16 led_task_event_process(uint8 task_id, uint16 task_event)
{
    if(task_event & SYS_EVENT_MSG)
    {
        osal_sys_msg_t *msg_pkt;
        msg_pkt = (osal_sys_msg_t *)osal_msg_receive(task_id);

        while(msg_pkt)
        {
            osal_msg_deallocate((uint8 *)msg_pkt);
            msg_pkt = (osal_sys_msg_t *)osal_msg_receive(task_id);
        }
        return (task_event ^ SYS_EVENT_MSG);
    }

    if(task_event & LED_BLINK_EVENT)
    {
        board_led_toggle();
        return task_event ^ LED_BLINK_EVENT;
    }

    return 0;
}
