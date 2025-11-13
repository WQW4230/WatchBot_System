#ifndef _STM32_LINK_H_
#define _STM32_LINK_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UART_TX_PIN 40
#define UART_RX_PIN 41

void stm32_link_init(void);
void stm32_send_frame(void);
void stm32_receive_task(void);

#ifdef __cplusplus
}
#endif

#endif
