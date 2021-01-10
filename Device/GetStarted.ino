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

const char SSL_CA_PEM[] = 	"-----BEGIN CERTIFICATE-----\n"
							"MIIHOjCCBiKgAwIBAgIRANpZiZaYeDOQEOtqI6KfpjkwDQYJKoZIhvcNAQELBQAw\n"
							"gY8xCzAJBgNVBAYTAkdCMRswGQYDVQQIExJHcmVhdGVyIE1hbmNoZXN0ZXIxEDAO\n"
							"BgNVBAcTB1NhbGZvcmQxGDAWBgNVBAoTD1NlY3RpZ28gTGltaXRlZDE3MDUGA1UE\n"
							"AxMuU2VjdGlnbyBSU0EgRG9tYWluIFZhbGlkYXRpb24gU2VjdXJlIFNlcnZlciBD\n"
							"QTAeFw0yMDAyMDMwMDAwMDBaFw0yMjAyMDIyMzU5NTlaMBsxGTAXBgNVBAMMECou\n"
							"aW50ZWdyb21hdC5jb20wggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQDd\n"
							"A2PXF5NrzhgwbCMZiuOoyqrLre6c0G4HmgCqHKqwEHVrp4kP7/sacIUjuacZHGsQ\n"
							"/3unc4tSnzcE+YdT0b6zu8zRitdZoMLFeFP6lPIpB9IYYlGKv8M9/OaGPX3Hjwpk\n"
							"F+HDicVWQbNHg2OwaANuDMvfwX2YvW7IzEWk0Ct2Az7gapaTQTScLvAToZTEzYYs\n"
							"2yMfslgzZkbxlM8q4MLnkbaPn/LcIo1Xrmi0JX/T25/EIMFLmgmfGI5GMx4tGWJy\n"
							"hmxa+R9wqpdGWoFJtijxJNonzlMKRL4UQS7zvVcwj3DP1gtAi+v2M1TWBisds3Kf\n"
							"a5t1jdmyUTBVr7W3ZqrNcM381h2J61OD3w57rRwSgoa+OIx6bwq4WAEEpYW0AFLk\n"
							"YUkV4t120lwY2vUwFmJ6trjCfJi1l5ldck/4YTr4TM9cSP0GNnbhyV/9tOWT3z4p\n"
							"WA95FeBYEfRlgfLrYM0eMGtufnLuPC3q/ZwNPSDyW+8S61jOhlp52YqbWLq4i1pu\n"
							"6tSP+txjOTGYrF9PBvfd9qyJaT2b5wS4Im80wDy8wnKFtSE45genKmaV8fRdcRpm\n"
							"R1RTKcT1MrmBH/PmYjbVC4pMjHmLtauk13/q6vqWJIm0e5Au5tCcSW3JwJEsYufx\n"
							"rqlHfILhAcmpacZ42oA2FOkacOOf9ah5d7y6oaWcOQIDAQABo4IDAjCCAv4wHwYD\n"
							"VR0jBBgwFoAUjYxexFStiuF36Zv5mwXhuAGNYeEwHQYDVR0OBBYEFE1PLqtBfwx3\n"
							"2geg7ugiVoovShk2MA4GA1UdDwEB/wQEAwIFoDAMBgNVHRMBAf8EAjAAMB0GA1Ud\n"
							"JQQWMBQGCCsGAQUFBwMBBggrBgEFBQcDAjBJBgNVHSAEQjBAMDQGCysGAQQBsjEB\n"
							"AgIHMCUwIwYIKwYBBQUHAgEWF2h0dHBzOi8vc2VjdGlnby5jb20vQ1BTMAgGBmeB\n"
							"DAECATCBhAYIKwYBBQUHAQEEeDB2ME8GCCsGAQUFBzAChkNodHRwOi8vY3J0LnNl\n"
							"Y3RpZ28uY29tL1NlY3RpZ29SU0FEb21haW5WYWxpZGF0aW9uU2VjdXJlU2VydmVy\n"
							"Q0EuY3J0MCMGCCsGAQUFBzABhhdodHRwOi8vb2NzcC5zZWN0aWdvLmNvbTArBgNV\n"
							"HREEJDAighAqLmludGVncm9tYXQuY29tgg5pbnRlZ3JvbWF0LmNvbTCCAX4GCisG\n"
							"AQQB1nkCBAIEggFuBIIBagFoAHcARqVV63X6kSAwtaKJafTzfREsQXS+/Um4havy\n"
							"/HD+bUcAAAFwDRA+4wAABAMASDBGAiEA286Xo+dbQ0D+yJvvFGPMMUrQzu4n6IiU\n"
							"ilsvKPwyMLUCIQDRC2BAZahopZfTvqswg+On1V57kyGiQow23Na0DEICGwB2AG9T\n"
							"dqwx8DEZ2JkApFEV/3cVHBHZAsEAKQaNsgiaN9kTAAABcA0QPtEAAAQDAEcwRQIg\n"
							"Ub4q+RDuMc1kiZDu8kxgrR/vkps6J5YhvWj9WWoSL+cCIQCw95NiAexPd8Jqv2ID\n"
							"YSL43V/Hgx4Loc4g6gbfH5k5VQB1ACJFRQdZVSRWlj+hL/H3bYbgIyZjrcBLf13G\n"
							"g1xu4g8CAAABcA0QPtIAAAQDAEYwRAIga3UvQQKoD/ugq6nwBwxtcDDy/gB16ihT\n"
							"gbgeelHlfKUCIAw8uqAC2Wt59YW7ezfxJTeJc1cP1FDojL8LQMJv1oewMA0GCSqG\n"
							"SIb3DQEBCwUAA4IBAQAL8l1I2pDz5bhhGQL8ElTHrHRuvc79CYAewoku5Y54cDHV\n"
							"aB90oKQl/po6QSICnKbylJleS4PIK17jEvVKB4bLy9f8K5qzBZdn4sqbNqp6RLY8\n"
							"cXCne+R7mMbG+LZsZgJiDdzRofBRQTVAaPvUxTe4xQXSluUB1ry0I4Hp/dBeYX19\n"
							"/4g1J1CLmxg1kdT9Rt/xnlNTWjFb43qtlBBrEW7fszCVcGAFIVD2EfWZjumqGIUD\n"
							"0n2PXsf15dr7B6EN5hxN9a6mlTSml0ZPfsS6gZfUnZ439mlo5TCZ2jhddGxSxJHE\n"
							"iqE2sxsZcQ4jwkLPvOkNJg7psyQUiikKYApPowlS\n"
							"-----END CERTIFICATE-----\n";

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
  rgbLed.turnOff();
  
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

    //Serial.println(" ");

    void set_header(const char* key, const char* value);
    HTTPClient *httpClient = new HTTPClient(SSL_CA_PEM, HTTP_POST, "https://hook.integromat.com/yfxl2timjw1e1ilhvqyqajbh6k31p8yo");
    const Http_Response* result = httpClient->send();

    if (result == NULL)
      {
        Screen.print(2, "Failed");

        Serial.print("Error Code: ");
        Serial.println(httpClient->get_error());
      }
    else
      {
        delay(100);
        Screen.print(2, "Success");

        Serial.println("Body");
        Serial.println(result->body);
      }
    delete httpClient;

    
  }
  
  delay(100);  // Every 0.1 second delay
}

