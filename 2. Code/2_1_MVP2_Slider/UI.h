#ifndef UI_H
#define UI_H

void setupWebServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", generateWebPage());
  });

  server.on("/set_temp", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("mode")) {
      int mode = request->getParam("mode")->value().toInt();
      if (mode >= 0 && mode < 5) {
        updateLastActivity();
        clearUpdateTabMode();
        resumeTimerIfPaused();
        setTempMode((TempMode)mode);
        request->send(200, "text/plain", "OK");
      } else {
        request->send(400, "text/plain", "Invalid mode");
      }
    } else {
      request->send(400, "text/plain", "Missing mode");
    }
  });

  server.on("/save_mode_temp", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("mode")) {
      int mode = request->getParam("mode")->value().toInt();
      if (mode >= 1 && mode <= 3) {
        int currentTemp = getCurrentTargetTemp();
        if (currentTemp > 0) {
          setModeTemp(mode, currentTemp);
          updateLastActivity();
          clearUpdateTabMode();
          request->send(200, "text/plain", "SAVED");
        } else {
          request->send(400, "text/plain", "No target temp");
        }
      } else {
        request->send(400, "text/plain", "Invalid mode");
      }
    } else {
      request->send(400, "text/plain", "Missing mode");
    }
  });

  server.on("/adjust_temp", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("delta")) {
      int delta = request->getParam("delta")->value().toInt();
      int newTemp;
      
      if (isCustomPWM && customPWMValue == 0) {
        newTemp = currentCustomTemp + delta;
      } else if (currentTempMode == TEMP_200) {
        newTemp = MODE_1_TEMP + delta;
      } else if (currentTempMode == TEMP_220) {
        newTemp = MODE_2_TEMP + delta;
      } else if (currentTempMode == TEMP_240) {
        newTemp = MODE_3_TEMP + delta;
      } else {
        newTemp = 150 + delta;
      }
      
      newTemp = constrain(newTemp, 50, 350);
      updateLastActivity();
      clearUpdateTabMode();
      resumeTimerIfPaused();
      
      currentCustomTemp = newTemp;
      currentTempMode = TEMP_OFF;
      isCustomPWM = true;
      customPWMValue = 0;
      initPID();
      startTimer();
      setLEDMode();
      
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Missing delta");
    }
  });

  server.on("/set_pwm", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("value")) {
      int value = request->getParam("value")->value().toInt();
      updateLastActivity();
      clearUpdateTabMode();
      resumeTimerIfPaused();
      setPWMByPercent(value);
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Missing value");
    }
  });

  server.on("/timer_reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    updateLastActivity();
    resetTimer();
    request->send(200, "text/plain", "OK");
  });

  server.on("/timer_pause", HTTP_GET, [](AsyncWebServerRequest *request) {
    updateLastActivity();
    pauseTimer();
    request->send(200, "text/plain", "OK");
  });

  server.on("/cycle_adjust", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("delta")) {
      int delta = request->getParam("delta")->value().toInt();
      updateLastActivity();
      heatCycleCount = max(0, heatCycleCount + delta);
      saveHeatCycleCount();
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Missing delta");
    }
  });

  server.on("/cycle_reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    updateLastActivity();
    resetHeatCycleCount();
    request->send(200, "text/plain", "OK");
  });

  server.on("/pwm_adjust", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("delta")) {
      int delta = request->getParam("delta")->value().toInt();
      int currentPWMPercent;
      
      if (currentTempMode == TEMP_OFF && !isCustomPWM) {
        currentPWMPercent = 0;
      } else if (currentTempMode == TEMP_MAX) {
        currentPWMPercent = 100;
      } else {
        currentPWMPercent = map(pidOutput, 0, 255, 0, 100);
      }
      
      int newPWMPercent = constrain(currentPWMPercent + delta, 0, 100);
      updateLastActivity();
      clearUpdateTabMode();
      resumeTimerIfPaused();
      setPWMByPercent(newPWMPercent);
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Missing delta");
    }
  });

  server.on("/set_led_mode", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("mode")) {
      int mode = request->getParam("mode")->value().toInt();
      if (mode >= 0 && mode <= 2) {
        updateLastActivity();
        clearUpdateTabMode();
        setLEDModeType((LEDMode)mode);
        request->send(200, "text/plain", "OK");
      } else {
        request->send(400, "text/plain", "Invalid mode");
      }
    } else {
      request->send(400, "text/plain", "Missing mode");
    }
  });

  server.on("/toggle_led", HTTP_GET, [](AsyncWebServerRequest *request) {
    updateLastActivity();
    clearUpdateTabMode();
    toggleLEDForceState();
    request->send(200, "text/plain", "OK");
  });

  server.on("/update_tab_active", HTTP_GET, [](AsyncWebServerRequest *request) {
    setUpdateTabActive();
    request->send(200, "text/plain", "OK");
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String currentModeStr;
    int pwmPercent = 0;
    
    if (isCustomPWM && customPWMValue > 0) {
      pwmPercent = map(pidOutput, 0, 255, 0, 100);
      if (pwmPercent == 0) {
        currentModeStr = "OFF";
      } else {
        currentModeStr = String(pwmPercent) + "%";
      }
    } else if (isCustomPWM && customPWMValue == 0) {
      currentModeStr = String(currentCustomTemp) + "°C";
      pwmPercent = map(pidOutput, 0, 255, 0, 100);
    } else {
      if (currentTempMode == TEMP_OFF) {
        currentModeStr = "OFF";
      } else if (currentTempMode == TEMP_200) {
        currentModeStr = String(MODE_1_TEMP) + "°C";
      } else if (currentTempMode == TEMP_220) {
        currentModeStr = String(MODE_2_TEMP) + "°C";
      } else if (currentTempMode == TEMP_240) {
        currentModeStr = String(MODE_3_TEMP) + "°C";
      } else if (currentTempMode == TEMP_MAX) {
        currentModeStr = "100%";
      }
      
      if (currentTempMode == TEMP_MAX) {
        pwmPercent = 100;
      } else if (currentTempMode != TEMP_OFF) {
        pwmPercent = map(pidOutput, 0, 255, 0, 100);
      }
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

    String ledModeStr;
    switch (currentLEDMode) {
      case LED_OFF:
        ledModeStr = "OFF";
        break;
      case LED_TEMP:
        ledModeStr = "TEMP";
        break;
      case LED_FLOW:
        ledModeStr = "FLOW";
        break;
    }

    String json = "{";
    json += "\"mode\":\"" + currentModeStr + "\",";
    json += "\"temp\":" + String((int)temperature) + ",";
    json += "\"pwm\":" + String(pwmPercent) + ",";
    json += "\"battery_v\":" + String(batteryVoltage, 1) + ",";
    json += "\"battery_p\":" + String(batteryPercent) + ",";
    json += "\"timer\":\"" + timerString + "\",";
    json += "\"timer_paused\":" + String(timerPaused ? "true" : "false") + ",";
    json += "\"cycles\":" + String(heatCycleCount) + ",";
    json += "\"led_mode\":\"" + ledModeStr + "\",";
    json += "\"mode1_temp\":" + String(MODE_1_TEMP) + ",";
    json += "\"mode2_temp\":" + String(MODE_2_TEMP) + ",";
    json += "\"mode3_temp\":" + String(MODE_3_TEMP) + ",";
    json += "\"dark_theme\":" + String(isDarkTheme ? "true" : "false");
    json += "}";
    
    request->send(200, "application/json", json);
  });

  server.on("/get_theme", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{\"dark\":" + String(isDarkTheme ? "true" : "false") + "}";
    request->send(200, "application/json", json);
  });

  server.on("/set_theme", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("dark")) {
      bool dark = request->getParam("dark")->value() == "true";
      setTheme(dark);
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Missing dark parameter");
    }
  });

  server.on("/main_tab_active", HTTP_GET, [](AsyncWebServerRequest *request) {
    clearUpdateTabMode();
    request->send(200, "text/plain", "OK");
  });

  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    bool shouldReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot ? "OK" : "FAIL");
    response->addHeader("Connection", "close");
    request->send(response);
    
    if (shouldReboot) {
      handleOTASuccess();
      delay(2000);
      ESP.restart();
    }
  }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
      startOTA();
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }
    }
    if (!Update.hasError()) {
      if (Update.write(data, len) != len) {
        Update.printError(Serial);
      }
    }
    if (final) {
      if (Update.end(true)) {
        Serial.printf("Update Success: %uB\n", index + len);
      } else {
        Update.printError(Serial);
      }
    }
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->redirect("/");
  });
}

String generateWebPage() {
  String currentModeStr;
  int pwmPercent = 0;
  
  if (isCustomPWM && customPWMValue > 0) {
    pwmPercent = map(pidOutput, 0, 255, 0, 100);
    if (pwmPercent == 0) {
      currentModeStr = "OFF";
    } else {
      currentModeStr = String(pwmPercent) + "%";
    }
  } else if (isCustomPWM && customPWMValue == 0) {
    currentModeStr = String(currentCustomTemp) + "Â°C";
    pwmPercent = map(pidOutput, 0, 255, 0, 100);
  } else {
    if (currentTempMode == TEMP_OFF) {
      currentModeStr = "OFF";
    } else if (currentTempMode == TEMP_200) {
      currentModeStr = String(MODE_1_TEMP) + "Â°C";
    } else if (currentTempMode == TEMP_220) {
      currentModeStr = String(MODE_2_TEMP) + "Â°C";
    } else if (currentTempMode == TEMP_240) {
      currentModeStr = String(MODE_3_TEMP) + "Â°C";
    } else if (currentTempMode == TEMP_MAX) {
      currentModeStr = "100%";
    }
    
    if (currentTempMode == TEMP_MAX) {
      pwmPercent = 100;
    } else if (currentTempMode != TEMP_OFF) {
      pwmPercent = map(pidOutput, 0, 255, 0, 100);
    }
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

  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover'>";
  html += "<title>Temperature Controller</title>";
  html += "<style>";
  html += ":root{";
  html += "--bg-primary:#ffffff;";
  html += "--bg-secondary:#f8f9fa;";
  html += "--bg-card:#ffffff;";
  html += "--text-primary:#1d1d1f;";
  html += "--text-secondary:#6e6e73;";
  html += "--border-color:#d2d2d7;";
  html += "--shadow:0 4px 6px -1px rgba(0,0,0,0.1);";
  html += "}";
  html += "[data-theme='dark']{";
  html += "--bg-primary:#000000;";
  html += "--bg-secondary:#1c1c1e;";
  html += "--bg-card:#2c2c2e;";
  html += "--text-primary:#f2f2f7;";
  html += "--text-secondary:#8e8e93;";
  html += "--border-color:#38383a;";
  html += "--shadow:0 4px 6px -1px rgba(0,0,0,0.3);";
  html += "}";
  html += "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;margin:0;padding:0;background:var(--bg-primary);color:var(--text-primary);transition:all 0.3s ease;padding-bottom:20px;overflow:hidden;position:fixed;width:100%;height:100vh;touch-action:manipulation;overscroll-behavior:none;-webkit-overflow-scrolling:touch}";
  html += ".container{margin:1%;height:100vh;overflow:hidden;box-sizing:border-box}";
  html += ".header{display:flex;justify-content:space-between;align-items:center;margin-bottom:10px}";
  html += ".mode{font-size:2.2em;margin:0;font-weight:600;color:var(--text-primary)}";
  html += ".theme-toggle{width:50px;height:26px;background:var(--bg-card);border:1px solid var(--border-color);border-radius:13px;position:relative;cursor:pointer;transition:all 0.3s ease;box-shadow:inset 0 1px 3px rgba(0,0,0,0.1)}";
  html += ".theme-toggle:hover{transform:scale(1.05)}";
  html += ".theme-icon{width:22px;height:22px;border-radius:50%;position:absolute;top:1px;left:1px;transition:all 0.3s cubic-bezier(0.175,0.885,0.32,1.275);display:flex;align-items:center;justify-content:center;font-size:12px;background:#ffcc02;box-shadow:0 2px 4px rgba(0,0,0,0.2)}";
  html += "[data-theme='dark'] .theme-icon{transform:translateX(24px);background:#5e5ce6}";
  html += ".tabs{display:flex;background:var(--bg-secondary);margin:0;padding:0;border-radius:12px;overflow:hidden;box-shadow:var(--shadow)}";
  html += ".tab{flex:1;padding:15px;text-align:center;border:none;background:transparent;cursor:pointer;font-size:1.2em;color:var(--text-secondary);transition:all 0.3s ease;font-weight:500}";
  html += ".tab.active{background:var(--bg-card);color:var(--text-primary);box-shadow:0 1px 3px rgba(0,0,0,0.1)}";
  html += ".tab-content{display:none;padding:10px 0}";
  html += ".tab-content.active{display:block}";
  html += ".info-row{display:flex;align-items:center;gap:2px;margin:12px 0 10px 0;font-size:0.95em;color:var(--text-secondary);font-weight:500}";
  html += ".info-item{min-width:0;white-space:nowrap}";
  html += ".info-item:first-child{flex:0 0 auto;text-align:left}";
  html += ".info-item:nth-child(2){flex:0 0 auto;text-align:center;margin:0 auto}";
  html += ".info-item:last-child{flex:0 0 auto;text-align:right}";
  html += ".btn{width:100%;padding:16px;font-size:1.8em;margin:6px 0;border:none;border-radius:12px;color:white;cursor:pointer;user-select:none;font-weight:600;transition:all 0.2s ease;box-shadow:0 2px 8px rgba(0,0,0,0.15)}";
  html += ".btn:hover{transform:translateY(-1px);box-shadow:0 4px 12px rgba(0,0,0,0.2)}";
  html += ".btn:active{transform:translateY(0);box-shadow:0 1px 4px rgba(0,0,0,0.2)}";
  html += ".btn-small{width:100%;padding:10px;font-size:1.2em;margin:4px 0;border:none;border-radius:10px;color:white;cursor:pointer;font-weight:500;transition:all 0.2s ease}";
  html += ".red{background:linear-gradient(135deg,#ff3b30,#d70015);border:1px solid rgba(255,59,48,0.3)}";
  html += ".blue{background:linear-gradient(135deg,#007aff,#0051d5);border:1px solid rgba(0,122,255,0.3)}";
  html += ".gray{background:linear-gradient(135deg,#8e8e93,#636366);border:1px solid rgba(142,142,147,0.3)}";
  html += ".orange{background:linear-gradient(135deg,#ff9500,#ff6300);border:1px solid rgba(255,149,0,0.3)}";
  html += ".darkred{background:linear-gradient(135deg,#d70015,#a50000);border:1px solid rgba(215,0,21,0.3)}";
  html += ".green{background:linear-gradient(135deg,#34c759,#248a3d);border:1px solid rgba(52,199,89,0.3)}";
  html += ".purple{background:linear-gradient(135deg,#af52de,#7c2d92);border:1px solid rgba(175,82,222,0.3)}";
  html += ".cyan{background:linear-gradient(135deg,#5ac8fa,#007aff);border:1px solid rgba(90,200,250,0.3)}";
  html += ".yellow{background:linear-gradient(135deg,#ffcc02,#ffab00);border:1px solid rgba(255,204,2,0.3)}";
  html += ".magenta{background:linear-gradient(135deg,#ff2d92,#af52de);border:1px solid rgba(255,45,146,0.3)}";
  html += ".row{display:flex;justify-content:space-between;gap:2%}";
  html += ".row .btn{flex:1;padding:15px;font-size:1.15em}";
  html += ".row .btn-small{flex:1;padding:8px;font-size:1.0em;margin:2px}";
  html += ".timer{font-size:1.4em;margin:0;font-weight:600;color:var(--text-primary);flex:1}";
  html += ".timer-row{display:flex;align-items:center;gap:15px;background:var(--bg-card);padding:10px;border-radius:12px;margin:10px 0;box-shadow:var(--shadow)}";
  html += ".timer-btn{padding:8px 12px;font-size:0.9em;border:none;border-radius:8px;color:white;cursor:pointer;width:auto;font-weight:500;transition:all 0.2s ease}";
  html += ".cycle-row{display:flex;align-items:center;gap:10px;margin:10px 0;background:var(--bg-card);padding:10px;border-radius:12px;box-shadow:var(--shadow)}";
  html += ".cycle-count{font-size:1.4em;color:var(--text-primary);font-weight:600;flex:1}";
  html += ".cycle-btn{padding:8px 12px;font-size:0.9em;border:none;border-radius:8px;color:white;cursor:pointer;background:linear-gradient(135deg,#ff3b30,#d70015);font-weight:500}";
  html += ".cycle-btn-small{padding:8px 12px;font-size:0.85em;border:none;border-radius:6px;color:white;cursor:pointer;background:linear-gradient(135deg,#007aff,#0051d5);font-weight:500;width:64px;min-width:64px}";
  html += ".cycle-btn-small:first-of-type{margin-left:10px}";
  html += ".cycle-btn-small:last-of-type{margin-left:4px}";
  html += ".disabled{opacity:0.6;pointer-events:none}";
  html += ".ota-section{text-align:center;padding:40px 20px;background:var(--bg-card);border-radius:16px;margin:15px 0;box-shadow:var(--shadow)}";
  html += ".ota-title{font-size:2.2em;margin-bottom:25px;color:var(--text-primary);font-weight:700}";
  html += ".ota-input{width:100%;max-width:400px;padding:15px;font-size:1.2em;border:2px solid var(--border-color);border-radius:12px;margin-bottom:30px;background:var(--bg-primary);color:var(--text-primary);transition:all 0.3s ease}";
  html += ".ota-input:focus{outline:none;border-color:#007aff;box-shadow:0 0 0 3px rgba(0,122,255,0.1)}";
  html += ".ota-btn{width:100%;max-width:400px;padding:15px;font-size:1.5em;border:none;border-radius:12px;color:white;background:linear-gradient(135deg,#007aff,#0051d5);cursor:pointer;font-weight:600;transition:all 0.2s ease}";
  html += ".version{text-align:center;font-size:1.1em;color:var(--text-secondary);margin:20px 0;font-weight:500}";
  html += ".holding{background:linear-gradient(135deg,#ff3b30,#d70015) !important;animation:pulse 1s infinite}";
  html += ".flash-saved{animation:flashSaved 0.6s ease-in-out}";
  html += "@keyframes pulse{0%,100%{transform:scale(1)}50%{transform:scale(1.02)}}";
  html += "@keyframes flashSaved{0%,100%{background:var(--bg-card)}16.66%,33.33%{background:#ffffff;color:#000000}50%,66.66%{background:var(--bg-card)}83.33%{background:#ffffff;color:#000000}}";
  html += "</style>";
  html += "</head><body data-theme='" + String(isDarkTheme ? "dark" : "light") + "'>";
  
  html += "<div class='tabs'>";
  html += "<button class='tab active' onclick='showTab(\"main\")'>Main</button>";
  html += "<button class='tab' onclick='showTab(\"update\")'>Update</button>";
  html += "</div>";
  
  html += "<div class='container'>";
  html += "<div id='main' class='tab-content active'>";
  html += "<div class='header'>";
  html += "<div class='mode'>Mode: " + currentModeStr + "</div>";
  html += "<div class='theme-toggle' onclick='toggleTheme()'>";
  html += "<div class='theme-icon'>" + String(isDarkTheme ? "🌙" : "☀️") + "</div>";
  html += "</div>";
  html += "</div>";
  html += "<div class='info-row'>";
  html += "<span class='info-item'><strong>Temp:</strong> " + String((int)temperature) + "°C</span>";
  html += "<span class='info-item'><strong>PWM:</strong> " + String(pwmPercent) + "%</span>";
  html += "<span class='info-item'><strong>Battery:</strong> " + String(batteryVoltage, 1) + "V " + String(batteryPercent) + "%</span>";
  html += "</div>";
  html += "<div class='timer-row'>";
  html += "<div class='timer'>" + timerString + "</div>";
  html += "<button class='timer-btn purple' onclick='timerReset(this)'>Reset</button>";
  html += "<button class='timer-btn cyan' onclick='timerPause(this)'>" + String(timerPaused ? "Resume" : "Pause") + "</button>";
  html += "</div>";
  html += "<div class='cycle-row'>";
  html += "<div class='cycle-count'>Cycles: " + String(heatCycleCount) + "</div>";
  html += "<button class='cycle-btn' onclick='cycleReset(this)'>Reset</button>";
  html += "<button class='cycle-btn-small' onclick='cycleAdjust(-1, this)'>-</button>";
  html += "<button class='cycle-btn-small' onclick='cycleAdjust(1, this)'>+</button>";
  html += "</div>";
  html += "<div class='row'>";
  html += "<button class='btn orange temp-btn' data-mode='1' data-temp='" + String(MODE_1_TEMP) + "'>" + String(MODE_1_TEMP) + "°C</button>";
  html += "<button class='btn red temp-btn' data-mode='2' data-temp='" + String(MODE_2_TEMP) + "'>" + String(MODE_2_TEMP) + "°C</button>";
  html += "<button class='btn darkred temp-btn' data-mode='3' data-temp='" + String(MODE_3_TEMP) + "'>" + String(MODE_3_TEMP) + "°C</button>";
  html += "</div>";
  html += "<div class='row'>";
  html += "<button class='btn cyan' onclick='adjustTemp(-10, this)'>-10°C</button>";
  html += "<button class='btn red' onclick='adjustTemp(10, this)'>+10°C</button>";
  html += "</div>";
  html += "<div class='row'>";
  html += "<button class='btn blue' onclick='adjustPWM(-10, this)'>PWM-10%</button>";
  html += "<button class='btn red' onclick='adjustPWM(10, this)'>PWM+10%</button>";
  html += "</div>";
  html += "<button class='btn green' onclick='setTemp(4, this)'>100%</button>";
  html += "<button class='btn gray' onclick='setTemp(0, this)'>OFF</button>";
  
  String ledOffClass = (currentLEDMode == LED_OFF) ? "gray" : "yellow";
  String ledTempClass = (currentLEDMode == LED_TEMP) ? "orange" : "blue";
  String ledFlowClass = (currentLEDMode == LED_FLOW) ? "magenta" : "purple";
  
  html += "<div class='row'>";
  html += "<button class='btn-small " + ledOffClass + "' onclick='setLEDMode(0, this)'>LED OFF</button>";
  html += "<button class='btn-small " + ledTempClass + "' onclick='setLEDMode(1, this)'>LED TEMP</button>";
  html += "<button class='btn-small " + ledFlowClass + "' onclick='setLEDMode(2, this)'>LED FLOW</button>";
  html += "</div>";
  html += "</div>";
  
  html += "<div id='update' class='tab-content'>";
  html += "<div class='ota-section'>";
  html += "<div class='ota-title'>Firmware Update</div>";
  html += "<form id='otaForm' enctype='multipart/form-data'>";
  html += "<input type='file' class='ota-input' name='update' accept='.bin' required>";
  html += "<br><button type='submit' class='ota-btn'>Update Firmware</button>";
  html += "</form>";
  html += "<div class='version'>Current Version: " + String(FIRMWARE_VERSION) + "</div>";
  html += "</div></div></div>";
  
  html += "<script>";
  html += "let autoRefreshTimer;";
  html += "let currentTab = 'main';";
  html += "let updateTabTimer;";
  html += "let holdTimers = {};";
  html += "let isHolding = false;";
  html += "function toggleTheme(){";
  html += "const body = document.body;";
  html += "const themeIcon = document.querySelector('.theme-icon');";
  html += "const currentTheme = body.getAttribute('data-theme');";
  html += "const newTheme = currentTheme === 'dark' ? 'light' : 'dark';";
  html += "const isDark = newTheme === 'dark';";
  html += "body.setAttribute('data-theme', newTheme);";
  html += "themeIcon.textContent = isDark ? '🌙' : '☀️';";
  html += "try{";
  html += "if(typeof(Storage) !== 'undefined'){";
  html += "localStorage.setItem('cleanHeatTheme', newTheme);";
  html += "}";
  html += "fetch('/set_theme?dark=' + isDark).then(response => {";
  html += "if(response.ok) console.log('Theme saved on server:', newTheme);";
  html += "else console.log('Failed to save theme on server');";
  html += "}).catch(e => console.log('Error saving theme:', e));";
  html += "}catch(e){console.log('Could not save theme:', e);}}";
  html += "function loadTheme(){";
  html += "fetch('/get_theme').then(response => response.json()).then(data => {";
  html += "const serverTheme = data.dark ? 'dark' : 'light';";
  html += "console.log('Server theme:', serverTheme);";
  html += "const body = document.body;";
  html += "const currentTheme = body.getAttribute('data-theme');";
  html += "if(currentTheme !== serverTheme){";
  html += "const themeIcon = document.querySelector('.theme-icon');";
  html += "body.setAttribute('data-theme', serverTheme);";
  html += "if(themeIcon) themeIcon.textContent = serverTheme === 'dark' ? '🌙' : '☀️';";
  html += "}";
  html += "try{";
  html += "if(typeof(Storage) !== 'undefined'){";
  html += "localStorage.setItem('cleanHeatTheme', serverTheme);";
  html += "}";
  html += "}catch(e){}";
  html += "}).catch(e => {";
  html += "console.log('Could not load theme from server');";
  html += "});}"; 
  html += "function preloadTheme(){return;}";
  html += "function disableButton(btn){btn.classList.add('disabled');}";
  html += "function enableButton(btn){btn.classList.remove('disabled');}";
  html += "function updateStatus(){";
  html += "if(currentTab !== 'main' || isHolding) return;";
  html += "fetch('/status').then(response => response.json()).then(data => {";
  html += "document.querySelector('.mode').textContent = 'Mode: ' + data.mode;";
  html += "document.querySelector('.info-row').innerHTML = '<span class=\"info-item\"><strong>Temp:</strong> ' + data.temp + '°C</span><span class=\"info-item\"><strong>PWM:</strong> ' + data.pwm + '%</span><span class=\"info-item\"><strong>Battery:</strong> ' + data.battery_v + 'V ' + data.battery_p + '%</span>';";
  html += "document.querySelector('.timer').textContent = data.timer;";
  html += "document.querySelector('.cycle-count').textContent = 'Cycles: ' + data.cycles;";
  html += "const pauseBtn = document.querySelector('.timer-btn.cyan');";
  html += "if(pauseBtn) pauseBtn.textContent = data.timer_paused === 'true' ? 'Resume' : 'Pause';";
  html += "const ledBtns = document.querySelectorAll('.btn-small');";
  html += "if(ledBtns.length >= 3){";
  html += "ledBtns[0].className = 'btn-small ' + (data.led_mode === 'OFF' ? 'gray' : 'yellow');";
  html += "ledBtns[1].className = 'btn-small ' + (data.led_mode === 'TEMP' ? 'orange' : 'blue');";
  html += "ledBtns[2].className = 'btn-small ' + (data.led_mode === 'FLOW' ? 'magenta' : 'purple');";
  html += "}";
  html += "const tempBtns = document.querySelectorAll('.temp-btn');";
  html += "if(tempBtns[0]) tempBtns[0].textContent = data.mode1_temp + '°C';";
  html += "if(tempBtns[1]) tempBtns[1].textContent = data.mode2_temp + '°C';";
  html += "if(tempBtns[2]) tempBtns[2].textContent = data.mode3_temp + '°C';";
  html += "const serverDarkTheme = data.dark_theme === true || data.dark_theme === 'true';";
  html += "const currentTheme = document.body.getAttribute('data-theme');";
  html += "const currentIsDark = currentTheme === 'dark';";
  html += "if(serverDarkTheme !== currentIsDark){";
  html += "const newTheme = serverDarkTheme ? 'dark' : 'light';";
  html += "document.body.setAttribute('data-theme', newTheme);";
  html += "const themeIcon = document.querySelector('.theme-icon');";
  html += "if(themeIcon) themeIcon.textContent = serverDarkTheme ? '🌙' : '☀️';";
  html += "}";
  html += "}).catch(error => console.error('Status update error:', error));}"; 
  html += "function startStatusUpdates(){";
  html += "updateStatus();";
  html += "setInterval(updateStatus, 1000);}";
  html += "function makeRequest(url, button, callback){";
  html += "disableButton(button);";
  html += "fetch(url).then(response => {";
  html += "if(response.ok){";
  html += "setTimeout(() => {";
  html += "enableButton(button);";
  html += "setTimeout(() => updateStatus(), 100);";
  html += "if(callback) callback(response);";
  html += "}, 300);";
  html += "}else{enableButton(button);console.error('Request failed');}";
  html += "}).catch(error => {enableButton(button);console.error('Error:', error);});}";
  html += "function showTab(tabName){";
  html += "var tabs = document.querySelectorAll('.tab');";
  html += "var contents = document.querySelectorAll('.tab-content');";
  html += "for(var i = 0; i < tabs.length; i++){tabs[i].classList.remove('active');}";
  html += "for(var i = 0; i < contents.length; i++){contents[i].classList.remove('active');}";
  html += "var clickedTab = null;";
  html += "for(var i = 0; i < tabs.length; i++){";
  html += "if(tabs[i].textContent === tabName || (tabName === 'main' && tabs[i].textContent === 'Main') || (tabName === 'update' && tabs[i].textContent === 'Update')){";
  html += "clickedTab = tabs[i]; break;}}";
  html += "if(clickedTab) clickedTab.classList.add('active');";
  html += "document.getElementById(tabName).classList.add('active');";
  html += "currentTab = tabName;";
  html += "clearTimeout(autoRefreshTimer);";
  html += "clearInterval(updateTabTimer);";
  html += "if(tabName === 'update'){";
  html += "fetch('/update_tab_active');";
  html += "updateTabTimer = setInterval(function(){fetch('/update_tab_active');}, 5000);";
  html += "}else{";
  html += "fetch('/main_tab_active');";
  html += "startStatusUpdates();}}"; 

  html += "function setTemp(mode, btn){makeRequest('/set_temp?mode='+mode, btn);}";
  html += "function adjustTemp(delta, btn){makeRequest('/adjust_temp?delta='+delta, btn);}";
  html += "function adjustPWM(delta, btn){makeRequest('/pwm_adjust?delta='+delta, btn);}";
  html += "function setLEDMode(mode, btn){makeRequest('/set_led_mode?mode='+mode, btn);}";
  html += "function toggleLED(btn){makeRequest('/toggle_led', btn);}";
  html += "function timerReset(btn){makeRequest('/timer_reset', btn);}";
  html += "function timerPause(btn){makeRequest('/timer_pause', btn);}";
  html += "function cycleReset(btn){makeRequest('/cycle_reset', btn);}";
  html += "function cycleAdjust(delta, btn){makeRequest('/cycle_adjust?delta='+delta, btn);}";
  html += "function startHold(btn, mode){";
  html += "isHolding = true;";
  html += "btn.classList.add('holding');";
  html += "holdTimers[mode] = setTimeout(function(){";
  html += "fetch('/save_mode_temp?mode=' + mode).then(response => response.text()).then(result => {";
  html += "if(result === 'SAVED'){";
  html += "btn.textContent = 'SAVED!';";
  html += "btn.classList.remove('holding');";
  html += "btn.classList.add('flash-saved');";
  html += "setTimeout(() => {";
  html += "updateStatus();";
  html += "btn.classList.remove('flash-saved');";
  html += "isHolding = false;";
  html += "}, 1000);";
  html += "}";
  html += "}).catch(error => {";
  html += "console.error('Error:', error);";
  html += "btn.classList.remove('holding');";
  html += "isHolding = false;";
  html += "});";
  html += "}, 2000);}";
  html += "function cancelHold(btn, mode){";
  html += "isHolding = false;";
  html += "clearTimeout(holdTimers[mode]);";
  html += "btn.classList.remove('holding');}";
  html += "document.addEventListener('DOMContentLoaded', function(){";
  html += "const tempButtons = document.querySelectorAll('.temp-btn');";
  html += "tempButtons.forEach(function(btn){";
  html += "const mode = btn.dataset.mode;";
  html += "btn.addEventListener('mousedown', function(e){";
  html += "e.preventDefault();";
  html += "startHold(btn, mode);";
  html += "});";
  html += "btn.addEventListener('mouseup', function(e){";
  html += "e.preventDefault();";
  html += "if(holdTimers[mode]){";
  html += "cancelHold(btn, mode);";
  html += "setTemp(mode, btn);";
  html += "}";
  html += "});";
  html += "btn.addEventListener('mouseleave', function(e){";
  html += "if(holdTimers[mode]){";
  html += "cancelHold(btn, mode);";
  html += "}";
  html += "});";
  html += "btn.addEventListener('touchstart', function(e){";
  html += "e.preventDefault();";
  html += "startHold(btn, mode);";
  html += "});";
  html += "btn.addEventListener('touchend', function(e){";
  html += "e.preventDefault();";
  html += "if(holdTimers[mode]){";
  html += "cancelHold(btn, mode);";
  html += "setTemp(mode, btn);";
  html += "}";
  html += "});";
  html += "btn.addEventListener('touchcancel', function(e){";
  html += "if(holdTimers[mode]){";
  html += "cancelHold(btn, mode);";
  html += "}";
  html += "});";
  html += "btn.addEventListener('click', function(e){";
  html += "e.preventDefault();";
  html += "});";
  html += "});";
  html += "});";
  html += "document.getElementById('otaForm').addEventListener('submit', function(e){";
  html += "e.preventDefault();";
  html += "clearInterval(updateTabTimer);";
  html += "const formData = new FormData(this);";
  html += "const submitBtn = this.querySelector('.ota-btn');";
  html += "submitBtn.textContent = 'Updating...';";
  html += "submitBtn.style.backgroundColor = '#ff9800';";
  html += "fetch('/update', {method: 'POST', body: formData}).then(response => response.text()).then(result => {";
  html += "if(result === 'OK'){";
  html += "submitBtn.textContent = 'Success!';";
  html += "submitBtn.style.backgroundColor = '#4caf50';";
  html += "setTimeout(() => {window.location.reload();}, 3000);";
  html += "}else{";
  html += "submitBtn.textContent = 'Failed!';";
  html += "submitBtn.style.backgroundColor = '#f44336';";
  html += "setTimeout(() => {submitBtn.textContent = 'Update Firmware';submitBtn.style.backgroundColor = '#2196f3';}, 3000);";
  html += "}}).catch(error => {";
  html += "console.error('Error:', error);";
  html += "submitBtn.textContent = 'Error!';";
  html += "submitBtn.style.backgroundColor = '#f44336';";
  html += "setTimeout(() => {submitBtn.textContent = 'Update Firmware';submitBtn.style.backgroundColor = '#2196f3';}, 3000);";
  html += "});});";
  html += "startStatusUpdates();";
  html += "</script></body></html>";

  return html;
}

#endif