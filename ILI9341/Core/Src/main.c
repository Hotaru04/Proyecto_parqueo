/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "ili9341.h"
#include "Bitmaps.h"

#include "parqueo.h"
#include "mpr121.h"

#include "neopixel.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define PARQUEO_MODO_DEMO 0

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim4;
DMA_HandleTypeDef hdma_tim4_ch2;

/* USER CODE BEGIN PV */
extern uint16_t fondo[];

extern uint16_t fondoPinguin[];


uint32_t ultimo_sensor = 0;

uint8_t estables[4] = {0};
uint8_t consecutivas[4] = {0};


volatile uint8_t mpr_direccion = 0;
volatile uint8_t mpr_init = 255;
volatile uint8_t mpr_bits = 0;
volatile uint8_t mpr_ecr = 0;

volatile uint32_t mpr_ok = 0;
volatile uint32_t mpr_fallos = 0;
volatile uint32_t mpr_error = 0;


volatile uint8_t prueba_etapa = 0;

volatile uint8_t resultado_scan[4] = {
    255, 255, 255, 255
};

volatile uint32_t error_scan[4] = {0};

volatile uint32_t estado_i2c = 0;

/* ==========================================
 * COMUNICACION I2C2 CON ESP32 MAESTRO
 * Direccion del TFT: 0x09
 * ========================================== */

// Mis cuatro sensores MPR121: P5-P8
volatile uint8_t grupoB_local = 0x00;

// Datos recibidos del ESP32: P1-P4
volatile uint8_t grupoA_recibido = 0x00;

// Banderas de comunicacion
volatile uint8_t i2c2_rx_valido = 0;
volatile uint8_t i2c2_rearmar = 0;

// Buffers de transmision y recepcion
static uint8_t i2c2_tx_byte = 0x00;
static uint8_t i2c2_rx_byte = 0x00;

// Variables de diagnostico
volatile uint32_t i2c2_error = 0;
volatile uint32_t i2c2_tx_count = 0;
volatile uint32_t i2c2_rx_count = 0;

volatile uint32_t i2c2_listen_status = 0xFFFFFFFF;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM4_Init(void);
static void MX_I2C2_Init(void);
/* USER CODE BEGIN PFP */

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
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_TIM4_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */


  HAL_Delay(200);

  // Escaneo I2C solo para verificar que el sensor está presente
  for (uint8_t i = 0; i < 4; i++)
  {
      uint8_t addr = 0x5A + i;

      HAL_StatusTypeDef estado =
          HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 2, 100);

      resultado_scan[i] = estado;
      error_scan[i] = HAL_I2C_GetError(&hi2c1);

      if (estado == HAL_OK)
      {
          mpr_direccion = addr;
          break;
      }
  }

  estado_i2c = HAL_I2C_GetState(&hi2c1);

  // Inicializar MPR121 si fue encontrado
  if (mpr_direccion == 0x5A)
  {
      mpr_init = MPR121_Init(&hi2c1);
  }
  else
  {
      mpr_init = 254;   // no encontrado
  }

  LCD_Init();
  Parqueo_Init(0);
  NeoPixel_Init();

  // Activar I2C2 como esclavo del ESP32
  i2c2_listen_status = HAL_I2C_EnableListen_IT(&hi2c2);

  if (i2c2_listen_status != HAL_OK)
  {
      i2c2_error = HAL_I2C_GetError(&hi2c2);
      Error_Handler();
  }

  HAL_Delay(200);

  // Escaneo I2C solo para verificar que el sensor está presente
  for (uint8_t i = 0; i < 4; i++)
  {
      uint8_t addr = 0x5A + i;

      HAL_StatusTypeDef estado =
          HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 2, 100);

      resultado_scan[i] = estado;
      error_scan[i] = HAL_I2C_GetError(&hi2c1);

      if (estado == HAL_OK)
      {
          mpr_direccion = addr;
          break;
      }
  }

  estado_i2c = HAL_I2C_GetState(&hi2c1);

  // Inicializar MPR121 si fue encontrado
  if (mpr_direccion == 0x5A)
  {
      mpr_init = MPR121_Init(&hi2c1);
  }
  else
  {
      mpr_init = 254;   // no encontrado
  }

  LCD_Init();
  Parqueo_Init(0);
  NeoPixel_Init();
//  NeoPixel_SetEstado(1, 1);
//  NeoPixel_SetEstado(2, 1);
//  NeoPixel_SetEstado(3, 1);
//  NeoPixel_SetEstado(4, 1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {

		 // Recuperar escucha I2C2 si ocurrió un error
		    if (i2c2_rearmar)
		    {
		        i2c2_rearmar = 0;

		        i2c2_listen_status = HAL_I2C_EnableListen_IT(&hi2c2);

		        if (i2c2_listen_status != HAL_OK)
		        {
		            i2c2_rearmar = 1;
		        }
		    }

		    // Leer sensores locales del MPR121
		    if (mpr_init == HAL_OK && HAL_GetTick() - ultimo_sensor >= 50)
		Parqueo_Update();
		    HAL_Delay(10);


		if (mpr_init == HAL_OK && HAL_GetTick() - ultimo_sensor >= 50)
		    {
		        ultimo_sensor = HAL_GetTick();

		        uint8_t lectura = 0;

		        if (MPR121_Leer4(&hi2c1, &lectura) == HAL_OK)
		        {
		            mpr_bits = lectura;
		            mpr_ok++;

		            // Guardar los cuatro sensores locales P5-P8
		            grupoB_local = lectura & 0x0F;
		            for (uint8_t i = 0; i < 4; i++)
		            {
		                uint8_t nuevo = (lectura >> i) & 1;
		                Parqueo_SetOcupado(i + 1, nuevo);
		            }
		        }
		        else
		        {
		            mpr_fallos++;
		            mpr_ecr = HAL_I2C_GetError(&hi2c1);
		        }
		    }

		    // Actualizar los ocho parqueos
		    Parqueo_ActualizarGrupos(grupoA_recibido, grupoB_local);

		    // Actualizar pantalla y NeoPixels
		    Parqueo_Update();
		    NeoPixel_Update();

		    Parqueo_Update();


		    NeoPixel_Update();
		    HAL_Delay(10);



    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

//
//				for (int x = 0; x < 319-42; x++) {
//					int anim = (x/10)%4;
//					// anim 0 1 2 3
//					LCD_Sprite(x, 116-29, 42, 29, link, 4, anim, 0, 0);
//					//V_line( x -1, 100, 50, 0x0DFE);
//					HAL_Delay(15);
//
//				}
//				for (int var = 319-24; var > 0;  var--) {
//					int anim = (var / 10) % 4;
//					LCD_Sprite(var, 100, 24, 30, sonics, 4, anim, 1, 0);
//					V_line(var + 24, 100, 30, 0x0DFE);
//					HAL_Delay(15);
//				}
//
//		for (int x = 0; x < 319-24; x++) {
//			int anim = (x/10)%4;
//			// anim 0 1 2 3
//			LCD_Sprite(x, 100, 24, 30, sonics, 4, anim, 0, 0);
//			V_line( x -1, 100, 30, 0x0DFE);
//			HAL_Delay(15);
//
//		}
//		for (int var = 319-24; var > 0;  var--) {
//			int anim = (var / 10) % 4;
//			LCD_Sprite(var, 100, 24, 30, sonics, 4, anim, 1, 0);
//			V_line(var + 24, 100, 30, 0x0DFE);
//			HAL_Delay(15);
//		}

//		for (int var = 0; var < 319-26;  var++) {
//			 int anim = (var / 5) % 4;
//			LCD_Sprite(var,100,26,16,link,4,anim,0,0);
//			V_line( var -1, 100, 16, 0x4d9e);
//			 HAL_Delay(15);
//		}
//		for (int var = 319-26; var > 0;  var--) {
//					 int anim = (var / 5) % 4;
//					LCD_Sprite(var,100,26,16,link,4,anim,1,0);
//					V_line( var +27, 100, 16, 0x4d9e);
//					 HAL_Delay(15);
//				}
//
//		for (int x = 0; x < 320 - 16; x++) {
//		    int anim2 = (x / 10) % 4;
//		    LCD_Sprite(x,100,16,16,kirbys,4,anim2,0,0);
//		    V_line( x -1, 100, 16, 0x74DA);
//		    HAL_Delay(15);
//		  }
//		  for (int x = 320-16; x > 0; x--) {
//		    int anim2 = (x / 10) % 4;
//		    LCD_Sprite(x,100,16,16,kirbys,4,anim2,1,0);
//		    V_line( x +16, 100, 16, 0x74DA);
//		    HAL_Delay(15);
//		  }




	}
  /* USER CODE END 3 */
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
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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
  hi2c1.Init.OwnAddress1 = 0;
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
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 18;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

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
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */
  HAL_TIM_MspPostInit(&htim4);

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LCD_RST_Pin|LCD_D1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LCD_RD_Pin|LCD_WR_Pin|LCD_RS_Pin|LCD_D7_Pin
                          |LCD_D0_Pin|LCD_D2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin|LCD_D6_Pin|LCD_D3_Pin|LCD_D5_Pin
                          |LCD_D4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LCD_RST_Pin LCD_D1_Pin */
  GPIO_InitStruct.Pin = LCD_RST_Pin|LCD_D1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_RD_Pin LCD_WR_Pin LCD_RS_Pin LCD_D7_Pin
                           LCD_D0_Pin LCD_D2_Pin */
  GPIO_InitStruct.Pin = LCD_RD_Pin|LCD_WR_Pin|LCD_RS_Pin|LCD_D7_Pin
                          |LCD_D0_Pin|LCD_D2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_CS_Pin LCD_D5_Pin LCD_D4_Pin */
  GPIO_InitStruct.Pin = LCD_CS_Pin|LCD_D5_Pin|LCD_D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_D6_Pin LCD_D3_Pin */
  GPIO_InitStruct.Pin = LCD_D6_Pin|LCD_D3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */


/* ==========================================
 * CALLBACKS I2C2 - STM32 TFT
 * ESP32 MAESTRO / STM32 ESCLAVO 0x09
 * ========================================== */

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c,
                          uint8_t TransferDirection,
                          uint16_t AddrMatchCode)
{
    (void)AddrMatchCode;

    if (hi2c->Instance != I2C2)
        return;

    if (TransferDirection == I2C_DIRECTION_RECEIVE)
    {
        // ESP32 quiere LEER:
        // Enviar los cuatro sensores locales P5-P8

        i2c2_tx_byte = grupoB_local & 0x0F;

        if (HAL_I2C_Slave_Seq_Transmit_IT(
                hi2c,
                &i2c2_tx_byte,
                1,
                I2C_FIRST_AND_LAST_FRAME) != HAL_OK)
        {
            i2c2_error = HAL_I2C_GetError(hi2c);
        }
    }
    else
    {
        // ESP32 quiere ESCRIBIR:
        // Recibir los sensores P1-P4

        if (HAL_I2C_Slave_Seq_Receive_IT(
                hi2c,
                &i2c2_rx_byte,
                1,
                I2C_FIRST_AND_LAST_FRAME) != HAL_OK)
        {
            i2c2_error = HAL_I2C_GetError(hi2c);
        }
    }
}


/* Transmision completada */
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C2)
    {
        i2c2_tx_count++;
    }
}


/* Recepcion completada */
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C2)
    {
        // Guardar los cuatro bits recibidos
        grupoA_recibido = i2c2_rx_byte & 0x0F;

        i2c2_rx_valido = 1;

        i2c2_rx_count++;
    }
}


/* Transaccion terminada: volver a escuchar */
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C2)
    {
        i2c2_listen_status = HAL_I2C_EnableListen_IT(hi2c);

        if (i2c2_listen_status != HAL_OK)
        {
            i2c2_rearmar = 1;
        }
    }
}


/* Manejo de errores */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C2)
    {
        i2c2_error = hi2c->ErrorCode;

        i2c2_rearmar = 1;
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
	while (1) {
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
