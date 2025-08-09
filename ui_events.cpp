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
        if (uiUpdateInProgress) return;
        
        int arcValue = pendingArcValue;
        
        // Control the brightness of the currently selected lamp
        switch (lastSelectedLamp) {
            case 1: // Main lamp
                if (abs(arcValue - currentBrightness) > 1) {
                    currentBrightness = arcValue;
                    Serial.print("Arc controlling Main Lamp brightness: ");
                    Serial.print(currentBrightness);
                    Serial.println("%");
                    if (MQTTclient.connected()) {
                        setBrightness(currentBrightness);
                    }
                }
                break;
                
            case 2: // IKEA lamp
                if (abs(arcValue - ikeaCurrentBrightness) > 1) {
                    ikeaCurrentBrightness = arcValue;
                    Serial.print("Arc controlling IKEA Lamp brightness: ");
                    Serial.print(ikeaCurrentBrightness);
                    Serial.println("%");
                    if (MQTTclient.connected()) {
                        setIkeaBrightness(ikeaCurrentBrightness);
                    }
                }
                break;
                
            case 3: // Svetlo1 lamp
                if (abs(arcValue - svetlo1CurrentBrightness) > 1) {
                    svetlo1CurrentBrightness = arcValue;
                    Serial.print("Arc controlling Svetlo1 Lamp brightness: ");
                    Serial.print(svetlo1CurrentBrightness);
                    Serial.println("%");
                    if (MQTTclient.connected()) {
                        setSvetlo1Brightness(svetlo1CurrentBrightness);
                    }
                }
                break;
                
            case 4: // Svetlo2 lamp
                if (abs(arcValue - svetlo2CurrentBrightness) > 1) {
                    svetlo2CurrentBrightness = arcValue;
                    Serial.print("Arc controlling Svetlo2 Lamp brightness: ");
                    Serial.print(svetlo2CurrentBrightness);
                    Serial.println("%");
                    if (MQTTclient.connected()) {
                        setSvetlo2Brightness(svetlo2CurrentBrightness);
                    }
                }
                break;
                
            case 5: // Svetlo3 lamp
                if (abs(arcValue - svetlo3CurrentBrightness) > 1) {
                    svetlo3CurrentBrightness = arcValue;
                    Serial.print("Arc controlling Svetlo3 Lamp brightness: ");
                    Serial.print(svetlo3CurrentBrightness);
                    Serial.println("%");
                    if (MQTTclient.connected()) {
                        setSvetlo3Brightness(svetlo3CurrentBrightness);
                    }
                }
                break;
        }
        
        // Clear the pending update
        arcUpdatePending = false;
        pendingArcValue = -1;
        lastArcChange = currentTime;
    }
}

void updateAllVisualStates() {
    // Update main lamp visual state
    if (uic_Button1 != NULL) {
        if (lampIsOn) {
            lv_imgbtn_set_state(uic_Button1, LV_IMGBTN_STATE_PRESSED);
        } else {
            lv_imgbtn_set_state(uic_Button1, LV_IMGBTN_STATE_RELEASED);
        }
    }
    
    // Update IKEA lamp visual state
    if (ui_ikea != NULL) {
        if (ikeaLampIsOn) {
            lv_imgbtn_set_state(ui_ikea, LV_IMGBTN_STATE_PRESSED);
        } else {
            lv_imgbtn_set_state(ui_ikea, LV_IMGBTN_STATE_RELEASED);
        }
    }
    
    // Update svetlo1 lamp visual state
    if (ui_svetlo1 != NULL) {
        if (svetlo1LampIsOn) {
            lv_imgbtn_set_state(ui_svetlo1, LV_IMGBTN_STATE_PRESSED);
        } else {
            lv_imgbtn_set_state(ui_svetlo1, LV_IMGBTN_STATE_RELEASED);
        }
    }
    
    // Update svetlo2 lamp visual state
    if (ui_svetlo2 != NULL) {
        if (svetlo2LampIsOn) {
            lv_imgbtn_set_state(ui_svetlo2, LV_IMGBTN_STATE_PRESSED);
        } else {
            lv_imgbtn_set_state(ui_svetlo2, LV_IMGBTN_STATE_RELEASED);
        }
    }
    
    // Update svetlo3 lamp visual state
    if (ui_svetlo3 != NULL) {
        if (svetlo3LampIsOn) {
            lv_imgbtn_set_state(ui_svetlo3, LV_IMGBTN_STATE_PRESSED);
        } else {
            lv_imgbtn_set_state(ui_svetlo3, LV_IMGBTN_STATE_RELEASED);
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
  
  int targetBrightness = 50; // Default fallback
  
  // Get the current brightness of the selected lamp
  switch (lastSelectedLamp) {
    case 1: // Main lamp
      targetBrightness = currentBrightness;
      break;
    case 2: // IKEA lamp
      targetBrightness = ikeaCurrentBrightness;
      break;
    case 3: // Svetlo1 lamp
      targetBrightness = svetlo1CurrentBrightness;
      break;
    case 4: // Svetlo2 lamp
      targetBrightness = svetlo2CurrentBrightness;
      break;
    case 5: // Svetlo3 lamp
      targetBrightness = svetlo3CurrentBrightness;
      break;
  }
  
  // Update arc value to match selected lamp's brightness
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
    
    if (uiUpdateInProgress) return; // Prevent feedback loops
    
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

// Event callback for main lamp Image Button (ui_Button1)
void ui_Button1_event_cb(lv_event_t * e) {
  if (uiUpdateInProgress) return; // Prevent feedback loops
  
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t * target = lv_event_get_target(e);
  
  if (event_code == LV_EVENT_CLICKED) {
    recordUserActivity(); // Add user activity tracking
    unsigned long currentTime = millis();
    unsigned long timeSinceLastTap = currentTime - lastTapTime[1];
    
    // Check if this is a double-tap
    if (timeSinceLastTap < doubleTapWindow) {
      // Double-tap: Toggle lamp ON/OFF
      Serial.println("Main Lamp DOUBLE-TAP - toggling lamp");
      
      if (MQTTclient.connected()) {
        toggleLamp();
      } else {
        Serial.println("MQTT not connected - cannot toggle main lamp");
      }
      
      // Reset tap time to prevent triple-tap issues
      lastTapTime[1] = 0;
    } else {
      // Single-tap: Select for arc control only (no visual change)
      Serial.println("Main Lamp SINGLE-TAP - selected for arc control");
      lastSelectedLamp = 1;
      updateArcForSelectedLamp();
      
      // Store tap time for double-tap detection
      lastTapTime[1] = currentTime;
    }
  }
  else if (event_code == LV_EVENT_PRESSED) {
    recordUserActivity(); // Add user activity tracking
    Serial.println("Main Lamp Image Button pressed");
  }
  else if (event_code == LV_EVENT_RELEASED) {
    Serial.println("Main Lamp Image Button released");
  }
}

// Event callback for IKEA lamp Image Button (ui_ikea)
void ui_ikea_event_cb(lv_event_t * e) {
  if (ikeaUiUpdateInProgress) return; // Prevent feedback loops
  
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t * target = lv_event_get_target(e);
  
  if (event_code == LV_EVENT_CLICKED) {
    recordUserActivity(); // Add user activity tracking
    unsigned long currentTime = millis();
    unsigned long timeSinceLastTap = currentTime - lastTapTime[2];
    
    // Check if this is a double-tap
    if (timeSinceLastTap < doubleTapWindow) {
      // Double-tap: Toggle lamp ON/OFF
      Serial.println("IKEA Lamp DOUBLE-TAP - toggling lamp");
      
      if (MQTTclient.connected()) {
        toggleIkeaLamp();
      } else {
        Serial.println("MQTT not connected - cannot toggle IKEA lamp");
      }
      
      lastTapTime[2] = 0;
    } else {
      // Single-tap: Select for arc control only (no visual change)
      Serial.println("IKEA Lamp SINGLE-TAP - selected for arc control");
      lastSelectedLamp = 2;
      updateArcForSelectedLamp();
      
      lastTapTime[2] = currentTime;
    }
  }
  else if (event_code == LV_EVENT_PRESSED) {
    recordUserActivity(); // Add user activity tracking
    Serial.println("IKEA Lamp Image Button pressed");
  }
  else if (event_code == LV_EVENT_RELEASED) {
    Serial.println("IKEA Lamp Image Button released");
  }
}

// Event callback for svetlo1 lamp Image Button (ui_svetlo1)
void ui_svetlo1_event_cb(lv_event_t * e) {
  if (svetlo1UiUpdateInProgress) return; // Prevent feedback loops
  
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t * target = lv_event_get_target(e);
  
  if (event_code == LV_EVENT_CLICKED) {
    recordUserActivity(); // Add user activity tracking
    unsigned long currentTime = millis();
    unsigned long timeSinceLastTap = currentTime - lastTapTime[3];
    
    // Check if this is a double-tap
    if (timeSinceLastTap < doubleTapWindow) {
      // Double-tap: Toggle lamp ON/OFF
      Serial.println("Svetlo1 Lamp DOUBLE-TAP - toggling lamp");
      
      if (MQTTclient.connected()) {
        toggleSvetlo1Lamp();
      } else {
        Serial.println("MQTT not connected - cannot toggle svetlo1 lamp");
      }
      
      lastTapTime[3] = 0;
    } else {
      // Single-tap: Select for arc control only (no visual change)
      Serial.println("Svetlo1 Lamp SINGLE-TAP - selected for arc control");
      lastSelectedLamp = 3;
      updateArcForSelectedLamp();
      
      lastTapTime[3] = currentTime;
    }
  }
  else if (event_code == LV_EVENT_PRESSED) {
    recordUserActivity(); // Add user activity tracking
    Serial.println("Svetlo1 Lamp Image Button pressed");
  }
  else if (event_code == LV_EVENT_RELEASED) {
    Serial.println("Svetlo1 Lamp Image Button released");
  }
}

// Event callback for svetlo2 lamp Image Button (ui_svetlo2)
void ui_svetlo2_event_cb(lv_event_t * e) {
  if (svetlo2UiUpdateInProgress) return; // Prevent feedback loops
  
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t * target = lv_event_get_target(e);
  
  if (event_code == LV_EVENT_CLICKED) {
    recordUserActivity(); // Add user activity tracking
    unsigned long currentTime = millis();
    unsigned long timeSinceLastTap = currentTime - lastTapTime[4];
    
    // Check if this is a double-tap
    if (timeSinceLastTap < doubleTapWindow) {
      // Double-tap: Toggle lamp ON/OFF
      Serial.println("Svetlo2 Lamp DOUBLE-TAP - toggling lamp");
      
      if (MQTTclient.connected()) {
        toggleSvetlo2Lamp();
      } else {
        Serial.println("MQTT not connected - cannot toggle svetlo2 lamp");
      }
      
      lastTapTime[4] = 0;
    } else {
      // Single-tap: Select for arc control only (no visual change)
      Serial.println("Svetlo2 Lamp SINGLE-TAP - selected for arc control");
      lastSelectedLamp = 4;
      updateArcForSelectedLamp();
      
      lastTapTime[4] = currentTime;
    }
  }
  else if (event_code == LV_EVENT_PRESSED) {
    recordUserActivity(); // Add user activity tracking
    Serial.println("Svetlo2 Lamp Image Button pressed");
  }
  else if (event_code == LV_EVENT_RELEASED) {
    Serial.println("Svetlo2 Lamp Image Button released");
  }
}

// Event callback for svetlo3 lamp Image Button (ui_svetlo3)
void ui_svetlo3_event_cb(lv_event_t * e) {
  if (svetlo3UiUpdateInProgress) return; // Prevent feedback loops
  
  lv_event_code_t event_code = lv_event_get_code(e);
  lv_obj_t * target = lv_event_get_target(e);
  
  if (event_code == LV_EVENT_CLICKED) {
    recordUserActivity(); // Add user activity tracking
    unsigned long currentTime = millis();
    unsigned long timeSinceLastTap = currentTime - lastTapTime[5];
    
    // Check if this is a double-tap
    if (timeSinceLastTap < doubleTapWindow) {
      // Double-tap: Toggle lamp ON/OFF
      Serial.println("Svetlo3 Lamp DOUBLE-TAP - toggling lamp");
      
      if (MQTTclient.connected()) {
        toggleSvetlo3Lamp();
      } else {
        Serial.println("MQTT not connected - cannot toggle svetlo3 lamp");
      }
      
      lastTapTime[5] = 0;
    } else {
      // Single-tap: Select for arc control only (no visual change)
      Serial.println("Svetlo3 Lamp SINGLE-TAP - selected for arc control");
      lastSelectedLamp = 5;
      updateArcForSelectedLamp();
      
      lastTapTime[5] = currentTime;
    }
  }
  else if (event_code == LV_EVENT_PRESSED) {
    recordUserActivity(); // Add user activity tracking
    Serial.println("Svetlo3 Lamp Image Button pressed");
  }
  else if (event_code == LV_EVENT_RELEASED) {
    Serial.println("Svetlo3 Lamp Image Button released");
  }
}

// Event callback for Stan AC automation Image Button (ui_acbutton)
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
        lv_arc_set_value(ui_Arc1, currentBrightness);
        Serial.println("ui_Arc1 event callbacks set up");
        Serial.print("Arc initialized to control ");
        Serial.println(lampNames[lastSelectedLamp]);
    } else {
        Serial.println("Warning: ui_Arc1 not found!");
    }

    if (uic_Button1 != NULL) {
        lv_obj_add_event_cb(uic_Button1, ui_Button1_event_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(uic_Button1, ui_Button1_event_cb, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(uic_Button1, ui_Button1_event_cb, LV_EVENT_RELEASED, NULL);
        Serial.println("uic_Button1 (Main Lamp) event callbacks set up");
    } else {
        Serial.println("Warning: uic_Button1 not found!");
    }

    if (ui_ikea != NULL) {
        lv_obj_add_event_cb(ui_ikea, ui_ikea_event_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(ui_ikea, ui_ikea_event_cb, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_ikea, ui_ikea_event_cb, LV_EVENT_RELEASED, NULL);
        Serial.println("ui_ikea (IKEA Lamp) event callbacks set up");
    } else {
        Serial.println("Warning: ui_ikea not found!");
    }

    if (ui_svetlo1 != NULL) {
        lv_obj_add_event_cb(ui_svetlo1, ui_svetlo1_event_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(ui_svetlo1, ui_svetlo1_event_cb, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_svetlo1, ui_svetlo1_event_cb, LV_EVENT_RELEASED, NULL);
        Serial.println("ui_svetlo1 (Svetlo1 Lamp) event callbacks set up");
    } else {
        Serial.println("Warning: ui_svetlo1 not found!");
    }

    if (ui_svetlo2 != NULL) {
        lv_obj_add_event_cb(ui_svetlo2, ui_svetlo2_event_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(ui_svetlo2, ui_svetlo2_event_cb, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_svetlo2, ui_svetlo2_event_cb, LV_EVENT_RELEASED, NULL);
        Serial.println("ui_svetlo2 (Svetlo2 Lamp) event callbacks set up");
    } else {
        Serial.println("Warning: ui_svetlo2 not found!");
    }

    if (ui_svetlo3 != NULL) {
        lv_obj_add_event_cb(ui_svetlo3, ui_svetlo3_event_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_add_event_cb(ui_svetlo3, ui_svetlo3_event_cb, LV_EVENT_PRESSED, NULL);
        lv_obj_add_event_cb(ui_svetlo3, ui_svetlo3_event_cb, LV_EVENT_RELEASED, NULL);
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