#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <esp_err.h>
#include <string.h>
#include <math.h>
#include <bh1750.h>
#include <dht.h>

#define ADDR BH1750_ADDR_LO
#define I2C_MASTER_SCL 22
#define I2C_MASTER_SDA 21
#define DHT_TYPE_AM2301 1
#define DHT11_PIN 2

// Instatiate shared memory variables
uint16_t lux = 0;
float temperature = 0, humidity = 0;

// Instatiate mutexes handlers
static SemaphoreHandle_t luminosity_mutex = NULL;
static SemaphoreHandle_t temperature_mutex = NULL;

// Declare RTOS task functions
void read_luminosity_measure(void *arg) {
  bool read_state;
  i2c_dev_t dev;
  memset(&dev, 0, sizeof(i2c_dev_t)); // Allocate I2C zero descriptor

  ESP_ERROR_CHECK(bh1750_init_desc(&dev, ADDR, 0, I2C_MASTER_SDA, I2C_MASTER_SCL)); // Initialize I2C descriptor
  ESP_ERROR_CHECK(bh1750_setup(&dev, BH1750_MODE_CONTINUOUS, BH1750_RES_HIGH)); // Setup BH1750 sensor

  while(1) // Infinite task
  {
    xSemaphoreTake(luminosity_mutex, portMAX_DELAY); // Protect shared variables region

    read_state = bh1750_read(&dev, &lux); // Get updated sensor data

    if (read_state != ESP_OK)
      printf("Could not read luminosity data\n"); // Report reading problems

    xSemaphoreGive(luminosity_mutex); // Free from protection

    vTaskDelay(pdMS_TO_TICKS(100)); // ~100ms delay
  }
}

void read_temperature_humidity_measure(void *arg) {
  bool read_state;
  gpio_set_pull_mode(DHT11_PIN, GPIO_PULLUP_ONLY);

  while(1) // Infinite task
  {
    xSemaphoreTake(temperature_mutex, portMAX_DELAY); // Protect shared variables region

    read_state = dht_read_float_data(DHT_TYPE_AM2301, DHT11_PIN, &humidity, &temperature); // Get updated sensor data

    if (read_state != ESP_OK)
      printf("Could not read temperature/humidity data\n"); // Report reading problems

    xSemaphoreGive(temperature_mutex); // Free from protection

    vTaskDelay(pdMS_TO_TICKS(2000)); // ~2s delay (not too often, so it doesn't heat up)
    // http://www.kandrsmith.org/RJS/Misc/Hygrometers/dht_sht_how_fast.html
  }
}

void send_data(void *arg) {
  while(1) // Infinite task
  {
    xSemaphoreTake(temperature_mutex, portMAX_DELAY); // Protect shared variables region
    printf("\n💧 Humidity: %.1f%%\n", humidity);
    printf("🌡  Temperature: %.1fC\n", temperature);
    xSemaphoreGive(temperature_mutex); // Free from protection

    xSemaphoreTake(luminosity_mutex, portMAX_DELAY); // Protect shared variables region
    printf("💡 Lux: %d\n", lux);
    xSemaphoreGive(luminosity_mutex); // Free from protection

    vTaskDelay(pdMS_TO_TICKS(2000)); // ~2s delay
  }
}

void app_main(void)
{
  ESP_ERROR_CHECK(i2cdev_init()); // Initialize I2C

  // Create mutexes
  luminosity_mutex = xSemaphoreCreateMutex();
  temperature_mutex = xSemaphoreCreateMutex();

  // Create tasks
  xTaskCreatePinnedToCore
  (
    read_luminosity_measure, // Task function pointer
    "Luminosity", // Task name
    4096, // Task stack depth
    NULL, // Task argument pointer
    0, // Task priority
    NULL, // Created task pointer
    APP_CPU_NUM // CPU to run task
  );

  xTaskCreatePinnedToCore
  (
    read_temperature_humidity_measure, // Task function pointer
    "Temperature", // Task name
    4096, // Task stack depth
    NULL, // Task argument pointer
    0, // Task priority
    NULL, // Created task pointer
    APP_CPU_NUM // CPU to run task
  );

  xTaskCreatePinnedToCore
  (
    send_data, // Task function pointer
    "Send data", // Task name
    4096, // Task stack depth
    NULL, // Task argument pointer
    1, // Task priority
    NULL, // Created task pointer
    APP_CPU_NUM // CPU to run task
  );

  vTaskDelete(NULL); // Delete "main" task
}
