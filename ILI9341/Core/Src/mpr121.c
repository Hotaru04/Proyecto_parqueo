//
//#include "mpr121.h"
//
///* Dirección del MPR121 (ADDR conectado a GND) */
//#define MPR121_DIRECCION  (0x5AU << 1)
//#define MPR121_TIMEOUT_MS 100U
//
///* Escritura de un registro del MPR121 */
//static HAL_StatusTypeDef MPR121_Escribir(
//    I2C_HandleTypeDef *hi2c,
//    uint8_t registro,
//    uint8_t valor)
//{
//    return HAL_I2C_Mem_Write(
//        hi2c,
//        MPR121_DIRECCION,
//        registro,
//        I2C_MEMADD_SIZE_8BIT,
//        &valor,
//        1U,
//        MPR121_TIMEOUT_MS
//    );
//}
//
///* Inicialización del MPR121 */
//HAL_StatusTypeDef MPR121_Init(I2C_HandleTypeDef *hi2c)
//{
//    if (hi2c == NULL)
//        return HAL_ERROR;
//
//    HAL_Delay(50);
//
//    /* Comprobar comunicación I2C */
//    if (HAL_I2C_IsDeviceReady(
//            hi2c,
//            MPR121_DIRECCION,
//            3U,
//            MPR121_TIMEOUT_MS) != HAL_OK)
//    {
//        return HAL_ERROR;
//    }
//
//    /* Reiniciar el MPR121 */
//    if (MPR121_Escribir(hi2c, 0x80, 0x63) != HAL_OK)
//        return HAL_ERROR;
//
//    HAL_Delay(5);
//
//    /* Detener mediciones para configurar */
//    if (MPR121_Escribir(hi2c, 0x5E, 0x00) != HAL_OK)
//        return HAL_ERROR;
//
//    /* Configuración de filtros */
//    static const uint8_t filtros[][2] = {
//        {0x2B, 0x01},
//        {0x2C, 0x01},
//        {0x2D, 0x00},
//        {0x2E, 0x00},
//        {0x2F, 0x01},
//        {0x30, 0x01},
//        {0x31, 0xFF},
//        {0x32, 0x02}
//    };
//
//    for (uint8_t i = 0;
//         i < sizeof(filtros) / sizeof(filtros[0]);
//         i++)
//    {
//        if (MPR121_Escribir(
//                hi2c,
//                filtros[i][0],
//                filtros[i][1]) != HAL_OK)
//        {
//            return HAL_ERROR;
//        }
//    }
//
//    /* Configuración de umbrales */
//    for (uint8_t electrodo = 0; electrodo < 4U; electrodo++)
//    {
//        uint8_t registro = (uint8_t)(0x41U + 2U * electrodo);
//
//        /* Touch threshold = 12 */
//        if (MPR121_Escribir(
//                hi2c, registro, 0x0C) != HAL_OK)
//        {
//            return HAL_ERROR;
//        }
//
//        /* Release threshold = 6 */
//        if (MPR121_Escribir(
//                hi2c, (uint8_t)(registro + 1U), 0x06) != HAL_OK)
//        {
//            return HAL_ERROR;
//        }
//    }
//
//    /* Configuración AFE2 */
//    if (MPR121_Escribir(hi2c, 0x5D, 0x04) != HAL_OK)
//        return HAL_ERROR;
//
//    /* Habilitar ELE0, ELE1, ELE2 y ELE3 */
//    if (MPR121_Escribir(hi2c, 0x5E, 0x84) != HAL_OK)
//        return HAL_ERROR;
//
//    HAL_Delay(50);
//
//    return HAL_OK;
//}
//
///* Leer estados táctiles de los primeros cuatro electrodos */
//HAL_StatusTypeDef MPR121_Leer4(
//    I2C_HandleTypeDef *hi2c,
//    uint8_t *estados)
//{
//    if (hi2c == NULL || estados == NULL)
//        return HAL_ERROR;
//
//    uint8_t lectura = 0;
//
//    HAL_StatusTypeDef resultado = HAL_I2C_Mem_Read(
//        hi2c,
//        MPR121_DIRECCION,
//        0x00,
//        I2C_MEMADD_SIZE_8BIT,
//        &lectura,
//        1U,
//        MPR121_TIMEOUT_MS
//    );
//
//    if (resultado == HAL_OK)
//    {
//        *estados = lectura & 0x0FU;
//    }
//
//    return resultado;
//}


#include "mpr121.h"

/* Dirección del MPR121 (ADDR conectado a GND) */
#define MPR121_DIRECCION  (0x5AU << 1)
#define MPR121_TIMEOUT_MS 100U

/* Escritura de un registro del MPR121 */
static HAL_StatusTypeDef MPR121_Escribir(
    I2C_HandleTypeDef *hi2c,
    uint8_t registro,
    uint8_t valor)
{
    return HAL_I2C_Mem_Write(
        hi2c,
        MPR121_DIRECCION,
        registro,
        I2C_MEMADD_SIZE_8BIT,
        &valor,
        1U,
        MPR121_TIMEOUT_MS
    );
}

/* Inicialización del MPR121 */
HAL_StatusTypeDef MPR121_Init(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == NULL)
        return HAL_ERROR;

    HAL_Delay(50);

    /* Comprobar comunicación I2C */
    if (HAL_I2C_IsDeviceReady(
            hi2c,
            MPR121_DIRECCION,
            3U,
            MPR121_TIMEOUT_MS) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* Reiniciar el MPR121 */
    if (MPR121_Escribir(hi2c, 0x80, 0x63) != HAL_OK)
        return HAL_ERROR;

    HAL_Delay(5);

    /* Detener mediciones para configurar */
    if (MPR121_Escribir(hi2c, 0x5E, 0x00) != HAL_OK)
        return HAL_ERROR;

    /* Configuración de filtros */
    static const uint8_t filtros[][2] = {
        {0x2B, 0x01},
        {0x2C, 0x01},
        {0x2D, 0x00},
        {0x2E, 0x00},
        {0x2F, 0x01},
        {0x30, 0x01},
        {0x31, 0xFF},
        {0x32, 0x02}
    };

    for (uint8_t i = 0;
         i < sizeof(filtros) / sizeof(filtros[0]);
         i++)
    {
        if (MPR121_Escribir(
                hi2c,
                filtros[i][0],
                filtros[i][1]) != HAL_OK)
        {
            return HAL_ERROR;
        }
    }

    /* Configuración de umbrales para máxima sensibilidad */
    for (uint8_t electrodo = 0; electrodo < 4U; electrodo++)
    {
        uint8_t registro = (uint8_t)(0x41U + 2U * electrodo);

        /* Touch threshold = 2 (Extremadamente sensible) */
        if (MPR121_Escribir(
                hi2c, registro, 0x02) != HAL_OK)
        {
            return HAL_ERROR;
        }

        /* Release threshold = 1 */
        if (MPR121_Escribir(
                hi2c, (uint8_t)(registro + 1U), 0x01) != HAL_OK)
        {
            return HAL_ERROR;
        }
    }

    /* Configuración AFE1 (0x5C): FFI=00, CDC=0x01.
       Corriente de carga muy baja (1µA) */
    if (MPR121_Escribir(hi2c, 0x5C, 0x01) != HAL_OK)
        return HAL_ERROR;

    /* Configuración AFE2 (0x5D): CDT=2µs (0x44) para mayor tiempo de carga y sensibilidad */
    if (MPR121_Escribir(hi2c, 0x5D, 0x44) != HAL_OK)
        return HAL_ERROR;

    /* Habilitar ELE0, ELE1, ELE2 y ELE3 */
    if (MPR121_Escribir(hi2c, 0x5E, 0x84) != HAL_OK)
        return HAL_ERROR;

    HAL_Delay(50);

    return HAL_OK;
}

/* Leer estados táctiles de los primeros cuatro electrodos */
HAL_StatusTypeDef MPR121_Leer4(
    I2C_HandleTypeDef *hi2c,
    uint8_t *estados)
{
    if (hi2c == NULL || estados == NULL)
        return HAL_ERROR;

    uint8_t lectura = 0;

    HAL_StatusTypeDef resultado = HAL_I2C_Mem_Read(
        hi2c,
        MPR121_DIRECCION,
        0x00,
        I2C_MEMADD_SIZE_8BIT,
        &lectura,
        1U,
        MPR121_TIMEOUT_MS
    );

    if (resultado == HAL_OK)
    {
        *estados = lectura & 0x0FU;
    }

    return resultado;
}


