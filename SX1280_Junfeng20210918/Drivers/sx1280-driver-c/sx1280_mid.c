#include "sx1280_mid.h"
#include "sx1280-hal.h"
#include "radio.h"
#include <string.h>
#include <stdio.h>

/*!
 * \brief Used to display firmware version UART flow
 */
#define FIRMWARE_VERSION    ( ( char* )"Firmware Version: 5709de2d" )

/*!
 * Select mode of operation for the Ping Ping application
 */
//#define MODE_BLE
// #define MODE_LORA
#define MODE_GFSK
//#define MODE_FLRC


#define RF_BL_ADV_CHANNEL_38             			2426000000 // Hz
#define RF_BL_ADV_CHANNEL_0                     	2404000000 // Hz

/*!
 * \brief Defines the nominal frequency
 */
#define RF_FREQUENCY                                RF_BL_ADV_CHANNEL_0 // Hz

/*!
 * \brief Defines the output power in dBm
 *
 * \remark The range of the output power is [-18..+13] dBm
 */
#define TX_OUTPUT_POWER                             13

/*!
 * \brief Defines the buffer size, i.e. the payload size
 */
#define BUFFER_SIZE                                 20

/*!
 * \brief Number of tick size steps for tx timeout
 */
#define TX_TIMEOUT_VALUE                            10000 // ms

/*!
 * \brief Number of tick size steps for rx timeout
 */
// #define RX_TIMEOUT_VALUE                            1000 // ms
#define RX_TIMEOUT_VALUE                            5000 // ms

/*!
 * \brief Size of ticks (used for Tx and Rx timeout)
 */
#define RX_TIMEOUT_TICK_SIZE                        RADIO_TICK_SIZE_1000_US

/*!
 * \brief Defines the size of the token defining message type in the payload
 */
#define PINGPONGSIZE                                4


/*!
 * \brief Defines the states of the application
 */
typedef enum
{
    APP_LOWPOWER,
    APP_RX,
    APP_RX_TIMEOUT,
    APP_RX_ERROR,
    APP_TX,
    APP_TX_TIMEOUT,
}AppStates_t;


/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void OnTxDone( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void OnRxDone( void );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void OnRxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Error event
 */
void OnRxError( IrqErrorCode_t );


/*!
 * \brief Define the possible message type for this application
 */
const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

/*!
 * \brief All the callbacks are stored in a structure
 */
RadioCallbacks_t Callbacks =
{
    &OnTxDone,        // txDone
    &OnRxDone,        // rxDone
    NULL,             // syncWordDone
    NULL,             // headerDone
    &OnTxTimeout,     // txTimeout
    &OnRxTimeout,     // rxTimeout
    &OnRxError,       // rxError
    NULL,             // rangingDone
    NULL,             // cadDone
};

/*!
 * \brief The size of the buffer
 */
uint8_t BufferSize = BUFFER_SIZE;

/*!
 * \brief The buffer
 */
uint8_t Buffer[BUFFER_SIZE];

/*!
 * \brief Mask of IRQs to listen to in rx mode
 */
uint16_t RxIrqMask = IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT;

/*!
 * \brief Mask of IRQs to listen to in tx mode
 */
uint16_t TxIrqMask = IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT;

/*!
 * \brief The State of the application
 */
AppStates_t AppState = APP_LOWPOWER;

#if defined( MODE_BLE )
/*!
 * \brief In case of BLE, the payload must contain the header
 */
typedef union
{
    struct BleAdvHeaderField_s
    {
        uint8_t pduType: 4;
        uint8_t rfu1:2;
        uint8_t txAddr:1;
        uint8_t rxAddr:1;
        uint8_t length:6;
        uint8_t rfu2:2;
    } Fields;
    uint8_t Serial[ 2 ];
}BleAdvHeaders_t;
BleAdvHeaders_t ble_header_adv;
#endif // MODE_BLE

PacketParams_t packetParams;

PacketStatus_t packetStatus;

ModulationParams_t modulationParams;

void sx1280_app_init(void)
{
     Radio.Init( &Callbacks );
    Radio.SetRegulatorMode( USE_DCDC ); // Can also be set in LDO mode but consume more power
    memset( &Buffer, 0x00, BufferSize );

    printf( "\n\n\r     SX1280 Ping Pong Demo Application. %s\n\n\r", FIRMWARE_VERSION );
    printf( "\n\n\r     Radio firmware version 0x%x\n\n\r", Radio.GetFirmwareVersion( ) );

#if defined( MODE_BLE )

    printf( "\nPing Pong running in BLE mode\n\r" );
    modulationParams.PacketType = PACKET_TYPE_BLE;
    modulationParams.Params.Ble.BitrateBandwidth = GFSK_BLE_BR_1_000_BW_1_2;
    modulationParams.Params.Ble.ModulationIndex = GFSK_BLE_MOD_IND_0_50;
    modulationParams.Params.Ble.ModulationShaping = RADIO_MOD_SHAPING_BT_0_5;

    packetParams.PacketType = PACKET_TYPE_BLE;
    packetParams.Params.Ble.BlePacketType = BLE_EYELONG_1_0;
    packetParams.Params.Ble.ConnectionState = BLE_PAYLOAD_LENGTH_MAX_37_BYTES;
    packetParams.Params.Ble.CrcField = BLE_CRC_3B;
    packetParams.Params.Ble.Whitening = RADIO_WHITENING_ON;

#elif defined( MODE_GFSK )

    printf( "\nPing Pong running in GFSK mode\n\r" );
    modulationParams.PacketType = PACKET_TYPE_GFSK;
    modulationParams.Params.Gfsk.BitrateBandwidth = GFSK_BLE_BR_0_125_BW_0_3;
    modulationParams.Params.Gfsk.ModulationIndex = GFSK_BLE_MOD_IND_1_00;
    modulationParams.Params.Gfsk.ModulationShaping = RADIO_MOD_SHAPING_BT_1_0;

    packetParams.PacketType = PACKET_TYPE_GFSK;
    packetParams.Params.Gfsk.PreambleLength = PREAMBLE_LENGTH_32_BITS;
    packetParams.Params.Gfsk.SyncWordLength = GFSK_SYNCWORD_LENGTH_5_BYTE;
    packetParams.Params.Gfsk.SyncWordMatch = RADIO_RX_MATCH_SYNCWORD_1;
    packetParams.Params.Gfsk.HeaderType = RADIO_PACKET_VARIABLE_LENGTH;
    packetParams.Params.Gfsk.PayloadLength = BUFFER_SIZE;
    packetParams.Params.Gfsk.CrcLength = RADIO_CRC_3_BYTES;
    packetParams.Params.Gfsk.Whitening = RADIO_WHITENING_ON;

#elif defined( MODE_LORA )

    printf( "\nPing Pong running in LORA mode\n\r" );
    modulationParams.PacketType = PACKET_TYPE_LORA;
    modulationParams.Params.LoRa.SpreadingFactor = LORA_SF12;
    modulationParams.Params.LoRa.Bandwidth = LORA_BW_1600;
    modulationParams.Params.LoRa.CodingRate = LORA_CR_LI_4_7;

    packetParams.PacketType = PACKET_TYPE_LORA;
    packetParams.Params.LoRa.PreambleLength = 12;
    packetParams.Params.LoRa.HeaderType = LORA_PACKET_VARIABLE_LENGTH;
    packetParams.Params.LoRa.PayloadLength = BUFFER_SIZE;
    packetParams.Params.LoRa.CrcMode = LORA_CRC_ON;
    packetParams.Params.LoRa.InvertIQ = LORA_IQ_NORMAL;

#elif defined( MODE_FLRC )

    printf( "\nPing Pong running in FLRC mode\n\r" );
    modulationParams.PacketType = PACKET_TYPE_FLRC;
    modulationParams.Params.Flrc.BitrateBandwidth = FLRC_BR_0_260_BW_0_3;
    modulationParams.Params.Flrc.CodingRate = FLRC_CR_1_2;
    modulationParams.Params.Flrc.ModulationShaping = RADIO_MOD_SHAPING_BT_1_0;

    packetParams.PacketType = PACKET_TYPE_FLRC;
    packetParams.Params.Flrc.PreambleLength = PREAMBLE_LENGTH_32_BITS;
    packetParams.Params.Flrc.SyncWordLength = FLRC_SYNCWORD_LENGTH_4_BYTE;
    packetParams.Params.Flrc.SyncWordMatch = RADIO_RX_MATCH_SYNCWORD_1;
    packetParams.Params.Flrc.HeaderType = RADIO_PACKET_VARIABLE_LENGTH;
    packetParams.Params.Flrc.PayloadLength = BUFFER_SIZE;
    packetParams.Params.Flrc.CrcLength = RADIO_CRC_3_BYTES;
    packetParams.Params.Flrc.Whitening = RADIO_WHITENING_OFF;

#else
#error "Please select the mode of operation for the Ping Ping demo"
#endif

    Radio.SetStandby( STDBY_RC );
    Radio.SetPacketType( modulationParams.PacketType );
    Radio.SetModulationParams( &modulationParams );
    Radio.SetPacketParams( &packetParams );
    Radio.SetRfFrequency( RF_FREQUENCY );
    Radio.SetBufferBaseAddresses( 0x00, 0x00 );
    Radio.SetTxParams( TX_OUTPUT_POWER, RADIO_RAMP_02_US );
    
    SX1280SetPollingMode( );

    #if defined( MODE_BLE )
        // only used in GENERIC and BLE mode
        Radio.SetSyncWord( 1, ( uint8_t[] ){ 0xDD, 0xA0, 0x96, 0x69, 0xDD } );
        Radio.WriteRegister(0x9c7, 0x55 );
        Radio.WriteRegister(0x9c8, 0x55 );
        Radio.WriteRegister(0x9c9, 0x55 );
        //Radio.WriteRegister( 0x9c5, 0x33 );
        Radio.SetBleAdvertizerAccessAddress( );
        Radio.SetWhiteningSeed( 0x33 );
        ble_header_adv.Fields.length = PINGPONGSIZE + 2;
        ble_header_adv.Fields.pduType = 2;
    #endif // MODE_BLE

    Radio.SetDioIrqParams( RxIrqMask, RxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE );

    Radio.SetStandby(STDBY_XOSC);

}

void sx1280_app_tx(void)
{
     uint16_t irqRegs;
     //Radio.SetStandby(STDBY_RC);
     Radio.SetStandby(STDBY_XOSC);
    Radio.SetDioIrqParams(TxIrqMask, TxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE);
    memcpy( Buffer, PingMsg, PINGPONGSIZE );
    BufferSize=4;
    Radio.SendPayload(Buffer, BufferSize, (TickTime_t){RX_TIMEOUT_TICK_SIZE, TX_TIMEOUT_VALUE});
    AppState = APP_LOWPOWER;
    while (1)
    {
        //polling the register to confirm the txdone
        
        irqRegs = SX1280GetIrqStatus( );
        // SX1280ClearIrqStatus( IRQ_RADIO_ALL );
        SX1280ClearIrqStatus( irqRegs );

        if(irqRegs&IRQ_TX_DONE == IRQ_TX_DONE)
        {
            printf("-----the IRQ status is 0x%04x\n",irqRegs);
            printf("tx finished\n");
            break;
        }

        if(irqRegs&IRQ_RX_TX_TIMEOUT==IRQ_RX_TX_TIMEOUT)
        {
            printf("tx timeout\n");
            break;
        }
        HAL_Delay(10);
        // printf("the IRQ status is 0x%04x\n",irqRegs);
        //SX1280ProcessIrqs( );
        // if(AppState == APP_TX)
        // {
        //     printf("tx finished\n");
        //     break;
        // }
        // else if(AppState == APP_TX_TIMEOUT)
        // {
        //     break;
        // }
        // else
        // {
        //     ;//break;
        // }

    }
    
}
void sx1280_app_rx(void)
{
    uint16_t irqRegs;
    int i;
    // Radio.SetStandby(STDBY_RC);
    Radio.SetDioIrqParams(RxIrqMask, RxIrqMask, IRQ_RADIO_NONE, IRQ_RADIO_NONE);
    Radio.SetStandby(STDBY_XOSC);
    
    Radio.SetRx((TickTime_t){RX_TIMEOUT_TICK_SIZE, RX_TIMEOUT_VALUE});
    AppState = APP_LOWPOWER;
    while (1)
    {
         irqRegs = SX1280GetIrqStatus( );
        // SX1280ClearIrqStatus( IRQ_RADIO_ALL );
        SX1280ClearIrqStatus( irqRegs );
         printf("-----the IRQ status is 0x%04x\n",irqRegs);
        if(irqRegs&IRQ_RX_DONE == IRQ_RX_DONE)
        {
            printf("-----the IRQ status is 0x%04x\n",irqRegs);
            printf("rx finished\n");
            Radio.GetPayload(Buffer, &BufferSize, BUFFER_SIZE);
            for(i=0;i<BufferSize;i++)
            {
                printf("%c,",Buffer[i]);
            }
            printf("\n");
            SX1280GetPacketStatus(&packetStatus);
            break;
        }

        if(irqRegs&IRQ_RX_TX_TIMEOUT==IRQ_RX_TX_TIMEOUT)
        {
            printf("Rx timeout\n");
            break;
        }
        HAL_Delay(10);
        //polling the register to confirm the rxdone
        // SX1280ProcessIrqs();
        // if (AppState == APP_RX)
        // {
        //     Radio.GetPayload(Buffer, &BufferSize, BUFFER_SIZE);
        //     SX1280GetPacketStatus(&packetStatus);
        //     printf("receive packet successfully");
        //     break;
        // }
    }
}
void OnTxDone( void )
{
    AppState = APP_TX;
}

void OnRxDone( void )
{
    AppState = APP_RX;
}

void OnTxTimeout( void )
{
    AppState = APP_TX_TIMEOUT;
    printf( "<>>>>>>>>TXE\n\r" ); 
}

void OnRxTimeout( void )
{
    AppState = APP_RX_TIMEOUT;
}

void OnRxError( IrqErrorCode_t errorCode )
{
    AppState = APP_RX_ERROR;
    printf( "RXE<>>>>>>>>\n\r" ); 
}

void OnRangingDone( IrqRangingCode_t val )
{
}

void OnCadDone( bool channelActivityDetected )
{
}
