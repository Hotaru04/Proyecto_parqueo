/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "neopixel.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* --- HC-SR04 --- */
#define TRIG1_PORT  GPIOC
#define TRIG1_PIN   GPIO_PIN_2
#define ECHO1_PORT  GPIOC
#define ECHO1_PIN   GPIO_PIN_3

#define TRIG2_PORT  GPIOB
#define TRIG2_PIN   GPIO_PIN_2
#define ECHO2_PORT  GPIOB
#define ECHO2_PIN   GPIO_PIN_10

/* --- Sensores digitales --- */
#define HW488_PORT  GPIOC
#define HW488_PIN   GPIO_PIN_0
#define IR_PORT     GPIOC
#define IR_PIN      GPIO_PIN_1

/* --- 7-segmentos (catodo comun) --- */
#define SEG_A_PORT  GPIOA
#define SEG_A_PIN   GPIO_PIN_0
#define SEG_B_PORT  GPIOA
#define SEG_B_PIN   GPIO_PIN_1
#define SEG_C_PORT  GPIOA
#define SEG_C_PIN   GPIO_PIN_4
#define SEG_D_PORT  GPIOA
#define SEG_D_PIN   GPIO_PIN_6
#define SEG_E_PORT  GPIOA
#define SEG_E_PIN   GPIO_PIN_7
#define SEG_F_PORT  GPIOA
#define SEG_F_PIN   GPIO_PIN_8
#define SEG_G_PORT  GPIOA
#define SEG_G_PIN   GPIO_PIN_9

/* --- Umbrales --- */
#define DISTANCIA_UMBRAL_CM  30    // Menos de 30cm = carro detectado
#define HC_SR04_TIMEOUT_US   30000 // Timeout de medicion (30ms)

#define DISTANCIA_UMBRAL_CM  5     // Menos o igual a 5cm = carro detectado
#define HC_SR04_TIMEOUT_US   30000 // Timeout de medicion (30ms)

/* --- Servos (TIM3 a 50Hz, 1 count = 1us) --- */
#define SERVO_CERRADO  1000   // 1ms = ~0 grados (barrera abajo)
#define SERVO_ABIERTO  1500   // 1.5ms = ~90 grados (barrera arriba)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
DMA_HandleTypeDef hdma_tim4_ch3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
volatile uint8_t grupoA = 0x00;    // Mis 4 sensores (bit0=P1..bit3=P4)
volatile uint8_t grupoB = 0x00;    // 4 sensores del companero (recibidos por I2C)
volatile uint8_t i2cTxByte = 0x00; // Byte a transmitir cuando el master lee
volatile uint8_t i2cRxByte = 0x00; // Byte recibido cuando el master escribe
char debugMsg[100];

/* --- Distancias HC-SR04 (para debug, se llenan en leerSensores) --- */
uint32_t debug_dist1 = 0;
uint32_t debug_dist2 = 0;

/* --- Servos (comandos recibidos por I2C, bits 4-5) --- */
volatile uint8_t servo1_cmd = 0;   // 0=cerrado(0°), 1=abierto(90°)
volatile uint8_t servo2_cmd = 0;
uint8_t servo_toggle = 0;         // Estado toggle del boton B1
uint8_t btn_prev = 1;             // B1 previo (1=suelto, activo LOW)

/* Tabla 7-segmentos: digitos 0-8 (catodo comun)
 * Formato bits: 0gfedcba
 *   0 = 0x3F   1 = 0x06   2 = 0x5B   3 = 0x4F
 *   4 = 0x66   5 = 0x6D   6 = 0x7D   7 = 0x07
 *   8 = 0x7F
 */
const uint8_t tabla7seg[9] = {
    0x3F,  // 0
    0x06,  // 1
    0x5B,  // 2
    0x4F,  // 3
    0x66,  // 4
    0x6D,  // 5
    0x7D,  // 6
    0x07,  // 7
    0x7F   // 8
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
void delayUs(uint32_t us);
uint32_t medirDistanciaCm(GPIO_TypeDef *trigPort, uint16_t trigPin,
                          GPIO_TypeDef *echoPort, uint16_t echoPin);
uint8_t leerSensores(void);
void mostrar7seg(uint8_t digito);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  // Arrancar timer de microsegundos
  HAL_TIM_Base_Start(&htim2);

  // Activar I2C en modo Listen (detecta direccion de transferencia)
  HAL_I2C_EnableListen_IT(&hi2c1);

  // Debug
  sprintf(debugMsg, "STM32 Parqueo - I2C 0x08 Listen - 8 espacios\r\n");
  HAL_UART_Transmit(&huart2, (uint8_t*)debugMsg, strlen(debugMsg), 100);

  // Mostrar 8 (todos libres) al inicio
  mostrar7seg(8);

  NeoPixel_Init();
  NeoPixel_Update();  // Test: enviar 8 LEDs verdes al arranque
  HAL_Delay(10);      // Esperar DMA (~1ms real, 10ms de margen)
  sprintf(debugMsg, "NeoPixel test: 8 verdes enviados por DMA\r\n");
  HAL_UART_Transmit(&huart2, (uint8_t*)debugMsg, strlen(debugMsg), 100);

  // Arrancar PWM de servos (posicion cerrada por defecto)
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);  // Servo 1 en PB4
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);  // Servo 2 en PB5
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	    // 1. Leer mis 4 sensores
	    grupoA = leerSensores();
	    i2cTxByte = grupoA;  // Preparar para cuando el master lea

	    // 2. Contar espacios libres TOTALES (mis 4 + 4 del companero)
	    uint8_t ocupados = 0;
	    for (int i = 0; i < 4; i++) {
	        if (grupoA & (1 << i)) ocupados++;
	        if (grupoB & (1 << i)) ocupados++;
	    }

	    // DEBUG: ver distancias crudas
	    uint32_t d1 = medirDistanciaCm(TRIG1_PORT, TRIG1_PIN, ECHO1_PORT, ECHO1_PIN);
	    uint32_t d2 = medirDistanciaCm(TRIG2_PORT, TRIG2_PIN, ECHO2_PORT, ECHO2_PIN);
	    sprintf(debugMsg, "D1:%lucm D2:%lucm\r\n", d1, d2);
	    // DEBUG: ver distancias (ya medidas en leerSensores, sin repetir)
	    sprintf(debugMsg, "D1:%lucm D2:%lucm\r\n", debug_dist1, debug_dist2);
	    HAL_UART_Transmit(&huart2, (uint8_t*)debugMsg, strlen(debugMsg), 100);

	    uint8_t libres = 8 - ocupados;

	    // 3. Actualizar display 7-segmentos (muestra 0-8)
	    mostrar7seg(libres);

	    // 4. Debug por UART
	    sprintf(debugMsg, "A:0b%c%c%c%c B:0b%c%c%c%c Libres:%d/8\r\n",
	            (grupoA & 0x08) ? '1' : '0',
	            (grupoA & 0x04) ? '1' : '0',
	            (grupoA & 0x02) ? '1' : '0',
	            (grupoA & 0x01) ? '1' : '0',
	            (grupoB & 0x08) ? '1' : '0',
	            (grupoB & 0x04) ? '1' : '0',
	            (grupoB & 0x02) ? '1' : '0',
	            (grupoB & 0x01) ? '1' : '0',
	            libres);
	    HAL_UART_Transmit(&huart2, (uint8_t*)debugMsg, strlen(debugMsg), 100);

	    // 5. Actualizar NeoPixels segun estado de parqueos
	    // Mapeo fisico: Sensor1->NP3, Sensor2->NP4, Sensor3->NP1, Sensor4->NP2
	    NeoPixel_SetEstado(1, (grupoA >> 2) & 1);  // NP1 <- Sensor 3 (bit 2)
	    NeoPixel_SetEstado(2, (grupoA >> 3) & 1);  // NP2 <- Sensor 4 (bit 3)
	    NeoPixel_SetEstado(3, (grupoA >> 0) & 1);  // NP3 <- Sensor 1 (bit 0)
	    NeoPixel_SetEstado(4, (grupoA >> 1) & 1);  // NP4 <- Sensor 2 (bit 1)
	    for (int i = 0; i < 4; i++) {
	        NeoPixel_SetEstado(i + 5, (grupoB >> i) & 1);  // espacios 5-8
	    }
	    NeoPixel_Update();

	    // 6. Actualizar servos segun comandos I2C (bits 4-5)
	    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1,
	        servo1_cmd ? SERVO_ABIERTO : SERVO_CERRADO);
	    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2,
	        servo2_cmd ? SERVO_ABIERTO : SERVO_CERRADO);

	    // 7. Polling del boton B1 (PC13) para toggle de servos
	    uint8_t btn_now = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
	    if (btn_prev == 1 && btn_now == 0) {
	        // Flanco de bajada detectado: toggle ambos servos
	        servo_toggle = !servo_toggle;
	        servo1_cmd = servo_toggle;
	        servo2_cmd = servo_toggle;
	        sprintf(debugMsg, "BTN: Servos -> %s\r\n",
	                servo_toggle ? "ABIERTO(90)" : "CERRADO(0)");
	        HAL_UART_Transmit(&huart2, (uint8_t*)debugMsg, strlen(debugMsg), 100);
	        // Actualizar servos de inmediato (no esperar al paso 6)
	        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1,
	            servo1_cmd ? SERVO_ABIERTO : SERVO_CERRADO);
	        __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2,
	            servo2_cmd ? SERVO_ABIERTO : SERVO_CERRADO);
	    }
	    btn_prev = btn_now;

	    HAL_Delay(500);

  /* USER CODE END 3 */
}
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 16;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 83;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0xFFFFFFFF;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 83;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 19999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 1000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 104;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream7_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|LD2_Pin
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PC0 PC1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PC2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PC3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA4 LD2_Pin
                           PA6 PA7 PA8 PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|LD2_Pin
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */


/* --- Delay en microsegundos usando TIM2 --- */
void delayUs(uint32_t us) {
    uint32_t start = __HAL_TIM_GET_COUNTER(&htim2);
    while ((__HAL_TIM_GET_COUNTER(&htim2) - start) < us);
}

/* --- Medir distancia con HC-SR04 (retorna cm, 999 si timeout) --- */
uint32_t medirDistanciaCm(GPIO_TypeDef *trigPort, uint16_t trigPin,
                          GPIO_TypeDef *echoPort, uint16_t echoPin)
{
    uint32_t start, elapsed;

    // Pulso TRIG de 10us
    HAL_GPIO_WritePin(trigPort, trigPin, GPIO_PIN_RESET);
    delayUs(2);
    HAL_GPIO_WritePin(trigPort, trigPin, GPIO_PIN_SET);
    delayUs(10);
    HAL_GPIO_WritePin(trigPort, trigPin, GPIO_PIN_RESET);

    // Esperar ECHO suba (con timeout)
    start = __HAL_TIM_GET_COUNTER(&htim2);
    while (HAL_GPIO_ReadPin(echoPort, echoPin) == GPIO_PIN_RESET) {
        if ((__HAL_TIM_GET_COUNTER(&htim2) - start) > HC_SR04_TIMEOUT_US)
            return 999;
    }

    // Medir duracion del pulso ECHO
    start = __HAL_TIM_GET_COUNTER(&htim2);
    while (HAL_GPIO_ReadPin(echoPort, echoPin) == GPIO_PIN_SET) {
        if ((__HAL_TIM_GET_COUNTER(&htim2) - start) > HC_SR04_TIMEOUT_US)
            return 999;
    }
    elapsed = __HAL_TIM_GET_COUNTER(&htim2) - start;

    // Distancia = tiempo_us / 58
    return elapsed / 58;
}

/* --- Leer los 4 sensores, retorna 4 bits --- */
uint8_t leerSensores(void) {
    uint8_t estado = 0x00;

    // Sensor 1: HC-SR04 #1
    uint32_t dist1 = medirDistanciaCm(TRIG1_PORT, TRIG1_PIN, ECHO1_PORT, ECHO1_PIN);
    if (dist1 < DISTANCIA_UMBRAL_CM)
        estado |= 0x01;  // bit 0

    // Sensor 2: HC-SR04 #2
    uint32_t dist2 = medirDistanciaCm(TRIG2_PORT, TRIG2_PIN, ECHO2_PORT, ECHO2_PIN);
    if (dist2 < DISTANCIA_UMBRAL_CM)
        estado |= 0x02;  // bit 1

    // Sensor 3: HW-488 (activo LOW = detectado)
    if (HAL_GPIO_ReadPin(HW488_PORT, HW488_PIN) == GPIO_PIN_RESET)
    debug_dist1 = medirDistanciaCm(TRIG1_PORT, TRIG1_PIN, ECHO1_PORT, ECHO1_PIN);
    if (debug_dist1 <= DISTANCIA_UMBRAL_CM)
        estado |= 0x01;  // bit 0

    // Sensor 2: HC-SR04 #2
    debug_dist2 = medirDistanciaCm(TRIG2_PORT, TRIG2_PIN, ECHO2_PORT, ECHO2_PIN);
    if (debug_dist2 <= DISTANCIA_UMBRAL_CM)
        estado |= 0x02;  // bit 1

    // Sensor 3: IR tipo LM393 (activo HIGH = detectado)
    if (HAL_GPIO_ReadPin(HW488_PORT, HW488_PIN) == GPIO_PIN_SET)
        estado |= 0x04;  // bit 2

    // Sensor 4: IR/LM393 (activo LOW = detectado)
    if (HAL_GPIO_ReadPin(IR_PORT, IR_PIN) == GPIO_PIN_SET)
        estado |= 0x08;  // bit 3

    return estado;
}

/* --- Mostrar digito 0-8 en display 7-segmentos --- */
void mostrar7seg(uint8_t digito) {
    if (digito > 8) digito = 8;

    uint8_t seg = ~tabla7seg[digito] & 0x7F;  // invertir los 7 bits

    HAL_GPIO_WritePin(SEG_A_PORT, SEG_A_PIN, (seg & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(SEG_A_PORT, SEG_A_PIN, (seg & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SEG_B_PORT, SEG_B_PIN, (seg & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SEG_C_PORT, SEG_C_PIN, (seg & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SEG_D_PORT, SEG_D_PIN, (seg & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SEG_E_PORT, SEG_E_PIN, (seg & 0x10) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SEG_F_PORT, SEG_F_PIN, (seg & 0x20) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SEG_G_PORT, SEG_G_PIN, (seg & 0x40) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}


/* =====================================================
 * I2C LISTEN MODE CALLBACKS
 * El ESP32 master hace dos cosas:
 *   1. Lee de nosotros -> le mandamos grupoA (nuestros 4 sensores)
 *   2. Nos escribe     -> recibimos grupoB (4 sensores del companero)
 * ===================================================== */

/* Llamado cuando el master nos direcciona (inicio de transferencia) */
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c,
                          uint8_t TransferDirection,
                          uint16_t AddrMatchCode)
{
    if (hi2c->Instance == I2C1)
    {
        if (TransferDirection == I2C_DIRECTION_RECEIVE)
        {
            // El master quiere LEER de nosotros -> transmitir grupoA
            i2cTxByte = grupoA;
            HAL_I2C_Slave_Seq_Transmit_IT(hi2c, (uint8_t*)&i2cTxByte, 1,
                                          I2C_FIRST_AND_LAST_FRAME);
        }
        else
        {
            // El master quiere ESCRIBIR en nosotros -> recibir grupoB
            HAL_I2C_Slave_Seq_Receive_IT(hi2c, (uint8_t*)&i2cRxByte, 1,
                                         I2C_FIRST_AND_LAST_FRAME);
        }
    }
}

/* Llamado cuando terminamos de transmitir al master */
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    // Nada extra, el ListenCplt rearmara el listen
}

/* Llamado cuando recibimos dato del master */
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        // Guardar los 4 bits del companero
        grupoB = i2cRxByte & 0x0F;
        // Guardar los 4 bits del companero + comandos de servo
        grupoB = i2cRxByte & 0x0F;
        servo1_cmd = (i2cRxByte >> 4) & 0x01;
        servo2_cmd = (i2cRxByte >> 5) & 0x01;
    }
}

/* Llamado cuando termina la transaccion completa (STOP en el bus) */
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
    // Rearmar el listen para la siguiente transaccion
    HAL_I2C_EnableListen_IT(hi2c);
}

/* Error: reiniciar I2C y reactivar listen */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1)
    {
        sprintf(debugMsg, "I2C ERR: 0x%08lX\r\n", hi2c->ErrorCode);
        HAL_UART_Transmit(&huart2, (uint8_t*)debugMsg, strlen(debugMsg), 100);

        HAL_I2C_DeInit(&hi2c1);
        HAL_I2C_Init(&hi2c1);
        HAL_I2C_EnableListen_IT(&hi2c1);
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
