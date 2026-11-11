
/*
 * NeoPixel WS2812B con TIM4_CH2 (PB7) + DMA.
 *
 * STM32F446RE:
 * TIM4 = 84 MHz
 * Prescaler = 0
 * ARR = 104
 * PWM = 800 kHz
 *
 * DMA:
 * DMA1 Stream3, Channel 2
 * Memory/Peripheral = HALFWORD
 * Mode = NORMAL
 *
 * 4 NeoPixel en orden GRB.
 */

#include "neopixel.h"
#include "main.h"

extern TIM_HandleTypeDef htim4;

#define NP_T0H            34U
#define NP_T1H            67U

#define NP_BITS_LED       24U

// Periodos en bajo antes de transmitir
#define NP_START_SLOTS    256U

// Periodos en bajo despues de transmitir
#define NP_RESET_SLOTS    256U

#define NP_RED            50U
#define NP_GREEN          50U

#define NP_BUFFER_SIZE (NP_START_SLOTS + \
                        NEOPIXEL_COUNT * NP_BITS_LED + \
                        NP_RESET_SLOTS)

// Estado de los 4 parqueos
static uint8_t np_ocupado[NEOPIXEL_COUNT];

// Buffer PWM para DMA
static uint16_t np_pwm[NP_BUFFER_SIZE];

// Variables de control
static volatile uint8_t np_busy = 0;
static volatile uint8_t np_dirty = 1;


// Convertir un byte en 8 pulsos PWM
static void np_agregar_byte(uint16_t *salida, uint8_t valor)
{
    for (uint8_t bit = 0; bit < 8U; bit++)
    {
        if (valor & (0x80U >> bit))
        {
            salida[bit] = NP_T1H;
        }
        else
        {
            salida[bit] = NP_T0H;
        }
    }
}


// Inicializar NeoPixel
void NeoPixel_Init(void)
{
    for (uint8_t i = 0; i < NEOPIXEL_COUNT; i++)
    {
        np_ocupado[i] = 0U;
    }

    np_busy = 0U;
    np_dirty = 1U;

    // Dejar la salida PWM en bajo
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0U);
    __HAL_TIM_SET_COUNTER(&htim4, 0U);
}


// Cambiar estado de un parqueo
void NeoPixel_SetEstado(uint8_t espacio, uint8_t ocupado)
{
    if (espacio < 1U || espacio > NEOPIXEL_COUNT)
    {
        return;
    }

    uint8_t indice = espacio - 1U;
    uint8_t nuevo = ocupado ? 1U : 0U;

    if (np_ocupado[indice] != nuevo)
    {
        np_ocupado[indice] = nuevo;
        np_dirty = 1U;
    }
}


// Actualizar tira de NeoPixel
void NeoPixel_Update(void)
{
    if (np_busy || !np_dirty)
    {
        return;
    }

    uint16_t posicion = 0U;

    // 1. Periodos iniciales en bajo
    // Permiten estabilizar el inicio de la trama
    for (uint16_t i = 0; i < NP_START_SLOTS; i++)
    {
        np_pwm[posicion++] = 0U;
    }

    // 2. Datos de los cuatro NeoPixel
    for (uint8_t i = 0; i < NEOPIXEL_COUNT; i++)
    {
        uint8_t g = np_ocupado[i] ? 0U : NP_GREEN;
        uint8_t r = np_ocupado[i] ? NP_RED : 0U;
        uint8_t b = 0U;

        // Orden GRB del WS2812B
        np_agregar_byte(&np_pwm[posicion], g);
        posicion += 8U;

        np_agregar_byte(&np_pwm[posicion], r);
        posicion += 8U;

        np_agregar_byte(&np_pwm[posicion], b);
        posicion += 8U;
    }

    // 3. Periodos finales en bajo
    for (uint16_t i = 0; i < NP_RESET_SLOTS; i++)
    {
        np_pwm[posicion++] = 0U;
    }

    // 4. Preparar temporizador
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0U);
    __HAL_TIM_SET_COUNTER(&htim4, 0U);

    np_busy = 1U;
    np_dirty = 0U;

    // 5. Iniciar transmision por DMA
    if (HAL_TIM_PWM_Start_DMA(
            &htim4,
            TIM_CHANNEL_2,
            (uint32_t *)np_pwm,
            posicion) != HAL_OK)
    {
        np_busy = 0U;
        np_dirty = 1U;
    }
}


// Callback al finalizar la transmision
void HAL_TIM_PWM_PulseFinishedCallback(
    TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM4 &&
        htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
        HAL_TIM_PWM_Stop_DMA(htim, TIM_CHANNEL_2);

        __HAL_TIM_SET_COMPARE(
            htim,
            TIM_CHANNEL_2,
            0U
        );

        np_busy = 0U;
    }
}
