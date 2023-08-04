#include <Arduino.h>
#if defined(ESP32) || defined(ARDUINO_RASPBERRY_PI_PICO_W)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "PMS.h"
#include <SoftwareSerial.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

const char *ssid = "A_WiFi";
const char *password = "passwordA";

#define API_KEY "AIzaSyA3AvqCKyjhGPI2RKPH34pVyoT_kqKOvv0"
#define DATABASE_URL "https://weatherstation-a1501-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define USER_EMAIL "r.tanapat37@gmail.com"
#define USER_PASSWORD "tanapat94"
#define DATABASE_SECRET "LN3mKQgdq853BB1RvDHsvyqNDPioHnTQG1RU68Wf"
//#define FIREBASE_PROJECT_ID "weatherstation-a1501"
//#define FIREBASE_CLIENT_EMAIL "r.tanapat37@gmail.com"
//const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----XXXXXXXXXXXX-----END PRIVATE KEY-----\n";

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme;

int UVsensorIn = A0; // Output from the sensor
float uvIntensity;

SoftwareSerial pmsSerial(D6, D7); // RX,TX
PMS pms(pmsSerial);
PMS::DATA data;

#define button_Pin 14

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
String weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

void setup()
{
  Serial.begin(115200);
  pmsSerial.begin(9600);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  //  config.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
  //  config.service_account.data.project_id = FIREBASE_PROJECT_ID;
  //  config.service_account.data.private_key = PRIVATE_KEY;
  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  String base_path = "/UsersData/";
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  String var = "$userId";
  String val = "($userId === auth.uid && auth.token.premium_account === true && auth.token.admin === true)";
  Firebase.RTDB.setReadWriteRules(&fbdo, base_path, var, val, val, DATABASE_SECRET);

  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("ok");
    signupOK = true;
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  //  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
  //    Serial.println(F("SSD1306 allocation failed"));
  //    for (;;); // Don't proceed, loop forever
  //  }
  display.clearDisplay();

  bme.begin();
  //  if (!bme.begin()) {
  //    Serial.println("Could not find a valid BME680 sensor, check wiring!");
  //    while (1);
  //  }
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  pinMode(UVsensorIn, INPUT);
  pinMode(button_Pin, INPUT);

  timeClient.begin();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Weather Station");
  display.setCursor(0, 10);
  display.println("V 1.0");
  display.display();
  delay(2000);
}

void loop()
{
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();

    Firebase.RTDB.setFloat(&fbdo, "sensor/temperature", bme.temperature);
    Firebase.RTDB.setFloat(&fbdo, "sensor/humidity", bme.humidity);
    Firebase.RTDB.setFloat(&fbdo, "sensor/pressure", bme.pressure / 100);
    Firebase.RTDB.setFloat(&fbdo, "sensor/altitude", bme.readAltitude(SEALEVELPRESSURE_HPA));
    Firebase.RTDB.setFloat(&fbdo, "sensor/gas_resistance", bme.gas_resistance / 1000.0);
    Firebase.RTDB.setFloat(&fbdo, "sensor/uv_intensity", uvIntensity);
    Firebase.RTDB.setFloat(&fbdo, "sensor/PM_1_0", data.PM_AE_UG_1_0);
    Firebase.RTDB.setFloat(&fbdo, "sensor/PM_2_5", data.PM_AE_UG_2_5);
    Firebase.RTDB.setFloat(&fbdo, "sensor/PM_10_0", data.PM_AE_UG_10_0);

    Serial.print(bme.temperature);
    Serial.print(" ");
    Serial.print(bme.humidity);
    Serial.print(" ");
    Serial.print(bme.pressure / 100);
    Serial.print(" ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.print(" ");
    Serial.print(bme.gas_resistance / 1000.0);
    Serial.print(" ");
    Serial.print(uvIntensity);
    Serial.print(" ");
    Serial.print(data.PM_AE_UG_1_0);
    Serial.print(" ");
    Serial.print(data.PM_AE_UG_2_5);
    Serial.print(" ");
    Serial.println(data.PM_AE_UG_10_0);
  }

  if (digitalRead(button_Pin) == 1)
  {
    display_Time_update();
  }
  else
  {
    static unsigned long timer0 = millis();
    const unsigned long period0 = 1000;
    if (millis() - timer0 >= period0)
    {
      timer0 += period0; /*Serial.print("T1000 \t"); Serial.println(millis());*/
      if (pms.readUntil(data))
      {
        Serial.print("PM 1.0 (ug/m3): ");
        Serial.println(data.PM_AE_UG_1_0);
        Serial.print("PM 2.5 (ug/m3): ");
        Serial.println(data.PM_AE_UG_2_5);
        Serial.print("PM 10.0 (ug/m3): ");
        Serial.println(data.PM_AE_UG_10_0);
      }
      else
      {
        Serial.println("No data.");
      }
    }
    static unsigned long timer1 = millis();
    const unsigned long period1 = 1000;
    if (millis() - timer1 >= period1)
    {
      timer1 += period1; /*Serial.print("T1000 \t"); Serial.println(millis());*/
      display_Sensor_update();
    }
    if (!bme.performReading())
    {
      Serial.println("Failed to perform reading :(");
      return;
    }
  }
}

int averageAnalogRead(int pinToRead)
{
  byte numberOfReadings = 8;
  unsigned int runningValue = 0;
  for (int x = 0; x < numberOfReadings; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;
  return (runningValue);
}
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void display_Sensor_update()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("T:");
  display.print(bme.temperature);
  display.println(" C");
  display.setCursor(70, 0);
  display.print("H:");
  display.print(bme.humidity);
  display.println(" %");
  display.setCursor(0, 10);
  display.print("P:");
  display.print(bme.pressure / 100);
  display.println(" hPa");
  display.setCursor(70, 10);
  display.print("A:");
  display.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  display.println(" m");
  display.setCursor(0, 20);
  display.print("G:");
  display.print(bme.gas_resistance / 1000.0);
  display.println(" KOhms");

  display.drawLine(0, 30, 128, 30, SSD1306_WHITE);
  int uvLevel = averageAnalogRead(UVsensorIn);
  float outputVoltage = 3.3 * uvLevel / 1024;
  uvIntensity = mapfloat(outputVoltage, 0.99, 2.9, 0.0, 15.0);
  display.setCursor(0, 33);
  display.print("UV:");
  display.print(uvIntensity);
  display.println(" mW/cm^2");

  display.drawLine(0, 43, 128, 43, SSD1306_WHITE);
  display.setCursor(0, 46);
  display.println("PM1.0");
  display.setCursor(0, 56);
  display.print(data.PM_AE_UG_1_0);

  display.setCursor(44, 46);
  display.println("PM2.5");
  display.setCursor(44, 56);
  display.print(data.PM_AE_UG_2_5);

  display.setCursor(88, 46);
  display.println("PM10.0");
  display.setCursor(88, 56);
  display.print(data.PM_AE_UG_10_0);

  display.display();
}

void display_Time_update()
{
  static unsigned long timer2 = millis();
  const unsigned long period2 = 1000;
  if (millis() - timer2 >= period2)
  {
    timer2 += period2; /*Serial.print("T1000 \t"); Serial.println(millis());*/

    timeClient.update();
    timeClient.setTimeOffset(25200);
    time_t epochTime = timeClient.getEpochTime();
    // Print complete Time:
    String formattedTime = timeClient.getFormattedTime();
    String weekDay = weekDays[timeClient.getDay()];
    struct tm *ptm = gmtime((time_t *)&epochTime);
    int monthDay = ptm->tm_mday;
    int currentMonth = ptm->tm_mon + 1;
    String currentMonthName = months[currentMonth - 1];
    int currentYear = ptm->tm_year + 1900;
    // Print complete Date:
    String currentDate = String(monthDay) + " " + String(currentMonthName) + " " + String(currentYear);

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    ;
    display.setCursor(52, 0);
    display.print("TIME");
    display.drawRoundRect(0, 9, 128, 20, 3, SSD1306_WHITE);
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(17, 12);
    display.print(timeClient.getFormattedTime());
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(17, 31);
    display.print(weekDay);
    display.setCursor(17, 41);
    display.print(currentDate);

    Serial.println(timeClient.getFormattedTime());
    Serial.println(currentDate);
    display.display();
  }
}
