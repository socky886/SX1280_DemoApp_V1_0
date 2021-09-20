/*!
 * \file      hw.c
 *
 * \brief     Hardware driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Semtech
 */

#include "hw.h"

/***************************************************************************\
 * RAM data
\***************************************************************************/
extern void uartCallback( uint8_t uartEvent, uint8_t status, void *param );

/***************************************************************************\
 *  External functions definition
\***************************************************************************/

void HwInit( void )
{
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init( );

    /* Configure the system clock */
    SystemClock_Config( );

    /* Initialize all configured peripherals */
    GpioInit( );
    //MX_RTC_Init( );

    UartInit( uartCallback );
    SpiInit( );

    // I2cInit( I2C_1 );
	
	// ----------------------------------------------------------------
	// SX9306 Capacitive Touch Sensor
    // SX9306ProximityInit( );
}


void HwSetLowPower( void )
{
    SpiDeInit( );
    UartDeInit( );
    GpioDeInit( );

#ifdef STM32L4XX_FAMILY
	/* Enter Stop Mode */
    HAL_PWREx_EnterSTOP2Mode( PWR_STOPENTRY_WFE );
#else
    /* Enter Stop Mode ==> this function name is different on every board*/
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFE);
#endif
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config( void )
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Configure LSE Drive Capability
    */
    __HAL_RCC_LSEDRIVE_CONFIG( RCC_LSEDRIVE_LOW );

    /**Initializes the CPU, AHB and APB busses clocks
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = 16;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 8;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;

    if( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK )
    {
        _Error_Handler( __FILE__, __LINE__ );
    }

    /**Initializes the CPU, AHB and APB busses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if( HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3 ) != HAL_OK )
    {
        _Error_Handler( __FILE__, __LINE__ );
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_I2C1;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;

    if( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInit ) != HAL_OK )
    {
        _Error_Handler( __FILE__, __LINE__ );
    }

    /**Configure the main internal regulator output voltage
    */
    if( HAL_PWREx_ControlVoltageScaling( PWR_REGULATOR_VOLTAGE_SCALE1 ) != HAL_OK )
    {
        _Error_Handler( __FILE__, __LINE__ );
    }

    /**Configure the Systick interrupt time
    */
    HAL_SYSTICK_Config( HAL_RCC_GetHCLKFreq( ) / 1000 );

    /**Configure the Systick
    */
    HAL_SYSTICK_CLKSourceConfig( SYSTICK_CLKSOURCE_HCLK );

    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority( SysTick_IRQn, 0, 0 );
}

/**
  * @brief Provide accurate delay (in milliseconds) based on variable incremented.
  * @note In the default implementation , SysTick timer is the source of time base.
  *       It is used to generate interrupts at regular time intervals where uwTick
  *       is incremented.
  * @note This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @param Delay: specifies the delay time length, in milliseconds.
  * @retval None
  */
void HAL_Delay( uint32_t Delay )
{
    uint32_t tickstart = 0;
    tickstart = HAL_GetTick( );
    while( ( HAL_GetTick( ) - tickstart ) < Delay );
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
    while( 1 )
    {

    }
}

void uartCallback( uint8_t uartEvent, uint8_t status, void *param ){

}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed( uint8_t* file, uint32_t line )
{
    _Error_Handler( file, line );
}

#endif
