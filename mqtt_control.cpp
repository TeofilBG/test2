#include "mqtt_control.h"
#include "secrets.h"  // Include the secrets file
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Configuration for each controllable lamp
LampConfig lampConfigs[NUM_LAMPS] = {
  {
    "bedroom/led_lamp/light/led_lamp/command",
    "bedroom/led_lamp/light/led_lamp/state",
    "bedroom/led_lamp/number/led_brightness_mqtt/command",
    "bedroom/led_lamp/switch/led_lamp_mqtt_control/command",
    "bedroom/led_lamp/number/led_brightness_mqtt/state",
    "bedroom/led_lamp/switch/led_lamp_mqtt_control/state",
    50,
    false,
    false
  },
  {
    "bedroom/lampa12vled2_12v_led_strip/light/12v_led_strip/command",
    "bedroom/lampa12vled2_12v_led_strip/light/12v_led_strip/state",
    "bedroom/lampa12vled2_12v_led_strip/number/command",
    "bedroom/lampa12vled2_12v_led_strip/switch/command",
    "bedroom/lampa12vled2_12v_led_strip/number/state",
    "bedroom/lampa12vled2_12v_led_strip/switch/state",
    50,
    false,
    false
  },
  {
    "light/led_lampa2_led_lamp/light/led_lamp/command",
    "light/led_lampa2_led_lamp/light/led_lamp/state",
    "light/led_lampa2_led_lamp/number/led_brightness_mqtt/command",
    "light/led_lampa2_led_lamp/switch/led_lamp_mqtt_control/command",
    "light/led_lampa2_led_lamp/number/led_brightness_mqtt/state",
    "light/led_lampa2_led_lamp/switch/led_lamp_mqtt_control/state",
    50,
    false,
    false
  },
  {
    "light/led_lampa3_led_lamp/light/led_lamp/command",
    "light/led_lampa3_led_lamp/light/led_lamp/state",
    "light/led_lampa3_led_lamp/number/led_brightness_mqtt/command",
    "light/led_lampa3_led_lamp/switch/led_lamp_mqtt_control/command",
    "light/led_lampa3_led_lamp/number/led_brightness_mqtt/state",
    "light/led_lampa3_led_lamp/switch/led_lamp_mqtt_control/state",
    50,
    false,
    false
  },
  {
    "bedroom/lampa12vled_12v_led_strip/light/12v_led_strip/command",
    "bedroom/lampa12vled_12v_led_strip/light/12v_led_strip/state",
    "bedroom/lampa12vled_12v_led_strip/number/command",
    "bedroom/lampa12vled_12v_led_strip/switch/command",
    "bedroom/lampa12vled_12v_led_strip/number/state",
    "bedroom/lampa12vled_12v_led_strip/switch/state",
    50,
    false,
    false
  }
};


// MQTT topics for Stan AC automation button (ui_acbutton)
const char* stan_ac_on_automation_topic = "homeassistant/automation/stan_ac_on/trigger";
const char* stan_ac_off_automation_topic = "homeassistant/automation/stan_ac_off/trigger";
const char* stan_ac_status_topic = "esp32/automation/status";

// MQTT topics for Lab AC automation button (ui_acbuttonOFF)
const char* lab_ac_on_automation_topic = "homeassistant/automation/lab_klima_on/trigger";
const char* lab_ac_off_automation_topic = "homeassistant/automation/lab_klima_off/trigger";
const char* lab_ac_status_topic = "esp32/automation/status";  // Same status topic, different payloads

// MQTT topics for All Lights automation button (ui_SvaSvetla)
const char* sva_svetla_automation_topic = "homeassistant/automation/sva_svetla/trigger";
const char* sva_svetla_status_topic = "esp32/automation/status";  // Same status topic, different payloads

// Home Assistant API configuration for temperature - now using defines from secrets.h
const char* ha_server = MQTT_BROKER_IP;  // Using the same IP as MQTT broker
const int ha_port = 8123;
const char* ha_token = HA_ACCESS_TOKEN;
const char* temp_sensor_entity = "sensor.teperatura2_temperature";

// Network and MQTT credentials - now using defines from secrets.h
const char* mqtt_server = MQTT_BROKER_IP;
const char* MQTTuser = MQTT_USERNAME;
const char* MQTTpwd = MQTT_PASSWORD;
const char* HostName = DEVICE_HOSTNAME;

// MQTT objects
WiFiClient espClient;
PubSubClient MQTTclient(espClient);

// Global user interaction flag
bool userInteracting = false;

// Stan AC state tracking
bool stanAcIsOn = false;
bool acUiUpdateInProgress = false;  // Renamed for backward compatibility

// Lab AC state tracking
bool labAcIsOn = false;
bool labAcUiUpdateInProgress = false;

// All Lights state tracking
bool svaSvetlaInProgress = false;
bool svaSvetlaUiUpdateInProgress = false;

// Temperature state tracking
float currentTemperature = 0.0;
bool temperatureAvailable = false;
unsigned long lastTempRead = 0;
const unsigned long tempReadInterval = 30000; // 30 seconds

// Timing variables
unsigned long lastUserInteraction = 0;
const unsigned long userInteractionTimeout = 1000;

// Initialize MQTT
void initMQTT() {
  MQTTclient.setServer(mqtt_server, MQTT_BROKER_PORT);
  MQTTclient.setCallback(mqttCallback);
}

// MQTT connection function
void connectToMQTT() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Cannot connect to MQTT.");
    return;
  }
  
  Serial.print("Connecting to MQTT server: ");
  Serial.println(mqtt_server);
  
  int attempts = 0;
  while (!MQTTclient.connected() && attempts < 10) {
    if (MQTTclient.connect(HostName, MQTTuser, MQTTpwd)) {
      Serial.println("MQTT connected successfully!");
      
      // Subscribe to state topics for all lamps
      for (int i = 0; i < NUM_LAMPS; ++i) {
        MQTTclient.subscribe(lampConfigs[i].lightStateTopic);
        MQTTclient.subscribe(lampConfigs[i].brightnessStateTopic);
        MQTTclient.subscribe(lampConfigs[i].switchStateTopic);
      }
      Serial.println("Subscribed to lamp state topics");
      
      // Subscribe to AC automation status topics (both Stan and Lab use same topic, different payloads)
      MQTTclient.subscribe(stan_ac_status_topic);  // Same as lab_ac_status_topic
      Serial.println("Subscribed to AC automation status topics");
      
    } else {
      Serial.print("MQTT connection failed, rc=");
      Serial.print(MQTTclient.state());
      Serial.println(" Retrying in 2 seconds...");
      delay(2000);
      attempts++;
    }
  }
  
  if (!MQTTclient.connected()) {
    Serial.println("Failed to connect to MQTT broker!");
  }
}

// MQTT callback function to handle incoming messages
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String message = String((char*)payload);
  
  Serial.print("Received MQTT message on topic: ");
  Serial.print(topic);
  Serial.print(" | Message: ");
  Serial.println(message);

  // Check lamp topics
  for (int i = 0; i < NUM_LAMPS; ++i) {
    LampConfig &lamp = lampConfigs[i];
    if (String(topic) == lamp.lightStateTopic) {
      if (message.indexOf("\"state\":\"ON\"") > -1) {
        lamp.isOn = true;
      } else if (message.indexOf("\"state\":\"OFF\"") > -1) {
        lamp.isOn = false;
      }

      int brightnessStart = message.indexOf("\"brightness\":") + 13;
      int brightnessEnd = message.indexOf(",", brightnessStart);
      if (brightnessEnd == -1) brightnessEnd = message.indexOf("}", brightnessStart);
      if (brightnessStart > 12 && brightnessEnd > brightnessStart) {
        String brightnessStr = message.substring(brightnessStart, brightnessEnd);
        int brightness = brightnessStr.toInt();
        lamp.currentBrightness = map(brightness, 1, 255, 1, 100);
      }

      updateLampUIFromMQTT(i);
      return;
    }
  }

  // Handle AC automation status updates (both Stan and Lab AC use same topic)
  if (String(topic) == stan_ac_status_topic) {  // Same as lab_ac_status_topic
    if (message == "stan_ac_on_executed") {
      Serial.println("✅ Stan AC Automation CONFIRMED: stan_ac_on_executed - Stan AC is now ON!");
      stanAcIsOn = true;
      updateStanACUIFromMQTT();
    } else if (message == "stan_ac_off_executed") {
      Serial.println("✅ Stan AC Automation CONFIRMED: stan_ac_off_executed - Stan AC is now OFF!");
      stanAcIsOn = false;
      updateStanACUIFromMQTT();
    } else if (message == "lab_klima_on_executed") {
      Serial.println("✅ Lab AC Automation CONFIRMED: lab_klima_on_executed - Lab AC is now ON!");
      labAcIsOn = true;
      updateLabACUIFromMQTT();
    } else if (message == "lab_klima_off_executed") {
      Serial.println("✅ Lab AC Automation CONFIRMED: lab_klima_off_executed - Lab AC is now OFF!");
      labAcIsOn = false;
      updateLabACUIFromMQTT();
    } else if (message == "sva_svetla_executed") {
      Serial.println("✅ All Lights Automation CONFIRMED: sva_svetla_executed - All lights toggled!");
      svaSvetlaInProgress = false;
      updateSvaSvetlaUIFromMQTT();
    } else {
      Serial.print("AC Status received: ");
      Serial.println(message);
    }
  }
}

// Update UI elements based on MQTT feedback for main lamp
void updateLampUIFromMQTT(int index) {
  if (index < 0 || index >= NUM_LAMPS) return;
  LampConfig &lamp = lampConfigs[index];

  unsigned long currentTime = millis();
  if (userInteracting || (currentTime - lastUserInteraction < userInteractionTimeout)) {
    return;
  }

  if (lamp.uiUpdateInProgress) return;
  lamp.uiUpdateInProgress = true;

  // Update arc if it's currently controlling this lamp
  if (ui_Arc1 != NULL && lastSelectedLamp == index + 1) {
    int currentArcValue = lv_arc_get_value(ui_Arc1);
    if (abs(currentArcValue - lamp.currentBrightness) > 2) {
      lv_arc_set_value(ui_Arc1, lamp.currentBrightness);
    }
  }

  // Update corresponding button visual state
  lv_obj_t* buttons[NUM_LAMPS] = {uic_Button1, ui_ikea, ui_svetlo1, ui_svetlo2, ui_svetlo3};
  if (buttons[index] != NULL) {
    if (lamp.isOn) {
      lv_imgbtn_set_state(buttons[index], LV_IMGBTN_STATE_PRESSED);
    } else {
      lv_imgbtn_set_state(buttons[index], LV_IMGBTN_STATE_RELEASED);
    }
  }

  lamp.uiUpdateInProgress = false;
}

// Update UI elements based on MQTT feedback for Stan AC automation
void updateStanACUIFromMQTT() {
  if (acUiUpdateInProgress) return;
  
  acUiUpdateInProgress = true;
  
  // Always update Stan AC button visual state based on actual ON/OFF status
  if (ui_acbutton != NULL) {
    if (stanAcIsOn) {
      lv_obj_add_state(ui_acbutton, LV_STATE_CHECKED);
      Serial.println("Stan AC Button visual state: ON (checked state)");
    } else {
      lv_obj_clear_state(ui_acbutton, LV_STATE_CHECKED);
      Serial.println("Stan AC Button visual state: OFF (unchecked state)");
    }
  }
  
  // Always update Stan AC status label
  if (ui_AC1label != NULL) {
    if (stanAcIsOn) {
      lv_label_set_text(ui_AC1label, "ON");
      Serial.println("Stan AC Status Label updated: ON");
    } else {
      lv_label_set_text(ui_AC1label, "OFF");
      Serial.println("Stan AC Status Label updated: OFF");
    }
  }
  
  acUiUpdateInProgress = false;
}

// Update UI elements based on MQTT feedback for Lab AC automation
void updateLabACUIFromMQTT() {
  if (labAcUiUpdateInProgress) return;
  
  labAcUiUpdateInProgress = true;
  
  // Always update Lab AC button visual state based on actual ON/OFF status
  if (ui_acbuttonOFF != NULL) {
    if (labAcIsOn) {
      lv_obj_add_state(ui_acbuttonOFF, LV_STATE_CHECKED);
      Serial.println("Lab AC Button visual state: ON (checked state)");
    } else {
      lv_obj_clear_state(ui_acbuttonOFF, LV_STATE_CHECKED);
      Serial.println("Lab AC Button visual state: OFF (unchecked state)");
    }
  }
  
  // Always update Lab AC status label
  if (ui_AC1label2 != NULL) {
    if (labAcIsOn) {
      lv_label_set_text(ui_AC1label2, "ON");
      Serial.println("Lab AC Status Label updated: ON");
    } else {
      lv_label_set_text(ui_AC1label2, "OFF");
      Serial.println("Lab AC Status Label updated: OFF");
    }
  }
  
  labAcUiUpdateInProgress = false;
}

// Update UI elements based on MQTT feedback for All Lights automation
void updateSvaSvetlaUIFromMQTT() {
  if (svaSvetlaUiUpdateInProgress) return;
  
  svaSvetlaUiUpdateInProgress = true;
  
  // Update All Lights button visual state (briefly show as active during execution)
  if (ui_SvaSvetla != NULL) {
    if (svaSvetlaInProgress) {
      lv_obj_add_state(ui_SvaSvetla, LV_STATE_CHECKED);
      Serial.println("All Lights Button visual state: ACTIVE (processing)");
    } else {
      lv_obj_clear_state(ui_SvaSvetla, LV_STATE_CHECKED);
      Serial.println("All Lights Button visual state: READY (normal)");
    }
  }
  
  svaSvetlaUiUpdateInProgress = false;
}

// Generic lamp control functions
void setLampBrightness(LampConfig& lamp, int brightness) {
  if (!lamp.isOn) {
    turnLampOn(lamp);
    delay(100);
  }
  int haBrightness = map(brightness, 1, 100, 1, 255);
  String jsonPayload = "{\"state\":\"ON\",\"brightness\":" + String(haBrightness) + "}";
  if (!MQTTclient.publish(lamp.lightCommandTopic, jsonPayload.c_str())) {
    Serial.println("Failed to set lamp brightness");
  }
}

void turnLampOn(LampConfig& lamp) {
  String jsonPayload = "{\"state\":\"ON\",\"brightness\":" + String(map(lamp.currentBrightness, 1, 100, 1, 255)) + "}";
  if (MQTTclient.publish(lamp.lightCommandTopic, jsonPayload.c_str())) {
    lamp.isOn = true;
  } else {
    Serial.println("Failed to turn lamp ON");
  }
}

void turnLampOff(LampConfig& lamp) {
  String jsonPayload = "{\"state\":\"OFF\"}";
  if (MQTTclient.publish(lamp.lightCommandTopic, jsonPayload.c_str())) {
    lamp.isOn = false;
  } else {
    Serial.println("Failed to turn lamp OFF");
  }
}

void toggleLamp(LampConfig& lamp) {
  if (!MQTTclient.connected()) {
    Serial.println("MQTT not connected - cannot toggle lamp");
    return;
  }
  if (lamp.isOn) {
    turnLampOff(lamp);
  } else {
    turnLampOn(lamp);
  }
}

// Stan AC Automation functions
void triggerStanACOn() {
  if (!MQTTclient.connected()) {
    Serial.println("MQTT not connected - cannot trigger Stan AC ON");
    return;
  }
  
  if (MQTTclient.publish(stan_ac_on_automation_topic, "ON")) {
    Serial.println("🌡️ Stan AC ON triggered: automation.stan_ac_on");
    Serial.println("   Published: ON to homeassistant/automation/stan_ac_on/trigger");
    Serial.println("   Waiting for confirmation...");
  } else {
    Serial.println("❌ Failed to trigger Stan AC ON automation");
  }
}

void triggerStanACOff() {
  if (!MQTTclient.connected()) {
    Serial.println("MQTT not connected - cannot trigger Stan AC OFF");
    return;
  }
  
  if (MQTTclient.publish(stan_ac_off_automation_topic, "OFF")) {
    Serial.println("❄️ Stan AC OFF triggered: automation.stan_ac_off");
    Serial.println("   Published: OFF to homeassistant/automation/stan_ac_off/trigger");
    Serial.println("   Waiting for confirmation...");
  } else {
    Serial.println("❌ Failed to trigger Stan AC OFF automation");
  }
}

void toggleStanAC() {
  if (!MQTTclient.connected()) {
    Serial.println("MQTT not connected - cannot toggle Stan AC");
    return;
  }
  
  if (stanAcIsOn) {
    triggerStanACOff();
  } else {
    triggerStanACOn();
  }
}

// Lab AC Automation functions
void triggerLabACOn() {
  if (!MQTTclient.connected()) {
    Serial.println("MQTT not connected - cannot trigger Lab AC ON");
    return;
  }
  
  if (MQTTclient.publish(lab_ac_on_automation_topic, "ON")) {
    Serial.println("🌡️ Lab AC ON triggered: automation.lab_klima_on");
    Serial.println("   Published: ON to homeassistant/automation/lab_klima_on/trigger");
    Serial.println("   Waiting for confirmation...");
  } else {
    Serial.println("❌ Failed to trigger Lab AC ON automation");
  }
}

void triggerLabACOff() {
  if (!MQTTclient.connected()) {
    Serial.println("MQTT not connected - cannot trigger Lab AC OFF");
    return;
  }
  
  if (MQTTclient.publish(lab_ac_off_automation_topic, "OFF")) {
    Serial.println("❄️ Lab AC OFF triggered: automation.lab_klima_off");
    Serial.println("   Published: OFF to homeassistant/automation/lab_klima_off/trigger");
    Serial.println("   Waiting for confirmation...");
  } else {
    Serial.println("❌ Failed to trigger Lab AC OFF automation");
  }
}

void toggleLabAC() {
  if (!MQTTclient.connected()) {
    Serial.println("MQTT not connected - cannot toggle Lab AC");
    return;
  }
  
  if (labAcIsOn) {
    triggerLabACOff();
  } else {
    triggerLabACOn();
  }
}

// All Lights Automation functions
void triggerSvaSvetla() {
  if (!MQTTclient.connected()) {
    Serial.println("MQTT not connected - cannot trigger All Lights automation");
    return;
  }
  
  if (MQTTclient.publish(sva_svetla_automation_topic, "ON")) {
    Serial.println("💡 All Lights automation triggered: automation.sva_svetla");
    Serial.println("   Published: ON to homeassistant/automation/sva_svetla/trigger");
    Serial.println("   Waiting for confirmation...");
    svaSvetlaInProgress = true;
    updateSvaSvetlaUIFromMQTT();
  } else {
    Serial.println("❌ Failed to trigger All Lights automation");
  }
}

// Temperature functions
void readTemperatureFromHA() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastTempRead < tempReadInterval) {
    return;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    if (ui_temp != NULL) {
      lv_label_set_text(ui_temp, "No WiFi");
    }
    return;
  }
  
  HTTPClient http;
  WiFiClient client;
  
  String url = "http://" + String(ha_server) + ":" + String(ha_port) + "/api/states/" + String(temp_sensor_entity);
  
  Serial.print("🌡️ Requesting temperature from: ");
  Serial.println(url);
  
  http.begin(client, url);
  http.addHeader("Authorization", "Bearer " + String(ha_token));
  http.addHeader("Content-Type", "application/json");
  
  int httpResponseCode = http.GET();
  
  Serial.print("HTTP Response Code: ");
  Serial.println(httpResponseCode);
  
  if (httpResponseCode == 200) {
    String payload = http.getString();
    Serial.print("Response payload: ");
    Serial.println(payload);
    
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      String stateStr = doc["state"];
      
      if (stateStr != "unavailable" && stateStr != "unknown" && stateStr != "" && stateStr != "null") {
        currentTemperature = stateStr.toFloat();
        
        if (currentTemperature > -50 && currentTemperature < 100) {
          temperatureAvailable = true;
          updateTemperatureDisplay(currentTemperature);
          Serial.print("✅ Temperature read from HA: ");
          Serial.print(currentTemperature);
          Serial.println("°C");
        } else {
          temperatureAvailable = false;
          if (ui_temp != NULL) {
            lv_label_set_text(ui_temp, "Invalid");
          }
          Serial.println("❌ Temperature out of range");
        }
      } else {
        temperatureAvailable = false;
        if (ui_temp != NULL) {
          lv_label_set_text(ui_temp, "N/A");
        }
        Serial.println("❌ Temperature state unavailable");
      }
    } else {
      temperatureAvailable = false;
      if (ui_temp != NULL) {
        lv_label_set_text(ui_temp, "JSON Err");
      }
      Serial.print("❌ JSON parsing error: ");
      Serial.println(error.c_str());
    }
  } else if (httpResponseCode == 404) {
    temperatureAvailable = false;
    if (ui_temp != NULL) {
      lv_label_set_text(ui_temp, "OFFLINE");
    }
    Serial.print("❌ 404 Error - Temperature sensor not found: ");
    Serial.println(temp_sensor_entity);
    Serial.println("💡 Check if the entity exists in Home Assistant");
  } else if (httpResponseCode == 401) {
    temperatureAvailable = false;
    if (ui_temp != NULL) {
      lv_label_set_text(ui_temp, "Auth Err");
    }
    Serial.println("❌ 401 Error - Authentication failed. Check HA_ACCESS_TOKEN");
  } else {
    temperatureAvailable = false;
    if (ui_temp != NULL) {
      lv_label_set_text(ui_temp, "HTTP Err");
    }
    Serial.print("❌ HTTP Error: ");
    Serial.println(httpResponseCode);
    
    // Print response body for debugging
    String errorBody = http.getString();
    if (errorBody.length() > 0) {
      Serial.print("Error response: ");
      Serial.println(errorBody);
    }
  }
  
  http.end();
  lastTempRead = currentTime;
}

void updateTemperatureDisplay(float temperature) {
  if (ui_temp != NULL) {
    char tempStr[16];
    
    // Format temperature with 1 decimal place and °C symbol
    if (temperature == (int)temperature) {
      // If temperature is a whole number, show without decimal
      snprintf(tempStr, sizeof(tempStr), "%.0f°C", temperature);
    } else {
      // Show with 1 decimal place
      snprintf(tempStr, sizeof(tempStr), "%.1f°C", temperature);
    }
    
    lv_label_set_text(ui_temp, tempStr);
    Serial.print("Temperature display updated: ");
    Serial.println(tempStr);
  } else {
    Serial.println("Warning: ui_temp label not found!");
  }
}