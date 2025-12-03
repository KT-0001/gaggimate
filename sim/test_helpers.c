/**
 * Test Helper Functions
 * 
 * C functions exposed to JavaScript for test automation
 */

#include "lvgl.h"
#include <stdio.h>
#include <string.h>
#include <emscripten.h>

// External screen objects from UI
extern lv_obj_t *ui_InitScreen;
extern lv_obj_t *ui_StandbyScreen;
extern lv_obj_t *ui_BrewScreen;
extern lv_obj_t *ui_GrindScreen;
extern lv_obj_t *ui_MenuScreen;
extern lv_obj_t *ui_ProfileScreen;
extern lv_obj_t *ui_StatusScreen;
extern lv_obj_t *ui_SimpleProcessScreen;

// External UI event handlers
extern void onBrewScreen(lv_event_t *e);
extern void onGrindScreen(lv_event_t *e);
extern void onMenuClick(lv_event_t *e);
extern void onStandby(lv_event_t *e);
extern void onProfileScreen(lv_event_t *e);
extern void onStatusScreen(lv_event_t *e);
extern void onBrewTempRaise(lv_event_t *e);
extern void onBrewTempLower(lv_event_t *e);
extern void onBrewTimeRaise(lv_event_t *e);
extern void onBrewTimeLower(lv_event_t *e);
extern void onGrindTimeRaise(lv_event_t *e);
extern void onGrindTimeLower(lv_event_t *e);
extern void onBrewStart(lv_event_t *e);
extern void onGrindStart(lv_event_t *e);
extern void onFlush(lv_event_t *e);

// Screen name buffer for returning to JavaScript
static char screen_name_buffer[64] = "unknown";

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
        onMenuClick(&dummy_event);
    } else if (strcmp(button_id, "back_button") == 0) {
        onStandby(&dummy_event);
    } else if (strcmp(button_id, "profile_button") == 0) {
        onProfileScreen(&dummy_event);
    } else if (strcmp(button_id, "status_button") == 0) {
        onStatusScreen(&dummy_event);
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
    } else if (strcmp(button_id, "flush_button") == 0) {
        onFlush(&dummy_event);
    } else {
        printf("[TEST] Unknown button ID: %s\n", button_id);
    }
}
