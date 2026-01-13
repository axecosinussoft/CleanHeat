#ifndef CONFIG_H
#define CONFIG_H

#define FIRMWARE_VERSION "1.0.65"

extern int MODE_1_TEMP;
extern int MODE_2_TEMP;
extern int MODE_3_TEMP;

#define DEFAULT_MODE_1_TEMP 210
#define DEFAULT_MODE_2_TEMP 220
#define DEFAULT_MODE_3_TEMP 240

#define TEMP_CORRECTION 15.0

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define SDA_PIN 6
#define SCL_PIN 7
#define SCREEN_ADDRESS 0x3C

#define MAX_CLK_PIN 2
#define MAX_CS_PIN  1
#define MAX_DO_PIN  0

#define BUTTON_PIN 9
#define HEATER_PIN 3
#define BATTERY_PIN 4

#define LED_PIN 8
#define NUM_LEDS 1
#define LED_BRIGHTNESS 200

#define PWM_FREQ 1000
#define PWM_RESOLUTION 8
#define PWM_CHANNEL 0

#define VOLTAGE_DIVIDER_FACTOR 3.15
#define ADC_REFERENCE_VOLTAGE 3.3
#define ADC_RESOLUTION 4095.0
#define BATTERY_MIN_VOLTAGE 3.0
#define BATTERY_MAX_VOLTAGE 4.2

#define VOLTAGE_FILTER_ALPHA 0.1
#define VOLTAGE_SAMPLES 10
#define VOLTAGE_SAMPLE_INTERVAL 10

#define TEMP_UPDATE_INTERVAL 500
#define VOLTAGE_UPDATE_INTERVAL 2000
#define DEBOUNCE_DELAY 50
#define INIT_DELAY 500
#define LONG_PRESS_DURATION 1000
#define DOUBLE_CLICK_TIMEOUT 500
#define FLASH_DURATION 100

#define PID_UPDATE_INTERVAL 200
#define TEMP_TOLERANCE 2.0

#define KP_AGGRESSIVE 8.0
#define KI_AGGRESSIVE 0.5
#define KD_AGGRESSIVE 2.0

#define KP_CONSERVATIVE 3.0
#define KI_CONSERVATIVE 0.2
#define KD_CONSERVATIVE 1.0

#define TEMP_GAP_THRESHOLD 10.0
#define INTEGRAL_WINDUP_LIMIT 50.0
#define DERIVATIVE_FILTER_ALPHA 0.3

#define SLEEP_WARNING_DURATION 5000
#define BATTERY_BLINK_INTERVAL 600
#define SCREEN_FLASH_DURATION 100
#define WAKE_BUTTON_BLOCK_TIME 500

#define AUTO_SLEEP_TIMEOUT 30000
#define AUTO_SLEEP_TIMEOUT_LED_ACTIVE 120000

#define CYCLE_COUNT_THRESHOLD 20000
#define CYCLE_CONTINUATION_TEMP_THRESHOLD 80.0

#define LED_FLOW_DEFAULT_SPEED 50
#define LED_FLOW_DEFAULT_PULSE_MIN_INTERVAL 5000
#define LED_FLOW_DEFAULT_PULSE_MAX_INTERVAL 15000
#define LED_FLOW_DEFAULT_PULSE_DURATION 800
#define LED_FLOW_DEFAULT_PULSE_COUNT 3

extern const char* ssid;
extern const byte DNS_PORT;

extern DNSServer dnsServer;
extern AsyncWebServer server;
extern Adafruit_SSD1306 display;
extern MAX6675 thermocouple;
extern Preferences prefs;

enum TempMode { TEMP_OFF, TEMP_200, TEMP_220, TEMP_240, TEMP_MAX };
extern TempMode currentTempMode;
extern bool isCustomPWM;
extern int customPWMValue;
extern int currentCustomTemp;

enum LEDMode { LED_OFF, LED_TEMP, LED_FLOW };
extern LEDMode currentLEDMode;

extern const float targetTemps[5];
extern const String tempLabels[5];

struct PIDData {
  float kp, ki, kd;
  float lastError;
  float integral;
  float lastTemp;
  float filteredDerivative;
  bool isStable;
  int stableCount;
  unsigned long lastTime;
};

extern PIDData pid;

extern float temperature;
extern float batteryVoltage;
extern float filteredVoltage;
extern int batteryPercent;

extern bool timerRunning;
extern bool timerPaused;
extern unsigned long timerStartMillis;
extern unsigned long timerPausedTime;
extern unsigned long timerElapsed;
extern unsigned long lastDisplayMillis;
extern bool timerFlashActive;
extern unsigned long timerFlashStart;
extern bool screenFlashActive;
extern unsigned long screenFlashStart;

extern int lastButtonState;
extern int buttonState;
extern unsigned long lastDebounceTime;
extern unsigned long buttonPressTime;
extern unsigned long buttonReleaseTime;
extern bool longPressHandled;
extern bool waitingForDoubleClick;
extern unsigned long firstClickTime;
extern int clickCount;
extern bool waitingForMultiClick;
extern unsigned long lastClickTime;

extern unsigned long previousTempMillis;
extern unsigned long previousVoltageMillis;
extern unsigned long voltageSampleMillis;
extern int voltageSampleIndex;
extern float voltageSampleSum;
extern bool voltageReadingActive;
extern unsigned long initStartTime;
extern bool isInitialized;

extern int pidOutput;

extern bool sleepWarningActive;
extern unsigned long sleepWarningStart;
extern volatile bool batteryBlinkState;
extern unsigned long lastBatteryBlink;
extern bool anyButtonPressed;

extern bool wakeFromSleepFlag;
extern unsigned long wakeTime;
extern bool resetTimerAfterSleep;

extern unsigned long lastActivityTime;

extern int heatCycleCount;
extern bool heatingActive;
extern bool cycleCounted;
extern unsigned long heatingStartTime;
extern bool isCycleContinuation;

#include <FastLED.h>
extern CRGB leds[NUM_LEDS];
extern unsigned long lastLedUpdate;
extern unsigned long ledTransitionStart;
extern bool ledTransitioning;
extern CRGB ledCurrentColor;
extern CRGB ledTargetColor;
extern bool ledForceDisabled;
extern bool bootRainbowActive;
extern unsigned long bootRainbowStart;
extern bool ledToggleTransition;

extern unsigned long ledFlowStartTime;
extern float ledFlowHue;
extern unsigned long ledFlowPulseStart;
extern bool ledFlowPulseActive;
extern int ledFlowPulseCount;
extern int ledFlowPulseTarget;
extern unsigned long ledFlowNextPulse;
extern int ledFlowSpeed;
extern int ledFlowPulseMinInterval;
extern int ledFlowPulseMaxInterval;
extern int ledFlowPulseDuration;
extern int ledFlowPulseMaxCount;

extern bool otaInProgress;
extern bool otaSuccess;
extern unsigned long otaStartTime;
extern unsigned long otaLedUpdate;
extern bool otaLedState;
extern int otaBlinkCount;
extern unsigned long otaBlinkStart;
extern bool otaBlinkPhase;

extern bool updateTabActive;
extern unsigned long lastUpdateTabActivity;
extern CRGB otaWaitColor1;
extern CRGB otaWaitColor2;
extern unsigned long otaTransitionStart;
extern CRGB otaCurrentColor;

extern bool notificationActive;
extern unsigned long notificationStart;
extern String notificationText1;
extern String notificationText2;

extern int previousWiFiClients;
extern bool isDarkTheme;

void loadThemeSettings();
void saveThemeSettings();
void setTheme(bool dark);

static const unsigned char PROGMEM wifi_bitmap_12[] = {
  0x00, 0x00, 0x1F, 0x80, 0x60, 0x60, 0x80, 0x10, 0x0F, 0x00, 0x30, 0xC0, 
  0x40, 0x20, 0x0F, 0x00, 0x10, 0x80, 0x00, 0x00, 0x06, 0x00, 0x06, 0x00
};

static const unsigned char PROGMEM battery_bitmap[] = {
  0x7F, 0xFF, 0x00, 0x40, 0x01, 0x80, 0x40, 0x01, 0x80, 0x40, 0x01, 0x80,
  0x40, 0x01, 0x80, 0x40, 0x01, 0x80, 0x40, 0x01, 0x80, 0x7F, 0xFF, 0x00
};

void initHeatCycleCounter();
void updateHeatCycleCounter();
void saveHeatCycleCount();
void resetHeatCycleCount();
void adjustHeatCycleCount(int delta);
void updateLastActivity();
void checkAutoSleep();
float getActualTargetTemp(TempMode mode);
void initPID();
void updatePID();
void setTempMode(TempMode mode);
void setCustomTemp(int temp);
void startTimer();
void pauseTimer();
void resetTimer();
void resumeTimerIfPaused();
void handleSingleClick();
void handleDoubleClick();
void handleTripleClick();
void handleLongPress();
void handleSleepWarning();
void goToSleep();
void handleButton();
void startVoltageReading();
void processVoltageSample();
void updateBatteryVoltage();
int calculateBatteryPercent(float voltage);
void setPWMByPercent(int percent);
void loadTempSettings();
void saveTempSettings();
void setModeTemp(int mode, int temp);
int getCurrentTargetTemp();

void setupWebServer();
String generateWebPage();
void updateDisplay();
void updateLED();
void setLEDMode();
void setLEDModeType(LEDMode mode);
void toggleLEDForceState();
void initLEDFlow();
void updateLEDFlow();
void startOTA();
void handleOTAProgress();
void handleOTASuccess();
void checkUpdateTabActivity();
void setUpdateTabActive();
void clearUpdateTabMode();
void showNotification(String line1, String line2);
void updateNotification();
void checkWiFiClients();
void processSerialCommands();
void loadLEDFlowSettings();
void saveLEDFlowSettings();

#endif
