// ST77916 Display Driver for SquareLine Studio UI with MQTT LED Control and Time
// Modularized version with separated UI Events, MQTT and Display Panel modules
// Main file - coordinates all modules - CLEAN VERSION WITH SCREEN DIMMING

#include <lvgl.h>
#include <ui.h>  // SquareLine Studio generated UI
#include <ESP_IOExpander.h>
#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"         // Include secrets file
#include "mqtt_control.h"    // MQTT module
#include "display_panel.h"   // Display Panel module
#include "ui_events.h"       // UI Events module

/*Don't forget to set Sketchbook location in File/Preferences to the path of your UI project (the parent folder of this INO file)*/

// Network credentials - now using defines from secrets.h
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// NTP Configuration - now using defines from secrets.h
const char* ntpServer = NTP_SERVER;
const long gmtOffset_sec = GMT_OFFSET_SEC;
const int daylightOffset_sec = DAYLIGHT_OFFSET_SEC;

// Time variables
bool timeConfigured = false;

// Timing variables
unsigned long lastWiFiCheck = 0;
unsigned long lastMQTTCheck = 0;
unsigned long lastClockUpdate = 0;

const unsigned long wifiCheckInterval = 30000;  // 30 seconds
const unsigned long mqttCheckInterval = 5000;   // 5 seconds
const unsigned long clockUpdateInterval = 1000; // 1 second for clock updates

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char * buf)
{
    Serial.printf(buf);
    Serial.flush();
}
#endif

// Time configuration function
void configureTime() {
    if (!timeConfigured && WiFi.status() == WL_CONNECTED) {
        Serial.println("Configuring time with NTP...");
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        
        struct tm timeinfo;
        int attempts = 0;
        while (!getLocalTime(&timeinfo) && attempts < 15) {
            Serial.print("Waiting for time configuration... attempt ");
            Serial.println(attempts + 1);
            delay(1000);
            attempts++;
        }
        
        if (getLocalTime(&timeinfo)) {
            timeConfigured = true;
            Serial.println("Time configured successfully!");
            updateClock();
        } else {
            Serial.println("Failed to configure time - will retry");
            timeConfigured = false;
        }
    }
}

// Clock update function
void updateClock() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastClockUpdate < clockUpdateInterval) {
        return;
    }
    
    if (!timeConfigured) {
        if (ui_time != NULL) {
            lv_label_set_text(ui_time, "--:--");
        }
        return;
    }
    
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        char timeStr[10];
        strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
        
        if (ui_time != NULL) {
            lv_label_set_text(ui_time, timeStr);
        }
        
        static int lastMinute = -1;
        if (timeinfo.tm_min != lastMinute) {
            Serial.print("Clock updated: ");
            Serial.println(timeStr);
            lastMinute = timeinfo.tm_min;
        }
    } else {
        if (ui_time != NULL) {
            lv_label_set_text(ui_time, "??:??");
        }
    }
    
    lastClockUpdate = currentTime;
}

// WiFi connection function - now using hostname from secrets.h
void connectToWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  WiFi.setHostname(DEVICE_HOSTNAME);  // Now using define from secrets.h
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected successfully!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("");
    Serial.println("Failed to connect to WiFi!");
  }
}

void setup()
{
    Serial.begin(115200);
    delay(2000);

    String LVGL_Arduino = "Hello Arduino! ";
    LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    Serial.println(LVGL_Arduino);
    Serial.println("ST77916 Display - Modular Version with Smooth Arc Control + Screen Dimming");

    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb(my_print);
#endif

    // Initialize display panel (from display_panel module)
    initDisplayBuffers();
    esp_panel_init();

    lv_disp_t *disp = registerDisplayDriver();
    registerTouchDriver();

    // Initialize UI
    ui_init();

    // Initialize WiFi
    connectToWiFi();
    
    // Initialize MQTT (from mqtt_control module)
    initMQTT();
    connectToMQTT();

    // Initialize time
    configureTime();
    
    // Initialize time display
    if (ui_time != NULL) {
        lv_label_set_text(ui_time, "Loading...");
        Serial.println("ui_time initialized");
    } else {
        Serial.println("Warning: ui_time not found!");
    }

    // Initialize temperature display
    if (ui_temp != NULL) {
        lv_label_set_text(ui_temp, "--°C");
        Serial.println("ui_temp initialized");
    } else {
        Serial.println("Warning: ui_temp not found!");
    }

    // Setup all UI event handlers (from ui_events module)
    setupUIEventHandlers();

    // Initialize screen dimming
    initializeScreenDimming();

    Serial.println("Setup complete - Modular Smart Home Controller with Screen Dimming!");
    Serial.println("SMOOTH ARC CONTROL ACTIVE:");
    Serial.println("- Single tap any lamp button to select it for brightness control (no visual change)");
    Serial.println("- Double tap any lamp button to toggle ON/OFF (visual state reflects actual status)");
    Serial.println("- Use ui_Arc1 to control the selected lamp's brightness (now with smooth stabilization)");
    Serial.println("- Visual states now reflect actual ON/OFF status from MQTT");
    Serial.println("- Screen dims to 30% brightness after 10 seconds of inactivity");
    Serial.println("");
    Serial.println("CONTROLS AVAILABLE:");
    Serial.println("- uic_Button1: Single tap = select for arc, Double tap = toggle main lamp");
    Serial.println("- ui_ikea: Single tap = select for arc, Double tap = toggle IKEA lamp"); 
    Serial.println("- ui_svetlo1: Single tap = select for arc, Double tap = toggle svetlo1 lamp");
    Serial.println("- ui_svetlo2: Single tap = select for arc, Double tap = toggle svetlo2 lamp");
    Serial.println("- ui_svetlo3: Single tap = select for arc, Double tap = toggle svetlo3 lamp");
    Serial.println("- ui_acbutton: Click to toggle Stan AC ON/OFF (visual state reflects actual status)");
    Serial.println("- ui_acbuttonOFF: Click to toggle Lab AC ON/OFF (visual state reflects actual status)");
    Serial.println("- ui_SvaSvetla: Click to toggle ALL LIGHTS at once (automation.sva_svetla)");
    Serial.println("- ui_AC1label: Displays Stan AC status (updates from MQTT)");
    Serial.println("- ui_AC1label2: Displays Lab AC status (updates from MQTT)");
    Serial.println("- ui_Arc1: Controls brightness of currently selected lamp (SMOOTH & STABLE)");
    Serial.println("- ui_time: Displays current time");
    Serial.println("- ui_temp: Displays temperature from ESP32 DHT22 sensor");
    Serial.print("Currently selected for arc control: ");
    Serial.println(lampNames[lastSelectedLamp]);
}

void loop()
{
    unsigned long currentTime = millis();
    
    // Check WiFi connection periodically
    if (currentTime - lastWiFiCheck >= wifiCheckInterval) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi disconnected. Reconnecting...");
            connectToWiFi();
            timeConfigured = false;
        }
        lastWiFiCheck = currentTime;
    }
    
    // Configure time if not done yet
    if (!timeConfigured && WiFi.status() == WL_CONNECTED) {
        configureTime();
    }
    
    // Check MQTT connection periodically
    if (currentTime - lastMQTTCheck >= mqttCheckInterval) {
        if (!MQTTclient.connected()) {
            Serial.println("MQTT disconnected. Reconnecting...");
            connectToMQTT();
        }
        lastMQTTCheck = currentTime;
    }
    
    // Process MQTT messages
    MQTTclient.loop();
    
    // Process any pending arc updates (smooth arc control)
    processArcUpdates();
    
    // Read temperature from Home Assistant periodically
    readTemperatureFromHA();
    
    // Update clock
    updateClock();
    
    // Update all visual states periodically to ensure they match actual status
    static unsigned long lastVisualUpdate = 0;
    if (currentTime - lastVisualUpdate >= 1000) { // Update every second
        updateAllVisualStates();
        lastVisualUpdate = currentTime;
    }
    
    // Check for screen dimming
    checkScreenDimming();
    
    // Handle LVGL tasks
    lv_timer_handler();
    
    delay(5);
}