/**
 * Test Helper Functions (C++ linkage for UI events)
 * 
 * Exposes C-callable functions to JavaScript while invoking C++ UI handlers.
 */

#include "lvgl.h"
#include <stdio.h>
#include <string.h>
#include <emscripten.h>

#include "ui.h"
// Declare simulator stub handlers provided by ui_stubs.cpp
extern "C" void onMenuScreen(lv_event_t *e);
extern "C" void onStandbyScreen(lv_event_t *e);
extern "C" void onGrindStart(lv_event_t *e);
extern "C" void onStatusScreen(lv_event_t *e);
extern "C" void onBrewStart(lv_event_t *e);
extern "C" void onVolumetricClick(lv_event_t *e);
extern "C" void onProfileSelect(lv_event_t *e);
extern "C" void onNextProfile(lv_event_t *e);
extern "C" void onPreviousProfile(lv_event_t *e);
extern "C" void onProfileLoad(lv_event_t *e);
extern "C" void onBrewTempRaise(lv_event_t *e);
extern "C" void onBrewTempLower(lv_event_t *e);
extern "C" void onBrewTimeRaise(lv_event_t *e);
extern "C" void onBrewTimeLower(lv_event_t *e);
extern "C" void onGrindTimeRaise(lv_event_t *e);
extern "C" void onGrindTimeLower(lv_event_t *e);

// External screen objects from UI (declared in headers)
extern lv_obj_t *ui_InitScreen;
extern lv_obj_t *ui_StandbyScreen;
extern lv_obj_t *ui_BrewScreen;
extern lv_obj_t *ui_GrindScreen;
extern lv_obj_t *ui_MenuScreen;
extern lv_obj_t *ui_ProfileScreen;
extern lv_obj_t *ui_StatusScreen;
extern lv_obj_t *ui_SimpleProcessScreen;

// Screen name buffer for returning to JavaScript
static char screen_name_buffer[64] = "unknown";

extern "C" {
/**
 * Read current Brew dial and label values for assertions.
 * Returns: pressure bar (float), temp C (float), weight g (float), time s (float)
 */
// Delegate telemetry via JSON from ui_stubs.cpp to avoid relying on internal globals
extern "C" const char* getBrewTelemetryJSON();

EMSCRIPTEN_KEEPALIVE
void getBrewTelemetry(float* pressure_bar, float* temp_c, float* weight_g, float* time_s) {
    *pressure_bar = 0.0f;
    *temp_c = 0.0f;
    *weight_g = 0.0f;
    *time_s = 0.0f;
    const char* json = getBrewTelemetryJSON();
    if (!json) return;
    // Parse simple JSON fields (pressure,temp,weight,elapsed)
    float p=0, t=0, w=0, e=0;
    sscanf(json, "{\"pressure\":%f,\"temp\":%f,\"weight\":%f,\"elapsed\":%f}", &p, &t, &w, &e);
    *pressure_bar = p;
    *temp_c = t;
    *weight_g = w;
    *time_s = e;
}

/**
 * Get the name of the currently active screen
 */
EMSCRIPTEN_KEEPALIVE
const char* getCurrentScreenName(void) {
    lv_obj_t *active_screen = lv_scr_act();
    
    if (active_screen == ui_InitScreen) {
        strcpy(screen_name_buffer, "InitScreen");
    } else if (active_screen == ui_StandbyScreen) {
        strcpy(screen_name_buffer, "StandbyScreen");
    } else if (active_screen == ui_BrewScreen) {
        strcpy(screen_name_buffer, "BrewScreen");
    } else if (active_screen == ui_GrindScreen) {
        strcpy(screen_name_buffer, "GrindScreen");
    } else if (active_screen == ui_MenuScreen) {
        strcpy(screen_name_buffer, "MenuScreen");
    } else if (active_screen == ui_ProfileScreen) {
        strcpy(screen_name_buffer, "ProfileScreen");
    } else if (active_screen == ui_StatusScreen) {
        strcpy(screen_name_buffer, "StatusScreen");
    } else if (active_screen == ui_SimpleProcessScreen) {
        strcpy(screen_name_buffer, "SimpleProcessScreen");
    } else {
        strcpy(screen_name_buffer, "unknown");
    }
    
    return screen_name_buffer;
}

/**
 * Simulate a button click by calling the appropriate event handler
 */
EMSCRIPTEN_KEEPALIVE
void simulateButtonClick(const char* button_id) {
    printf("[TEST] Simulating click: %s\n", button_id);
    
    // Create a dummy event object
    lv_event_t dummy_event;
    memset(&dummy_event, 0, sizeof(lv_event_t));
    dummy_event.code = LV_EVENT_CLICKED;
    
    // Map button IDs to event handlers
    if (strcmp(button_id, "brew_button") == 0) {
        onBrewScreen(&dummy_event);
    } else if (strcmp(button_id, "grind_button") == 0) {
        onGrindScreen(&dummy_event);
    } else if (strcmp(button_id, "menu_button") == 0) {
        onMenuScreen(&dummy_event);
    } else if (strcmp(button_id, "back_button") == 0) {
        onStandbyScreen(&dummy_event);
    } else if (strcmp(button_id, "profile_button") == 0 || strcmp(button_id, "profile_select_button") == 0) {
        onProfileSelect(&dummy_event);
    } else if (strcmp(button_id, "status_button") == 0) {
        onStatusScreen(&dummy_event);
    } else if (strcmp(button_id, "profile_next_button") == 0) {
        onNextProfile(&dummy_event);
    } else if (strcmp(button_id, "profile_previous_button") == 0) {
        onPreviousProfile(&dummy_event);
    } else if (strcmp(button_id, "profile_choose_button") == 0) {
        onProfileLoad(&dummy_event);
    } else if (strcmp(button_id, "temp_raise_button") == 0) {
        onBrewTempRaise(&dummy_event);
    } else if (strcmp(button_id, "temp_lower_button") == 0) {
        onBrewTempLower(&dummy_event);
    } else if (strcmp(button_id, "time_raise_button") == 0) {
        onBrewTimeRaise(&dummy_event);
    } else if (strcmp(button_id, "time_lower_button") == 0) {
        onBrewTimeLower(&dummy_event);
    } else if (strcmp(button_id, "grind_time_raise_button") == 0) {
        onGrindTimeRaise(&dummy_event);
    } else if (strcmp(button_id, "grind_time_lower_button") == 0) {
        onGrindTimeLower(&dummy_event);
    } else if (strcmp(button_id, "brew_start_button") == 0) {
        onBrewStart(&dummy_event);
    } else if (strcmp(button_id, "grind_start_button") == 0) {
        onGrindStart(&dummy_event);
    } else if (strcmp(button_id, "volumetric_button") == 0) {
        onVolumetricClick(&dummy_event);
    } else if (strcmp(button_id, "flush_button") == 0) {
        onFlush(&dummy_event);
    } else {
        printf("[TEST] Unknown button ID: %s\n", button_id);
    }
}

} // extern "C"
