#include <Arduino.h>
#include <WiFi.h>
#include <driver/i2s.h>
#include <esp_task_wdt.h>
#include <Wire.h>
#include "I2SMicSampler.h"
#include "ADCSampler.h"
#include "I2SOutput.h"
#include "config.h"
#include "Application.h"
#include "SPIFFS.h"
#include "IntentProcessor.h"
#include "Speaker.h"
#include "IndicatorLight.h"
#include <WiFiMulti.h>
#include <dotstar_wing.h>

// MAC address etc. //////////////////////////////////////////////////////////
void sayHi();
extern char MAC_ADDRESS[];
void getMAC(char *);
char MAC_ADDRESS[13]; // MAC addresses are 12 chars, plus the NULL terminator

WiFiMulti wifiMulti;

// i2s config for using the internal ADC
i2s_config_t adcI2SConfig = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_LSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};

// i2s config for reading from both channels of I2S
i2s_config_t i2sMemsConfigBothChannels = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_MIC_CHANNEL,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0};

// i2s microphone pins
i2s_pin_config_t i2s_mic_pins = {
    .bck_io_num = I2S_MIC_SERIAL_CLOCK,
    .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_MIC_SERIAL_DATA};

// i2s speaker pins
i2s_pin_config_t i2s_speaker_pins = {
    .bck_io_num = I2S_SPEAKER_SERIAL_CLOCK,
    .ws_io_num = I2S_SPEAKER_LEFT_RIGHT_CLOCK,
    .data_out_num = I2S_SPEAKER_SERIAL_DATA,
    .data_in_num = I2S_PIN_NO_CHANGE};

// This task does all the heavy lifting for our application
void applicationTask(void *param)
{
  Application *application = static_cast<Application *>(param);

  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);
  while (true)
  {
    // wait for some audio samples to arrive
    uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, xMaxBlockTime);
    if (ulNotificationValue > 0)
    {
      application->run();
    }
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting up");

  wifiMulti.addAP(_MULTI_SSID1, _MULTI_KEY1);
  wifiMulti.addAP(_MULTI_SSID2, _MULTI_KEY2);
  wifiMulti.addAP(_MULTI_SSID3, _MULTI_KEY3);
  wifiMulti.addAP(_MULTI_SSID4, _MULTI_KEY4);
  wifiMulti.addAP(_MULTI_SSID5, _MULTI_KEY5);
  wifiMulti.addAP(_MULTI_SSID6, _MULTI_KEY6);

  for(int i = 0; i<10; i++) {
    if ((wifiMulti.run() == WL_CONNECTED)) {
      Serial.println("connected to WiFi");
    } else {
      Serial.println("not connected to WiFi");
      delay(200);
    }
  }

  // Try to mount SPIFFS, format if it fails
  if (!SPIFFS.begin(true, "/spiffs", 13)) {
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
  }

  // Check if we can create and read a file
  File testFile = SPIFFS.open("/test.txt", FILE_WRITE);
  if (!testFile) {
      Serial.println("Failed to create test file");
      return;
  }
  testFile.println("Hello SPIFFS");
  testFile.close();

  testFile = SPIFFS.open("/test.txt", FILE_READ);
  if (!testFile) {
      Serial.println("Failed to read test file");
      return;
  }

  while (testFile.available()) {
      Serial.write(testFile.read());
  }
  testFile.close();

  Serial.println("\nSPIFFS is mounted and working correctly");

  sayHi();

  Serial.printf("ssid=%s\n", WIFI_SSID);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.printf("Total heap: %d\n", ESP.getHeapSize());
  Serial.printf("Free heap: %d\n", ESP.getFreeHeap());

  // make sure we don't get killed for our long running tasks
  esp_task_wdt_init(10, false);

  // start up the I2S input (from either an I2S microphone or Analogue microphone via the ADC)
#ifdef USE_I2S_MIC_INPUT
  // Direct i2s input from INMP441 or the SPH0645
  I2SSampler *i2s_sampler = new I2SMicSampler(i2s_mic_pins, false);
#else
  // Use the internal ADC
  I2SSampler *i2s_sampler = new ADCSampler(ADC_UNIT_1, ADC_MIC_CHANNEL);
#endif

  // start the i2s speaker output
  I2SOutput *i2s_output = new I2SOutput();
  i2s_output->start(I2S_NUM_1, i2s_speaker_pins);
  Speaker *speaker = new Speaker(i2s_output);

  // indicator light to show when we are listening
  IndicatorLight *indicator_light = new IndicatorLight();

  // and the intent processor
  IntentProcessor *intent_processor = new IntentProcessor(speaker);
  intent_processor->addDevice("kitchen", LED_KITCHEN_PIN);
  // bedroom works on bare feather but not unphone due to pin clash
  intent_processor->addDevice("bedroom", LED_BEDROOM_PIN);
  intent_processor->addDevice("table", LED_TABLE_PIN);

  // create our application
  Application *application = new Application(i2s_sampler, intent_processor, speaker, indicator_light);

  // set up the i2s sample writer task
  TaskHandle_t applicationTaskHandle;
  xTaskCreate(applicationTask, "Application Task", 8192, application, 1, &applicationTaskHandle);

  // start sampling from i2s device - use I2S_NUM_0 as that's the one that supports the internal ADC
#ifdef USE_I2S_MIC_INPUT
  i2s_sampler->start(I2S_NUM_0, i2sMemsConfigBothChannels, applicationTaskHandle);
#else
  i2s_sampler->start(I2S_NUM_0, adcI2SConfig, applicationTaskHandle);
#endif

  // init the dotstar wing if required
#ifdef RUN_DOTSTAR_WING
  dotstar_wing_setup();
#endif

  // unphone
//#ifdef UNPHONE_SPIN
  pinMode(2 /*EXPANDER_POWER*/, OUTPUT);
  digitalWrite(2 /*EXPANDER_POWER*/, HIGH);
//#endif
}

void loop()
{
  // run the dotstar wing if required
#ifdef RUN_DOTSTAR_WING
  if(dotstar_wing_state_on()) dotstar_wing_loop();
#else
  vTaskDelay(1000);
#endif
}

void getMAC(char *buf) { // the MAC is 6 bytes, so needs careful conversion...
  uint64_t mac = ESP.getEfuseMac(); // ...to string (high 2, low 4):
  char rev[13];
  sprintf(rev, "%04X%08X", (uint16_t) (mac >> 32), (uint32_t) mac);

  // the byte order in the ESP has to be reversed relative to normal Arduino
  for(int i=0, j=11; i<=10; i+=2, j-=2) {
    buf[i] = rev[j - 1];
    buf[i + 1] = rev[j];
  }
  buf[12] = '\0';
}

void sayHi() {
  printf("\nHi there.\n");
  getMAC(MAC_ADDRESS);            // store the MAC address as a chip identifier
  Serial.printf("ESP32 MAC = %s\n", MAC_ADDRESS); // print the ESP's "ID"

  #ifdef ARDUINO_FEATHER_ESP32
    printf("ARDUINO_FEATHER_ESP32 is defined\n");
  #endif

  #ifdef ESP_IDF_VERSION_MAJOR
    printf( // IDF version
      "IDF version: %d.%d.%d\n",
      ESP_IDF_VERSION_MAJOR, ESP_IDF_VERSION_MINOR, ESP_IDF_VERSION_PATCH
    );
  #endif
  #ifdef ESP_ARDUINO_VERSION_MAJOR
    printf(
      "ESP_ARDUINO_VERSION_MAJOR=%d; MINOR=%d; PATCH=%d\n",
      ESP_ARDUINO_VERSION_MAJOR, ESP_ARDUINO_VERSION_MINOR,
      ESP_ARDUINO_VERSION_PATCH
    );
  #endif

  #ifdef ARDUINO_ARCH_ESP32
    printf("ARDUINO_ARCH_ESP32 is defined\n");
  #endif
  #ifdef ESP_PLATFORM
    printf("ESP_PLATFORM is defined\n");
  #endif
  #ifdef ESP32
    printf("ESP32 is defined\n");
  #endif
  #ifdef IDF_VER
    printf("IDF_VER=%s\n", IDF_VER);
  #endif
  #ifdef ARDUINO
    printf("ARDUINO=%d\n", ARDUINO);
  #endif
  #ifdef ARDUINO_BOARD
    printf("ARDUINO_BOARD=%s\n", ARDUINO_BOARD);
  #endif
  #ifdef ARDUINO_VARIANT
    printf("ARDUINO_VARIANT=%s\n", ARDUINO_VARIANT);
  #endif

  #ifdef ARDUINO_IDE_BUILD
    printf("ARDUINO_IDE_BUILD is defined\n");
  #else
    printf("no definition of ARDUINO_IDE_BUILD\n");
  #endif

  printf("Marvin mic and speaker pins:\n");
  printf("I2S_MIC_SERIAL_CLOCK=%d\n",           I2S_MIC_SERIAL_CLOCK);
  printf("I2S_MIC_LEFT_RIGHT_CLOCK=%d\n",       I2S_MIC_LEFT_RIGHT_CLOCK);
  printf("I2S_MIC_SERIAL_DATA=%d\n",            I2S_MIC_SERIAL_DATA);
  printf("I2S_SPEAKER_SERIAL_CLOCK=%d\n",       I2S_SPEAKER_SERIAL_CLOCK);
  printf("I2S_SPEAKER_LEFT_RIGHT_CLOCK=%d\n",   I2S_SPEAKER_LEFT_RIGHT_CLOCK);
  printf("I2S_SPEAKER_SERIAL_DATA=%d\n",        I2S_SPEAKER_SERIAL_DATA);
  printf("LED_KITCHEN_PIN=%d\n",                LED_KITCHEN_PIN);
  printf("LED_BEDROOM_PIN=%d\n",                LED_BEDROOM_PIN);
  printf("LED_TABLE_PIN=%d\n",                  LED_TABLE_PIN);
}
