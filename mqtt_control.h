#ifndef MQTT_CONTROL_H
#define MQTT_CONTROL_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <lvgl.h>
#include <ui.h>

// MQTT topics for first LED lamp control (ui_Button1)
extern const char* light_command_topic;
extern const char* light_state_topic;
extern const char* brightness_topic;
extern const char* switch_topic;
extern const char* brightness_state_topic;
extern const char* switch_state_topic;

// MQTT topics for IKEA LED strip control (ui_ikea)
extern const char* ikea_light_command_topic;
extern const char* ikea_light_state_topic;
extern const char* ikea_brightness_topic;
extern const char* ikea_switch_topic;
extern const char* ikea_brightness_state_topic;
extern const char* ikea_switch_state_topic;

// MQTT topics for svetlo1 LED lamp control (ui_svetlo1)
extern const char* svetlo1_light_command_topic;
extern const char* svetlo1_light_state_topic;
extern const char* svetlo1_brightness_topic;
extern const char* svetlo1_switch_topic;
extern const char* svetlo1_brightness_state_topic;
extern const char* svetlo1_switch_state_topic;

// MQTT topics for svetlo2 LED lamp control (ui_svetlo2)
extern const char* svetlo2_light_command_topic;
extern const char* svetlo2_light_state_topic;
extern const char* svetlo2_brightness_topic;
extern const char* svetlo2_switch_topic;
extern const char* svetlo2_brightness_state_topic;
extern const char* svetlo2_switch_state_topic;

// MQTT topics for svetlo3 LED strip control (ui_svetlo3)
extern const char* svetlo3_light_command_topic;
extern const char* svetlo3_light_state_topic;
extern const char* svetlo3_brightness_topic;
extern const char* svetlo3_switch_topic;
extern const char* svetlo3_brightness_state_topic;
extern const char* svetlo3_switch_state_topic;

// MQTT topics for Stan AC automation button (ui_acbutton)
extern const char* stan_ac_on_automation_topic;
extern const char* stan_ac_off_automation_topic;
extern const char* stan_ac_status_topic;

// MQTT topics for Lab AC automation button (ui_acbuttonOFF)
extern const char* lab_ac_on_automation_topic;
extern const char* lab_ac_off_automation_topic;
extern const char* lab_ac_status_topic;

// MQTT topics for All Lights automation button (ui_SvaSvetla)
extern const char* sva_svetla_automation_topic;
extern const char* sva_svetla_status_topic;

// Home Assistant API configuration for temperature
extern const char* ha_server;
extern const int ha_port;
extern const char* ha_token;
extern const char* temp_sensor_entity;

// Network and MQTT credentials
extern const char* mqtt_server;
extern const char* MQTTuser;
extern const char* MQTTpwd;
extern const char* HostName;

// External objects that need to be accessed
extern WiFiClient espClient;
extern PubSubClient MQTTclient;

// State variables for all 5 lamps
extern int currentBrightness;
extern bool lampIsOn;
extern bool uiUpdateInProgress;
extern bool userInteracting;

extern int ikeaCurrentBrightness;
extern bool ikeaLampIsOn;
extern bool ikeaUiUpdateInProgress;

extern int svetlo1CurrentBrightness;
extern bool svetlo1LampIsOn;
extern bool svetlo1UiUpdateInProgress;

extern int svetlo2CurrentBrightness;
extern bool svetlo2LampIsOn;
extern bool svetlo2UiUpdateInProgress;

extern int svetlo3CurrentBrightness;
extern bool svetlo3LampIsOn;
extern bool svetlo3UiUpdateInProgress;

// Stan AC state tracking
extern bool stanAcIsOn;
extern bool acUiUpdateInProgress;  // Renamed for backward compatibility

// Lab AC state tracking
extern bool labAcIsOn;
extern bool labAcUiUpdateInProgress;

// All Lights state tracking
extern bool svaSvetlaInProgress;
extern bool svaSvetlaUiUpdateInProgress;

// Temperature state tracking
extern float currentTemperature;
extern bool temperatureAvailable;
extern unsigned long lastTempRead;
extern const unsigned long tempReadInterval;

// Timing variables
extern unsigned long lastUserInteraction;
extern const unsigned long userInteractionTimeout;

// Function declarations
void initMQTT();
void connectToMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);

// Main lamp functions
void setBrightness(int brightness);
void turnLampOn();
void turnLampOff();
void toggleLamp();
void updateUIFromMQTT();

// IKEA lamp functions
void setIkeaBrightness(int brightness);
void turnIkeaLampOn();
void turnIkeaLampOff();
void toggleIkeaLamp();
void updateIkeaUIFromMQTT();

// Svetlo1 lamp functions
void setSvetlo1Brightness(int brightness);
void turnSvetlo1LampOn();
void turnSvetlo1LampOff();
void toggleSvetlo1Lamp();
void updateSvetlo1UIFromMQTT();

// Svetlo2 lamp functions
void setSvetlo2Brightness(int brightness);
void turnSvetlo2LampOn();
void turnSvetlo2LampOff();
void toggleSvetlo2Lamp();
void updateSvetlo2UIFromMQTT();

// Svetlo3 lamp functions
void setSvetlo3Brightness(int brightness);
void turnSvetlo3LampOn();
void turnSvetlo3LampOff();
void toggleSvetlo3Lamp();
void updateSvetlo3UIFromMQTT();

// Stan AC Automation functions
void triggerStanACOn();
void triggerStanACOff();
void toggleStanAC();
void updateStanACUIFromMQTT();

// Lab AC Automation functions
void triggerLabACOn();
void triggerLabACOff();
void toggleLabAC();
void updateLabACUIFromMQTT();

// All Lights Automation functions
void triggerSvaSvetla();
void updateSvaSvetlaUIFromMQTT();

// Temperature functions
void readTemperatureFromHA();
void updateTemperatureDisplay(float temperature);

// UI state access (from ui_events module)
extern int lastSelectedLamp;

// Legacy AC functions (for backward compatibility)
#define acIsOn stanAcIsOn
#define triggerACOn triggerStanACOn
#define triggerACOff triggerStanACOff
#define toggleAC toggleStanAC
#define updateACUIFromMQTT updateStanACUIFromMQTT

#endif