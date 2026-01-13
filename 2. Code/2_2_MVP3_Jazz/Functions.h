#ifndef FUNCTIONS_H
#define FUNCTIONS_H

const char* ssid = "Clean Heat";
const byte DNS_PORT = 53;

DNSServer dnsServer;
AsyncWebServer server(80);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
MAX6675 thermocouple(MAX_CLK_PIN, MAX_CS_PIN, MAX_DO_PIN);
Preferences prefs;

TempMode currentTempMode = TEMP_OFF;
bool isCustomPWM = false;
int customPWMValue = 0;
int currentCustomTemp = DEFAULT_MODE_1_TEMP;

LEDMode currentLEDMode = LED_TEMP;

int MODE_1_TEMP = DEFAULT_MODE_1_TEMP;
int MODE_2_TEMP = DEFAULT_MODE_2_TEMP;
int MODE_3_TEMP = DEFAULT_MODE_3_TEMP;

const float targetTemps[5] = {0, 0, 0, 0, 0};
const String tempLabels[5] = {"OFF", "", "", "", "100%"};

PIDData pid;

float temperature = 0.0;
float batteryVoltage = 0.0;
float filteredVoltage = 0.0;
int batteryPercent = 0;

bool timerRunning = false;
bool timerPaused = false;
unsigned long timerStartMillis = 0;
unsigned long timerPausedTime = 0;
unsigned long timerElapsed = 0;
unsigned long lastDisplayMillis = 0;
bool timerFlashActive = false;
unsigned long timerFlashStart = 0;
bool screenFlashActive = false;
unsigned long screenFlashStart = 0;

int lastButtonState = HIGH;
int buttonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long buttonPressTime = 0;
unsigned long buttonReleaseTime = 0;
bool longPressHandled = false;
bool waitingForDoubleClick = false;
unsigned long firstClickTime = 0;
int clickCount = 0;
bool waitingForMultiClick = false;
unsigned long lastClickTime = 0;

unsigned long previousTempMillis = 0;
unsigned long previousVoltageMillis = 0;
unsigned long voltageSampleMillis = 0;
int voltageSampleIndex = 0;
float voltageSampleSum = 0;
bool voltageReadingActive = false;
unsigned long initStartTime = 0;
bool isInitialized = false;

int pidOutput = 0;

bool sleepWarningActive = false;
unsigned long sleepWarningStart = 0;
volatile bool batteryBlinkState = false;
unsigned long lastBatteryBlink = 0;
bool anyButtonPressed = false;

bool wakeFromSleepFlag = false;
unsigned long wakeTime = 0;
bool resetTimerAfterSleep = false;

unsigned long lastActivityTime = 0;

int heatCycleCount = 0;
bool heatingActive = false;
bool cycleCounted = false;
unsigned long heatingStartTime = 0;
bool isCycleContinuation = false;

CRGB leds[NUM_LEDS];
unsigned long lastLedUpdate = 0;
unsigned long ledTransitionStart = 0;
bool ledTransitioning = false;
CRGB ledCurrentColor = CRGB::Black;
CRGB ledTargetColor = CRGB::Black;
bool ledForceDisabled = false;
bool bootRainbowActive = true;
unsigned long bootRainbowStart = 0;
bool ledToggleTransition = false;

unsigned long ledFlowStartTime = 0;
float ledFlowHue = 0.0;
unsigned long ledFlowPulseStart = 0;
bool ledFlowPulseActive = false;
int ledFlowPulseCount = 0;
int ledFlowPulseTarget = 0;
unsigned long ledFlowNextPulse = 0;
int ledFlowSpeed = LED_FLOW_DEFAULT_SPEED;
int ledFlowPulseMinInterval = LED_FLOW_DEFAULT_PULSE_MIN_INTERVAL;
int ledFlowPulseMaxInterval = LED_FLOW_DEFAULT_PULSE_MAX_INTERVAL;
int ledFlowPulseDuration = LED_FLOW_DEFAULT_PULSE_DURATION;
int ledFlowPulseMaxCount = LED_FLOW_DEFAULT_PULSE_COUNT;

bool otaInProgress = false;
bool otaSuccess = false;
unsigned long otaStartTime = 0;
unsigned long otaLedUpdate = 0;
bool otaLedState = false;
int otaBlinkCount = 0;
unsigned long otaBlinkStart = 0;
bool otaBlinkPhase = false;

bool updateTabActive = false;
unsigned long lastUpdateTabActivity = 0;
CRGB otaWaitColor1 = CRGB(0x12, 0x00, 0xff);
CRGB otaWaitColor2 = CRGB(0xf6, 0x00, 0xff);
unsigned long otaTransitionStart = 0;
CRGB otaCurrentColor = CRGB::Black;

bool notificationActive = false;
unsigned long notificationStart = 0;
String notificationText1 = "";
String notificationText2 = "";

int previousWiFiClients = 0;
bool isDarkTheme = false;

void loadThemeSettings() {
  prefs.begin("theme", false);
  isDarkTheme = prefs.getBool("dark", false);
  prefs.end();
}

void saveThemeSettings() {
  prefs.begin("theme", false);
  prefs.putBool("dark", isDarkTheme);
  prefs.end();
}

void setTheme(bool dark) {
  isDarkTheme = dark;
  saveThemeSettings();
}

void loadLEDFlowSettings() {
  prefs.begin("ledflow", false);
  ledFlowSpeed = prefs.getInt("speed", LED_FLOW_DEFAULT_SPEED);
  ledFlowPulseMinInterval = prefs.getInt("pulsemin", LED_FLOW_DEFAULT_PULSE_MIN_INTERVAL);
  ledFlowPulseMaxInterval = prefs.getInt("pulsemax", LED_FLOW_DEFAULT_PULSE_MAX_INTERVAL);
  ledFlowPulseDuration = prefs.getInt("duration", LED_FLOW_DEFAULT_PULSE_DURATION);
  ledFlowPulseMaxCount = prefs.getInt("maxcount", LED_FLOW_DEFAULT_PULSE_COUNT);
  currentLEDMode = (LEDMode)prefs.getInt("mode", LED_TEMP);
  prefs.end();
}

void saveLEDFlowSettings() {
  prefs.begin("ledflow", false);
  prefs.putInt("speed", ledFlowSpeed);
  prefs.putInt("pulsemin", ledFlowPulseMinInterval);
  prefs.putInt("pulsemax", ledFlowPulseMaxInterval);
  prefs.putInt("duration", ledFlowPulseDuration);
  prefs.putInt("maxcount", ledFlowPulseMaxCount);
  prefs.putInt("mode", (int)currentLEDMode);
  prefs.end();
}

void loadTempSettings() {
  prefs.begin("tempsettings", false);
  MODE_1_TEMP = prefs.getInt("mode1", DEFAULT_MODE_1_TEMP);
  MODE_2_TEMP = prefs.getInt("mode2", DEFAULT_MODE_2_TEMP);
  MODE_3_TEMP = prefs.getInt("mode3", DEFAULT_MODE_3_TEMP);
  prefs.end();
}

void saveTempSettings() {
  prefs.begin("tempsettings", false);
  prefs.putInt("mode1", MODE_1_TEMP);
  prefs.putInt("mode2", MODE_2_TEMP);
  prefs.putInt("mode3", MODE_3_TEMP);
  prefs.end();
}

void setModeTemp(int mode, int temp) {
  temp = constrain(temp, 50, 350);
  switch(mode) {
    case 1:
      MODE_1_TEMP = temp;
      break;
    case 2:
      MODE_2_TEMP = temp;
      break;
    case 3:
      MODE_3_TEMP = temp;
      break;
  }
  saveTempSettings();
}

int getCurrentTargetTemp() {
  if (isCustomPWM && customPWMValue == 0) {
    return currentCustomTemp;
  } else if (currentTempMode == TEMP_200) {
    return MODE_1_TEMP;
  } else if (currentTempMode == TEMP_220) {
    return MODE_2_TEMP;
  } else if (currentTempMode == TEMP_240) {
    return MODE_3_TEMP;
  }
  return 0;
}

void initLEDFlow() {
  ledFlowStartTime = millis();
  ledFlowHue = 0.0;
  ledFlowPulseActive = false;
  ledFlowPulseCount = 0;
  ledFlowNextPulse = millis() + random(ledFlowPulseMinInterval, ledFlowPulseMaxInterval);
}

void updateLEDFlow() {
  unsigned long currentTime = millis();
  
  if (ledFlowPulseActive) {
    if (currentTime - ledFlowPulseStart >= ledFlowPulseDuration) {
      ledFlowPulseActive = false;
      ledFlowPulseCount++;
      if (ledFlowPulseCount >= ledFlowPulseTarget) {
        ledFlowPulseCount = 0;
        ledFlowNextPulse = currentTime + random(ledFlowPulseMinInterval, ledFlowPulseMaxInterval);
      } else {
        ledFlowPulseStart = currentTime + 200;
        ledFlowPulseActive = true;
      }
    } else {
      float pulseProgress = (float)(currentTime - ledFlowPulseStart) / ledFlowPulseDuration;
      float pulseBrightness = sin(pulseProgress * PI);
      CRGB baseColor = CRGB::White;
      if (random(0, 3) == 0) {
        baseColor = CHSV(random(0, 255), 255, 255);
      }
      leds[0] = baseColor;
      leds[0].nscale8((uint8_t)(pulseBrightness * 255));
      FastLED.show();
      return;
    }
  }
  
  if (!ledFlowPulseActive && currentTime >= ledFlowNextPulse) {
    ledFlowPulseActive = true;
    ledFlowPulseStart = currentTime;
    ledFlowPulseTarget = random(1, ledFlowPulseMaxCount + 1);
    ledFlowPulseCount = 0;
  }
  
  if (!ledFlowPulseActive) {
    ledFlowHue += (float)ledFlowSpeed / 1000.0;
    if (ledFlowHue >= 255.0) ledFlowHue = 0.0;
    leds[0] = CHSV((uint8_t)ledFlowHue, 255, 255);
    FastLED.show();
  }
}

void initHeatCycleCounter() {
  prefs.begin("heatcycles", false);
  heatCycleCount = prefs.getInt("count", 0);
}

void updateHeatCycleCounter() {
  unsigned long currentMillis = millis();
  bool currentlyHeating = (currentTempMode != TEMP_OFF) || (isCustomPWM && customPWMValue > 0) || (isCustomPWM && customPWMValue == 0 && currentCustomTemp != MODE_1_TEMP);
  
  if (currentTempMode == TEMP_OFF && !isCustomPWM) {
    currentlyHeating = false;
  }
  
  if (currentlyHeating && !heatingActive) {
    heatingActive = true;
    heatingStartTime = currentMillis;
    cycleCounted = false;
    
    if (temperature > CYCLE_CONTINUATION_TEMP_THRESHOLD) {
      isCycleContinuation = true;
    } else {
      isCycleContinuation = false;
    }
  }
  
  if (!currentlyHeating && heatingActive) {
    heatingActive = false;
    cycleCounted = false;
  }
  
  if (heatingActive && !cycleCounted && !isCycleContinuation) {
    if (currentMillis - heatingStartTime >= CYCLE_COUNT_THRESHOLD) {
      cycleCounted = true;
      heatCycleCount++;
      saveHeatCycleCount();
      updateDisplay();
    }
  }
}

void saveHeatCycleCount() {
  prefs.putInt("count", heatCycleCount);
}

void resetHeatCycleCount() {
  heatCycleCount = 0;
  saveHeatCycleCount();
  updateDisplay();
}

void adjustHeatCycleCount(int delta) {
  heatCycleCount = max(0, heatCycleCount + delta);
  saveHeatCycleCount();
  updateDisplay();
}

void updateLastActivity() {
  lastActivityTime = millis();
}

void checkAutoSleep() {
  bool isHeating = (currentTempMode != TEMP_OFF) || (isCustomPWM && customPWMValue > 0) || (isCustomPWM && customPWMValue == 0 && currentCustomTemp != MODE_1_TEMP);
  
  if (currentTempMode == TEMP_OFF && !isCustomPWM) {
    isHeating = false;
  }
  
  if (!isHeating && !sleepWarningActive) {
    if (WiFi.softAPgetStationNum() > 0) {
      lastActivityTime = millis();
      return;
    }
    
    unsigned long sleepTimeout = (currentLEDMode != LED_OFF) ? AUTO_SLEEP_TIMEOUT_LED_ACTIVE : AUTO_SLEEP_TIMEOUT;
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastActivityTime >= sleepTimeout) {
      screenFlashActive = true;
      screenFlashStart = millis();
      updateDisplay();
      handleLongPress();
    }
  }
}

float getActualTargetTemp(TempMode mode) {
  if (mode == TEMP_OFF || mode == TEMP_MAX) {
    return 0;
  } else if (mode == TEMP_200) {
    return MODE_1_TEMP + TEMP_CORRECTION;
  } else if (mode == TEMP_220) {
    return MODE_2_TEMP + TEMP_CORRECTION;
  } else if (mode == TEMP_240) {
    return MODE_3_TEMP + TEMP_CORRECTION;
  }
  return 0;
}

void initPID() {
  pid.kp = KP_AGGRESSIVE;
  pid.ki = KI_AGGRESSIVE;
  pid.kd = KD_AGGRESSIVE;
  pid.lastError = 0;
  pid.integral = 0;
  pid.lastTemp = 0;
  pid.filteredDerivative = 0;
  pid.isStable = false;
  pid.stableCount = 0;
  pid.lastTime = millis();
}

void updatePID() {
  unsigned long now = millis();
  float dt = (now - pid.lastTime) / 1000.0;
  
  if (dt <= 0) return;
  
  if (currentTempMode == TEMP_OFF && !isCustomPWM) {
    ledcWrite(PWM_CHANNEL, 0);
    pidOutput = 0;
    pid.integral = 0;
    pid.lastError = 0;
    pid.isStable = false;
    pid.stableCount = 0;
    pid.lastTime = now;
    return;
  }
  
  if (currentTempMode == TEMP_MAX && !isCustomPWM) {
    ledcWrite(PWM_CHANNEL, 255);
    pidOutput = 255;
    pid.lastTime = now;
    return;
  }
  
  if (isCustomPWM && customPWMValue > 0) {
    pid.lastTime = now;
    return;
  }

  float targetTemp;
  if (isCustomPWM || (currentTempMode == TEMP_OFF && currentCustomTemp != MODE_1_TEMP)) {
    targetTemp = currentCustomTemp + TEMP_CORRECTION;
  } else {
    targetTemp = getActualTargetTemp(currentTempMode);
  }
  
  float error = targetTemp - temperature;
  float absError = abs(error);
  
  if (absError > TEMP_GAP_THRESHOLD) {
    pid.kp = KP_AGGRESSIVE;
    pid.ki = KI_AGGRESSIVE;
    pid.kd = KD_AGGRESSIVE;
  } else {
    pid.kp = KP_CONSERVATIVE;
    pid.ki = KI_CONSERVATIVE;
    pid.kd = KD_CONSERVATIVE;
  }
  
  pid.integral += error * dt;
  pid.integral = constrain(pid.integral, -INTEGRAL_WINDUP_LIMIT, INTEGRAL_WINDUP_LIMIT);
  
  float derivative = 0;
  if (pid.lastTemp != 0) {
    derivative = (temperature - pid.lastTemp) / dt;
    pid.filteredDerivative = DERIVATIVE_FILTER_ALPHA * derivative + 
                            (1.0 - DERIVATIVE_FILTER_ALPHA) * pid.filteredDerivative;
  }
  
  float output = pid.kp * error + pid.ki * pid.integral - pid.kd * pid.filteredDerivative;
  
  if (absError < TEMP_TOLERANCE) {
    pid.stableCount++;
    if (pid.stableCount >= 10) {
      pid.isStable = true;
    }
  } else {
    pid.stableCount = 0;
    pid.isStable = false;
  }
  
  pidOutput = constrain(output, 0, 255);
  ledcWrite(PWM_CHANNEL, pidOutput);
  
  if (isCustomPWM && customPWMValue > 0) {
    setLEDMode();
  }
  
  pid.lastError = error;
  pid.lastTemp = temperature;
  pid.lastTime = now;
}

void setTempMode(TempMode mode) {
  currentTempMode = mode;
  isCustomPWM = false;
  
  if (mode == TEMP_OFF) {
    ledcWrite(PWM_CHANNEL, 0);
    pidOutput = 0;
    customPWMValue = 0;
    currentCustomTemp = MODE_1_TEMP;
  }
  
  initPID();
  updateLastActivity();
  clearUpdateTabMode();
  setLEDMode();
  
  if (mode != TEMP_OFF) {
    startTimer();
  }
}

void setCustomTemp(int temp) {
  currentCustomTemp = constrain(temp, 50, 350);
  isCustomPWM = true;
  initPID();
  updateLastActivity();
  clearUpdateTabMode();
  setLEDMode();
  startTimer();
}

void startTimer() {
  if (resetTimerAfterSleep) {
    resetTimer();
    resetTimerAfterSleep = false;
  }
  
  if (!timerRunning && timerElapsed == 0) {
    timerRunning = true;
    timerPaused = false;
    timerStartMillis = millis();
    lastDisplayMillis = millis();
  }
}

void pauseTimer() {
  if (timerRunning && !timerPaused) {
    timerPaused = true;
    timerPausedTime = millis() - timerStartMillis;
  } else if (timerPaused) {
    timerPaused = false;
    timerStartMillis = millis() - timerPausedTime;
  }
}

void resetTimer() {
  timerRunning = false;
  timerPaused = false;
  timerElapsed = 0;
  timerPausedTime = 0;
}

void resumeTimerIfPaused() {
  if (timerPaused) {
    timerPaused = false;
    timerStartMillis = millis() - timerPausedTime;
  }
}

void handleSingleClick() {
  if (wakeFromSleepFlag && (millis() - wakeTime < WAKE_BUTTON_BLOCK_TIME)) {
    return;
  }
  
  updateLastActivity();
  clearUpdateTabMode();
  resumeTimerIfPaused();
  
  if (isCustomPWM) {
    setTempMode(TEMP_OFF);
  } else {
    TempMode nextMode = (TempMode)((currentTempMode + 1) % 5);
    setTempMode(nextMode);
  }
  updateDisplay();
}

void handleDoubleClick() {
  if (wakeFromSleepFlag && (millis() - wakeTime < WAKE_BUTTON_BLOCK_TIME)) {
    return;
  }
  
  updateLastActivity();
  clearUpdateTabMode();
  
  if (currentLEDMode == LED_OFF) {
    setLEDModeType(LED_TEMP);
    showNotification("LED", "TEMP");
  } else if (currentLEDMode == LED_TEMP) {
    setLEDModeType(LED_FLOW);
    showNotification("LED", "FLOW");
  } else {
    setLEDModeType(LED_OFF);
    showNotification("LED", "OFF");
  }
}

void handleTripleClick() {
  if (wakeFromSleepFlag && (millis() - wakeTime < WAKE_BUTTON_BLOCK_TIME)) {
    return;
  }
  
  updateLastActivity();
  clearUpdateTabMode();
  timerFlashActive = true;
  timerFlashStart = millis();
  
  if (timerRunning && !timerPaused) {
    timerPaused = true;
    timerPausedTime = millis() - timerStartMillis;
  } else if (timerPaused) {
    resetTimer();
    timerPaused = true;
  }
  updateDisplay();
}

void handleLongPress() {
  if (wakeFromSleepFlag && (millis() - wakeTime < WAKE_BUTTON_BLOCK_TIME)) {
    return;
  }
  
  updateLastActivity();
  clearUpdateTabMode();
  
  currentTempMode = TEMP_OFF;
  isCustomPWM = false;
  customPWMValue = 0;
  currentCustomTemp = MODE_1_TEMP;
  ledcWrite(PWM_CHANNEL, 0);
  pidOutput = 0;
  
  setLEDModeType(LED_OFF);
  
  initPID();
  setLEDMode();
  
  if (timerRunning && !timerPaused) {
    pauseTimer();
  }
  
  screenFlashActive = true;
  screenFlashStart = millis();
  updateDisplay();
  
  sleepWarningActive = true;
  sleepWarningStart = millis();
  batteryBlinkState = true;
  lastBatteryBlink = millis();
  anyButtonPressed = false;
}

void handleSleepWarning() {
  unsigned long currentMillis = millis();
  
  int reading = digitalRead(BUTTON_PIN);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {
        anyButtonPressed = true;
        sleepWarningActive = false;
        updateLastActivity();
        return;
      }
    }
  }
  lastButtonState = reading;
  
  if (currentMillis - lastBatteryBlink >= BATTERY_BLINK_INTERVAL) {
    batteryBlinkState = !batteryBlinkState;
    lastBatteryBlink = currentMillis;
  }
  
  if (currentMillis - sleepWarningStart >= SLEEP_WARNING_DURATION) {
    if (!anyButtonPressed) {
      goToSleep();
    } else {
      sleepWarningActive = false;
    }
  }
}

void goToSleep() {
  currentTempMode = TEMP_OFF;
  isCustomPWM = false;
  customPWMValue = 0;
  currentCustomTemp = MODE_1_TEMP;
  ledcWrite(PWM_CHANNEL, 0);
  pidOutput = 0;
  
  display.clearDisplay();
  display.display();
  
  FastLED.clear();
  FastLED.show();
  
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  
  gpio_wakeup_enable(GPIO_NUM_9, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
  
  esp_light_sleep_start();
  
  wakeFromSleepFlag = true;
  wakeTime = millis();
  resetTimerAfterSleep = true;
  updateLastActivity();
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid);
  
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(LED_BRIGHTNESS);
  setLEDMode();
  
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.setRotation(1);
  updateDisplay();
}

void handleButton() {
  if (wakeFromSleepFlag && (millis() - wakeTime >= WAKE_BUTTON_BLOCK_TIME)) {
    wakeFromSleepFlag = false;
  }
  
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        updateLastActivity();
        buttonPressTime = millis();
        longPressHandled = false;
      } 
      else {
        buttonReleaseTime = millis();
        
        if (!longPressHandled) {
          if (!waitingForMultiClick) {
            waitingForMultiClick = true;
            clickCount = 1;
            firstClickTime = millis();
            lastClickTime = millis();
          } else {
            if (millis() - lastClickTime <= DOUBLE_CLICK_TIMEOUT) {
              clickCount++;
              lastClickTime = millis();
              
              if (clickCount == 3) {
                waitingForMultiClick = false;
                handleTripleClick();
                clickCount = 0;
              }
            } else {
              waitingForMultiClick = false;
              handleSingleClick();
              clickCount = 0;
            }
          }
        }
      }
    }
  }

  if (buttonState == LOW && !longPressHandled) {
    if (millis() - buttonPressTime >= LONG_PRESS_DURATION) {
      longPressHandled = true;
      waitingForMultiClick = false;
      clickCount = 0;
      handleLongPress();
    }
  }
  
  if (waitingForMultiClick && (millis() - firstClickTime >= DOUBLE_CLICK_TIMEOUT)) {
    waitingForMultiClick = false;
    if (clickCount == 1) {
      handleSingleClick();
    } else if (clickCount == 2) {
      handleDoubleClick();
    }
    clickCount = 0;
  }

  lastButtonState = reading;
}

void startVoltageReading() {
  voltageReadingActive = true;
  voltageSampleIndex = 0;
  voltageSampleSum = 0;
  voltageSampleMillis = millis();
}

void processVoltageSample() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - voltageSampleMillis >= VOLTAGE_SAMPLE_INTERVAL) {
    int adcValue = analogRead(BATTERY_PIN);
    float voltage = (adcValue / ADC_RESOLUTION) * ADC_REFERENCE_VOLTAGE * VOLTAGE_DIVIDER_FACTOR;
    voltageSampleSum += voltage;
    voltageSampleIndex++;
    voltageSampleMillis = currentMillis;
    
    if (voltageSampleIndex >= VOLTAGE_SAMPLES) {
      float newVoltage = voltageSampleSum / VOLTAGE_SAMPLES;
      filteredVoltage = (VOLTAGE_FILTER_ALPHA * newVoltage) + ((1.0 - VOLTAGE_FILTER_ALPHA) * filteredVoltage);
      batteryVoltage = round(filteredVoltage * 10.0) / 10.0;
      batteryPercent = calculateBatteryPercent(filteredVoltage);
      voltageReadingActive = false;
      updateDisplay();
    }
  }
}

void updateBatteryVoltage() {
}

int calculateBatteryPercent(float voltage) {
  if (voltage <= BATTERY_MIN_VOLTAGE) return 0;
  if (voltage >= 8.25) return 100;
  if (voltage >= BATTERY_MAX_VOLTAGE) return 100;
  return constrain(((voltage - BATTERY_MIN_VOLTAGE) / (BATTERY_MAX_VOLTAGE - BATTERY_MIN_VOLTAGE)) * 100, 0, 100);
}

void setPWMByPercent(int percent) {
  percent = constrain(percent, 0, 100);
  int pwmValue = map(percent, 0, 100, 0, 255);
  ledcWrite(PWM_CHANNEL, pwmValue);
  pidOutput = pwmValue;
  isCustomPWM = true;
  customPWMValue = percent;
  currentTempMode = TEMP_OFF;
  startTimer();
  clearUpdateTabMode();
  setLEDMode();
  updateDisplay();
}

void setLEDTarget(CRGB color) {
  if (color != ledTargetColor) {
    ledTargetColor = color;
    if (!ledTransitioning) {
      ledTransitioning = true;
      ledTransitionStart = millis();
      ledCurrentColor = leds[0];
      ledToggleTransition = false;
    }
  }
}

void setLEDModeType(LEDMode mode) {
  currentLEDMode = mode;
  
  if (mode == LED_OFF) {
    setLEDTarget(CRGB::Black);
  } else if (mode == LED_FLOW) {
    CRGB flowStartColor = CHSV(0, 255, 255);
    setLEDTarget(flowStartColor);
  } else if (mode == LED_TEMP) {
    setLEDMode();
  }
  
  saveLEDFlowSettings();
}

void toggleLEDForceState() {
  if (currentLEDMode == LED_OFF) {
    setLEDModeType(LED_TEMP);
  } else {
    setLEDModeType(LED_OFF);
  }
}

void startOTA() {
  otaInProgress = true;
  otaSuccess = false;
  otaStartTime = millis();
  otaLedUpdate = millis();
  otaLedState = false;
  otaBlinkCount = 0;
  
  currentTempMode = TEMP_OFF;
  isCustomPWM = false;
  customPWMValue = 0;
  currentCustomTemp = MODE_1_TEMP;
  ledcWrite(PWM_CHANNEL, 0);
  pidOutput = 0;
  
  updateDisplay();
}

void handleOTAProgress() {
  unsigned long currentTime = millis();
  
  if (currentTime - otaLedUpdate >= 250) {
    if (!ledTransitioning) {
      ledTransitioning = true;
      ledTransitionStart = currentTime;
      ledCurrentColor = leds[0];
      
      if (otaLedState) {
        ledTargetColor = CRGB(0xff, 0x00, 0x00);
      } else {
        ledTargetColor = CRGB(0x00, 0x00, 0xff);
      }
      
      otaLedState = !otaLedState;
      otaLedUpdate = currentTime;
    }
  }
}

void handleOTASuccess() {
  if (!otaSuccess) {
    otaSuccess = true;
    otaBlinkCount = 0;
    otaBlinkStart = millis();
    otaBlinkPhase = true;
    ledTransitioning = true;
    ledTransitionStart = millis();
    ledCurrentColor = leds[0];
    ledTargetColor = CRGB(0x80, 0x00, 0x80);
  }
  
  unsigned long currentTime = millis();
  
  if (otaBlinkCount < 4) {
    if (!ledTransitioning && (currentTime - otaBlinkStart >= 250)) {
      ledTransitioning = true;
      ledTransitionStart = currentTime;
      ledCurrentColor = leds[0];
      
      if (otaBlinkPhase) {
        if (otaBlinkCount % 2 == 0) {
          ledTargetColor = CRGB(0x80, 0x00, 0x80);
        } else {
          ledTargetColor = CRGB(0xff, 0x00, 0x00);
        }
      } else {
        ledTargetColor = CRGB::Black;
      }
      
      otaBlinkPhase = !otaBlinkPhase;
      
      if (!otaBlinkPhase) {
        otaBlinkCount++;
        if (otaBlinkCount < 4) {
          otaBlinkStart = currentTime + 250;
        }
      } else {
        otaBlinkStart = currentTime;
      }
    }
  } else if (!ledTransitioning) {
    ledTransitioning = true;
    ledTransitionStart = millis();
    ledCurrentColor = leds[0];
    ledTargetColor = CRGB(0x80, 0x00, 0x80);
  }
}

void checkUpdateTabActivity() {
  unsigned long currentTime = millis();
  if (updateTabActive && (currentTime - lastUpdateTabActivity > 10000)) {
    updateTabActive = false;
    if (!ledTransitioning) {
      ledTransitioning = true;
      ledTransitionStart = currentTime;
      ledCurrentColor = leds[0];
      setLEDMode();
    }
  }
}

void clearUpdateTabMode() {
  if (updateTabActive) {
    updateTabActive = false;
    if (!ledTransitioning) {
      ledTransitioning = true;
      ledTransitionStart = millis();
      ledCurrentColor = leds[0];
      setLEDMode();
    }
  }
}

void setUpdateTabActive() {
  updateTabActive = true;
  lastUpdateTabActivity = millis();
  if (!otaInProgress && !ledTransitioning) {
    ledTransitioning = true;
    ledTransitionStart = millis();
    ledCurrentColor = leds[0];
    otaTransitionStart = millis();
  }
}

void showNotification(String line1, String line2) {
  notificationActive = true;
  notificationStart = millis();
  notificationText1 = line1;
  notificationText2 = line2;
  updateDisplay();
}

void updateNotification() {
  if (notificationActive && (millis() - notificationStart >= 600)) {
    notificationActive = false;
    updateDisplay();
  }
}

void checkWiFiClients() {
  int currentWiFiClients = WiFi.softAPgetStationNum();
  
  if (previousWiFiClients > 0 && currentWiFiClients == 0) {
    clearUpdateTabMode();
  }
  
  previousWiFiClients = currentWiFiClients;
}

void processSerialCommands() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command.startsWith("flow_speed ")) {
      int value = command.substring(11).toInt();
      if (value >= 10 && value <= 200) {
        ledFlowSpeed = value;
        saveLEDFlowSettings();
        Serial.println("Flow speed set to: " + String(value));
      } else {
        Serial.println("Flow speed must be between 10-200");
      }
    } else if (command.startsWith("flow_pulse_min ")) {
      int value = command.substring(15).toInt();
      if (value >= 3000 && value <= 30000) {
        ledFlowPulseMinInterval = value;
        saveLEDFlowSettings();
        Serial.println("Flow pulse min interval set to: " + String(value));
      } else {
        Serial.println("Pulse min interval must be between 3000-30000");
      }
    } else if (command.startsWith("flow_pulse_max ")) {
      int value = command.substring(15).toInt();
      if (value >= 5000 && value <= 60000) {
        ledFlowPulseMaxInterval = value;
        saveLEDFlowSettings();
        Serial.println("Flow pulse max interval set to: " + String(value));
      } else {
        Serial.println("Pulse max interval must be between 5000-60000");
      }
    } else if (command.startsWith("flow_pulse_duration ")) {
      int value = command.substring(20).toInt();
      if (value >= 200 && value <= 2000) {
        ledFlowPulseDuration = value;
        saveLEDFlowSettings();
        Serial.println("Flow pulse duration set to: " + String(value));
      } else {
        Serial.println("Pulse duration must be between 200-2000");
      }
    } else if (command.startsWith("flow_pulse_count ")) {
      int value = command.substring(17).toInt();
      if (value >= 1 && value <= 10) {
        ledFlowPulseMaxCount = value;
        saveLEDFlowSettings();
        Serial.println("Flow pulse max count set to: " + String(value));
      } else {
        Serial.println("Pulse max count must be between 1-10");
      }
    } else if (command == "flow_settings") {
      Serial.println("=== LED Flow Settings ===");
      Serial.println("Speed: " + String(ledFlowSpeed) + " (10-200)");
      Serial.println("Pulse Min Interval: " + String(ledFlowPulseMinInterval) + " ms (3000-30000)");
      Serial.println("Pulse Max Interval: " + String(ledFlowPulseMaxInterval) + " ms (5000-60000)");
      Serial.println("Pulse Duration: " + String(ledFlowPulseDuration) + " ms (200-2000)");
      Serial.println("Pulse Max Count: " + String(ledFlowPulseMaxCount) + " (1-10)");
    } else if (command == "flow_help") {
      Serial.println("=== LED Flow Commands ===");
      Serial.println("flow_speed <10-200> - Set rainbow flow speed");
      Serial.println("flow_pulse_min <3000-30000> - Set min pulse interval");
      Serial.println("flow_pulse_max <5000-60000> - Set max pulse interval");
      Serial.println("flow_pulse_duration <200-2000> - Set pulse duration");
      Serial.println("flow_pulse_count <1-10> - Set max pulse count");
      Serial.println("flow_settings - Show current settings");
      Serial.println("flow_help - Show this help");
    }
  }
}

#endif