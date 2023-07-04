#define BLYNK_TEMPLATE_ID "TMPL6PdwhZK4m"
#define BLYNK_TEMPLATE_NAME "LAB2"

#include <WiFiClientSecure.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>  // Thêm thư viện ArduinoJson
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"
#include <Wire.h>
#include <BH1750.h>
#include <string.h>
#include <LiquidCrystal_I2C.h>

#define manual 0
#define auto 1
int mode = auto;

#define DHTTYPE DHT11
#define DHTPIN 04
DHT dht(DHTPIN, DHTTYPE);
// int sensor_pin = 02;
BH1750 lightMeter;

char auth[] = "JQ7pFFJ-BSXX7UKTC7YyOmIYuEOibkuO";
char ssid[] = "Galaxy M51517D";
char pass[] = "12345678";

int shumi = 0;
int lcdColumns = 16;
int lcdRows = 2;
float atemp = 0;
float ahumi = 50;
float natemp;
float nahumi;
int lcdcouter = 0;
int lux = 0;
int relayDen = 25;  // khai báo chân điều khiển relay
int relayBom = 26;
int relayQuat = 27;

bool relayControlBom = false;  // khởi tạo trạng thái đầu ra ban đầu
bool relayControlDen = false;
bool relayControlQuat = false;
bool chuyentrangthai = false;
int btnPin = 23;
int input_state;

TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3;
TaskHandle_t Task4;
TaskHandle_t Task5;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi Connected ");
  Serial.println("IP address:");
  Serial.println(WiFi.localIP());
  xTaskCreatePinnedToCore(Task1code, "Task1", 10000, NULL, 1, &Task1, 0);  // Blynk
  xTaskCreatePinnedToCore(Task2code, "Task2", 10000, NULL, 5, &Task2, 1);  //DHT11
  xTaskCreatePinnedToCore(Task3code, "Task3", 10000, NULL, 4, &Task3, 1);  //Mois
  xTaskCreatePinnedToCore(Task4code, "Task4", 10000, NULL, 6, &Task4, 1);  //Light
  xTaskCreatePinnedToCore(Task5code, "Task5", 10000, NULL, 3, &Task5, 0);  //Relay
}

BLYNK_WRITE(V5) {
  Serial.print("BW4 ");
  Serial.println(xPortGetCoreID());
  if (param.asInt() == 1) {
    relayControlBom = false;
    relayControlBom = !relayControlBom;
    digitalWrite(relayBom, relayControlBom);
  } else {
    relayControlBom = !relayControlBom;
    digitalWrite(relayBom, relayControlBom);
  }
}

BLYNK_WRITE(V2) {
  Serial.print("BW4 ");
  Serial.println(xPortGetCoreID());
  if (param.asInt() == 1) {
    relayControlDen = false;
    relayControlDen = !relayControlDen;
    digitalWrite(relayDen, relayControlDen);
  } else {
    relayControlDen = !relayControlDen;
    digitalWrite(relayDen, relayControlDen);
  }
}

BLYNK_WRITE(V6) {
  Serial.print("BW4 ");
  Serial.println(xPortGetCoreID());
  if (param.asInt() == 1) {
    relayControlQuat = false;
    relayControlQuat = !relayControlQuat;
    digitalWrite(relayQuat, relayControlQuat);
  } else {
    relayControlQuat = !relayControlQuat;
    digitalWrite(relayQuat, relayControlQuat);
  }
}

BLYNK_WRITE(V8) {
  Serial.print("BW4 ");
  Serial.println(xPortGetCoreID());
  if (param.asInt() == 1) {
    mode = auto;
  } else {
    mode = manual;
  }
}

void Task1code(void* parameter) {
  Blynk.begin(auth, ssid, pass);
  for (;;) {
    Serial.print("Task1 running on core ");
    Serial.println(xPortGetCoreID());
    Blynk.run();
    
    Blynk.virtualWrite(V0, atemp);
    Blynk.virtualWrite(V1, ahumi);
    Blynk.virtualWrite(V3, shumi);
    Blynk.virtualWrite(V4, lux);
    if(mode == auto){
    Blynk.virtualWrite(V5, relayControlBom);
    Blynk.virtualWrite(V2, relayControlDen);
    Blynk.virtualWrite(V6, relayControlQuat);
    }
    else{}
    // Blynk.virtualWrite(V8, mode);
    vTaskDelay(500);
  }
}

void Task2code(void* parameter) {
  dht.begin();
  for (;;) {
    Serial.print("Task3 running on core ");
    Serial.println(xPortGetCoreID());
    natemp = dht.readTemperature();
    nahumi = dht.readHumidity();
    natemp = dht.readTemperature();
    nahumi = dht.readHumidity();
    if (isnan(nahumi) || isnan(natemp)) {
      natemp = 0;
      nahumi = 0;
    }
    if (natemp != 0 && nahumi != 0) {
      atemp = natemp;
      ahumi = nahumi;
    }
    vTaskDelay(500);
  }
}

void Task3code(void* parameter) {
  int sensor_pin = 34;
  for (;;) {
    Serial.print("Task4 running on core ");
    Serial.println(xPortGetCoreID());
    shumi = analogRead(sensor_pin);
    shumi = map(shumi, 4095, 0, 0, 100);
    vTaskDelay(2000);
  }
}

void Task4code(void* parameter) {
  Wire.begin();
  lightMeter.begin();
  for (;;) {
    Serial.print("Task5 running on core ");
    Serial.println(xPortGetCoreID());
    lux = lightMeter.readLightLevel();  //1 cái nó lỗi nên phải 2 cái
    lux = lightMeter.readLightLevel();
    vTaskDelay(2000);
  }
}

void Task5code(void* parameter) {
  pinMode(relayDen, OUTPUT);  // thiết lập chế độ đầu ra cho chân điều khiển relay
  pinMode(relayBom, OUTPUT);
  pinMode(relayQuat, OUTPUT);
  // pinMode(btnPin, INPUT_PULLUP);
  WiFiClientSecure client;
  HTTPClient http;
  for (;;) {
    if (mode == manual) {
      if (http.begin("https://sgp1.blynk.cloud/external/api/get?token=JQ7pFFJ-BSXX7UKTC7YyOmIYuEOibkuO&V2&V5&V6")) {

        int httpCode = http.GET();
        if (httpCode > 0) {
          Serial.println(String("status code: ") + httpCode);
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            Serial.println();
            Serial.println("[Header]");
            for (int i = 0; i < http.headers(); i++) {
              Serial.println(http.headerName(i) + ": " + http.header(i));
            }
            Serial.println("[Payload]");
            String payload = http.getString();
            Serial.println(payload);

            // Deserializing JSON
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);

            // Lấy các giá trị từ JSON
            bool v2 = doc["V2"];
            bool v5 = doc["V5"];
            bool v6 = doc["V6"];

            // In ra các giá trị
            Serial.println("V2: " + String(v2));
            Serial.println("V5: " + String(v5));
            Serial.println("V6: " + String(v6));

            // Điều khiển relay dựa trên giá trị từ JSON
            if (v2 != relayControlDen) {
              relayControlDen = v2;
              digitalWrite(relayDen, relayControlDen ? HIGH : LOW);
            }

            if (v5 != relayControlBom) {
              relayControlBom = v5;
              digitalWrite(relayBom, relayControlBom ? HIGH : LOW);
            }

            if (v6 != relayControlQuat) {
              relayControlQuat = v6;
              digitalWrite(relayQuat, relayControlQuat ? HIGH : LOW);
            }
          }
        } else {
          Serial.print("Request failed, error: ");
          Serial.println(http.errorToString(httpCode));
        }
        http.end();
      } else {
        Serial.println("[HTTP] Unable to connect");
      }
    } else if (mode == auto) {
      if (atemp > 35 || shumi < 60) {
        relayControlBom = true;
        digitalWrite(relayBom, relayControlBom);
        delay(200);
      } else if (atemp < 34 && shumi > 62) {
        relayControlBom = false;
        digitalWrite(relayBom, relayControlBom);
        delay(200);
      } else {
      }
      if (atemp < 14 || lux < 3000) {
        relayControlDen = true;
        digitalWrite(relayDen, relayControlDen);
        delay(200);
      } else if (atemp > 15 && lux > 3100) {
        relayControlDen = false;
        digitalWrite(relayDen, relayControlDen);
        delay(200);
      } else {
      }
      if (atemp > 35 || ahumi > 70) {
        relayControlQuat = true;
        digitalWrite(relayQuat, relayControlQuat);
        delay(200);
      } else if (atemp < 34 && ahumi < 69) {
        relayControlQuat = false;
        digitalWrite(relayQuat, relayControlQuat);
        delay(200);
      } else {
      }
    }
    vTaskDelay(200);
  }
}

void loop() {
  vTaskDelete(NULL);
}