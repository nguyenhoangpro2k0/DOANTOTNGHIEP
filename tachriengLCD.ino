#define BLYNK_TEMPLATE_ID "TMPL6PdwhZK4m"
#define BLYNK_TEMPLATE_NAME "LAB2"
#define BLYNK_AUTH_TOKEN "JQ7pFFJ-BSXX7UKTC7YyOmIYuEOibkuO"
#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <string.h>

char ssid[] = "Galaxy M51517D";
char pass[] = "12345678";
int lcdColumns = 16;
int lcdRows = 2;
const int bt1 = 32;
const int bt2 = 25;
const int bt3 = 27;

const int led1 = 4;
const int led2 = 5;
const int led3 = 19;

boolean bt1State = HIGH;
boolean bt2State = HIGH;
boolean bt3State = HIGH;

LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
TaskHandle_t task1Handle;
TaskHandle_t task2Handle;
TaskHandle_t task3Handle;

#define manual 0
#define auto 1
int mode = auto;

int buttonPress = 0;

void task1(void* parameter) {
  String lcdData = "";
  String weatherUrl = "https://sgp1.blynk.cloud/external/api/get?token=JQ7pFFJ-BSXX7UKTC7YyOmIYuEOibkuO&V0&V1&V3&V4";
  HTTPClient http;
  for (;;) {
    if (http.begin(weatherUrl)) {
      int httpCode = http.GET();
      if (httpCode > 0) {
        Serial.println(String("status code: ") + httpCode);
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println("[Payload]");
          Serial.println(payload);

          DynamicJsonDocument doc(1024);
          deserializeJson(doc, payload);
          JsonObject root = doc.as<JsonObject>();

          float temp = root["V0"].as<float>();
          float humi = root["V1"].as<float>();
          int shumi = root["V3"].as<int>();
          int lux = root["V4"].as<int>();

          Serial.print("Temperature: ");
          Serial.println(temp);
          Serial.print("Humidity: ");
          Serial.println(humi);
          Serial.print("Moisture: ");
          Serial.println(shumi);
          Serial.print("Light: ");
          Serial.println(lux);
          int time = millis();
          lcd.setCursor(0, 0);
          lcdData = "Humi:" + String(humi) + " %";
          Serial.println(lcdData);
          lcd.print(lcdData);

          lcd.setCursor(0, 1);
          lcdData = "Temp:" + String(temp, 1) + " C";
          Serial.println(lcdData);
          lcd.print(lcdData);
          time = millis();
          while ((millis() - time) <= 1000)
            ;
          lcd.clear();

          time = millis();
          while ((millis() - time) <= 200)
            ;
          lcd.setCursor(0, 0);
          lcdData = "Soil:" + String(shumi) + " %";
          Serial.println(lcdData);
          lcd.print(lcdData);

          time = millis();
          while ((millis() - time) <= 1000)
            ;
          lcd.setCursor(0, 1);
          lcdData = "Light:" + String(lux) + " lx";
          Serial.println(lcdData);
          lcd.print(lcdData);

          time = millis();
          while ((millis() - time) <= 1000)
            ;
          lcd.clear();
        }
      } else {
        Serial.print("Request failed, error: ");
        Serial.println(http.errorToString(httpCode));
      }
      http.end();
    } else {
      Serial.println("[HTTP] Unable to connect");
    }
    vTaskDelay(500);
  }
}

void task2(void* parameter) {
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  for (;;) {
    Serial.print("Task2 running on core ");
    Serial.println(xPortGetCoreID());
    Blynk.run();
    // Blynk.virtualWrite(V8, mode);
    if ((mode == manual) && (buttonPress == 1)) {
      Blynk.virtualWrite(V2, digitalRead(led1));
      Blynk.virtualWrite(V5, digitalRead(led2));
      Blynk.virtualWrite(V6, digitalRead(led3));
      buttonPress = 0;
    } else {
    }
    vTaskDelay(500);
  }
}

void task3(void* parameter) {
  pinMode(bt1, INPUT_PULLUP);
  pinMode(bt2, INPUT_PULLUP);
  pinMode(bt3, INPUT_PULLUP);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  for (;;) {
    Serial.print("Task3 running on core ");
    Serial.println(xPortGetCoreID());
    if (mode == auto) {
      HTTPClient http;
      if (http.begin("https://sgp1.blynk.cloud/external/api/get?token=JQ7pFFJ-BSXX7UKTC7YyOmIYuEOibkuO&V2&V5&V6")) {
        int httpCode = http.GET();
        if (httpCode > 0) {
          Serial.println(String("status code: ") + httpCode);
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = http.getString();
            Serial.println("[Payload]");
            Serial.println(payload);

            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);

            bool v2 = doc["V2"];
            bool v5 = doc["V5"];
            bool v6 = doc["V6"];

            Serial.println("V2: " + String(v2));
            Serial.println("V5: " + String(v5));
            Serial.println("V6: " + String(v6));

            digitalWrite(led1, v2 ? HIGH : LOW);
            digitalWrite(led2, v5 ? HIGH : LOW);
            digitalWrite(led3, v6 ? HIGH : LOW);
          }
        } else {
          Serial.print("Request failed, error: ");
          Serial.println(http.errorToString(httpCode));
        }
        http.end();
      } else {
        Serial.println("[HTTP] Unable to connect");
      }
    } else {
      if (digitalRead(bt1) == LOW) {
        buttonPress = 1;
        if (bt1State == HIGH) {
          digitalWrite(led1, !digitalRead(led1));
          delay(200);
          bt1State = LOW;
        }
      } else {
        bt1State = HIGH;
      }
      if (digitalRead(bt2) == LOW) {
        buttonPress = 1;
        if (bt2State == HIGH) {
          digitalWrite(led2, !digitalRead(led2));
          delay(200);
          bt2State = LOW;
        }
      } else {
        bt2State = HIGH;
      }
      if (digitalRead(bt3) == LOW) {
        if (bt3State == HIGH) {
          buttonPress = 1;
          digitalWrite(led3, !digitalRead(led3));
          delay(200);
          bt3State = LOW;
        }
      } else {
        bt3State = HIGH;
      }
    }
    vTaskDelay(200);
  }
}

BLYNK_WRITE(V2) {
  if (param.asInt() == 1) {
    digitalWrite(led1, HIGH);
  } else {
    digitalWrite(led1, LOW);
  }
}

BLYNK_WRITE(V5) {
  if (param.asInt() == 1) {
    digitalWrite(led2, HIGH);
  } else {
    digitalWrite(led2, LOW);
  }
}

BLYNK_WRITE(V6) {
  if (param.asInt() == 1) {
    digitalWrite(led3, HIGH);
  } else {
    digitalWrite(led3, LOW);
  }
}

BLYNK_WRITE(V8) {
  if (param.asInt() == 1) {
    mode = auto;
  } else {
    mode = manual;
  }
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.clear();

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  xTaskCreate(task1, "Task 1", 10000, NULL, 3, &task1Handle);  //lcd
  xTaskCreate(task2, "Task 2", 10000, NULL, 2, &task2Handle);  //blynk
  xTaskCreate(task3, "Task 3", 10000, NULL, 1, &task3Handle);  //button
}

void loop() {
  vTaskDelete(NULL);
}