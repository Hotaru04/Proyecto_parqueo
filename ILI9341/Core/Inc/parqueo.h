#ifndef INC_PARQUEO_H_
#define INC_PARQUEO_H_

#include <stdint.h>

/* Espacios numerados del 1 al 8.
 * modo_demo = 1: carros entran y salen automaticamente.
 * modo_demo = 0: estado controlado desde sensores (Parqueo_SetOcupado).
 */
void Parqueo_Init(uint8_t modo_demo);
void Parqueo_Update(void);
void Parqueo_SetOcupado(uint8_t espacio, uint8_t ocupado);
uint8_t Parqueo_GetOcupado(uint8_t espacio);
uint8_t Parqueo_GetLibres(void);

#endif /* INC_PARQUEO_H_ */
