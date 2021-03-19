// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. 
// To get started please visit https://microsoft.github.io/azure-iot-developer-kit/docs/projects/connect-iot-hub?utm_source=ArduinoExtension&utm_medium=ReleaseNote&utm_campaign=VSCode

#include "AZ3166WiFi.h"
#include "AzureIotHub.h"
#include "DevKitMQTTClient.h"

#include "config.h"
#include "utility.h"
#include "SystemTickCounter.h"
#include "http_client.h"
#include "OledDisplay.h"
#include "Sensor.h"
#include "RGB_LED.h"

int messageCount = 1;
int sentMessageCount = 0;
static bool messageSending = true;
static uint64_t send_interval_ms;

#define LOOP_DELAY          1000

// Peripherals 
static DevI2C *ext_i2c;
static LSM6DSLSensor *accelgyroSensor;
static HTS221Sensor *tempSensor;
static LPS22HBSensor *pressureSensor;
static LIS2MDLSensor *magneticSensor;
static RGB_LED rgbLed;

// Indicate whether WiFi is ready
static bool hasWifi = false;

// Temperature sensor variables
static unsigned char tempSensorId;
static float humidity;
static float temperature;
static float pressure;

// Accelerometer, pedometer variables
static unsigned char accelGyroId;
static int stepCount;
static int xAxesData[3];
static int gAxesData[3];

// Magnetic variables
static unsigned char magSensorId;
static int axes[3];

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utilities

static void InitWifi()
{
  Screen.print(2, "Connecting...");
  
  if (WiFi.begin() == WL_CONNECTED)
  {
    IPAddress ip = WiFi.localIP();
    Screen.print(0, (ip.get_address()));
    hasWifi = true;
  }
  else
  {
    hasWifi = false;
    Screen.print(1, "No Wi-Fi\r\n ");
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Arduino sketch
void setup()
{

  rgbLed.setColor(256, 256, 0);

  // Initialize the Serial and Screen
  Screen.init();
  Screen.print(2, "Initializing...");
  Screen.print(3, " > Serial");
  Serial.begin(115200);

  delay(100);

  // Initialize the WiFi module
  Screen.print(3, " > WiFi");
  hasWifi = false;
  InitWifi();

  //Check for WIFI before moving further

  if (!hasWifi)
  {
    return;
  }

  delay(100);

  // Initialize LEDs
  Screen.print(3, " > LEDs");
  pinMode(45,OUTPUT);
  
  delay(100);

  // Initialize button
  Screen.print(3, " > Button");
  pinMode(USER_BUTTON_A, INPUT);
  pinMode(USER_BUTTON_B, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  delay(150);

  // Initialize the motion sensor
  Screen.print(3, " > Motion sensor     ");
  ext_i2c = new DevI2C(D14, D15);
  accelgyroSensor = new LSM6DSLSensor(*ext_i2c, D4, D5);
  accelgyroSensor->init(NULL); 
  accelgyroSensor->enableAccelerator();
  accelgyroSensor->enableGyroscope();
  accelgyroSensor->enablePedometer();
  accelgyroSensor->setPedometerThreshold(LSM6DSL_PEDOMETER_THRESHOLD_MID_LOW);
  stepCount = 0;

  delay(130);

  // Initialize the temperature sensor
  Screen.print(3, " > Temperature");
  tempSensor = new HTS221Sensor(*ext_i2c);
  tempSensor->init(NULL);

  delay(100);

  // Initialize the pressure sensor
  Screen.print(3, " > Pressure");
  pressureSensor = new LPS22HBSensor(*ext_i2c);
  pressureSensor->init(NULL);

  delay(500);

  // Initialize the magnetic sensor
  Screen.print(3, " > Magnetic");
  magneticSensor = new LIS2MDLSensor(*ext_i2c);
  magneticSensor->init(NULL);

  delay(300);

  temperature = -1;
  humidity = -1;
  pressure = -1;

  Screen.print(3, " > HTTP");
}

void loop()
{
  if (hasWifi)
  {
    
    //Serial.println("Serial Sensor Values");

    // Read temperature, humidity sensor
    tempSensor->enable();
    tempSensor->readId(&tempSensorId);
    //Serial.printf("tempSensorId: %d\n", tempSensorId);

    tempSensor->getTemperature(&temperature);
    //Serial.printf("Temperature: %f\n", temperature);

    tempSensor->getHumidity(&humidity);
    //Serial.printf("Humidity: %f\n", humidity);
    tempSensor->disable();
    tempSensor->reset();

    accelgyroSensor->readId(&accelGyroId);
    //Serial.printf("accelGyroId: %d\n", accelGyroId);

    // Read pedometer sensor
    accelgyroSensor->getStepCounter(&stepCount);
    //Serial.printf("Pedometer Step Count: %d\n", stepCount);

    // Read accelerometer sensor
    accelgyroSensor->getXAxes(xAxesData);
    //Serial.printf("Accelerometer X Axes: x=%d, y=%d, z=%d\n", xAxesData[0], xAxesData[1], xAxesData[2]);

    // Read gyroscope sensor
    accelgyroSensor->getGAxes(gAxesData);
    //Serial.printf("Accelerometer G Axes: x=%d, y=%d, z=%d\n", gAxesData[0], gAxesData[1], gAxesData[2]);

    // Read pressure sensor
    pressureSensor->getPressure(&pressure);
    //Serial.printf("Pressure=%f\n", pressure);

    // Read magnetic sensor
    magneticSensor->readId(&magSensorId);
    magneticSensor->getMAxes(axes);
    //Serial.printf("magSensorId: %d\n", magSensorId);
    //Serial.printf("Magnetometer Axes: x=%d, y=%d, z=%d\n", axes[0], axes[1], axes[2]);

    // Read pin
    float inputvalue = 0;
    int sensorpin = 16; 
    inputvalue = analogRead(sensorpin);
    float voltage = (inputvalue*3/1024);

    //Serial.println(" ");

    HTTPClient *httpClient = new HTTPClient(HTTP_POST, "http://URL/api/v1/CREDENTIALKEY/telemetry");

    char xmlBuff[285];

    char temp[sizeof(xmlBuff)];

    strcpy(temp,"{"); 
    
    strcat(temp,"'temperature':"); 
    strcat(temp,f2s(temperature, 3)); 
    strcat(temp,",");

    strcat(temp,"'pin0':"); 
    strcat(temp,f2s(voltage, 3)); 
    strcat(temp,",");

    strcat(temp,"'humidity':"); 
    strcat(temp,f2s(humidity, 3)); 
    strcat(temp,","); 

    strcat(temp,"'pressure':"); 
    strcat(temp,f2s(pressure, 3)); 
    strcat(temp,","); 

    strcat(temp,"'AccelerometerX':"); 
    strcat(temp,f2s(xAxesData[0],3)); 
    strcat(temp,","); 

    strcat(temp,"'AccelerometerY':"); 
    strcat(temp,f2s(xAxesData[1],3)); 
    strcat(temp,","); 

    strcat(temp,"'AccelerometerZ':"); 
    strcat(temp,f2s(xAxesData[2],3)); 
    strcat(temp,",");

    strcat(temp,"'GyroscopeX':"); 
    strcat(temp,f2s(gAxesData[0],3)); 
    strcat(temp,","); 

    strcat(temp,"'GyroscopeY':"); 
    strcat(temp,f2s(gAxesData[1],3)); 
    strcat(temp,","); 

    strcat(temp,"'GyroscopeZ':"); 
    strcat(temp,f2s(gAxesData[2],3)); 
    strcat(temp,",");

    strcat(temp,"'MagnetometerX':"); 
    strcat(temp,f2s(axes[0],3)); 
    strcat(temp,","); 

    strcat(temp,"'MagnetometerY':"); 
    strcat(temp,f2s(axes[1],3)); 
    strcat(temp,","); 

    strcat(temp,"'MagnetometerZ':"); 
    strcat(temp,f2s(axes[2],3)); 

    strcat(temp,"}");

    httpClient->set_header("Content-Type", "text/plain");

    const Http_Response* result = httpClient-> send(temp, strlen(temp));

    if (result == NULL)
      {
        Screen.print(2, "Failed");

        Serial.print("Error Code: ");
        Serial.println(httpClient->get_error());
        rgbLed.setColor(256,0,0);
      }
    else
      {
        delay(100);
        Screen.print(2, "Streaming");

        Serial.println("Body");
        Serial.println(result->body);
        Serial.println(httpClient->get_error());
        rgbLed.setColor(0,256,0);
      }
    delete httpClient;

    
  }
  digitalWrite(48,LOW);
  delay(2400);  // Every 2.5 second delay
  digitalWrite(48,HIGH);
}

