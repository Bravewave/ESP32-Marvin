// WiFi credentials
#include <marvin-private.h>
#define WIFI_SSID "Bravewave"
#define WIFI_PSWD "minecraftsteve"

// are you using an I2S microphone - comment this out if you want to use an analog mic and ADC input
#define USE_I2S_MIC_INPUT

// I2S Microphone Settings

// Which channel is the I2S microphone on? I2S_CHANNEL_FMT_ONLY_LEFT or I2S_CHANNEL_FMT_ONLY_RIGHT
#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT
// #define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_RIGHT
/*
#define I2S_MIC_SERIAL_CLOCK GPIO_NUM_33
#define I2S_MIC_LEFT_RIGHT_CLOCK GPIO_NUM_26
#define I2S_MIC_SERIAL_DATA GPIO_NUM_25
*/

//#ifdef UNPHONE_SPIN
//#define I2S_MIC_SERIAL_CLOCK GPIO_NUM_32
//#else
#define I2S_MIC_SERIAL_CLOCK GPIO_NUM_13
//#endif

#define I2S_MIC_LEFT_RIGHT_CLOCK GPIO_NUM_15

//#ifdef UNPHONE_SPIN
//#define I2S_MIC_SERIAL_DATA GPIO_NUM_33
//#else
#define I2S_MIC_SERIAL_DATA GPIO_NUM_21
//#endif

// Analog Microphone Settings - ADC1_CHANNEL_7 is GPIO35
#define ADC_MIC_CHANNEL ADC1_CHANNEL_7

// speaker settings
#define I2S_SPEAKER_SERIAL_CLOCK GPIO_NUM_14
#define I2S_SPEAKER_LEFT_RIGHT_CLOCK GPIO_NUM_12
#define I2S_SPEAKER_SERIAL_DATA GPIO_NUM_27

// command recognition settings

#define LED_KITCHEN_PIN GPIO_NUM_4
#define LED_BEDROOM_PIN GPIO_NUM_26
#define LED_TABLE_PIN GPIO_NUM_25
