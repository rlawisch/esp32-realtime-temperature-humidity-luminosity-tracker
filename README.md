# ESP32 RealTime Temperature, Humidity and Luminosity Tracker

This project uses an ESP32 (running the [ESP-IDF framework](https://github.com/espressif/esp-idf)) to track in real time (via [FreeRTOS](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html)) the luminosity (from a BH1750 sensor), temperature and humidity (from an AM2302 DHT sensor) measurements (via the awesome [esp-idf-lib](https://github.com/UncleRus/esp-idf-lib) library).

## Installation

Assuming you have a working ESP-IDF installation, just follow the [esp-idf-lib installation instructions](https://github.com/UncleRus/esp-idf-lib#esp32) to be able to build and run the project.
