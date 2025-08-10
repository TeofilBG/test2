#ifndef MQTT_CONTROL_H
#define MQTT_CONTROL_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <lvgl.h>
#include <ui.h>

// Struct holding all MQTT topics and state for a lamp
struct LampConfig {
  const char* lightCommandTopic;
  const char* lightStateTopic;
  const char* brightnessCommandTopic;
  const char* switchCommandTopic;
  const char* brightnessStateTopic;
  const char* switchStateTopic;
  int currentBrightness;
  bool isOn;
  bool uiUpdateInProgress;
};

// Number of controllable lamps
constexpr int NUM_LAMPS = 5;

// Array of lamp configurations
extern LampConfig lampConfigs[NUM_LAMPS];

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

// Global interaction state
extern bool userInteracting;

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

// Generic lamp control functions
void setLampBrightness(LampConfig& lamp, int brightness);
void turnLampOn(LampConfig& lamp);
void turnLampOff(LampConfig& lamp);
void toggleLamp(LampConfig& lamp);
void updateLampUIFromMQTT(int index);

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
