#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <max6675.h>
#include <esp_sleep.h>
#include <Preferences.h>
#include <FastLED.h>
#include <Update.h>

#include "Config.h"
#include "Functions.h"
#include "Display.h"
#include "UI.h"

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  
  pinMode(HEATER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BATTERY_PIN, INPUT);

  buttonState = digitalRead(BUTTON_PIN);
  lastButtonState = buttonState;

  esp_sleep_enable_gpio_wakeup();

  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(HEATER_PIN, PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL, 0);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    while (true);
  }

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(LED_BRIGHTNESS);
  FastLED.clear();
  FastLED.show();
  bootRainbowStart = millis();

  display.setRotation(1);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("TEMP CONTROLLER");
  display.println("Initializing...");
  display.display();

  loadTempSettings();
  loadThemeSettings();
  loadLEDFlowSettings();

  WiFi.softAP(ssid);
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  setupWebServer();
  server.begin();

  float initialVoltage = analogRead(BATTERY_PIN);
  initialVoltage = (initialVoltage / ADC_RESOLUTION) * ADC_REFERENCE_VOLTAGE * VOLTAGE_DIVIDER_FACTOR;
  filteredVoltage = initialVoltage;
  batteryVoltage = initialVoltage;

  initPID();
  initHeatCycleCounter();
  initLEDFlow();
  initStartTime = millis();
  lastActivityTime = millis();
  previousWiFiClients = 0;
  
  Serial.println("=== Clean Heat Controller Started ===");
  Serial.println("Version: " + String(FIRMWARE_VERSION));
  Serial.println("Type 'flow_help' for LED Flow commands");
}

void loop() {
  unsigned long currentMillis = millis();
  
  dnsServer.processNextRequest();
  processSerialCommands();
  
  updateLED();

  if (timerFlashActive && (currentMillis - timerFlashStart >= FLASH_DURATION)) {
    timerFlashActive = false;
    updateDisplay();
  }

  if (screenFlashActive && (currentMillis - screenFlashStart >= SCREEN_FLASH_DURATION)) {
    screenFlashActive = false;
    updateDisplay();
  }

  if (waitingForDoubleClick && (currentMillis - firstClickTime >= DOUBLE_CLICK_TIMEOUT)) {
    waitingForDoubleClick = false;
    handleSingleClick();
  }

  if (!isInitialized && (currentMillis - initStartTime >= INIT_DELAY)) {
    isInitialized = true;
    updateDisplay();
  }

  if (sleepWarningActive) {
    handleSleepWarning();
    
    if (currentMillis - previousTempMillis >= TEMP_UPDATE_INTERVAL) {
      previousTempMillis = currentMillis;
      temperature = thermocouple.readCelsius();
      Serial.print("Temp update: "); Serial.println(temperature);
      updateDisplay();
    }
    
    if (currentMillis - lastBatteryBlink >= BATTERY_BLINK_INTERVAL) {
      Serial.print("Before toggle - batteryBlinkState: "); Serial.println(batteryBlinkState);
      if (batteryBlinkState) {
        batteryBlinkState = false;
        Serial.println("Setting to FALSE");
      } else {
        batteryBlinkState = true;
        Serial.println("Setting to TRUE");
      }
      lastBatteryBlink = currentMillis;
      Serial.print("After toggle - batteryBlinkState: "); Serial.print(batteryBlinkState ? "ON" : "OFF");
      Serial.print(" ("); Serial.print(batteryBlinkState); Serial.print(")");
      Serial.print(" at millis: "); Serial.println(currentMillis);
      updateDisplay();
    }
    return;
  }

  if (otaInProgress) {
    if (currentMillis - previousTempMillis >= TEMP_UPDATE_INTERVAL) {
      previousTempMillis = currentMillis;
      temperature = thermocouple.readCelsius();
      updateDisplay();
    }
    return;
  }

  if (isInitialized) {
    handleButton();
    checkAutoSleep();
    updateHeatCycleCounter();
    checkUpdateTabActivity();
    updateNotification();
    checkWiFiClients();

    if (currentMillis - previousTempMillis >= TEMP_UPDATE_INTERVAL) {
      previousTempMillis = currentMillis;
      temperature = thermocouple.readCelsius();
      updatePID();
      updateDisplay();
    }

    if (currentMillis - previousVoltageMillis >= VOLTAGE_UPDATE_INTERVAL && !voltageReadingActive) {
      previousVoltageMillis = currentMillis;
      startVoltageReading();
    }
    
    if (voltageReadingActive) {
      processVoltageSample();
    }

    if (timerRunning && !timerPaused && (currentMillis - lastDisplayMillis >= 1000)) {
      lastDisplayMillis = currentMillis;
      updateDisplay();
    }
  }
}