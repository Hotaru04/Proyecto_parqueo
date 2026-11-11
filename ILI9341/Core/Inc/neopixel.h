#ifndef INC_NEOPIXEL_H_
#define INC_NEOPIXEL_H_

#include <stdint.h>

/* Primera etapa: P1-P4. Para los 8 espacios, cambiar 4U por 8U
   y conectar los 8 WS2812B en la misma cadena de datos. */
#define NEOPIXEL_COUNT 4U

/* Llamar una sola vez, despues de MX_TIM4_Init(). */
void NeoPixel_Init(void);

/* Numero de espacio: 1..NEOPIXEL_COUNT.
   ocupado = 1 -> rojo, ocupado = 0 -> verde. */
void NeoPixel_SetEstado(uint8_t espacio, uint8_t ocupado);

/* Llamar en el while(1); transmite solo cuando hay cambios. */
void NeoPixel_Update(void);

#endif /* INC_NEOPIXEL_H_ */
