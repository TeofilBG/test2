#include "ui_events.h"
#include <Arduino.h>  // For Serial, millis(), etc.

// UI interaction constants (these are local to UI events)
const unsigned long doubleTapWindow = 500; // 500ms window for double-tap
const unsigned long arcDebounceTime = 300; // Increased from 200ms to 300ms for smoother operation
const unsigned long arcStabilizationTime = 150; // Time to wait before processing arc changes

// Arc control variables (these are local to UI events)
int lastSelectedLamp = 1; // 1=main, 2=ikea, 3=svetlo1, 4=svetlo2, 5=svetlo3
const char* lampNames[6] = {"", "Main Lamp", "IKEA Strip", "Svetlo1", "Svetlo2", "Svetlo3"};
unsigned long lastArcChange = 0;
unsigned long lastTapTime[6] = {0}; // 0=unused, 1=main, 2=ikea, 3=svetlo1, 4=svetlo2, 5=svetlo3

// Arc stabilization variables
int pendingArcValue = -1;
unsigned long arcValueChangeTime = 0;
bool arcUpdatePending = false;

// Screen dimming variables - RENAMED TO AVOID CONFLICTS WITH MQTT_CONTROL
unsigned long lastScreenInteraction = 0;
bool isScreenDimmed = false;
const unsigned long screenDimmingTimeout = 10000; // 10 seconds in milliseconds
const uint8_t fullBrightness = 100;  // Full brightness
const uint8_t screenSaverBrightness =8;   // 8% brightness
bool isUserTouchingScreen = false;

// Screen dimming functions - RENAMED TO AVOID CONFLICTS
void initializeScreenDimming() {
    lastScreenInteraction = millis();
    isScreenDimmed = false;
    isUserTouchingScreen = false;
    Serial.println("Screen dimming initialized - will dim to 30% after 10 seconds of inactivity");
}

void checkScreenDimming() {
    unsigned long currentTime = millis();
    
    // If user is currently interacting, don't dim
    if (isUserTouchingScreen) {
        return;
    }
    
    // Check if it's time to dim the screen
    if (!isScreenDimmed && (currentTime - lastScreenInteraction >= screenDimmingTimeout)) {
        dimScreenToSaver();
    }
}

void dimScreenToSaver() {
    if (!isScreenDimmed) {
        Serial.println("Dimming screen to 30% brightness (screensaver mode)");
        set_brightness(screenSaverBrightness);
        isScreenDimmed = true;
    }
}

void restoreScreenBrightness() {
    if (isScreenDimmed) {
        Serial.println("Restoring screen to full brightness");
        set_brightness(fullBrightness);
        isScreenDimmed = false;
    }
}

void recordUserActivity() {
    unsigned long currentTime = millis();
    lastScreenInteraction = currentTime;
    
    // If screen was dimmed, restore it immediately
    restoreScreenBrightness();
}

// Function to process pending arc updates (call this from main loop)
void processArcUpdates() {
    unsigned long currentTime = millis();

    // Check if we have a pending arc update that's ready to process
    if (arcUpdatePending && (currentTime - arcValueChangeTime >= arcStabilizationTime)) {

        // Prevent MQTT feedback loops during user interaction
        if (lampConfigs[lastSelectedLamp - 1].uiUpdateInProgress) return;

        int arcValue = pendingArcValue;

        LampConfig &lamp = lampConfigs[lastSelectedLamp - 1];
        if (abs(arcValue - lamp.currentBrightness) > 1) {
            lamp.currentBrightness = arcValue;
            Serial.print("Arc controlling ");
            Serial.print(lampNames[lastSelectedLamp]);
            Serial.print(" brightness: ");
            Serial.print(lamp.currentBrightness);
            Serial.println("%");
            if (MQTTclient.connected()) {
                setLampBrightness(lamp, lamp.currentBrightness);
            }
        }

        // Clear the pending update
        arcUpdatePending = false;
        pendingArcValue = -1;
        lastArcChange = currentTime;
    }
}

void updateAllVisualStates() {
    lv_obj_t* buttons[NUM_LAMPS] = {uic_Button1, ui_ikea, ui_svetlo1, ui_svetlo2, ui_svetlo3};
    for (int i = 0; i < NUM_LAMPS; ++i) {
        if (buttons[i] != NULL) {
            if (lampConfigs[i].isOn) {
                lv_imgbtn_set_state(buttons[i], LV_IMGBTN_STATE_PRESSED);
            } else {
                lv_imgbtn_set_state(buttons[i], LV_IMGBTN_STATE_RELEASED);
            }
        }
    }
    
    // Update Stan AC visual state
    if (ui_acbutton != NULL) {
        if (stanAcIsOn) {
            lv_obj_add_state(ui_acbutton, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(ui_acbutton, LV_STATE_CHECKED);
        }
    }
    
    // Update Lab AC visual state
    if (ui_acbuttonOFF != NULL) {
        if (labAcIsOn) {
            lv_obj_add_state(ui_acbuttonOFF, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(ui_acbuttonOFF, LV_STATE_CHECKED);
        }
    }
    
    // Update Stan AC status label
    if (ui_AC1label != NULL) {
        if (stanAcIsOn) {
            lv_label_set_text(ui_AC1label, "ON");
        } else {
            lv_label_set_text(ui_AC1label, "OFF");
        }
    }
    
    // Update Lab AC status label
    if (ui_AC1label2 != NULL) {
        if (labAcIsOn) {
            lv_label_set_text(ui_AC1label2, "ON");
        } else {
            lv_label_set_text(ui_AC1label2, "OFF");
        }
    }
    
    // Update All Lights button state
    if (ui_SvaSvetla != NULL) {
        if (svaSvetlaInProgress) {
            lv_obj_add_state(ui_SvaSvetla, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(ui_SvaSvetla, LV_STATE_CHECKED);
        }
    }
}

// Function to update arc when switching between lamps
void updateArcForSelectedLamp() {
  if (ui_Arc1 == NULL) return;
  int targetBrightness = lampConfigs[lastSelectedLamp - 1].currentBrightness;
  lv_arc_set_value(ui_Arc1, targetBrightness);
  Serial.print("Arc switched to control ");
  Serial.print(lampNames[lastSelectedLamp]);
  Serial.print(" (current brightness: ");
  Serial.print(targetBrightness);
  Serial.println("%)");
}

// Event callbacks for UI elements
void ui_Arc1_event_cb(lv_event_t * e) {
  unsigned long currentTime = millis();
  
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t * target = lv_event_get_target(e);
  
  // Handle different event types
  if (event_code == LV_EVENT_PRESSED) {
    recordUserActivity(); // Add user activity tracking
    isUserTouchingScreen = true;
    lastScreenInteraction = currentTime;
    Serial.print("User started interacting with arc (controlling ");
    Serial.print(lampNames[lastSelectedLamp]);
    Serial.println(")");
  }
  else if (event_code == LV_EVENT_RELEASED) {
    recordUserActivity(); // Add user activity tracking
    isUserTouchingScreen = false;
    lastScreenInteraction = currentTime;
    Serial.print("User stopped interacting with arc (");
    Serial.print(lampNames[lastSelectedLamp]);
    Serial.println(")");
  }
  else if (event_code == LV_EVENT_VALUE_CHANGED) {
    // Improved debouncing and stabilization
    if (currentTime - lastArcChange < arcDebounceTime) {
      return;
    }
    
    if (lampConfigs[lastSelectedLamp - 1].uiUpdateInProgress) return; // Prevent feedback loops
    
    recordUserActivity(); // Add user activity tracking
    isUserTouchingScreen = true;
    lastScreenInteraction = currentTime;
    
    int arcValue = lv_arc_get_value(target);
    
    // Instead of immediately sending MQTT commands, use stabilization
    // This reduces jitter by waiting for the user to "settle" on a value
    pendingArcValue = arcValue;
    arcValueChangeTime = currentTime;
    arcUpdatePending = true;
    
    // Optional: Show immediate visual feedback without MQTT delay
    Serial.print("Arc value changing to: ");
    Serial.print(arcValue);
    Serial.println("% (pending stabilization)");
  }
}

// Event callback for Stan AC automation Image Button (ui_acbutton)
void lamp_button_event_cb(lv_event_t * e) {
  int index = (int)(uintptr_t)lv_event_get_user_data(e);
  LampConfig &lamp = lampConfigs[index];
  if (lamp.uiUpdateInProgress) return;

  lv_event_code_t event_code = lv_event_get_code(e);
  if (event_code == LV_EVENT_CLICKED) {
    recordUserActivity();
    unsigned long currentTime = millis();
    unsigned long timeSinceLastTap = currentTime - lastTapTime[index + 1];
    if (timeSinceLastTap < doubleTapWindow) {
      Serial.print(lampNames[index + 1]);
      Serial.println(" DOUBLE-TAP - toggling lamp");
      if (MQTTclient.connected()) {
        toggleLamp(lamp);
      } else {
        Serial.println("MQTT not connected - cannot toggle lamp");
      }
      lastTapTime[index + 1] = 0;
    } else {
      Serial.print(lampNames[index + 1]);
      Serial.println(" SINGLE-TAP - selected for arc control");
      lastSelectedLamp = index + 1;
      updateArcForSelectedLamp();
      lastTapTime[index + 1] = currentTime;
    }
  } else if (event_code == LV_EVENT_PRESSED) {
    recordUserActivity();
    Serial.print(lampNames[index + 1]);
    Serial.println(" Image Button pressed");
  } else if (event_code == LV_EVENT_RELEASED) {
    Serial.print(lampNames[index + 1]);
    Serial.println(" Image Button released");
  }
}
void ui_acbutton_event_cb(lv_event_t * e) {
  if (acUiUpdateInProgress) return; // Prevent feedback loops
  
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t * target = lv_event_get_target(e);
  
  if (event_code == LV_EVENT_CLICKED) {
    recordUserActivity(); // Add user activity tracking
    Serial.println("Stan AC Button clicked - toggling Stan AC");
    
    if (MQTTclient.connected()) {
      toggleStanAC();
    } else {
      Serial.println("MQTT not connected - cannot toggle Stan AC");
    }
  }
  else if (event_code == LV_EVENT_PRESSED) {
    recordUserActivity(); // Add user activity tracking
    Serial.println("Stan AC Button pressed");
  }
  else if (event_code == LV_EVENT_RELEASED) {
    Serial.println("Stan AC Button released");
  }
}

// Event callback for Lab AC automation Image Button (ui_acbuttonOFF)
void ui_acbuttonOFF_event_cb(lv_event_t * e) {
  if (labAcUiUpdateInProgress) return; // Prevent feedback loops
  
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t * target = lv_event_get_target(e);
  
  if (event_code == LV_EVENT_CLICKED) {
    recordUserActivity(); // Add user activity tracking
    Serial.println("Lab AC Button clicked - toggling Lab AC");
    
    if (MQTTclient.connected()) {
      toggleLabAC();
    } else {
      Serial.println("MQTT not connected - cannot toggle Lab AC");
    }
  }
  else if (event_code == LV_EVENT_PRESSED) {
    recordUserActivity(); // Add user activity tracking
    Serial.println("Lab AC Button pressed");
  }
  else if (event_code == LV_EVENT_RELEASED) {
    Serial.println("Lab AC Button released");
  }
}

// Event callback for All Lights automation Image Button (ui_SvaSvetla)
void ui_SvaSvetla_event_cb(lv_event_t * e) {
  if (svaSvetlaUiUpdateInProgress) return; // Prevent feedback loops
  
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t * target = lv_event_get_target(e);
  
  if (event_code == LV_EVENT_CLICKED) {
    recordUserActivity(); // Add user activity tracking
    Serial.println("All Lights Button clicked - toggling all lights");
    
    if (MQTTclient.connected()) {
      triggerSvaSvetla();
    } else {
      Serial.println("MQTT not connected - cannot trigger all lights automation");
    }
  }
  else if (event_code == LV_EVENT_PRESSED) {
    recordUserActivity(); // Add user activity tracking
    Serial.println("All Lights Button pressed");
  }
  else if (event_code == LV_EVENT_RELEASED) {
    Serial.println("All Lights Button released");
  }
}

// UI initialization function
void setupUIEventHandlers() {
    Serial.println("Setting up UI event handlers...");

    if (ui_Arc1 != NULL) {
        lv_obj_add_event_cb(ui_Arc1, ui_Arc1_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
        lv_obj_add_event_cb(ui_Arc1, ui_Arc1_event_cb, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_Arc1, ui_Arc1_event_cb, LV_EVENT_RELEASED, NULL);
        lv_arc_set_value(ui_Arc1, lampConfigs[0].currentBrightness);
        Serial.println("ui_Arc1 event callbacks set up");
        Serial.print("Arc initialized to control ");
        Serial.println(lampNames[lastSelectedLamp]);
    } else {
        Serial.println("Warning: ui_Arc1 not found!");
    }

    if (uic_Button1 != NULL) {
        lv_obj_add_event_cb(uic_Button1, lamp_button_event_cb, LV_EVENT_CLICKED, (void*)0);
        lv_obj_add_event_cb(uic_Button1, lamp_button_event_cb, LV_EVENT_PRESSED, (void*)0);
        lv_obj_add_event_cb(uic_Button1, lamp_button_event_cb, LV_EVENT_RELEASED, (void*)0);
        Serial.println("uic_Button1 (Main Lamp) event callbacks set up");
    } else {
        Serial.println("Warning: uic_Button1 not found!");
    }

    if (ui_ikea != NULL) {
        lv_obj_add_event_cb(ui_ikea, lamp_button_event_cb, LV_EVENT_CLICKED, (void*)1);
        lv_obj_add_event_cb(ui_ikea, lamp_button_event_cb, LV_EVENT_PRESSED, (void*)1);
        lv_obj_add_event_cb(ui_ikea, lamp_button_event_cb, LV_EVENT_RELEASED, (void*)1);
        Serial.println("ui_ikea (IKEA Lamp) event callbacks set up");
    } else {
        Serial.println("Warning: ui_ikea not found!");
    }

    if (ui_svetlo1 != NULL) {
        lv_obj_add_event_cb(ui_svetlo1, lamp_button_event_cb, LV_EVENT_CLICKED, (void*)2);
        lv_obj_add_event_cb(ui_svetlo1, lamp_button_event_cb, LV_EVENT_PRESSED, (void*)2);
        lv_obj_add_event_cb(ui_svetlo1, lamp_button_event_cb, LV_EVENT_RELEASED, (void*)2);
        Serial.println("ui_svetlo1 (Svetlo1 Lamp) event callbacks set up");
    } else {
        Serial.println("Warning: ui_svetlo1 not found!");
    }

    if (ui_svetlo2 != NULL) {
        lv_obj_add_event_cb(ui_svetlo2, lamp_button_event_cb, LV_EVENT_CLICKED, (void*)3);
        lv_obj_add_event_cb(ui_svetlo2, lamp_button_event_cb, LV_EVENT_PRESSED, (void*)3);
        lv_obj_add_event_cb(ui_svetlo2, lamp_button_event_cb, LV_EVENT_RELEASED, (void*)3);
        Serial.println("ui_svetlo2 (Svetlo2 Lamp) event callbacks set up");
    } else {
        Serial.println("Warning: ui_svetlo2 not found!");
    }

    if (ui_svetlo3 != NULL) {
        lv_obj_add_event_cb(ui_svetlo3, lamp_button_event_cb, LV_EVENT_CLICKED, (void*)4);
        lv_obj_add_event_cb(ui_svetlo3, lamp_button_event_cb, LV_EVENT_PRESSED, (void*)4);
        lv_obj_add_event_cb(ui_svetlo3, lamp_button_event_cb, LV_EVENT_RELEASED, (void*)4);
        Serial.println("ui_svetlo3 (Svetlo3 Lamp) event callbacks set up");
    } else {
        Serial.println("Warning: ui_svetlo3 not found!");
    }

    // Setup for Stan AC automation Image Button (ui_acbutton)
    if (ui_acbutton != NULL) {
        lv_obj_add_event_cb(ui_acbutton, ui_acbutton_event_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(ui_acbutton, ui_acbutton_event_cb, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_acbutton, ui_acbutton_event_cb, LV_EVENT_RELEASED, NULL);
        Serial.println("ui_acbutton (Stan AC Toggle) event callbacks set up");
    } else {
        Serial.println("Warning: ui_acbutton not found!");
    }

    // Setup for Lab AC automation Image Button (ui_acbuttonOFF)
    if (ui_acbuttonOFF != NULL) {
        lv_obj_add_event_cb(ui_acbuttonOFF, ui_acbuttonOFF_event_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(ui_acbuttonOFF, ui_acbuttonOFF_event_cb, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_acbuttonOFF, ui_acbuttonOFF_event_cb, LV_EVENT_RELEASED, NULL);
        Serial.println("ui_acbuttonOFF (Lab AC Toggle) event callbacks set up");
    } else {
        Serial.println("Warning: ui_acbuttonOFF not found!");
    }

    // Setup for All Lights automation Image Button (ui_SvaSvetla)
    if (ui_SvaSvetla != NULL) {
        lv_obj_add_event_cb(ui_SvaSvetla, ui_SvaSvetla_event_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(ui_SvaSvetla, ui_SvaSvetla_event_cb, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_SvaSvetla, ui_SvaSvetla_event_cb, LV_EVENT_RELEASED, NULL);
        Serial.println("ui_SvaSvetla (All Lights Toggle) event callbacks set up");
    } else {
        Serial.println("Warning: ui_SvaSvetla not found!");
    }

    // Initialize AC status labels
    if (ui_AC1label != NULL) {
        Serial.println("ui_AC1label (Stan AC Status Label) initialized");
    } else {
        Serial.println("Warning: ui_AC1label not found!");
    }

    if (ui_AC1label2 != NULL) {
        Serial.println("ui_AC1label2 (Lab AC Status Label) initialized");
    } else {
        Serial.println("Warning: ui_AC1label2 not found!");
    }

    // Set initial visual states based on actual ON/OFF status
    updateAllVisualStates();

    Serial.println("UI event handlers setup complete!");
}