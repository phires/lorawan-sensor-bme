#include "LowPower.h"
#include "loraWan.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

uint8_t mydata[12];
uint8_t *tempArray;
uint8_t *pressureArray;
uint8_t *humidityArray;

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;

void setup() {
    Serial.begin(9600);
    Serial.println(F("Starting"));
    if (!bme.begin(0x76)) {
		  Serial.println("Could not find a valid BME280 sensor, check wiring!");
		  while (1);
	  }
    lorawan_setup();
}

void loop() {
    float temp = bme.readTemperature();
    float pres = bme.readPressure() / 100.0F;
    float humi = bme.readHumidity();

    Serial.print(F("Temperature: "));
    Serial.print(temp);
    Serial.print(F("°C, "));
    Serial.print(F("pressure: "));
    Serial.print(pres);
    Serial.print(F("hPa, "));
    Serial.print(F("humidity: "));
    Serial.print(humi);
    Serial.print("%");
    Serial.println();
    
    tempArray = (uint8_t*)(&temp);
    pressureArray = (uint8_t*)(&pres);
    humidityArray = (uint8_t*)(&humi);

    memcpy(&mydata[0], &tempArray[0], 4);
    memcpy(&mydata[4], &pressureArray[0], 4);
    memcpy(&mydata[8], &humidityArray[0], 4);

    lorawan_send(1, mydata, 12, false, NULL, NULL, NULL);
    
    // Sleep for about 4.2 minutes
    // Using a for at this point creates some more load
    // within the wakeup cycle, therefore we do it the
    // ugly way.    
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}