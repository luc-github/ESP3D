#include <Arduino.h>

#include "ESP32Ticker.h"
#include "DHTesp.h"

/** Initialize DHT sensor 1 */
DHTesp dhtSensor1;
/** Initialize DHT sensor 2 */
DHTesp dhtSensor2;
/** Initialize DHT sensor 3 */
DHTesp dhtSensor3;
/** Task handle for the light value read task */
TaskHandle_t tempTaskHandle = NULL;
/** Pin number for DHT11 1 data pin */
int dhtPin1 = 17;
/** Pin number for DHT11 2 data pin */
int dhtPin2 = 16;
/** Pin number for DHT11 3 data pin */
int dhtPin3 = 27;
/** Ticker for temperature reading */
Ticker tempTicker;
/** Flags for temperature readings finished */
bool gotNewTemperature = false;
/** Data from sensor 1 */
TempAndHumidity sensor1Data;
/** Data from sensor 2 */
TempAndHumidity sensor2Data;
/** Data from sensor 3 */
TempAndHumidity sensor3Data;

/* Flag if main loop is running */
bool tasksEnabled = false;

/**

/**
 * Task to reads temperature from DHT11 sensor
 * @param pvParameters
 *		pointer to task parameters
 */
void tempTask(void *pvParameters) {
	Serial.println("tempTask loop started");
	while (1) // tempTask loop
	{
		if (tasksEnabled && !gotNewTemperature) { // Read temperature only if old data was processed already
			// Reading temperature for humidity takes about 250 milliseconds!
			// Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
			sensor1Data = dhtSensor1.getTempAndHumidity();	// Read values from sensor 1
			sensor2Data = dhtSensor2.getTempAndHumidity();	// Read values from sensor 1
			sensor3Data = dhtSensor3.getTempAndHumidity();	// Read values from sensor 1
			gotNewTemperature = true;
		}
		vTaskSuspend(NULL);
	}
}

/**
 * triggerGetTemp
 * Sets flag dhtUpdated to true for handling in loop()
 * called by Ticker tempTicker
 */
void triggerGetTemp() {
	if (tempTaskHandle != NULL) {
		 xTaskResumeFromISR(tempTaskHandle);
	}
}

/**
 * Arduino setup function (called once after boot/reboot)
 */
void setup() {
	Serial.begin(115200);
	Serial.println("Example for 3 DHT11/22 sensors");

	// Initialize temperature sensor 1
	dhtSensor1.setup(dhtPin1, DHTesp::DHT11);
	// Initialize temperature sensor 2
	dhtSensor2.setup(dhtPin2, DHTesp::DHT11);
	// Initialize temperature sensor 3
	dhtSensor3.setup(dhtPin3, DHTesp::DHT11);

	// Start task to get temperature
	xTaskCreatePinnedToCore(
			tempTask,											 /* Function to implement the task */
			"tempTask ",										/* Name of the task */
			4000,													 /* Stack size in words */
			NULL,													 /* Task input parameter */
			5,															/* Priority of the task */
			&tempTaskHandle,								/* Task handle. */
			1);														 /* Core where the task should run */

	if (tempTaskHandle == NULL) {
		Serial.println("[ERROR] Failed to start task for temperature update");
	} else {
		// Start update of environment data every 30 seconds
		tempTicker.attach(30, triggerGetTemp);
	}

	// Signal end of setup() to tasks
	tasksEnabled = true;
} // End of setup.


/**
 * loop
 * Arduino loop function, called once 'setup' is complete (your own code
 * should go here)
 */
void loop() {
	if (gotNewTemperature) {
		Serial.println("Sensor 1 data:");
		Serial.println("Temp: " + String(sensor1Data.temperature,2) + "'C Humidity: " + String(sensor1Data.humidity,1) + "%");
		Serial.println("Sensor 2 data:");
		Serial.println("Temp: " + String(sensor2Data.temperature,2) + "'C Humidity: " + String(sensor2Data.humidity,1) + "%");
		Serial.println("Sensor 3 data:");
		Serial.println("Temp: " + String(sensor3Data.temperature,2) + "'C Humidity: " + String(sensor3Data.humidity,1) + "%");
		gotNewTemperature = false;
	}
} // End of loop
