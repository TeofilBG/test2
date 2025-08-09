#ifndef UI_EVENTS_H
#define UI_EVENTS_H

#include <lvgl.h>
#include <ui.h>
#include "mqtt_control.h"
#include "display_panel.h"

// UI interaction constants
extern const unsigned long doubleTapWindow;
extern const unsigned long arcDebounceTime;
extern const unsigned long arcStabilizationTime;

// Arc control variables
extern int lastSelectedLamp;
extern const char* lampNames[6];
extern unsigned long lastArcChange;
extern unsigned long lastTapTime[6];

// Arc stabilization variables
extern int pendingArcValue;
extern unsigned long arcValueChangeTime;
extern bool arcUpdatePending;

// Screen dimming variables - RENAMED TO AVOID CONFLICTS
extern unsigned long lastScreenInteraction;
extern bool isScreenDimmed;
extern const unsigned long screenDimmingTimeout;
extern const uint8_t fullBrightness;
extern const uint8_t screenSaverBrightness;
extern bool isUserTouchingScreen;

// Function declarations
void processArcUpdates();
void updateAllVisualStates();
void updateArcForSelectedLamp();
void setupUIEventHandlers();

// Screen dimming functions - RENAMED TO AVOID CONFLICTS
void initializeScreenDimming();
void checkScreenDimming();
void dimScreenToSaver();
void restoreScreenBrightness();
void recordUserActivity();

// Event callback functions
void ui_Arc1_event_cb(lv_event_t * e);
void ui_Button1_event_cb(lv_event_t * e);
void ui_ikea_event_cb(lv_event_t * e);
void ui_svetlo1_event_cb(lv_event_t * e);
void ui_svetlo2_event_cb(lv_event_t * e);
void ui_svetlo3_event_cb(lv_event_t * e);
void ui_acbutton_event_cb(lv_event_t * e);
void ui_acbuttonOFF_event_cb(lv_event_t * e);
void ui_SvaSvetla_event_cb(lv_event_t * e);

#endif