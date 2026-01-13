#ifndef DISPLAY_H
#define DISPLAY_H

CRGB blendColors(CRGB color1, CRGB color2, float ratio) {
  return CRGB(
    color1.r + (color2.r - color1.r) * ratio,
    color1.g + (color2.g - color1.g) * ratio,
    color1.b + (color2.b - color1.b) * ratio
  );
}

CRGB getPWMColor(int pwmPercent) {
  pwmPercent = constrain(pwmPercent, 0, 100);
  
  if (pwmPercent <= 25) {
    float ratio = (float)pwmPercent / 25.0;
    return blendColors(CRGB(0x24, 0x00, 0xff), CRGB(0x00, 0xff, 0x00), ratio);
  } else if (pwmPercent <= 50) {
    float ratio = (float)(pwmPercent - 25) / 25.0;
    return blendColors(CRGB(0x00, 0xff, 0x00), CRGB(0xff, 0x7e, 0x00), ratio);
  } else if (pwmPercent <= 80) {
    float ratio = (float)(pwmPercent - 50) / 30.0;
    return blendColors(CRGB(0xff, 0x7e, 0x00), CRGB(0xff, 0x00, 0x06), ratio);
  } else {
    float ratio = (float)(pwmPercent - 80) / 20.0;
    return blendColors(CRGB(0xff, 0x00, 0x06), CRGB(0xff, 0x00, 0x60), ratio);
  }
}

CRGB getTempColor(int temperature) {
  temperature = constrain(temperature, 160, 350);
  
  if (temperature <= 200) {
    float ratio = (float)(temperature - 160) / 40.0;
    return blendColors(CRGB(0x24, 0x00, 0xff), CRGB(0x00, 0xff, 0x7e), ratio);
  } else if (temperature <= 210) {
    float ratio = (float)(temperature - 200) / 10.0;
    return blendColors(CRGB(0x00, 0xff, 0x7e), CRGB(0x00, 0xff, 0x54), ratio);
  } else if (temperature <= 220) {
    float ratio = (float)(temperature - 210) / 10.0;
    return blendColors(CRGB(0x00, 0xff, 0x54), CRGB(0xcc, 0xff, 0x00), ratio);
  } else if (temperature <= 240) {
    float ratio = (float)(temperature - 220) / 20.0;
    return blendColors(CRGB(0xcc, 0xff, 0x00), CRGB(0xff, 0x00, 0x00), ratio);
  } else {
    float ratio = (float)(temperature - 240) / 110.0;
    return blendColors(CRGB(0xff, 0x00, 0x00), CRGB(0xff, 0x00, 0xa8), ratio);
  }
}

void setLEDMode() {
  if (currentLEDMode == LED_OFF) {
    return;
  }
  
  CRGB newTargetColor = CRGB::Black;
  
  if (currentLEDMode == LED_TEMP) {
    bool isHeating = (currentTempMode != TEMP_OFF) || (isCustomPWM && (customPWMValue > 0 || currentCustomTemp != MODE_1_TEMP));
    
    if (isHeating) {
      if (isCustomPWM && customPWMValue > 0) {
        int currentPWMPercent = map(pidOutput, 0, 255, 0, 100);
        newTargetColor = getPWMColor(currentPWMPercent);
      } else if (isCustomPWM && customPWMValue == 0) {
        newTargetColor = getTempColor(currentCustomTemp);
      } else if (currentTempMode == TEMP_200) {
        newTargetColor = getTempColor(MODE_1_TEMP);
      } else if (currentTempMode == TEMP_220) {
        newTargetColor = getTempColor(MODE_2_TEMP);
      } else if (currentTempMode == TEMP_240) {
        newTargetColor = getTempColor(MODE_3_TEMP);
      } else if (currentTempMode == TEMP_MAX) {
        newTargetColor = getPWMColor(100);
      }
    } else {
      newTargetColor = getTempColor(200);
    }
  }
  
  if (newTargetColor != ledTargetColor) {
    ledTargetColor = newTargetColor;
    if (!ledTransitioning) {
      ledToggleTransition = false;
      ledTransitioning = true;
      ledTransitionStart = millis();
    }
  }
}

void updateLED() {
  unsigned long currentTime = millis();
  
  if (otaInProgress) {
    if (otaSuccess) {
      handleOTASuccess();
    } else {
      handleOTAProgress();
    }
    return;
  }
  
  if (updateTabActive && !otaInProgress) {
    if (currentTime - lastLedUpdate >= 16) {
      unsigned long elapsed = currentTime - otaTransitionStart;
      float phase = (float)(elapsed % 1000) / 1000.0;
      float sinValue = (sin(phase * 2 * PI) + 1.0) / 2.0;
      CRGB currentColor = blendColors(otaWaitColor1, otaWaitColor2, sinValue);
      leds[0] = currentColor;
      FastLED.show();
      lastLedUpdate = currentTime;
    }
    return;
  }
  
  if (bootRainbowActive) {
    if (bootRainbowStart == 0) {
      bootRainbowStart = currentTime;
    }
    
    if (currentTime - bootRainbowStart >= 1700) {
      bootRainbowActive = false;
      leds[0] = CRGB::Black;
      FastLED.show();
      return;
    }
    
    unsigned long elapsed = currentTime - bootRainbowStart;
    uint8_t brightness = 255;
    uint8_t hue = 0;
    
    if (elapsed < 400) {
      brightness = (uint8_t)((float)elapsed / 400.0 * 255);
      hue = 0;
    } else if (elapsed < 1400) {
      brightness = 255;
      float rainbowProgress = (float)(elapsed - 400) / 1000.0;
      hue = (uint8_t)(rainbowProgress * 255);
    } else {
      brightness = (uint8_t)((float)(1700 - elapsed) / 300.0 * 255);
      float rainbowProgress = (float)(elapsed - 400) / 1000.0;
      hue = (uint8_t)(rainbowProgress * 255);
    }
    
    leds[0] = CHSV(hue, 255, brightness);
    FastLED.show();
    return;
  }
  
  if (currentLEDMode == LED_FLOW) {
    updateLEDFlow();
    return;
  }
  
  if (ledTransitioning) {
    unsigned long transitionTime = currentTime - ledTransitionStart;
    unsigned long transitionDuration = ledToggleTransition ? 300 : 600;
    
    if (transitionTime >= transitionDuration) {
      ledTransitioning = false;
      ledToggleTransition = false;
      ledCurrentColor = ledTargetColor;
      leds[0] = ledCurrentColor;
      FastLED.show();
    } else {
      float progress = (float)transitionTime / (float)transitionDuration;
      CRGB transitionColor = blendColors(ledCurrentColor, ledTargetColor, progress);
      leds[0] = transitionColor;
      FastLED.show();
    }
    return;
  }
  
  if (currentLEDMode == LED_OFF) {
    leds[0] = CRGB::Black;
    FastLED.show();
    return;
  }
  
  if (ledTargetColor == CRGB::Black) {
    leds[0] = CRGB::Black;
    FastLED.show();
    return;
  }
  
  if (currentTime - lastLedUpdate >= 16) {
    if (currentTempMode == TEMP_MAX) {
      CRGB color1 = CRGB(0xff, 0x00, 0x00);
      CRGB color2 = CRGB(0xff, 0x96, 0x00);
      unsigned long cycleTime = 1000;
      
      float phase = (float)(currentTime % cycleTime) / cycleTime;
      float sinValue = (sin(phase * 2 * PI) + 1.0) / 2.0;
      
      CRGB currentColor = blendColors(color1, color2, sinValue);
      leds[0] = currentColor;
      FastLED.show();
    } else {
      leds[0] = ledTargetColor;
      FastLED.show();
    }
    
    lastLedUpdate = currentTime;
  }
}

void updateDisplay() {
  display.clearDisplay();

  bool invertScreen = screenFlashActive;
  
  if (invertScreen) {
    display.fillRect(0, 0, 32, 128, SSD1306_WHITE);
  }

  String mainText;
  if (currentTempMode == TEMP_OFF && !isCustomPWM) {
    mainText = "OFF";
  } else if (currentTempMode == TEMP_200) {
    mainText = String(MODE_1_TEMP);
  } else if (currentTempMode == TEMP_220) {
    mainText = String(MODE_2_TEMP);
  } else if (currentTempMode == TEMP_240) {
    mainText = String(MODE_3_TEMP);
  } else if (currentTempMode == TEMP_MAX) {
    mainText = "100%";
  } else if (isCustomPWM && customPWMValue > 0) {
    int currentPWMPercent = map(pidOutput, 0, 255, 0, 100);
    mainText = String(currentPWMPercent) + "%";
  } else if (isCustomPWM && customPWMValue == 0) {
    mainText = String(currentCustomTemp);
  }

  int mainTextWidth = mainText.length() * 6;
  bool showDegreeSymbol = (currentTempMode == TEMP_200 || currentTempMode == TEMP_220 || 
                          currentTempMode == TEMP_240 || (isCustomPWM && customPWMValue == 0));
  
  int totalMainWidth = mainTextWidth;
  if (showDegreeSymbol) {
    totalMainWidth += 8;
  }

  int mainTextColor = invertScreen ? SSD1306_BLACK : SSD1306_WHITE;
  int bgColor = invertScreen ? SSD1306_WHITE : SSD1306_BLACK;
  
  bool isOffMode = (currentTempMode == TEMP_OFF && !isCustomPWM);
  
  if (!invertScreen && !isOffMode) {
    display.fillRect(0, 0, 32, 12, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
  } else {
    display.setTextColor(mainTextColor);
  }
  
  display.setTextSize(1);
  
  if (isOffMode && !invertScreen) {
    int charSpacing = 8;
    display.setCursor(1, 2);
    display.print("O");
    display.setCursor(2, 2);
    display.print("O");
    
    display.setCursor(1 + charSpacing, 2);
    display.print("F");
    display.setCursor(2 + charSpacing, 2);
    display.print("F");
    
    display.setCursor(1 + charSpacing * 2, 2);
    display.print("F");
    display.setCursor(2 + charSpacing * 2, 2);
    display.print("F");
  } else {
    display.setCursor(1, 2);
    display.print(mainText);
    display.setCursor(2, 2);
    display.print(mainText);
  }
  
  if (showDegreeSymbol) {
    int degreeX = 1 + mainTextWidth + 3;
    int degreeColor = (invertScreen || isOffMode) ? mainTextColor : SSD1306_BLACK;
    display.drawCircle(degreeX, 4, 1, degreeColor);
    display.setCursor(degreeX + 3, 2);
    display.print("C");
    if (isOffMode && !invertScreen) {
      display.drawCircle(degreeX + 1, 4, 1, mainTextColor);
      display.setCursor(degreeX + 4, 2);
      display.print("C");
    } else {
      display.drawCircle(degreeX + 1, 4, 1, degreeColor);
      display.setCursor(degreeX + 4, 2);
      display.print("C");
    }
  }

  display.setTextColor(mainTextColor);
  display.setTextSize(1);
  display.setCursor(1, 14);
  
  String tempStr = String((int)temperature);
  if (tempStr.length() > 4) {
    tempStr = tempStr.substring(0, 4);
  }
  display.print(tempStr);
  
  int tempTextWidth = tempStr.length() * 6;
  int tempDegreeX = 1 + tempTextWidth + 2;
  display.drawCircle(tempDegreeX, 16, 1, mainTextColor);
  display.setCursor(tempDegreeX + 3, 14);
  display.print("C");

  String pwmString;
  int currentPWM = 0;
  if (currentTempMode == TEMP_MAX) {
    currentPWM = 100;
  } else if ((currentTempMode != TEMP_OFF && !isCustomPWM) || (isCustomPWM)) {
    currentPWM = map(pidOutput, 0, 255, 0, 100);
  }
  
  if (currentPWM == 0) {
    pwmString = "OFF";
  } else {
    pwmString = String(currentPWM) + "%";
  }
  
  display.setCursor(1, 30);
  display.print(pwmString);

  if (notificationActive) {
    display.fillRect(0, 48, 32, 24, SSD1306_BLACK);
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    
    int line1Width = notificationText1.length() * 6;
    int line2Width = notificationText2.length() * 6;
    
    int line1X = (32 - line1Width) / 2;
    int line2X = (32 - line2Width) / 2;
    
    display.setCursor(line1X, 51);
    display.print(notificationText1);
    display.setCursor(line2X, 61);
    display.print(notificationText2);
  } else {
    String counterStr = String(heatCycleCount);
    int frameWidth = (counterStr.length() <= 1) ? 12 : (counterStr.length() * 6 + 4);
    
    display.drawRect(1, 48, frameWidth, 12, mainTextColor);
    int counterX = 1 + (frameWidth - counterStr.length() * 6) / 2;
    display.setCursor(counterX, 51);
    display.print(counterStr);

    int wifiX = 32 - 12 - 1;
    display.drawBitmap(wifiX, 48, wifi_bitmap_12, 12, 12, mainTextColor);
    if (WiFi.softAPgetStationNum() > 0) {
      display.fillCircle(wifiX + 10, 58, 2, mainTextColor);
    }

    unsigned long elapsed;
    if (timerRunning && !timerPaused) {
      elapsed = millis() - timerStartMillis;
    } else if (timerPaused) {
      elapsed = timerPausedTime;
    } else {
      elapsed = timerElapsed;
    }
    unsigned int minutes = elapsed / 60000;
    unsigned int seconds = (elapsed / 1000) % 60;
    String timerString = (minutes < 10 ? "0" + String(minutes) : String(minutes)) + ":" + (seconds < 10 ? "0" + String(seconds) : String(seconds));
    
    display.setCursor(1, 64);
    if (otaInProgress) {
      display.print("Update");
    } else if (timerFlashActive) {
      int timerStringWidth = timerString.length() * 6;
      display.fillRect(1, 63, timerStringWidth + 2, 9, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      display.setCursor(2, 64);
      display.print(timerString);
      display.setTextColor(mainTextColor);
    } else {
      display.print(timerString);
    }
  }

  if (sleepWarningActive) {
    unsigned long currentTime = millis();
    unsigned long remainingTime = SLEEP_WARNING_DURATION - (currentTime - sleepWarningStart);
    
    if (remainingTime > 300) {
      unsigned long elapsedTime = currentTime - sleepWarningStart;
      unsigned long animationDuration = SLEEP_WARNING_DURATION - 300;
      float timePerPixel = (float)animationDuration / 32.0;
      int pixelsToRemove = (int)(elapsedTime / timePerPixel);
      pixelsToRemove = constrain(pixelsToRemove, 0, 32);
      
      int lineLength = 32 - pixelsToRemove;
      if (lineLength > 0) {
        display.drawLine(0, 88, lineLength, 88, mainTextColor);
      }
    }
  } else {
    display.drawLine(0, 88, 32, 88, mainTextColor);
  }

  display.setCursor(1, 98);
  display.print(String(batteryPercent) + "%");

  display.setCursor(1, 108);
  display.print(String(batteryVoltage, 1) + "V");

  bool showBatteryFill = true;
  if (sleepWarningActive) {
    showBatteryFill = batteryBlinkState;
  }
  
  display.drawRect(0, 118, 30, 10, mainTextColor);
  display.drawRect(30, 121, 2, 4, mainTextColor);
  
  if (showBatteryFill) {
    int fillWidth = map(constrain(batteryPercent, 0, 100), 0, 100, 0, 26);
    if (fillWidth > 0) {
      display.fillRect(2, 120, fillWidth, 6, mainTextColor);
    }
  }

  display.display();
}

#endif