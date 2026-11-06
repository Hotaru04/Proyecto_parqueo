#ifndef INC_MPR121_H_
#define INC_MPR121_H_

#include "stm32f4xx_hal.h"

/* Usa los electrodos ELE0, ELE1, ELE2 y ELE3. */
HAL_StatusTypeDef MPR121_Init(I2C_HandleTypeDef *hi2c);
/* Devuelve un byte con los estados de P1-P4 en los bits 0-3. */
HAL_StatusTypeDef MPR121_Leer4(I2C_HandleTypeDef *hi2c, uint8_t *estados);

#endif /* INC_MPR121_H_ */
