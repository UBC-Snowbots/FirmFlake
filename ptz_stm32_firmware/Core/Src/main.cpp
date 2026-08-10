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

// #include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <main_c.h>
#include <main.hpp>
#include <app.hpp>


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// PrimitiveDigitalOutputSTM32G4XX usrled0_gpio(pindefs::USRLED_0_PORT, pindefs::USRLED_0_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
// PrimitiveDigitalOutputSTM32G4XX usrled1_gpio(pindefs::USRLED_1_PORT, pindefs::USRLED_1_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
// PrimitiveDigitalOutputSTM32G4XX usrled2_gpio(pindefs::USRLED_2_PORT, pindefs::USRLED_2_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);
// PrimitiveDigitalOutputSTM32G4XX usrled3_gpio(pindefs::USRLED_3_PORT, pindefs::USRLED_3_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL);

ClockSTM32G4XX clock;

// UsrLed usrled0(usrled0_gpio);
// UsrLed usrled1(usrled1_gpio);
// UsrLed usrled2(usrled2_gpio);
// UsrLed usrled3(usrled3_gpio);

// UsrLedControl usr_led_ctl(clock, usrled0, usrled1, usrled2, usrled3);

//TODO find a spot for these
static constexpr uint32_t PWM_CLOCK_PRESCALER = (CLOCK_FREQ_MHz - 1u);
static constexpr uint32_t PWM_PERIOD = (1000u - 1u);
static constexpr uint32_t DUTY_FULL = (PWM_PERIOD + 1u); // 1000 = 100%

FDCAN_HandleTypeDef hfdcan1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
// TIM_HandleTypeDef htim5;

// PwmConfig led_panel_pwm_config{
//   .pwm_clock_prescaler = CLOCK_FREQ_MHz, //1 MHz
//   .ticks_per_pwm_period = 1000u
// };

ServoSTM32G4XX servo_tilt_(pindefs::servo_tilt_hw, htim3, SERVO_PWM_50HZ_16MHZ, SERVO_CONFIG_CONTINUOUS, 0);
ServoSTM32G4XX servo_pan_(pindefs::servo_pan_hw, htim3, SERVO_PWM_50HZ_16MHZ, SERVO_CONFIG_CONTINUOUS, 0);

CanSTM32G4XX can(hfdcan1);

App app(clock, servo_tilt_, servo_pan_, can);

// static void debug_blink(uint8_t n)
// {
//     for (uint8_t i = 0; i < n * 2u; i++)
//     {
//         HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
//         HAL_Delay(120);
//     }
// }

/* Private function prototypes -----------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
void SystemClock_Config(void);
void init_pwm_timers(void); //TODO organize?
static void MX_GPIO_Init(void);
static void MX_FDCAN1_Init(void);


// void StartDefaultTask(void *argument);
#ifdef __cplusplus
}
#endif

int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();

  // MX_FDCAN1_Init();
  // MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  // osKernelInitialize();

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
  // defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  // osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  clock.init();
//   usr_led_ctl.init();  // ← add this
//       usr_led_ctl.turn_on(UsrLedIndex::LED_0); // should turn on after 5s
//       clock.init();
// for(;;){
//   if(clock.get_ms() > 5000)
// {
//     usr_led_ctl.turn_off(UsrLedIndex::LED_0); // should turn on after 5s
// }

// }
// init_pwm_timers();
  app.main();
  // __HAL_RCC_TIM2_CLK_ENABLE();
  // led_panels[0].init();
  // usrled0.init();
  // usr_led_ctl.init();
  // // usr_led_ctl[0].toggle();
  // led_stack_controller.init_all();
  // uint32_t STEP_DELAY_MS = 10;
  for(;;)
  {
    // debug_blink(50);

  //   led_panels[0].set_duty(90.0);
  //   // Code should never reach here. App takes over.
  // // usrled0.toggle();
  // // usr_led_ctl[0].toggle();
  // usr_led_ctl[3].toggle();
  // // usr_led_ctl[2].toggle();
  // // usr_led_ctl[3].toggle();
  

  // HAL_Delay(1000);

  // for (uint32_t d = 0; d <= 100; d += 1)
  //   {
  //     float duty = (float)d / 10.0f; 
  //     led_panels[0].set_duty(duty);
  //     HAL_Delay(STEP_DELAY_MS);
  //   }
  //   usrled0.toggle();
  //           for (int32_t d = 100; d >= 0; d -= (int32_t)1)
  //   {
  //     led_stack_controller.turn_all_off();
  //     HAL_Delay(STEP_DELAY_MS);
  //   }

  //   for (int32_t d = 100; d >= 0; d -= (int32_t)1)
  //   {
  //         float duty = (float)d / 10.0f; 
  //       led_panels[0].set_duty(duty);
  //     HAL_Delay(STEP_DELAY_MS);
  //   }
  }


  // uint32_t STEP_DELAY_MS = 10u;
  // usrled0.init();
  // if(led_stack_controller.init_all() != LedPanelStatus::OK){
  //   for(;;)
  //   {
  //     usrled0.turn_off();
  //   }
  // }
  // while (1)
  // {
  //   usrled0.toggle();




  //   /* USER CODE END WHILE */
  //   // osDelay(100);

  // }
  /* USER CODE END 3 */
}


#ifdef __cplusplus
extern "C" {
#endif
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

// /**
//   * @brief FDCAN1 Initialization Function
//   * @param None
//   * @retval None
//   */
// static void MX_FDCAN1_Init(void)
// {

//   /* USER CODE BEGIN FDCAN1_Init 0 */

//   /* USER CODE END FDCAN1_Init 0 */

//   /* USER CODE BEGIN FDCAN1_Init 1 */

//   /* USER CODE END FDCAN1_Init 1 */
//   hfdcan1.Instance = FDCAN1;
//   hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
//   hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
//   hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
//   hfdcan1.Init.AutoRetransmission = DISABLE;
//   hfdcan1.Init.TransmitPause = DISABLE;
//   hfdcan1.Init.ProtocolException = DISABLE;
//   hfdcan1.Init.NominalPrescaler = 16;
//   hfdcan1.Init.NominalSyncJumpWidth = 1;
//   hfdcan1.Init.NominalTimeSeg1 = 2;
//   hfdcan1.Init.NominalTimeSeg2 = 2;
//   hfdcan1.Init.DataPrescaler = 1;
//   hfdcan1.Init.DataSyncJumpWidth = 1;
//   hfdcan1.Init.DataTimeSeg1 = 1;
//   hfdcan1.Init.DataTimeSeg2 = 1;
//   hfdcan1.Init.StdFiltersNbr = 0;
//   hfdcan1.Init.ExtFiltersNbr = 0;
//   hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
//   if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
//   {
//     Error_Handler();
//   }
//   /* USER CODE BEGIN FDCAN1_Init 2 */

//   /* USER CODE END FDCAN1_Init 2 */

// }

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_ADC12_CLK_ENABLE();   // ADC1 and ADC2 share this enable bit
  // __HAL_RCC_ADC345_CLK_ENABLE();   // ADC3, ADC4, ADC5 share this enable bit

/* USER CODE BEGIN MX_GPIO_Init_2 */

/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
// void StartDefaultTask(void *argument)
// {
//   /* USER CODE BEGIN 5 */
//   /* Infinite loop */
//   for(;;)
//   {
//     osDelay(1);
//   }
//   /* USER CODE END 5 */
// }

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
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
// usr_led_ctl.init();
  while (1)
  {
    // usr_led_ctl.blink_led(UsrLedIndex::LED_1, 500);
    // usr_led_ctl.update();
    HAL_Delay(10);
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

#ifdef __cplusplus
}
#endif
