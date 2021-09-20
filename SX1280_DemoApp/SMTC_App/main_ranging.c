/*!
 * \file      main-RangingDemo.c
 *
 * \brief     Main Program body of the Ranging demo.
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


/* Includes ------------------------------------------------------------------*/

#include "hw.h"
#include "radio.h"
#include "sx1280.h"
#include "radio.h"
#include "utilities.h"

#include "demoRanging.h"
#include "RangingDisplay.h"


/****************************************************************************\
 * CONSTANT and MACRO definition
\****************************************************************************/

#define DEMO_SETTING_ENTITY                          MASTER

#define PROXBUFFERSIZE                                  20


/***************************************************************************\
 * RAM data
\***************************************************************************/

/***************************************************************************\
 * Local function Prototype
\***************************************************************************/

/***************************************************************************\
 * Ranging Demo Main function
\***************************************************************************/

int main( void )
{
	uint16_t FwVersion=0;
	
    HwInit( );
	
	GpioWrite(LED_TX_PORT, LED_TX_PIN, 1);
    GpioWrite(LED_RX_PORT, LED_RX_PIN, 1);


    /* 1- Initialize the Ranging Application */
    RangingDemoInitApplication( DEMO_SETTING_ENTITY );

    RangingDemoSetRangingParameters( 40u, DEMO_RNG_ADDR_1, DEMO_RNG_ANT_1, DEMO_RNG_UNIT_SEL_M );
    RangingDemoSetRadioParameters( LORA_SF6, LORA_BW_1600, LORA_CR_4_5, DEMO_CENTRAL_FREQ_PRESET2, DEMO_POWER_TX_MAX );
	
	Radio.Reset();
	FwVersion = Radio.GetFirmwareVersion();
		
	if(DEMO_SETTING_ENTITY == MASTER)
	{
		printf("Ranging Demo as Master , firmware %d \n\r",FwVersion);
	}
	else
	{
		printf("Ranging Demo as Slave , firmware %d  \n\r",FwVersion);
	}
	
	GpioWrite(LED_TX_PORT, LED_TX_PIN, 0);
    GpioWrite(LED_RX_PORT, LED_RX_PIN, 0);

    /* Infinite loop */
    while(1){

        RangingDemoStatus_t demoStatus;
		 GpioWrite(LED_TX_PORT, LED_TX_PIN, 1);
        // Run the ranging demo.
        do{
            demoStatus = RangingDemoRun( );
        }while( demoStatus == DEMO_RANGING_RUNNING );

		GpioWrite(LED_TX_PORT, LED_TX_PIN, 0);
        // If master, display the ranging result.
        if( DEMO_SETTING_ENTITY == MASTER ){
            RangingDisplayUartOutputData( );
            RangingDisplayUartOutputDistance( );
			HAL_Delay(1000);
        }

        if( demoStatus != DEMO_RANGING_TERMINATED ){
            RangingDemoReset( );
        }

    }
}

/***************************************************************************\
 * Local Function Definition
\***************************************************************************/


