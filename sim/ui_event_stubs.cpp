// Minimal UI event stubs for simulator
#include "ui.h"

extern "C" {

void onMenuClick(lv_event_t *e) {
    (void)e;
    lv_disp_load_scr(ui_MenuScreen);
}

void onStandby(lv_event_t *e) {
    (void)e;
    lv_disp_load_scr(ui_StandbyScreen);
}

void onBrewScreen(lv_event_t *e) {
    (void)e;
    lv_disp_load_scr(ui_BrewScreen);
}

void onGrindScreen(lv_event_t *e) {
    (void)e;
    lv_disp_load_scr(ui_GrindScreen);
}

void onBrewTempRaise(lv_event_t *e) { (void)e; }
void onBrewTempLower(lv_event_t *e) { (void)e; }
void onBrewTimeRaise(lv_event_t *e) { (void)e; }
void onBrewTimeLower(lv_event_t *e) { (void)e; }
void onGrindTimeRaise(lv_event_t *e) { (void)e; }
void onGrindTimeLower(lv_event_t *e) { (void)e; }
void onBrewStart(lv_event_t *e) {
    (void)e;
    lv_disp_load_scr(ui_SimpleProcessScreen);
}
void onGrindToggle(lv_event_t *e) {
    (void)e;
    lv_disp_load_scr(ui_SimpleProcessScreen);
}
void onFlush(lv_event_t *e) { (void)e; }

void onStatusScreenLoad(lv_event_t *e) { (void)e; }
void onProfileSelect(lv_event_t *e) { (void)e; }

}
