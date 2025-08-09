#include "mqtt_control.h"
#include "secrets.h"  // Include the secrets file
#include <HTTPClient.h>
#include <ArduinoJson.h>

// MQTT topics for first LED lamp control (ui_Button1)
const char* light_command_topic = "bedroom/led_lamp/light/led_lamp/command";
const char* light_state_topic = "bedroom/led_lamp/light/led_lamp/state";
const char* brightness_topic = "bedroom/led_lamp/number/led_brightness_mqtt/command";
const char* switch_topic = "bedroom/led_lamp/switch/led_lamp_mqtt_control/command";
const char* brightness_state_topic = "bedroom/led_lamp/number/led_brightness_mqtt/state";
const char* switch_state_topic = "bedroom/led_lamp/switch/led_lamp_mqtt_control/state";

// MQTT topics for IKEA LED strip control (ui_ikea)
const char* ikea_light_command_topic = "bedroom/lampa12vled2_12v_led_strip/light/12v_led_strip/command";
const char* ikea_light_state_topic = "bedroom/lampa12vled2_12v_led_strip/light/12v_led_strip/state";
const char* ikea_brightness_topic = "bedroom/lampa12vled2_12v_led_strip/number/command";
const char* ikea_switch_topic = "bedroom/lampa12vled2_12v_led_strip/switch/command";
const char* ikea_brightness_state_topic = "bedroom/lampa12vled2_12v_led_strip/number/state";
const char* ikea_switch_state_topic = "bedroom/lampa12vled2_12v_led_strip/switch/state";

// MQTT topics for svetlo1 LED lamp control (ui_svetlo1)
const char* svetlo1_light_command_topic = "light/led_lampa2_led_lamp/light/led_lamp/command";
const char* svetlo1_light_state_topic = "light/led_lampa2_led_lamp/light/led_lamp/state";
const char* svetlo1_brightness_topic = "light/led_lampa2_led_lamp/number/led_brightness_mqtt/command";
const char* svetlo1_switch_topic = "light/led_lampa2_led_lamp/switch/led_lamp_mqtt_control/command";
const char* svetlo1_brightness_state_topic = "light/led_lampa2_led_lamp/number/led_brightness_mqtt/state";
const char* svetlo1_switch_state_topic = "light/led_lampa2_led_lamp/switch/led_lamp_mqtt_control/state";

// MQTT topics for svetlo2 LED lamp control (ui_svetlo2)
const char* svetlo2_light_command_topic = "light/led_lampa3_led_lamp/light/led_lamp/command";
const char* svetlo2_light_state_topic = "light/led_lampa3_led_lamp/light/led_lamp/state";
const char* svetlo2_brightness_topic = "light/led_lampa3_led_lamp/number/led_brightness_mqtt/command";
const char* svetlo2_switch_topic = "light/led_lampa3_led_lamp/switch/led_lamp_mqtt_control/command";
const char* svetlo2_brightness_state_topic = "light/led_lampa3_led_lamp/number/led_brightness_mqtt/state";
const char* svetlo2_switch_state_topic = "light/led_lampa3_led_lamp/switch/led_lamp_mqtt_control/state";

// MQTT topics for svetlo3 LED strip control (ui_svetlo3)
const char* svetlo3_light_command_topic = "bedroom/lampa12vled_12v_led_strip/light/12v_led_strip/command";
const char* svetlo3_light_state_topic = "bedroom/lampa12vled_12v_led_strip/light/12v_led_strip/state";
const char* svetlo3_brightness_topic = "bedroom/lampa12vled_12v_led_strip/number/command";
const char* svetlo3_switch_topic = "bedroom/lampa12vled_12v_led_strip/switch/command";
const char* svetlo3_brightness_state_topic = "bedroom/lampa12vled_12v_led_strip/number/state";
const char* svetlo3_switch_state_topic = "bedroom/lampa12vled_12v_led_strip/switch/state";

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

// Control variables for first lamp (ui_Button1)
int currentBrightness = 50;
bool lampIsOn = false;
bool uiUpdateInProgress = false;
bool userInteracting = false;

// Control variables for IKEA lamp (ui_ikea)
int ikeaCurrentBrightness = 50;
bool ikeaLampIsOn = false;
bool ikeaUiUpdateInProgress = false;

// Control variables for svetlo1 lamp (ui_svetlo1)
int svetlo1CurrentBrightness = 50;
bool svetlo1LampIsOn = false;
bool svetlo1UiUpdateInProgress = false;

// Control variables for svetlo2 lamp (ui_svetlo2)
int svetlo2CurrentBrightness = 50;
bool svetlo2LampIsOn = false;
bool svetlo2UiUpdateInProgress = false;

// Control variables for svetlo3 lamp (ui_svetlo3)
int svetlo3CurrentBrightness = 50;
bool svetlo3LampIsOn = false;
bool svetlo3UiUpdateInProgress = false;

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
      
      // Subscribe to state topics for first lamp
      MQTTclient.subscribe(light_state_topic);
      MQTTclient.subscribe(brightness_state_topic);
      MQTTclient.subscribe(switch_state_topic);
      Serial.println("Subscribed to first lamp state topics");
      
      // Subscribe to state topics for IKEA lamp
      MQTTclient.subscribe(ikea_light_state_topic);
      MQTTclient.subscribe(ikea_brightness_state_topic);
      MQTTclient.subscribe(ikea_switch_state_topic);
      Serial.println("Subscribed to IKEA lamp state topics");
      
      // Subscribe to state topics for svetlo1 lamp
      MQTTclient.subscribe(svetlo1_light_state_topic);
      MQTTclient.subscribe(svetlo1_brightness_state_topic);
      MQTTclient.subscribe(svetlo1_switch_state_topic);
      Serial.println("Subscribed to svetlo1 lamp state topics");
      
      // Subscribe to state topics for svetlo2 lamp
      MQTTclient.subscribe(svetlo2_light_state_topic);
      MQTTclient.subscribe(svetlo2_brightness_state_topic);
      MQTTclient.subscribe(svetlo2_switch_state_topic);
      Serial.println("Subscribed to svetlo2 lamp state topics");
      
      // Subscribe to state topics for svetlo3 lamp
      MQTTclient.subscribe(svetlo3_light_state_topic);
      MQTTclient.subscribe(svetlo3_brightness_state_topic);
      MQTTclient.subscribe(svetlo3_switch_state_topic);
      Serial.println("Subscribed to svetlo3 lamp state topics");
      
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
  
  // Handle main light state updates (ui_Button1)
  if (String(topic) == light_state_topic) {
    if (message.indexOf("\"state\":\"ON\"") > -1) {
      lampIsOn = true;
      Serial.println("Main Lamp state: ON");
    } else if (message.indexOf("\"state\":\"OFF\"") > -1) {
      lampIsOn = false;
      Serial.println("Main Lamp state: OFF");
    }
    
    int brightnessStart = message.indexOf("\"brightness\":") + 13;
    int brightnessEnd = message.indexOf(",", brightnessStart);
    if (brightnessEnd == -1) brightnessEnd = message.indexOf("}", brightnessStart);
    
    if (brightnessStart > 12 && brightnessEnd > brightnessStart) {
      String brightnessStr = message.substring(brightnessStart, brightnessEnd);
      int brightness = brightnessStr.toInt();
      currentBrightness = map(brightness, 1, 255, 1, 100);
      Serial.print("Main Lamp brightness: ");
      Serial.print(brightness);
      Serial.print(" (");
      Serial.print(currentBrightness);
      Serial.println("%)");
    }
    
    updateUIFromMQTT();
  }
  
  // Handle IKEA lamp state updates (ui_ikea)
  else if (String(topic) == ikea_light_state_topic) {
    if (message.indexOf("\"state\":\"ON\"") > -1) {
      ikeaLampIsOn = true;
      Serial.println("IKEA Lamp state: ON");
    } else if (message.indexOf("\"state\":\"OFF\"") > -1) {
      ikeaLampIsOn = false;
      Serial.println("IKEA Lamp state: OFF");
    }
    
    int brightnessStart = message.indexOf("\"brightness\":") + 13;
    int brightnessEnd = message.indexOf(",", brightnessStart);
    if (brightnessEnd == -1) brightnessEnd = message.indexOf("}", brightnessStart);
    
    if (brightnessStart > 12 && brightnessEnd > brightnessStart) {
      String brightnessStr = message.substring(brightnessStart, brightnessEnd);
      int brightness = brightnessStr.toInt();
      ikeaCurrentBrightness = map(brightness, 1, 255, 1, 100);
      Serial.print("IKEA Lamp brightness: ");
      Serial.print(brightness);
      Serial.print(" (");
      Serial.print(ikeaCurrentBrightness);
      Serial.println("%)");
    }
    
    updateIkeaUIFromMQTT();
  }
  
  // Handle svetlo1 lamp state updates (ui_svetlo1)
  else if (String(topic) == svetlo1_light_state_topic) {
    if (message.indexOf("\"state\":\"ON\"") > -1) {
      svetlo1LampIsOn = true;
      Serial.println("Svetlo1 Lamp state: ON");
    } else if (message.indexOf("\"state\":\"OFF\"") > -1) {
      svetlo1LampIsOn = false;
      Serial.println("Svetlo1 Lamp state: OFF");
    }
    
    int brightnessStart = message.indexOf("\"brightness\":") + 13;
    int brightnessEnd = message.indexOf(",", brightnessStart);
    if (brightnessEnd == -1) brightnessEnd = message.indexOf("}", brightnessStart);
    
    if (brightnessStart > 12 && brightnessEnd > brightnessStart) {
      String brightnessStr = message.substring(brightnessStart, brightnessEnd);
      int brightness = brightnessStr.toInt();
      svetlo1CurrentBrightness = map(brightness, 1, 255, 1, 100);
      Serial.print("Svetlo1 Lamp brightness: ");
      Serial.print(brightness);
      Serial.print(" (");
      Serial.print(svetlo1CurrentBrightness);
      Serial.println("%)");
    }
    
    updateSvetlo1UIFromMQTT();
  }
  
  // Handle svetlo2 lamp state updates (ui_svetlo2)
  else if (String(topic) == svetlo2_light_state_topic) {
    if (message.indexOf("\"state\":\"ON\"") > -1) {
      svetlo2LampIsOn = true;
      Serial.println("Svetlo2 Lamp state: ON");
    } else if (message.indexOf("\"state\":\"OFF\"") > -1) {
      svetlo2LampIsOn = false;
      Serial.println("Svetlo2 Lamp state: OFF");
    }
    
    int brightnessStart = message.indexOf("\"brightness\":") + 13;
    int brightnessEnd = message.indexOf(",", brightnessStart);
    if (brightnessEnd == -1) brightnessEnd = message.indexOf("}", brightnessStart);
    
    if (brightnessStart > 12 && brightnessEnd > brightnessStart) {
      String brightnessStr = message.substring(brightnessStart, brightnessEnd);
      int brightness = brightnessStr.toInt();
      svetlo2CurrentBrightness = map(brightness, 1, 255, 1, 100);
      Serial.print("Svetlo2 Lamp brightness: ");
      Serial.print(brightness);
      Serial.print(" (");
      Serial.print(svetlo2CurrentBrightness);
      Serial.println("%)");
    }
    
    updateSvetlo2UIFromMQTT();
  }
  
  // Handle svetlo3 lamp state updates (ui_svetlo3)
  else if (String(topic) == svetlo3_light_state_topic) {
    if (message.indexOf("\"state\":\"ON\"") > -1) {
      svetlo3LampIsOn = true;
      Serial.println("Svetlo3 Lamp state: ON");
    } else if (message.indexOf("\"state\":\"OFF\"") > -1) {
      svetlo3LampIsOn = false;
      Serial.println("Svetlo3 Lamp state: OFF");
    }
    
    int brightnessStart = message.indexOf("\"brightness\":") + 13;
    int brightnessEnd = message.indexOf(",", brightnessStart);
    if (brightnessEnd == -1) brightnessEnd = message.indexOf("}", brightnessStart);
    
    if (brightnessStart > 12 && brightnessEnd > brightnessStart) {
      String brightnessStr = message.substring(brightnessStart, brightnessEnd);
      int brightness = brightnessStr.toInt();
      svetlo3CurrentBrightness = map(brightness, 1, 255, 1, 100);
      Serial.print("Svetlo3 Lamp brightness: ");
      Serial.print(brightness);
      Serial.print(" (");
      Serial.print(svetlo3CurrentBrightness);
      Serial.println("%)");
    }
    
    updateSvetlo3UIFromMQTT();
  }
  
  // Handle AC automation status updates (both Stan and Lab AC use same topic)
  else if (String(topic) == stan_ac_status_topic) {  // Same as lab_ac_status_topic
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
void updateUIFromMQTT() {
  unsigned long currentTime = millis();
  
  if (userInteracting || (currentTime - lastUserInteraction < userInteractionTimeout)) {
    Serial.println("Skipping main lamp UI update - user is interacting");
    return;
  }
  
  if (uiUpdateInProgress) return;
  
  uiUpdateInProgress = true;
  
  // Update arc if it's currently controlling this lamp
  if (ui_Arc1 != NULL && lastSelectedLamp == 1) {
    int currentArcValue = lv_arc_get_value(ui_Arc1);
    if (abs(currentArcValue - currentBrightness) > 2) {
      Serial.print("Updating arc from MQTT (Main Lamp): ");
      Serial.print(currentArcValue);
      Serial.print(" -> ");
      Serial.println(currentBrightness);
      lv_arc_set_value(ui_Arc1, currentBrightness);
    }
  }
  
  // Always update visual state based on actual ON/OFF status
  if (uic_Button1 != NULL) {
    if (lampIsOn) {
      lv_imgbtn_set_state(uic_Button1, LV_IMGBTN_STATE_PRESSED);
      Serial.println("Main Lamp Image Button visual state: ON (pressed state)");
    } else {
      lv_imgbtn_set_state(uic_Button1, LV_IMGBTN_STATE_RELEASED);
      Serial.println("Main Lamp Image Button visual state: OFF (released state)");
    }
  }
  
  uiUpdateInProgress = false;
}

// Update UI elements based on MQTT feedback for IKEA lamp
void updateIkeaUIFromMQTT() {
  if (ikeaUiUpdateInProgress) return;
  
  ikeaUiUpdateInProgress = true;
  
  // Update arc if it's currently controlling this lamp
  if (ui_Arc1 != NULL && lastSelectedLamp == 2) {
    int currentArcValue = lv_arc_get_value(ui_Arc1);
    if (abs(currentArcValue - ikeaCurrentBrightness) > 2) {
      Serial.print("Updating arc from MQTT (IKEA Lamp): ");
      Serial.print(currentArcValue);
      Serial.print(" -> ");
      Serial.println(ikeaCurrentBrightness);
      lv_arc_set_value(ui_Arc1, ikeaCurrentBrightness);
    }
  }
  
  // Always update visual state based on actual ON/OFF status
  if (ui_ikea != NULL) {
    if (ikeaLampIsOn) {
      lv_imgbtn_set_state(ui_ikea, LV_IMGBTN_STATE_PRESSED);
      Serial.println("IKEA Lamp Image Button visual state: ON (pressed state)");
    } else {
      lv_imgbtn_set_state(ui_ikea, LV_IMGBTN_STATE_RELEASED);
      Serial.println("IKEA Lamp Image Button visual state: OFF (released state)");
    }
  }
  
  ikeaUiUpdateInProgress = false;
}

// Update UI elements based on MQTT feedback for svetlo1 lamp
void updateSvetlo1UIFromMQTT() {
  if (svetlo1UiUpdateInProgress) return;
  
  svetlo1UiUpdateInProgress = true;
  
  // Update arc if it's currently controlling this lamp
  if (ui_Arc1 != NULL && lastSelectedLamp == 3) {
    int currentArcValue = lv_arc_get_value(ui_Arc1);
    if (abs(currentArcValue - svetlo1CurrentBrightness) > 2) {
      Serial.print("Updating arc from MQTT (Svetlo1 Lamp): ");
      Serial.print(currentArcValue);
      Serial.print(" -> ");
      Serial.println(svetlo1CurrentBrightness);
      lv_arc_set_value(ui_Arc1, svetlo1CurrentBrightness);
    }
  }
  
  // Always update visual state based on actual ON/OFF status
  if (ui_svetlo1 != NULL) {
    if (svetlo1LampIsOn) {
      lv_imgbtn_set_state(ui_svetlo1, LV_IMGBTN_STATE_PRESSED);
      Serial.println("Svetlo1 Lamp Image Button visual state: ON (pressed state)");
    } else {
      lv_imgbtn_set_state(ui_svetlo1, LV_IMGBTN_STATE_RELEASED);
      Serial.println("Svetlo1 Lamp Image Button visual state: OFF (released state)");
    }
  }
  
  svetlo1UiUpdateInProgress = false;
}

// Update UI elements based on MQTT feedback for svetlo2 lamp
void updateSvetlo2UIFromMQTT() {
  if (svetlo2UiUpdateInProgress) return;
  
  svetlo2UiUpdateInProgress = true;
  
  // Update arc if it's currently controlling this lamp
  if (ui_Arc1 != NULL && lastSelectedLamp == 4) {
    int currentArcValue = lv_arc_get_value(ui_Arc1);
    if (abs(currentArcValue - svetlo2CurrentBrightness) > 2) {
      Serial.print("Updating arc from MQTT (Svetlo2 Lamp): ");
      Serial.print(currentArcValue);
      Serial.print(" -> ");
      Serial.println(svetlo2CurrentBrightness);
      lv_arc_set_value(ui_Arc1, svetlo2CurrentBrightness);
    }
  }
  
  // Always update visual state based on actual ON/OFF status
  if (ui_svetlo2 != NULL) {
    if (svetlo2LampIsOn) {
      lv_imgbtn_set_state(ui_svetlo2, LV_IMGBTN_STATE_PRESSED);
      Serial.println("Svetlo2 Lamp Image Button visual state: ON (pressed state)");
    } else {
      lv_imgbtn_set_state(ui_svetlo2, LV_IMGBTN_STATE_RELEASED);
      Serial.println("Svetlo2 Lamp Image Button visual state: OFF (released state)");
    }
  }
  
  svetlo2UiUpdateInProgress = false;
}

// Update UI elements based on MQTT feedback for svetlo3 lamp
void updateSvetlo3UIFromMQTT() {
  if (svetlo3UiUpdateInProgress) return;
  
  svetlo3UiUpdateInProgress = true;
  
  // Update arc if it's currently controlling this lamp
  if (ui_Arc1 != NULL && lastSelectedLamp == 5) {
    int currentArcValue = lv_arc_get_value(ui_Arc1);
    if (abs(currentArcValue - svetlo3CurrentBrightness) > 2) {
      Serial.print("Updating arc from MQTT (Svetlo3 Lamp): ");
      Serial.print(currentArcValue);
      Serial.print(" -> ");
      Serial.println(svetlo3CurrentBrightness);
      lv_arc_set_value(ui_Arc1, svetlo3CurrentBrightness);
    }
  }
  
  // Always update visual state based on actual ON/OFF status
  if (ui_svetlo3 != NULL) {
    if (svetlo3LampIsOn) {
      lv_imgbtn_set_state(ui_svetlo3, LV_IMGBTN_STATE_PRESSED);
      Serial.println("Svetlo3 Lamp Image Button visual state: ON (pressed state)");
    } else {
      lv_imgbtn_set_state(ui_svetlo3, LV_IMGBTN_STATE_RELEASED);
      Serial.println("Svetlo3 Lamp Image Button visual state: OFF (released state)");
    }
  }
  
  svetlo3UiUpdateInProgress = false;
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

// LED control functions for main lamp
void setBrightness(int brightness) {
  if (!lampIsOn) {
    turnLampOn();
    delay(100);
  }
  
  int haBrightness = map(brightness, 1, 100, 1, 255);
  String jsonPayload = "{\"state\":\"ON\",\"brightness\":" + String(haBrightness) + "}";
  
  if (MQTTclient.publish(light_command_topic, jsonPayload.c_str())) {
    Serial.print("Main Lamp brightness set to: ");
    Serial.print(brightness);
    Serial.println("%");
  } else {
    Serial.println("Failed to set main lamp brightness");
  }
}

void turnLampOn() {
  String jsonPayload = "{\"state\":\"ON\",\"brightness\":" + String(map(currentBrightness, 1, 100, 1, 255)) + "}";
  if (MQTTclient.publish(light_command_topic, jsonPayload.c_str())) {
    Serial.print("Turned main LED lamp ON at ");
    Serial.print(currentBrightness);
    Serial.println("% brightness");
    lampIsOn = true;
  } else {
    Serial.println("Failed to turn main lamp ON");
  }
}

void turnLampOff() {
  String jsonPayload = "{\"state\":\"OFF\"}";
  if (MQTTclient.publish(light_command_topic, jsonPayload.c_str())) {
    Serial.println("Turned main LED lamp OFF");
    lampIsOn = false;
  } else {
    Serial.println("Failed to turn main lamp OFF");
  }
}

void toggleLamp() {
  if (!MQTTclient.connected()) {
    Serial.println("MQTT not connected - cannot toggle main lamp");
    return;
  }
  
  if (lampIsOn) {
    turnLampOff();
  } else {
    turnLampOn();
  }
}

// LED control functions for IKEA lamp
void setIkeaBrightness(int brightness) {
  if (!ikeaLampIsOn) {
    turnIkeaLampOn();
    delay(100);
  }
  
  int haBrightness = map(brightness, 1, 100, 1, 255);
  String jsonPayload = "{\"state\":\"ON\",\"brightness\":" + String(haBrightness) + "}";
  
  if (MQTTclient.publish(ikea_light_command_topic, jsonPayload.c_str())) {
    Serial.print("IKEA Lamp brightness set to: ");
    Serial.print(brightness);
    Serial.println("%");
  } else {
    Serial.println("Failed to set IKEA lamp brightness");
  }
}

void turnIkeaLampOn() {
  String jsonPayload = "{\"state\":\"ON\",\"brightness\":" + String(map(ikeaCurrentBrightness, 1, 100, 1, 255)) + "}";
  if (MQTTclient.publish(ikea_light_command_topic, jsonPayload.c_str())) {
    Serial.print("Turned IKEA LED lamp ON at ");
    Serial.print(ikeaCurrentBrightness);
    Serial.println("% brightness");
    ikeaLampIsOn = true;
  } else {
    Serial.println("Failed to turn IKEA lamp ON");
  }
}

void turnIkeaLampOff() {
  String jsonPayload = "{\"state\":\"OFF\"}";
  if (MQTTclient.publish(ikea_light_command_topic, jsonPayload.c_str())) {
    Serial.println("Turned IKEA LED lamp OFF");
    ikeaLampIsOn = false;
  } else {
    Serial.println("Failed to turn IKEA lamp OFF");
  }
}

void toggleIkeaLamp() {
  if (!MQTTclient.connected()) {
    Serial.println("MQTT not connected - cannot toggle IKEA lamp");
    return;
  }
  
  if (ikeaLampIsOn) {
    turnIkeaLampOff();
  } else {
    turnIkeaLampOn();
  }
}

// LED control functions for svetlo1 lamp
void setSvetlo1Brightness(int brightness) {
  if (!svetlo1LampIsOn) {
    turnSvetlo1LampOn();
    delay(100);
  }
  
  int haBrightness = map(brightness, 1, 100, 1, 255);
  String jsonPayload = "{\"state\":\"ON\",\"brightness\":" + String(haBrightness) + "}";
  
  if (MQTTclient.publish(svetlo1_light_command_topic, jsonPayload.c_str())) {
    Serial.print("Svetlo1 Lamp brightness set to: ");
    Serial.print(brightness);
    Serial.println("%");
  } else {
    Serial.println("Failed to set svetlo1 lamp brightness");
  }
}

void turnSvetlo1LampOn() {
  String jsonPayload = "{\"state\":\"ON\",\"brightness\":" + String(map(svetlo1CurrentBrightness, 1, 100, 1, 255)) + "}";
  if (MQTTclient.publish(svetlo1_light_command_topic, jsonPayload.c_str())) {
    Serial.print("Turned Svetlo1 LED lamp ON at ");
    Serial.print(svetlo1CurrentBrightness);
    Serial.println("% brightness");
    svetlo1LampIsOn = true;
  } else {
    Serial.println("Failed to turn svetlo1 lamp ON");
  }
}

void turnSvetlo1LampOff() {
  String jsonPayload = "{\"state\":\"OFF\"}";
  if (MQTTclient.publish(svetlo1_light_command_topic, jsonPayload.c_str())) {
    Serial.println("Turned Svetlo1 LED lamp OFF");
    svetlo1LampIsOn = false;
  } else {
    Serial.println("Failed to turn svetlo1 lamp OFF");
  }
}

void toggleSvetlo1Lamp() {
  if (!MQTTclient.connected()) {
    Serial.println("MQTT not connected - cannot toggle svetlo1 lamp");
    return;
  }
  
  if (svetlo1LampIsOn) {
    turnSvetlo1LampOff();
  } else {
    turnSvetlo1LampOn();
  }
}

// LED control functions for svetlo2 lamp
void setSvetlo2Brightness(int brightness) {
  if (!svetlo2LampIsOn) {
    turnSvetlo2LampOn();
    delay(100);
  }
  
  int haBrightness = map(brightness, 1, 100, 1, 255);
  String jsonPayload = "{\"state\":\"ON\",\"brightness\":" + String(haBrightness) + "}";
  
  if (MQTTclient.publish(svetlo2_light_command_topic, jsonPayload.c_str())) {
    Serial.print("Svetlo2 Lamp brightness set to: ");
    Serial.print(brightness);
    Serial.println("%");
  } else {
    Serial.println("Failed to set svetlo2 lamp brightness");
  }
}

void turnSvetlo2LampOn() {
  String jsonPayload = "{\"state\":\"ON\",\"brightness\":" + String(map(svetlo2CurrentBrightness, 1, 100, 1, 255)) + "}";
  if (MQTTclient.publish(svetlo2_light_command_topic, jsonPayload.c_str())) {
    Serial.print("Turned Svetlo2 LED lamp ON at ");
    Serial.print(svetlo2CurrentBrightness);
    Serial.println("% brightness");
    svetlo2LampIsOn = true;
  } else {
    Serial.println("Failed to turn svetlo2 lamp ON");
  }
}

void turnSvetlo2LampOff() {
  String jsonPayload = "{\"state\":\"OFF\"}";
  if (MQTTclient.publish(svetlo2_light_command_topic, jsonPayload.c_str())) {
    Serial.println("Turned Svetlo2 LED lamp OFF");
    svetlo2LampIsOn = false;
  } else {
    Serial.println("Failed to turn svetlo2 lamp OFF");
  }
}

void toggleSvetlo2Lamp() {
  if (!MQTTclient.connected()) {
    Serial.println("MQTT not connected - cannot toggle svetlo2 lamp");
    return;
  }
  
  if (svetlo2LampIsOn) {
    turnSvetlo2LampOff();
  } else {
    turnSvetlo2LampOn();
  }
}

// LED control functions for svetlo3 lamp
void setSvetlo3Brightness(int brightness) {
  if (!svetlo3LampIsOn) {
    turnSvetlo3LampOn();
    delay(100);
  }
  
  int haBrightness = map(brightness, 1, 100, 1, 255);
  String jsonPayload = "{\"state\":\"ON\",\"brightness\":" + String(haBrightness) + "}";
  
  if (MQTTclient.publish(svetlo3_light_command_topic, jsonPayload.c_str())) {
    Serial.print("Svetlo3 Lamp brightness set to: ");
    Serial.print(brightness);
    Serial.println("%");
  } else {
    Serial.println("Failed to set svetlo3 lamp brightness");
  }
}

void turnSvetlo3LampOn() {
  String jsonPayload = "{\"state\":\"ON\",\"brightness\":" + String(map(svetlo3CurrentBrightness, 1, 100, 1, 255)) + "}";
  if (MQTTclient.publish(svetlo3_light_command_topic, jsonPayload.c_str())) {
    Serial.print("Turned Svetlo3 LED lamp ON at ");
    Serial.print(svetlo3CurrentBrightness);
    Serial.println("% brightness");
    svetlo3LampIsOn = true;
  } else {
    Serial.println("Failed to turn svetlo3 lamp ON");
  }
}

void turnSvetlo3LampOff() {
  String jsonPayload = "{\"state\":\"OFF\"}";
  if (MQTTclient.publish(svetlo3_light_command_topic, jsonPayload.c_str())) {
    Serial.println("Turned Svetlo3 LED lamp OFF");
    svetlo3LampIsOn = false;
  } else {
    Serial.println("Failed to turn svetlo3 lamp OFF");
  }
}

void toggleSvetlo3Lamp() {
  if (!MQTTclient.connected()) {
    Serial.println("MQTT not connected - cannot toggle svetlo3 lamp");
    return;
  }
  
  if (svetlo3LampIsOn) {
    turnSvetlo3LampOff();
  } else {
    turnSvetlo3LampOn();
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