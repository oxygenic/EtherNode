/*
 * hardware.h
 *
 *  Created on: Jun 30, 2025
 *      Author: universe
 */

#ifndef INC_HARDWARE_H_
#define INC_HARDWARE_H_

#include "main.h"

#define ANA_CALIB_FACTOR 5000.0F
#define UART_RECV_BUFFER 1000

extern int32_t hw_calibrate_analogue(const int32_t step,const char *cmd);
extern int32_t hw_get_analogue(const int8_t port);
extern void hw_gpio_set(const uint_fast8_t);
extern uint_fast8_t hw_gpio_get(void);
extern HAL_StatusTypeDef hw_pwm_set(const uint8_t out,const uint32_t freq, const uint32_t pulse);
extern void hw_init_digi_out_0(void);
extern void hw_init_digi_out_1_6_7(void);
extern uint8_t hw_write_config(void);


#endif /* INC_HARDWARE_H_ */
