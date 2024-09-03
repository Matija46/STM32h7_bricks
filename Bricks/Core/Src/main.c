/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "cmsis_os.h"

#include "ts.h"  // Include the touchscreen header file
#include "lcd.h" // Include the LCD header file if not already included

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "FreeRTOS.h"
#include "stlogo.h"
#include "app_timers.h"
#include <stdio.h>
#include "string.h"
#include "uart.h"
#include "retarget.h"
#include "dma.h"
#include "stdbool.h"

//#include "pdm2pcm_glo.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc3;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */
__IO uint32_t ButtonState = 0;
uint32_t x_size;
uint32_t y_size;
uint32_t playerx;
uint16_t timer_val_start, timer_val_end;
uint16_t elapsed_1st, elapsed_2nd, elapsed_3rd;
uint32_t BACKGROUND_COLOR = 0xff2e2d2d;
uint32_t bricksBroken = 0;
UART_HandleTypeDef 				UART3Handle;
TIM_HandleTypeDef    			TIM3Handle;

uint8_t time_str1[60];
uint8_t time_str2[40];

#define RECT_WIDTH  20
//#define RECT_HEIGHT 70
int16_t RECT_HEIGHT = 70;

#define BLOCK_WIDTH  20
#define BLOCK_HEIGHT 52
#define BALL_RADIUS 5

#define EASY_MODE_BRICKS_ROWS 5
#define MEDIUM_MODE_BRICKS_ROWS 6
#define HARD_MODE_BRICKS_ROWS 7
#define BRICKS_ROWS 5

uint8_t bricks[HARD_MODE_BRICKS_ROWS][BRICKS_ROWS];



int16_t ball_x, ball_y;
int16_t ball_dx, ball_dy;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC3_Init(void);
void StartDefaultTask(void *argument);


bool gameStart = false;
bool gameOver = false;
int16_t gameMode = 0;

static void MX_GPIO_Init(void);

/* USER CODE BEGIN PFP */
static int Display_InitialContent();
static void CPU_CACHE_Enable(void);
void UpdateBallPosition(int rowsSelected);
void DrawBall(int16_t x, int16_t y, uint32_t color);
void CheckCollisions(int rows);
void setupScreen();
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

  /* Configure the MPU attributes as Write Through */
//  MPU_Config();

  /* Enable the CPU Cache */
//  CPU_CACHE_Enable();

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  /* STM32H7xx HAL library initialization:
         - Systick timer is configured by default as source of time base, but user
           can eventually implement his proper time base source (a general purpose
           timer for example or other time source), keeping in mind that Time base
           duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
           handled in milliseconds basis.
         - Set NVIC Group Priority to 4
         - Low Level Initialization
  */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock to 400 MHz */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  //MX_GPIO_Init();
  //MX_ADC3_Init();

  /* USER CODE BEGIN 2 */
  /* When system initialization is finished, Cortex-M7 could wakeup (when needed) the Cortex-M4  by means of
        HSEM notification or by any D2 wakeup source (SEV,EXTI..)   */

  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_RED);

  BSP_LED_Off(LED_GREEN);

  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);

  /* Configure the User push-button in EXTI Mode */
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Configure TIM3 timebase */
  Init_TIM3(&TIM3Handle);

  /* Init UART3*/
  if (USART3_Init(&UART3Handle) != HAL_OK){
	  Error_Handler();
  }
  RetargetInit(&UART3Handle);

  BSP_LCD_Init(0, LCD_ORIENTATION_PORTRAIT);
  UTIL_LCD_SetFuncDriver(&LCD_Driver);

  ball_x = 330;
  ball_y = 272 / 2;
  ball_dx = -3;
  ball_dy = 3;

  TS_Init_t hTS;
  BSP_LCD_GetXSize(0, &x_size);
  BSP_LCD_GetYSize(0, &y_size);

  hTS.Width = x_size;
  hTS.Height = y_size;
  hTS.Orientation = TS_SWAP_XY;
  hTS.Accuracy = 5;

  uint16_t y = 0;
  uint16_t x = 0;
  uint16_t last_y = -1;
  int rowsSelected = 0;


  uint32_t ts_status = BSP_TS_Init(0, &hTS);
  if(ts_status != BSP_ERROR_NONE) {
    Error_Handler();
  }

  rowsSelected = Display_InitialContent();
  TS_State_t TS_State;

  while (1){
      	ts_status = BSP_TS_GetState(0, &TS_State);

		UpdateBallPosition(rowsSelected);


		if(bricksBroken == 5*rowsSelected){
			while(1){
				 UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLACK);
				 UTIL_LCD_Clear(UTIL_LCD_COLOR_BLACK);

				 //G
				 UTIL_LCD_FillRect(350, 44, 80,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(350, 44, 8,40, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(390-4, 62, 8, 22, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(430-8, 44, 8, 40, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(350, 44+40-8, 40,8, UTIL_LCD_COLOR_RED);


				 //A
				 UTIL_LCD_FillRect(350, 44+40-8+48, 40,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(350, 44+48, 40,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(390-4, 44+48, 8, 40, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(390+4, 44+48, 12, 8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(390+4+12, 44+48+8, 12, 8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(390+4+12+12, 44+48+8+8, 12, 8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(390+4+12, 44+48+8+8+8, 12, 8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(390+4, 44+48+8+8+8+8, 12, 8, UTIL_LCD_COLOR_RED);


				 //M
				 UTIL_LCD_FillRect(350, 44+48+48, 80,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(350, 44+48+80, 80,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(390+4+24, 70+22+48, 12, 8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(390+4+12, 70+22+48+8, 12, 8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(390+4, 48+44+48+8+8, 12, 8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(390+4+12, 48+44+48+8+8+8, 12, 8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(390+4+24, 48+44+48+8+8+8+8, 12, 8, UTIL_LCD_COLOR_RED);

				 //E
				 UTIL_LCD_FillRect(350, 44+48+48+48, 80,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(390-4, 44+48+48+48, 8, 40, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(350, 44+48+48+48, 8, 40, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(430-8, 44+48+48+48, 8, 40, UTIL_LCD_COLOR_RED);


				 //W
				 UTIL_LCD_FillRect(430-8-35-80, 44, 27+8,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(430-8-35-80-35, 44+8, 27+8,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(430-8-35-80-35-8, 44+8+8, 8,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(430-8-35-80-35, 44+8+8+8, 27,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(430-16-35-80, 44+8+8+8+8, 8,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(430-8-35-80-35, 44+8+32, 27,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(430-8-35-80-35-8, 44+8+32+8, 8,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(430-8-35-80-35,  44+8+32+16, 27+8,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(430-8-35-80, 44+8+32+24, 27+8,8, UTIL_LCD_COLOR_RED);

				 //O
				 UTIL_LCD_FillRect(430-80-88, 44+8+32+24+16, 80,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(430-80-88, 44+8+32+24+16, 8,40, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(430-80-88+72, 44+8+32+24+16, 8,40, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(430-80-88, 44+8+32+24+16+32, 80,8, UTIL_LCD_COLOR_RED);

				 //N
				 UTIL_LCD_FillRect(430-80-88, 44+8+32+24+64, 80,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(430-80-88, 44+8+32+24+64+32, 80,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(430-80-88+16, 44+8+32+24+64+32-8, 16,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(430-80-88+32, 44+8+32+24+64+32-16, 16,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(430-80-88+48, 44+8+32+24+64+32-24, 16,8, UTIL_LCD_COLOR_RED);
				 UTIL_LCD_FillRect(430-80-88+64, 44+8+32+24+64+32-32, 16,8, UTIL_LCD_COLOR_RED);

				 UTIL_LCD_FillRect(100, 272/2-115, 50, 230, UTIL_LCD_COLOR_BLUE);

				 //P
				 UTIL_LCD_FillRect(100+7, 32, 36,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+36-4, 32, 4,20, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18-2, 32, 4,20, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18-2, 32+16, 18,4, UTIL_LCD_COLOR_WHITE);

				 //L
				 UTIL_LCD_FillRect(100+7, 32+24, 36,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7, 32+24, 4,20, UTIL_LCD_COLOR_WHITE);

				 //A
				 UTIL_LCD_FillRect(100+7, 76+4, 18,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7, 76+20, 18,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18-2, 76+4, 4,20, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+2, 76+4, 5,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+7, 76+4+4, 5,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+12, 76+4+8, 5,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+7, 76+4+12, 5,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+2, 76+4+16, 5,4, UTIL_LCD_COLOR_WHITE);

				 //Y
				 UTIL_LCD_FillRect(100+7, 104+10-2, 22,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+12, 104, 5,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+7, 104+4, 5,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+2, 104+8, 5,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+7, 104+12, 5,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+12, 104+16, 5,4, UTIL_LCD_COLOR_WHITE);

				 //A
				 UTIL_LCD_FillRect(100+7, 135+4, 18,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7, 135+20, 18,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18-2, 135+4, 4,20, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+2, 135+4, 5,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+7, 135+4+4, 5,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+12, 135+4+8, 5,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+7, 135+4+12, 5,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+2, 135+4+16, 5,4, UTIL_LCD_COLOR_WHITE);

				 //G
				 UTIL_LCD_FillRect(100+7, 139+24, 36,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7, 139+24, 4,20, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7, 139+24+16, 18,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+16, 139+24+10, 4,10, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+32, 139+24, 4,20, UTIL_LCD_COLOR_WHITE);

				 //A
				 UTIL_LCD_FillRect(100+7, 183+4, 18,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7, 183+20, 18,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18-2, 183+4, 4,20, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+2, 183+4, 5,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+7, 183+4+4, 5,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+12, 183+4+8, 5,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+7, 183+4+12, 5,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+18+2, 183+4+16, 5,4, UTIL_LCD_COLOR_WHITE);

				 //I
				 UTIL_LCD_FillRect(100+7, 183+28, 36,4, UTIL_LCD_COLOR_WHITE);

				 //N
				 UTIL_LCD_FillRect(100+7, 219, 36,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7, 219+16, 36,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+29-7, 219+4, 7,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+29-14, 219+8, 7,4, UTIL_LCD_COLOR_WHITE);
				 UTIL_LCD_FillRect(100+7+29-21, 219+12, 7,4, UTIL_LCD_COLOR_WHITE);

				 while(1) {
					 ts_status = BSP_TS_GetState(0, &TS_State);
					 if (TS_State.TouchDetected) {
							 y = TS_State.TouchY;
							 x = TS_State.TouchX;
							if(x > 100 && x < 150 && y > 272/2-115 && y < 272/2+115){
								ball_x = 330;
								ball_y = 272 / 2;
								ball_dx = 3;
								ball_dy = 3;

								playerx = y_size / 2-RECT_HEIGHT/2;

								gameOver = false;
								gameStart = false;


								UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLACK);
								UTIL_LCD_Clear(UTIL_LCD_COLOR_BLACK);
								rowsSelected = Display_InitialContent();

								bricksBroken = 0;

								break;
							}

						 }
						 HAL_Delay(50);
					 }

			}
		}

	if(gameOver){
		 UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLACK);
		 UTIL_LCD_Clear(UTIL_LCD_COLOR_BLACK);

		 // g 8 a 8 m 8 e

		 //G
		 UTIL_LCD_FillRect(350, 44, 80,8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(350, 44, 8,40, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390-4, 62, 8, 22, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(430-8, 44, 8, 40, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(350, 44+40-8, 40,8, UTIL_LCD_COLOR_RED);


		 //A
		 UTIL_LCD_FillRect(350, 44+40-8+48, 40,8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(350, 44+48, 40,8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390-4, 44+48, 8, 40, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390+4, 44+48, 12, 8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390+4+12, 44+48+8, 12, 8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390+4+12+12, 44+48+8+8, 12, 8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390+4+12, 44+48+8+8+8, 12, 8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390+4, 44+48+8+8+8+8, 12, 8, UTIL_LCD_COLOR_RED);


		 //M
		 UTIL_LCD_FillRect(350, 44+48+48, 80,8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(350, 44+48+80, 80,8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390+4+24, 70+22+48, 12, 8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390+4+12, 70+22+48+8, 12, 8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390+4, 48+44+48+8+8, 12, 8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390+4+12, 48+44+48+8+8+8, 12, 8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390+4+24, 48+44+48+8+8+8+8, 12, 8, UTIL_LCD_COLOR_RED);

		 //E
		 UTIL_LCD_FillRect(350, 44+48+48+48, 80,8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390-4, 44+48+48+48, 8, 40, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(350, 44+48+48+48, 8, 40, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(430-8, 44+48+48+48, 8, 40, UTIL_LCD_COLOR_RED);


		 //O
		 UTIL_LCD_FillRect(262+8, 44, 80-16,8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(262+8, 44+32, 80-16,8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(350-88, 44+8, 8, 40-16, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(430-8-88, 44+8, 8, 40-16, UTIL_LCD_COLOR_RED);

		 //V
		 UTIL_LCD_FillRect(430-8-35-80, 44+48, 27+8,8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(430-8-35-80-35, 44+48+8, 27+8,8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(430-8-35-80-35-11, 44+48+8+8, 11,8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(430-8-35-80-35, 44+48+8+8+8, 27+8,8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(430-8-35-80, 44+48+8+8+8+8, 27+8,8, UTIL_LCD_COLOR_RED);

		 //E
		 UTIL_LCD_FillRect(262, 44+48+48, 80,8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390-4-88, 44+48+48, 8, 40, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(350-88, 44+48+48, 8, 40, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(430-8-88, 44+48+48, 8, 40, UTIL_LCD_COLOR_RED);

		 //R
		 UTIL_LCD_FillRect(262, 44+48+48+48, 80,8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390-4-88, 44+48+48+48, 8, 40, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(430-8-88, 44+48+48+48, 8, 40, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390-4-88, 44+48+48+48+32, 40, 8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390-4-80-16, 44+48+48+48+8, 8, 8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390-4-80-24, 44+48+48+48+16, 8, 8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390-4-80-32, 44+48+48+48+24, 8, 8, UTIL_LCD_COLOR_RED);
		 UTIL_LCD_FillRect(390-4-80-40, 44+48+48+48+32, 8, 8, UTIL_LCD_COLOR_RED);


		 UTIL_LCD_FillRect(100, 272/2-115, 50, 230, UTIL_LCD_COLOR_BLUE);
		 //P 4 L 4 A 4 Y 15 A 4 G 4 A 4 I 4 N
		 //text 164
		 //43

		 //P
		 UTIL_LCD_FillRect(100+7, 32, 36,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+36-4, 32, 4,20, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18-2, 32, 4,20, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18-2, 32+16, 18,4, UTIL_LCD_COLOR_WHITE);

		 //L
		 UTIL_LCD_FillRect(100+7, 32+24, 36,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7, 32+24, 4,20, UTIL_LCD_COLOR_WHITE);

		 //A
		 UTIL_LCD_FillRect(100+7, 76+4, 18,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7, 76+20, 18,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18-2, 76+4, 4,20, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+2, 76+4, 5,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+7, 76+4+4, 5,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+12, 76+4+8, 5,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+7, 76+4+12, 5,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+2, 76+4+16, 5,4, UTIL_LCD_COLOR_WHITE);

		 //Y
		 UTIL_LCD_FillRect(100+7, 104+10-2, 22,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+12, 104, 5,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+7, 104+4, 5,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+2, 104+8, 5,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+7, 104+12, 5,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+12, 104+16, 5,4, UTIL_LCD_COLOR_WHITE);

		 //A
		 UTIL_LCD_FillRect(100+7, 135+4, 18,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7, 135+20, 18,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18-2, 135+4, 4,20, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+2, 135+4, 5,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+7, 135+4+4, 5,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+12, 135+4+8, 5,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+7, 135+4+12, 5,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+2, 135+4+16, 5,4, UTIL_LCD_COLOR_WHITE);

		 //G
		 UTIL_LCD_FillRect(100+7, 139+24, 36,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7, 139+24, 4,20, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7, 139+24+16, 18,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+16, 139+24+10, 4,10, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+32, 139+24, 4,20, UTIL_LCD_COLOR_WHITE);

		 //A
		 UTIL_LCD_FillRect(100+7, 183+4, 18,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7, 183+20, 18,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18-2, 183+4, 4,20, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+2, 183+4, 5,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+7, 183+4+4, 5,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+12, 183+4+8, 5,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+7, 183+4+12, 5,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+18+2, 183+4+16, 5,4, UTIL_LCD_COLOR_WHITE);

		 //I
		 UTIL_LCD_FillRect(100+7, 183+28, 36,4, UTIL_LCD_COLOR_WHITE);

		 //N
		 UTIL_LCD_FillRect(100+7, 219, 36,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7, 219+16, 36,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+29-7, 219+4, 7,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+29-14, 219+8, 7,4, UTIL_LCD_COLOR_WHITE);
		 UTIL_LCD_FillRect(100+7+29-21, 219+12, 7,4, UTIL_LCD_COLOR_WHITE);


		 while(1) {
			 ts_status = BSP_TS_GetState(0, &TS_State);

			 if (TS_State.TouchDetected) {
				y = TS_State.TouchY;
				x = TS_State.TouchX;
				if(x > 100 && x < 150 && y > 272/2-115 && y < 272/2+115){
					ball_x = 330;
					ball_y = 272 / 2;
					ball_dx = 3;
					ball_dy = 3;

					playerx = y_size / 2-RECT_HEIGHT/2;

					gameOver = false;
					gameStart = false;


					UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLACK);
					UTIL_LCD_Clear(UTIL_LCD_COLOR_BLACK);
					rowsSelected = Display_InitialContent();

					bricksBroken = 0;

					break;
				}

			 }
			 HAL_Delay(50);
		 }
	}



    if (TS_State.TouchDetected) {
    	y = TS_State.TouchY;
    	if(y != last_y){
			gameStart = true;
			if(y < RECT_HEIGHT / 2){
				y = RECT_HEIGHT / 2;
			}

			if(y > y_size - RECT_HEIGHT / 2){
				y = y_size - RECT_HEIGHT / 2;
			}
			playerx = y;
			UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLACK);
			UTIL_LCD_FillRect(20, 0, RECT_WIDTH, y_size, UTIL_LCD_COLOR_BLACK);
			UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_BLUE);
			UTIL_LCD_FillRect(20, y - RECT_HEIGHT / 2, RECT_WIDTH, RECT_HEIGHT, UTIL_LCD_COLOR_BLUE);
    	}
    	last_y = y;
    }


    if (CheckForUserInput() > 0) {
      ButtonState = 0;
      BSP_AUDIO_OUT_Stop(0);
      BSP_AUDIO_OUT_DeInit(0);
      BSP_AUDIO_IN_Stop(1);
      BSP_AUDIO_IN_DeInit(1);
      return;
    }

    HAL_Delay(17);

    if (CheckForUserInput() > 0){
      ButtonState = 0;
      BSP_AUDIO_OUT_Stop(0);
      BSP_AUDIO_OUT_DeInit(0);
      BSP_AUDIO_IN_Stop(1);
      BSP_AUDIO_IN_DeInit(1);
      return;
    }
  }

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief  Display main demo messages
  * @param  None
  * @retval None
  */
int Display_InitialContent(void)
{
  BSP_LCD_GetXSize(0, &x_size);
  BSP_LCD_GetYSize(0, &y_size);

  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLACK);
  UTIL_LCD_Clear(UTIL_LCD_COLOR_BLACK);

  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);

  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_BLUE);


  UTIL_LCD_FillRect(50, 272 / 2 - 100, 50, 200, UTIL_LCD_COLOR_BLUE);
  UTIL_LCD_FillRect(150, 272 / 2 - 100, 50, 200, UTIL_LCD_COLOR_BLUE);
  UTIL_LCD_FillRect(250, 272 / 2 - 100, 50, 200, UTIL_LCD_COLOR_BLUE);

  UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLUE);

     //B
     UTIL_LCD_FillRect(350, 22, 8, 32, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(350, 22, 80,8, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(350+8, 62-8, 64,8, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(390-4, 22, 8, 32, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(430-8, 22, 8, 32, UTIL_LCD_COLOR_RED);

     UTIL_LCD_FillRect(390-4, 62-8, 8, 8, UTIL_LCD_COLOR_BLACK);


     //R
     UTIL_LCD_FillRect(350, 22+44, 80,8, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(390-4, 22+44, 8, 40, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(430-8, 22+44, 8, 40, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(390, 66+40-8, 40,8, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(390-4-8, 22+44+8, 8, 8, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(390-4-8*2, 22+44+8*2, 8, 8, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(390-4-8*3, 22+44+8*3, 8, 8, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(390-4-8*4, 22+44+8*4, 8, 8, UTIL_LCD_COLOR_RED);

     //I
     UTIL_LCD_FillRect(350, 22+40*2+4*2, 80,8, UTIL_LCD_COLOR_RED);

     //C
     UTIL_LCD_FillRect(350, 22+40*2+4*3+8, 80,8, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(350, 22+40*2+4*3+8, 8, 40, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(430-8, 22+40*2+4*3+8, 8, 40, UTIL_LCD_COLOR_RED);

     //K
     UTIL_LCD_FillRect(350, 22+40*3+4*4+8, 80,8, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(390-4-4, 22+40*3+4*4+8+8, 11, 8, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(390-8-11*1, 22+40*3+4*4+8+8*2, 11, 8, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(390-8-11*2, 22+40*3+4*4+8+8*3, 11, 8, UTIL_LCD_COLOR_RED);
	 UTIL_LCD_FillRect(390-8-11*3, 22+40*3+4*4+8+8*4, 11, 8, UTIL_LCD_COLOR_RED);
	 UTIL_LCD_FillRect(390-4+4, 22+40*3+4*4+8+8, 8, 8, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(390-8+11*1+4, 22+40*3+4*4+8+8*2, 11, 8, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(390-8+11*2+4, 22+40*3+4*4+8+8*3, 11, 8, UTIL_LCD_COLOR_RED);
	 UTIL_LCD_FillRect(390-8+11*3+4, 22+40*3+4*4+8+8*4, 11, 8, UTIL_LCD_COLOR_RED);

	 //S
     UTIL_LCD_FillRect(390, 22+40*4+5*4+8, 40,8, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(350, 22+40*4+5*4+8+40-8, 40,8, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(350, 22+40*4+5*4+8, 8, 40, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(430-8, 22+40*4+5*4+8, 8, 40, UTIL_LCD_COLOR_RED);
     UTIL_LCD_FillRect(390-4, 22+40*4+5*4+8, 8, 40, UTIL_LCD_COLOR_RED);


     //E
     UTIL_LCD_FillRect(50+7, 90, 36,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(50+7, 90, 4,20, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(50+7+36-4, 90, 4,20, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(50+7+18-2, 90, 4,20, UTIL_LCD_COLOR_WHITE);

     //A
     UTIL_LCD_FillRect(50+7, 110+4, 18,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(50+7, 130, 18,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(50+7+18-2, 110+4, 4,20, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(50+7+18+2, 110+4, 5,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(50+7+18+7, 110+4+4, 5,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(50+7+18+12, 110+4+8, 5,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(50+7+18+7, 110+4+12, 5,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(50+7+18+2, 110+4+16, 5,4, UTIL_LCD_COLOR_WHITE);

     //S
     UTIL_LCD_FillRect(50+7, 130+8, 4,20, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(50+7+36-4, 130+8, 4,20, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(50+7+18-2, 130+8, 4,20, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(50+7, 130+8+16, 18,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(50+7+16, 130+8, 18,4, UTIL_LCD_COLOR_WHITE);


     //Y
     UTIL_LCD_FillRect(50+7, 150+12+10-2, 22,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(50+7+18+12, 150+12, 5,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(50+7+18+7, 150+12+4, 5,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(50+7+18+2, 150+12+8, 5,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(50+7+18+7, 150+12+12, 5,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(50+7+18+12, 150+12+16, 5,4, UTIL_LCD_COLOR_WHITE);



     //H
     UTIL_LCD_FillRect(250+7, 90, 36,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(250+7+18-2, 90, 4,20, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(250+7, 90+20-4, 36,4, UTIL_LCD_COLOR_WHITE);


     //A
     UTIL_LCD_FillRect(250+7, 110+4, 18,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(250+7, 130, 18,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(250+7+18-2, 110+4, 4,20, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(250+7+18+2, 110+4, 5,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(250+7+18+7, 110+4+4, 5,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(250+7+18+12, 110+4+8, 5,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(250+7+18+7, 110+4+12, 5,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(250+7+18+2, 110+4+16, 5,4, UTIL_LCD_COLOR_WHITE);


     //R
     UTIL_LCD_FillRect(250+7+18-2, 130+8, 4,20, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(250+7+36-4, 130+8, 4,20, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(250+7+18-2, 130+8+16, 18,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(250+7, 130+8, 36,4, UTIL_LCD_COLOR_WHITE);

     UTIL_LCD_FillRect(250+7+18-2-4, 130+8+4, 4,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(250+7+18-2-8, 130+8+8, 4,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(250+7+18-2-12, 130+8+12, 4,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(250+7+18-2-16, 130+8+16, 4,4, UTIL_LCD_COLOR_WHITE);


     //D
     UTIL_LCD_FillRect(250+7, 150+12, 36,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(250+7+4, 150+12+16, 28,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(250+7+36-4, 150+12, 4,16, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(250+7, 150+12, 4,16, UTIL_LCD_COLOR_WHITE);


     //M
     UTIL_LCD_FillRect(150+7, 74, 36,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(150+7, 90, 36,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(150+7+28, 78, 4,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(150+7+24, 82, 4,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(150+7+28, 78+8, 4,4, UTIL_LCD_COLOR_WHITE);


     //E
     UTIL_LCD_FillRect(150+7, 98, 36,4, UTIL_LCD_COLOR_WHITE);
     UTIL_LCD_FillRect(150+7, 98, 4,20, UTIL_LCD_COLOR_WHITE);
	 UTIL_LCD_FillRect(150+7+36-4, 98, 4,20, UTIL_LCD_COLOR_WHITE);
	 UTIL_LCD_FillRect(150+7+18-2, 98, 4,20, UTIL_LCD_COLOR_WHITE);



	 //D
	 UTIL_LCD_FillRect(150+7, 98+24, 36,4, UTIL_LCD_COLOR_WHITE);
	 UTIL_LCD_FillRect(150+7+4, 98+24+16, 28,4, UTIL_LCD_COLOR_WHITE);
	 UTIL_LCD_FillRect(150+7+36-4, 98+24, 4,16, UTIL_LCD_COLOR_WHITE);
	 UTIL_LCD_FillRect(150+7, 98+24, 4,16, UTIL_LCD_COLOR_WHITE);

	 //I
	 UTIL_LCD_FillRect(150+7, 98+24+24, 36,4, UTIL_LCD_COLOR_WHITE);

	 //U
	 UTIL_LCD_FillRect(150+7+4, 98+24+24+8, 32,4, UTIL_LCD_COLOR_WHITE);
	 UTIL_LCD_FillRect(150+7+4, 98+24+24+8+16, 32,4, UTIL_LCD_COLOR_WHITE);
	 UTIL_LCD_FillRect(150+7, 98+24+24+12, 4,12, UTIL_LCD_COLOR_WHITE);

	 //M
	 UTIL_LCD_FillRect(150+7, 98+24+24+24+8, 36,4, UTIL_LCD_COLOR_WHITE);
	 UTIL_LCD_FillRect(150+7, 98+24+24+24+8+16, 36,4, UTIL_LCD_COLOR_WHITE);
	 UTIL_LCD_FillRect(150+7+28, 98+24+24+24+8+4, 4,4, UTIL_LCD_COLOR_WHITE);
	 UTIL_LCD_FillRect(150+7+24, 98+24+24+24+8+8, 4,4, UTIL_LCD_COLOR_WHITE);
	 UTIL_LCD_FillRect(150+7+28, 98+24+24+24+8+12, 4,4, UTIL_LCD_COLOR_WHITE);


  TS_State_t TS_State;
    while (1){
      BSP_TS_GetState(0, &TS_State);
      if (TS_State.TouchDetected){
    	  uint16_t y = TS_State.TouchY;
    	  uint16_t x = TS_State.TouchX;
    	  if(y > 272 / 2 - 100 && y < 271 / 2 + 100){
    		  if(x > 50 && x < 100){
    			  RECT_HEIGHT = 80;
    			  gameMode = 1;

    			  setupScreen();

    			  UTIL_LCD_FillRect(200, 272/2-20, 10, 40, UTIL_LCD_COLOR_RED);
    			  UTIL_LCD_FillRect(175, 272/2-10, 10, 30, UTIL_LCD_COLOR_RED);
    			  UTIL_LCD_FillRect(150, 272/2-20, 10, 40, UTIL_LCD_COLOR_RED);
    			  UTIL_LCD_FillRect(150, 272/2+10, 50, 10, UTIL_LCD_COLOR_RED);

    			  HAL_Delay(1000);

    			  UTIL_LCD_FillRect(150, 272/2-20, 60, 60, UTIL_LCD_COLOR_BLACK);
    			  UTIL_LCD_FillRect(200, 272/2-20, 10, 40, UTIL_LCD_COLOR_RED);
    			  UTIL_LCD_FillRect(175, 272/2-20, 10, 40, UTIL_LCD_COLOR_RED);
				  UTIL_LCD_FillRect(150, 272/2-20, 10, 40, UTIL_LCD_COLOR_RED);
				  UTIL_LCD_FillRect(150, 272/2-20, 25, 10, UTIL_LCD_COLOR_RED);
				  UTIL_LCD_FillRect(175, 272/2+10, 25, 10, UTIL_LCD_COLOR_RED);

    			  HAL_Delay(1000);

    			  UTIL_LCD_FillRect(150, 272/2-20, 60, 60, UTIL_LCD_COLOR_BLACK);
    			  UTIL_LCD_FillRect(150, 272/2+10, 60, 10, UTIL_LCD_COLOR_RED);

    			  HAL_Delay(1000);

    			  UTIL_LCD_FillRect(150, 272/2-20, 60, 60, UTIL_LCD_COLOR_BLACK);


    			  return 5;
    		  }
    		  else if(x > 150 && x < 200){
    			  RECT_HEIGHT = 50;
				  gameMode = 2;

    			  setupScreen();

				  UTIL_LCD_FillRect(200, 272/2-20, 10, 40, UTIL_LCD_COLOR_RED);
				  UTIL_LCD_FillRect(175, 272/2-10, 10, 30, UTIL_LCD_COLOR_RED);
				  UTIL_LCD_FillRect(150, 272/2-20, 10, 40, UTIL_LCD_COLOR_RED);
				  UTIL_LCD_FillRect(150, 272/2+10, 50, 10, UTIL_LCD_COLOR_RED);

				  HAL_Delay(1000);

				  UTIL_LCD_FillRect(150, 272/2-20, 60, 60, UTIL_LCD_COLOR_BLACK);
				  UTIL_LCD_FillRect(200, 272/2-20, 10, 40, UTIL_LCD_COLOR_RED);
				  UTIL_LCD_FillRect(175, 272/2-20, 10, 40, UTIL_LCD_COLOR_RED);
				  UTIL_LCD_FillRect(150, 272/2-20, 10, 40, UTIL_LCD_COLOR_RED);
				  UTIL_LCD_FillRect(150, 272/2-20, 25, 10, UTIL_LCD_COLOR_RED);
				  UTIL_LCD_FillRect(175, 272/2+10, 25, 10, UTIL_LCD_COLOR_RED);

				  HAL_Delay(1000);

				  UTIL_LCD_FillRect(150, 272/2-20, 60, 60, UTIL_LCD_COLOR_BLACK);
				  UTIL_LCD_FillRect(150, 272/2+10, 60, 10, UTIL_LCD_COLOR_RED);

				  HAL_Delay(1000);

				  UTIL_LCD_FillRect(150, 272/2-20, 60, 60, UTIL_LCD_COLOR_BLACK);


				  return 6;
    		  }
    		  else if(x > 250 && x < 300){
    			  RECT_HEIGHT = 30;
				  gameMode = 3;
    			  setupScreen();

				  UTIL_LCD_FillRect(200, 272/2-20, 10, 40, UTIL_LCD_COLOR_RED);
				  UTIL_LCD_FillRect(175, 272/2-10, 10, 30, UTIL_LCD_COLOR_RED);
				  UTIL_LCD_FillRect(150, 272/2-20, 10, 40, UTIL_LCD_COLOR_RED);
				  UTIL_LCD_FillRect(150, 272/2+10, 50, 10, UTIL_LCD_COLOR_RED);

				  HAL_Delay(1000);

				  UTIL_LCD_FillRect(150, 272/2-20, 60, 60, UTIL_LCD_COLOR_BLACK);
				  UTIL_LCD_FillRect(200, 272/2-20, 10, 40, UTIL_LCD_COLOR_RED);
				  UTIL_LCD_FillRect(175, 272/2-20, 10, 40, UTIL_LCD_COLOR_RED);
				  UTIL_LCD_FillRect(150, 272/2-20, 10, 40, UTIL_LCD_COLOR_RED);
				  UTIL_LCD_FillRect(150, 272/2-20, 25, 10, UTIL_LCD_COLOR_RED);
				  UTIL_LCD_FillRect(175, 272/2+10, 25, 10, UTIL_LCD_COLOR_RED);

				  HAL_Delay(1000);

				  UTIL_LCD_FillRect(150, 272/2-20, 60, 60, UTIL_LCD_COLOR_BLACK);
				  UTIL_LCD_FillRect(150, 272/2+10, 60, 10, UTIL_LCD_COLOR_RED);

				  HAL_Delay(1000);

				  UTIL_LCD_FillRect(150, 272/2-20, 60, 60, UTIL_LCD_COLOR_BLACK);


				  return 7;
			  }


    	  }

      }
      HAL_Delay(50);
    }




}

void setupScreen(){

	int brick_rows = 0;

	if (gameMode == 1) {
		brick_rows = EASY_MODE_BRICKS_ROWS;
	}
	else if (gameMode == 2) {
		brick_rows = MEDIUM_MODE_BRICKS_ROWS;
	}
	else if (gameMode == 3) {
		brick_rows = HARD_MODE_BRICKS_ROWS;
	}

	UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_BLACK);
	UTIL_LCD_Clear(UTIL_LCD_COLOR_BLACK);
	memset(bricks, 1, sizeof(bricks));
	for (uint16_t i = 0; i < brick_rows; i++) {
		for (uint16_t j = 0; j < BRICKS_ROWS; j++) {
			UTIL_LCD_FillRect(458 - BLOCK_WIDTH * (i + 1) - 2 * (i + 1), (2 * (j + 1)) + (BLOCK_HEIGHT * j), BLOCK_WIDTH, BLOCK_HEIGHT, UTIL_LCD_COLOR_RED);
		}
	}
	if(gameMode == 2){
		ball_x = ball_x - BLOCK_WIDTH;
	}
	if(gameMode == 3){
		ball_x = ball_x - BLOCK_WIDTH*2;
	}
	UTIL_LCD_SetTextColor(UTIL_LCD_COLOR_BLUE);
	UTIL_LCD_FillRect(20, 272/2 - RECT_HEIGHT / 2, RECT_WIDTH, RECT_HEIGHT, UTIL_LCD_COLOR_BLUE);
	DrawBall(ball_x, ball_y, UTIL_LCD_COLOR_WHITE);
}

/**
  * @brief  Check for user input
  * @param  None
  * @retval Input state (1 : active / 0 : Inactive)
  */
uint8_t CheckForUserInput(void)
{
  return ButtonState;
}

/**
* @brief  EXTI line detection callbacks.
* @param  GPIO_Pin: Specifies the pins connected EXTI line
* @retval None
*/
void BSP_PB_Callback(Button_TypeDef Button)
{
  if(Button == BUTTON_USER)
  {
    ButtonState = 1;
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 400000000 (Cortex-M7 CPU Clock)
  *            HCLK(Hz)                       = 200000000 (Cortex-M4 CPU, Bus matrix Clocks)
  *            AHB Prescaler                  = 2
  *            D1 APB3 Prescaler              = 2 (APB3 Clock  100MHz)
  *            D2 APB1 Prescaler              = 2 (APB1 Clock  100MHz)
  *            D2 APB2 Prescaler              = 2 (APB2 Clock  100MHz)
  *            D3 APB4 Prescaler              = 2 (APB4 Clock  100MHz)
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 5
  *            PLL_N                          = 160
  *            PLL_P                          = 2
  *            PLL_Q                          = 4
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  /*!< Supply configuration update enable */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;

  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    Error_Handler();
  }

/* Select PLL as system clock source and configure  bus clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 | \
                                 RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;
  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
  if(ret != HAL_OK)
  {
    Error_Handler();
  }

 /*
  Note : The activation of the I/O Compensation Cell is recommended with communication  interfaces
          (GPIO, SPI, FMC, QSPI ...)  when  operating at  high frequencies(please refer to product datasheet)
          The I/O Compensation Cell activation  procedure requires :
        - The activation of the CSI clock
        - The activation of the SYSCFG clock
        - Enabling the I/O Compensation Cell : setting bit[0] of register SYSCFG_CCCSR
 */

  /*activate CSI clock mondatory for I/O Compensation Cell*/
  __HAL_RCC_CSI_ENABLE() ;

  /* Enable SYSCFG clock mondatory for I/O Compensation Cell */
  __HAL_RCC_SYSCFG_CLK_ENABLE() ;

  /* Enables the I/O Compensation Cell */
  HAL_EnableCompensationCell();
}

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */

  /** Common config
  */
  hadc3.Instance = ADC3;
  hadc3.Init.Resolution = ADC_RESOLUTION_16B;
  hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc3.Init.LowPowerAutoWait = DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc3.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc3.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc3.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.SingleDiff = ADC_DIFFERENTIAL_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOK_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PI6 PI5 PI4 PI7 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_5|GPIO_PIN_4|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_SAI2;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pins : FDCAN2_RX_Pin FDCAN2_TX_Pin */
  GPIO_InitStruct.Pin = FDCAN2_RX_Pin|FDCAN2_TX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PK5 PK4 PK6 PK3
                           PK7 PK2 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_4|GPIO_PIN_6|GPIO_PIN_3
                          |GPIO_PIN_7|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOK, &GPIO_InitStruct);

  /*Configure GPIO pin : PG10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_SAI2;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : SDIO1_D2_Pin SDIO1_D3_Pin SDIO1_CK_Pin SDIO1_D0_Pin
                           SDIO1_D1_Pin SDIO1_D7_Pin SDIO1_D6_Pin */
  GPIO_InitStruct.Pin = SDIO1_D2_Pin|SDIO1_D3_Pin|SDIO1_CK_Pin|SDIO1_D0_Pin
                          |SDIO1_D1_Pin|SDIO1_D7_Pin|SDIO1_D6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PI1 PI0 PI9 PI12
                           PI14 PI15 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_0|GPIO_PIN_9|GPIO_PIN_12
                          |GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pins : PE1 PE0 PE10 PE9
                           PE11 PE12 PE15 PE8
                           PE13 PE7 PE14 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_0|GPIO_PIN_10|GPIO_PIN_9
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15|GPIO_PIN_8
                          |GPIO_PIN_13|GPIO_PIN_7|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF7_SPI2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : MII_TX_EN_Pin MII_TXD1_Pin MII_TXD0_Pin */
  GPIO_InitStruct.Pin = MII_TX_EN_Pin|MII_TXD1_Pin|MII_TXD0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_DISP_Pin PJ14 PJ12 PJ13
                           PJ11 PJ10 PJ9 PJ0
                           PJ8 PJ7 PJ6 PJ1
                           PJ5 PJ3 PJ4 */
  GPIO_InitStruct.Pin = LCD_DISP_Pin|GPIO_PIN_14|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_11|GPIO_PIN_10|GPIO_PIN_9|GPIO_PIN_0
                          |GPIO_PIN_8|GPIO_PIN_7|GPIO_PIN_6|GPIO_PIN_1
                          |GPIO_PIN_5|GPIO_PIN_3|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

  /*Configure GPIO pin : PD3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PI2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : PH15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : FDCAN1_RX_Pin FDCAN1_TX_Pin */
  GPIO_InitStruct.Pin = FDCAN1_RX_Pin|FDCAN1_TX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pin : MII_TXD3_Pin */
  GPIO_InitStruct.Pin = MII_TXD3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(MII_TXD3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_DISPD7_Pin */
  GPIO_InitStruct.Pin = LCD_DISPD7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LCD_DISPD7_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PE5 PE4 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_SAI4;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : SDIO1_D5_Pin SDIO1_D4_Pin */
  GPIO_InitStruct.Pin = SDIO1_D5_Pin|SDIO1_D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PG15 PG8 PG5 PG4
                           PG0 PG1 */
  GPIO_InitStruct.Pin = GPIO_PIN_15|GPIO_PIN_8|GPIO_PIN_5|GPIO_PIN_4
                          |GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : SDIO1_CMD_Pin */
  GPIO_InitStruct.Pin = SDIO1_CMD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_SDIO1;
  HAL_GPIO_Init(SDIO1_CMD_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PD0 PD1 PD15 PD14
                           PD10 PD9 PD8 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_15|GPIO_PIN_14
                          |GPIO_PIN_10|GPIO_PIN_9|GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OTG_FS2_ID_Pin */
  GPIO_InitStruct.Pin = USB_OTG_FS2_ID_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OTG_FS2_ID_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_OTG_FS2_P_Pin USB_OTG_FS2_N_Pin */
  GPIO_InitStruct.Pin = USB_OTG_FS2_P_Pin|USB_OTG_FS2_N_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG1_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : MII_RX_ER_Pin */
  GPIO_InitStruct.Pin = MII_RX_ER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(MII_RX_ER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PF2 PF1 PF0 PF3
                           PF5 PF4 PF13 PF14
                           PF12 PF15 PF11 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_1|GPIO_PIN_0|GPIO_PIN_3
                          |GPIO_PIN_5|GPIO_PIN_4|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_12|GPIO_PIN_15|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_INT_Pin */
  GPIO_InitStruct.Pin = LCD_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LCD_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_BL_Pin */
  GPIO_InitStruct.Pin = LCD_BL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(LCD_BL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PF6 PF7 PF10 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : PC0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PF9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : MII_MDC_Pin MII_TXD2_Pin MII_TX_CLK_Pin MII_RXD0_Pin
                           MII_RXD1_Pin */
  GPIO_InitStruct.Pin = MII_MDC_Pin|MII_TXD2_Pin|MII_TX_CLK_Pin|MII_RXD0_Pin
                          |MII_RXD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : MII_CRS_Pin MII_COL_Pin */
  GPIO_InitStruct.Pin = MII_CRS_Pin|MII_COL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : MII_MDIO_Pin MII_RX_CLK_Pin MII_RX_DV_Pin */
  GPIO_InitStruct.Pin = MII_MDIO_Pin|MII_RX_CLK_Pin|MII_RX_DV_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PH5 PH6 PH7 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : VCP_TX_Pin VCP_RX_Pin */
  GPIO_InitStruct.Pin = VCP_TX_Pin|VCP_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS2_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_FS2_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS2_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF9_TIM13;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PH9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pin : PD11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : MII_RX_D3_Pin MII_RX_D2_Pin */
  GPIO_InitStruct.Pin = MII_RX_D3_Pin|MII_RX_D2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LD1_Pin */
  GPIO_InitStruct.Pin = LD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_RST_Pin */
  GPIO_InitStruct.Pin = LCD_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*AnalogSwitch Config */
  HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA0, SYSCFG_SWITCH_PA0_OPEN);

  /*AnalogSwitch Config */
  HAL_SYSCFG_AnalogSwitchConfig(SYSCFG_SWITCH_PA1, SYSCFG_SWITCH_PA1_OPEN);
}

/* USER CODE BEGIN 4 */

void UpdateBallPosition(int rows) {
	//izbrišemo prejšnjo poticijo
	DrawBall(ball_x, ball_y, UTIL_LCD_COLOR_BLACK);

	//posodobimo koordinate žoge
	ball_x += ball_dx;
	ball_y += ball_dy;

	//preverimo nove koordinate
	CheckCollisions(rows);

	//če zoga pade na tla se igra zaključi
	if(ball_x-BALL_RADIUS <= 0){
		gameOver = true;
		return;
	}

	//narišemo žogo na novih koordinatih
	DrawBall(ball_x, ball_y, UTIL_LCD_COLOR_WHITE);
}

void DrawBall(int16_t x, int16_t y, uint32_t color) {
  UTIL_LCD_SetTextColor(color);
  UTIL_LCD_FillCircle(x, y, BALL_RADIUS, color);
}


void CheckCollisions(int rows) {
  if (ball_x <= BALL_RADIUS || ball_x >= x_size - BALL_RADIUS) {
    ball_dx = -ball_dx;
  }
  if (ball_y <= BALL_RADIUS+BALL_RADIUS/2 || ball_y >= y_size - (BALL_RADIUS+BALL_RADIUS/2)) {
    ball_dy = -ball_dy;
  }


  if(ball_x < 20+RECT_WIDTH+BALL_RADIUS+BALL_RADIUS/2 && ball_y-BALL_RADIUS > playerx - RECT_HEIGHT / 2 && ball_y+BALL_RADIUS < playerx + RECT_HEIGHT / 2) {
	  ball_dx = -ball_dx;
  }


  for (uint16_t i = 0; i < rows; i++) {
  	    for (uint16_t j = 0; j < BRICKS_ROWS; j++) {
  	        if (bricks[i][j]) {

  	        	//ugotovimo, kje se opeka nahaja
  	        	uint16_t brick_x = 458 - BLOCK_WIDTH * (i + 1) - 2 * (i + 1);
  	        	uint16_t brick_y = (2 * (j + 1)) + (BLOCK_HEIGHT * j);

  	        	//premerimo ali se je zgodil trk z njo
  				if (ball_x + BALL_RADIUS > brick_x && ball_x - BALL_RADIUS < brick_x + BLOCK_WIDTH &&
  					ball_y + BALL_RADIUS > brick_y && ball_y - BALL_RADIUS < brick_y + BLOCK_HEIGHT) {

  					//preverimo iz katere strani se je trk zgodil
  					if (ball_x < brick_x || ball_x > brick_x + BLOCK_WIDTH) {
  						ball_dx = -ball_dx;
  					}
  					if (ball_y < brick_y || ball_y > brick_y + BLOCK_HEIGHT) {
  						ball_dy = -ball_dy;
  					}

  					//odstranimo opeko
  					bricks[i][j] = 0;
  					bricksBroken++;
  					UTIL_LCD_FillRect(brick_x, brick_y, BLOCK_WIDTH, BLOCK_HEIGHT, UTIL_LCD_COLOR_BLACK);

  					return;
  				}
  	        }
  	    }
  	}

}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(200);
    //HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
    BSP_LED_On(LED_GREEN);
    osDelay(200);
    //HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
    BSP_LED_Off(LED_GREEN);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  BSP_LED_On(LED_RED);
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
