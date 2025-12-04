/**
 * @file ui_stubs.cpp
 * @brief Functional implementations for UI callbacks in simulator
 */

#include "lvgl.h"
#include "ui.h"
#include <vector>
#include <string>
#include <stdio.h>
#include <math.h>

extern "C" void onMenuScreen(lv_event_t *e);
extern "C" void onStandbyScreen(lv_event_t *e);

// Extern declarations for screens
extern lv_obj_t * ui_BrewScreen;
extern lv_obj_t * ui_GrindScreen;
extern lv_obj_t * ui_InitScreen;
extern lv_obj_t * ui_MenuScreen;
extern lv_obj_t * ui_ProfileScreen;
extern lv_obj_t * ui_SimpleProcessScreen;
extern lv_obj_t * ui_StandbyScreen;
extern lv_obj_t * ui_StatusScreen;
// Brew screen child references (generated symbols)
extern lv_obj_t * ui_BrewScreen_modeSwitch;
extern lv_obj_t * ui_BrewScreen_volumetricButton;
extern lv_obj_t * ui_BrewScreen_weightLabel;
extern lv_obj_t * ui_BrewScreen_profileInfo;
extern lv_obj_t * ui_BrewScreen_Container3;
extern lv_obj_t * ui_BrewScreen_profileName;
extern lv_obj_t * ui_BrewScreen_adjustments;
extern lv_obj_t * ui_BrewScreen_targetTemp;
extern lv_obj_t * ui_BrewScreen_targetDuration;
extern lv_obj_t * ui_BrewScreen_mainLabel3;
extern lv_obj_t * ui_BrewScreen_startButton;
extern lv_obj_t * ui_BrewScreen_acceptButton;
// Timer label for brew display (will be created dynamically)
static lv_obj_t * brew_timer_label = NULL;
// SimpleProcess screen (Steam/Water) child references
extern lv_obj_t * ui_SimpleProcessScreen_contentPanel5;
extern lv_obj_t * ui_SimpleProcessScreen_goButton;
extern lv_obj_t * ui_SimpleProcessScreen_targetTemp;
extern lv_obj_t * ui_SimpleProcessScreen_mainLabel5;
// Grind screen child references
extern lv_obj_t * ui_GrindScreen_targetDuration;
extern lv_obj_t * ui_GrindScreen_weightLabel;
extern lv_obj_t * ui_GrindScreen_contentPanel7;
extern lv_obj_t * ui_GrindScreen_startButton;
extern lv_obj_t * ui_GrindScreen_targetContainer;
// Dials (from component children)
extern lv_obj_t * uic_BrewScreen_dials_tempGauge;
extern lv_obj_t * uic_BrewScreen_dials_tempTarget;
extern lv_obj_t * uic_BrewScreen_dials_pressureGauge;
extern lv_obj_t * uic_BrewScreen_dials_pressureTarget;
extern lv_obj_t * uic_BrewScreen_dials_pressureText;
extern lv_obj_t * uic_BrewScreen_dials_tempText;

// SimpleProcessScreen dials
extern lv_obj_t * uic_SimpleProcessScreen_dials_tempGauge;
extern lv_obj_t * uic_SimpleProcessScreen_dials_pressureGauge;
extern lv_obj_t * uic_SimpleProcessScreen_dials_pressureText;
extern lv_obj_t * uic_SimpleProcessScreen_dials_tempText;

// Image references for arrow buttons (from ProfileScreen)
extern const lv_img_dsc_t ui_img_98036921; // left arrow
extern const lv_img_dsc_t ui_img_944513416; // right arrow

// Arrow buttons for profile navigation on BrewScreen
static lv_obj_t * ui_BrewScreen_previousProfileBtn = NULL;
static lv_obj_t * ui_BrewScreen_nextProfileBtn = NULL;

// Simulated state
static float sim_temperature = 93.0f;
static float sim_brew_time = 25.0f;
static float sim_grind_time = 18.0f;
static bool sim_brewing = false;
static bool sim_volumetric = false; // false=timed, true=volumetric
static int sim_profile_index = 0;
static const char * sim_profiles[] = {"Cremina Lever","9Bar","LM Leva","Classic"};
static const char * sim_profile_files[] = {"data/p/lever.json","data/p/9bar.json","data/p/lmleva.json","data/p/adapt.json"};
static bool sim_is_steam = true; // true=steam, false=water
static bool sim_coming_from_profile = false; // track if navigating from profile selection
// Navigation history stack for caret/back behavior
static std::vector<lv_obj_t*> screen_history;
// Current active screen tracker
static lv_obj_t *current_screen = NULL;

static void navigate_to(lv_obj_t *screen) {
    if (!screen) {
        printf("navigate_to: NULL screen pointer!\n");
        return;
    }
    
    if (current_screen && current_screen != screen) {
        // Only push to history if not immediately going back to the previous item
        // (This prevents ProfileScreen -> BrewScreen from creating circular history)
        if (screen_history.empty() || screen_history.back() != screen) {
            screen_history.push_back(current_screen);
        }
        // Hide previous screen
        lv_obj_add_flag(current_screen, LV_OBJ_FLAG_HIDDEN);
    }
    
    // Show target screen
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_HIDDEN);
    current_screen = screen;
    
    // Force active screen to be the target (for LVGL internal state)
    lv_disp_t *disp = lv_disp_get_default();
    if (disp) {
        disp->act_scr = screen;
    }
    
    // Trigger SCREEN_LOADED event so screen-specific initialization runs
    lv_event_send(screen, LV_EVENT_SCREEN_LOADED, NULL);
}
static void navigate_back() {
    if (!screen_history.empty()) {
        lv_obj_t *target = screen_history.back();
        screen_history.pop_back();
        printf("navigate_back: to %p\n", (void*)target);
        navigate_to(target);
    } else {
        printf("navigate_back: to Standby (history empty)\n");
        navigate_to(ui_StandbyScreen);
    }
}

static void show_toast(const char *msg) {
    lv_obj_t * scr = lv_scr_act();
    if (!scr) return;
    lv_obj_t * toast = lv_label_create(scr);
    lv_label_set_text(toast, msg);
    lv_obj_set_style_text_color(toast, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(toast, lv_color_hex(0x333333), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(toast, LV_OPA_70, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_hor(toast, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_ver(toast, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(toast, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(toast, LV_ALIGN_TOP_MID);
    lv_obj_set_y(toast, 6);
    lv_timer_create([](lv_timer_t *t){ lv_obj_del((lv_obj_t*)t->user_data); lv_timer_del(t); }, 1200, toast);
}

// Shot simulation state
static lv_timer_t * brew_timer = NULL;
static float sim_weight = 0.0f; // grams
static float sim_target_weight = 36.0f; // default target grams
static float sim_flow_rate_gps = 1.6f; // grams per second (simple model)
static float sim_pressure_bar = 0.0f; // live pressure
static float sim_target_pressure_bar = 9.0f; // target pressure
static float sim_temp_c = 93.0f; // live temp

// Phase-driven simulation
struct SimPhase {
    enum PhaseKind { PREINFUSION, BREW } kind;
    float duration_s; // seconds
    float target_pressure; // bar (if pump target is pressure)
    float target_flow_gps; // grams/sec (if pump target is flow)
    float target_temp_c; // 0 means unchanged
    float stop_volumetric_g; // 0 means ignore
};
static std::vector<SimPhase> sim_phases;
static int sim_phase_index = -1;
static float sim_phase_elapsed = 0.0f;
static float sim_total_elapsed = 0.0f;
static bool sim_simple_process_active = false;  // SimpleProcess (steam/water) state
static bool sim_grind_active = false;  // Grind state
static float sim_grind_weight = 0.0f;  // Current grind weight
static lv_obj_t * brew_weight_label = NULL;  // Weight display during brewing

static void sim_load_phases_for_profile(const char *name) {
    sim_phases.clear();
    sim_phase_index = -1;
    sim_phase_elapsed = 0.0f;
    // Minimal mappings based on actual profile data
    if (strcmp(name, "9 Bar Espresso") == 0 || strcmp(name, "9Bar") == 0 || strcmp(name, "9 bar profile") == 0) {
        // From 9bar.json: 28s duration, 36g target
        sim_phases.push_back({SimPhase::BREW, 28.0f, 9.0f, 0.0f, 93.0f, 36.0f});
    } else if (strcmp(name, "9 bar pre infusion ") == 0) {
        sim_phases.push_back({SimPhase::PREINFUSION, 6.0f, 2.0f, 0.0f, 93.0f, 0.0f});
        sim_phases.push_back({SimPhase::PREINFUSION, 6.0f, 9.0f, 0.0f, 93.0f, 0.0f});
        sim_phases.push_back({SimPhase::BREW, 20.0f, 9.0f, 0.0f, 93.0f, 36.0f});
    } else if (strcmp(name, "7 bar profile") == 0) {
        sim_phases.push_back({SimPhase::PREINFUSION, 8.0f, 2.0f, 0.0f, 93.0f, 0.0f});
        sim_phases.push_back({SimPhase::PREINFUSION, 5.0f, 7.0f, 0.0f, 93.0f, 0.0f});
        sim_phases.push_back({SimPhase::BREW, 20.0f, 7.0f, 0.0f, 93.0f, 36.0f});
    } else if (strcmp(name, "Damian's LM Leva") == 0 || strcmp(name, "LM Leva") == 0) {
        sim_phases.push_back({SimPhase::PREINFUSION, 2.0f, 1.8f, 0.0f, 89.0f, 0.0f});
        sim_phases.push_back({SimPhase::PREINFUSION, 20.0f, 1.8f, 0.0f, 89.0f, 0.0f});
        sim_phases.push_back({SimPhase::PREINFUSION, 10.0f, 2.2f, 0.0f, 88.5f, 0.0f});
        sim_phases.push_back({SimPhase::BREW, 5.0f, 8.0f, 0.0f, 88.0f, 0.0f});
        sim_phases.push_back({SimPhase::BREW, 5.0f, 8.0f, 0.0f, 88.0f, 0.0f});
        sim_phases.push_back({SimPhase::BREW, 58.0f, 2.2f, 0.0f, 88.0f, 36.0f});
    } else if (strcmp(name, "Cremina Lever") == 0) {
        // Lever machine profile from lever.json: 2+3+10+10+50=75s total, 36g target
        sim_phases.push_back({SimPhase::PREINFUSION, 2.0f, 1.1f, 0.0f, 86.5f, 0.0f});  // preinfusion start
        sim_phases.push_back({SimPhase::PREINFUSION, 3.0f, 1.1f, 0.0f, 86.5f, 0.0f});  // preinfusion
        sim_phases.push_back({SimPhase::PREINFUSION, 10.0f, 1.1f, 0.0f, 86.5f, 0.0f}); // soak
        sim_phases.push_back({SimPhase::BREW, 10.0f, 9.0f, 0.0f, 86.5f, 0.0f});        // ramp to 9 bar
        sim_phases.push_back({SimPhase::BREW, 50.0f, 3.0f, 0.0f, 87.0f, 36.0f});       // ramp-down to 3 bar, stop at 36g
    } else if (strcmp(name, "Classic") == 0) {
        // Classic 9 bar profile
        sim_phases.push_back({SimPhase::BREW, 30.0f, 9.0f, 0.0f, 93.0f, 36.0f});
    }
    
    // Calculate flow rate based on total duration and target weight
    if (!sim_phases.empty()) {
        float total_duration = 0.0f;
        float target_weight = 0.0f;
        for (const auto &phase : sim_phases) {
            total_duration += phase.duration_s;
            if (phase.stop_volumetric_g > 0) {
                target_weight = phase.stop_volumetric_g;
            }
        }
        // Set flow rate to achieve target weight over total duration
        if (total_duration > 0 && target_weight > 0) {
            sim_flow_rate_gps = target_weight / total_duration;
        }
    }
}

static void sim_begin_next_phase() {
    sim_phase_index++;
    sim_phase_elapsed = 0.0f;
    if (sim_phase_index >= 0 && sim_phase_index < (int)sim_phases.size()) {
        const SimPhase &p = sim_phases[sim_phase_index];
        // Set targets for this phase
        if (p.target_temp_c > 0) sim_temperature = p.target_temp_c;
        if (p.target_flow_gps > 0) sim_flow_rate_gps = p.target_flow_gps; else sim_flow_rate_gps = 1.6f;
        sim_target_pressure_bar = (p.target_pressure > 0) ? p.target_pressure : sim_target_pressure_bar;
        // Visual cue
        char msg[64];
        snprintf(msg, sizeof(msg), "Phase %d: %s", sim_phase_index+1, p.kind == SimPhase::PREINFUSION ? "Preinfusion" : "Brew");
        show_toast(msg);
    }
}

// UI event callbacks with actual functionality
extern "C" {
    // Visual confirmations for UI clicks, then delegate to existing logic
    void onBrewAcceptClicked(lv_event_t *e) {
        // Close the adjustments panel and return to normal Brew view
        if (ui_BrewScreen_adjustments) lv_obj_add_flag(ui_BrewScreen_adjustments, LV_OBJ_FLAG_HIDDEN);
        if (ui_BrewScreen_profileInfo) lv_obj_clear_flag(ui_BrewScreen_profileInfo, LV_OBJ_FLAG_HIDDEN);
        if (ui_BrewScreen_acceptButton) lv_obj_add_flag(ui_BrewScreen_acceptButton, LV_OBJ_FLAG_HIDDEN);
        if (ui_BrewScreen_startButton) lv_obj_clear_flag(ui_BrewScreen_startButton, LV_OBJ_FLAG_HIDDEN);
        show_toast("Settings accepted ☑");
        printf("Brew settings accepted, start button now visible\n");
    }
    void onProfileChooseClicked(lv_event_t *e) {
        show_toast("Choose clicked");
        onProfileLoad(e);
    }
    void onLoadStarted(lv_event_t * e) {
        // Transition from Init to Standby after loading
        lv_timer_t * timer = lv_timer_create([](lv_timer_t * t) {
            lv_scr_load(ui_StandbyScreen);
            lv_timer_del(t);
        }, 2000, NULL);
    }
    
    void onBrewStart(lv_event_t *e) {
        printf("[BUTTON] onBrewStart called\n");
        sim_brewing = !sim_brewing;
        printf("Brew %s\n", sim_brewing ? "started" : "stopped");
        // Start or stop in-place simulation on Brew screen
        if (sim_brewing) {
            // Reset state
            sim_weight = 0.0f;
            sim_phase_elapsed = 0.0f;
            sim_total_elapsed = 0.0f;
            // Try to read target weight from label if present (e.g., "36.0g")
            if (ui_BrewScreen_weightLabel) {
                const char *txt = lv_label_get_text(ui_BrewScreen_weightLabel);
                if (txt && strchr(txt, 'g')) {
                    float w = sim_target_weight;
                    if (sscanf(txt, "%f g", &w) == 1 || sscanf(txt, "%fg", &w) == 1) {
                        sim_target_weight = w;
                    }
                }
            }
            // Ensure Brew screen visible
            navigate_to(ui_BrewScreen);
            // Initialize phase machine if a profile was chosen
            const char *pname = sim_profiles[sim_profile_index];
            sim_load_phases_for_profile(pname);
            if (!sim_phases.empty()) {
                // Calculate flow rate based on total duration and target weight
                float total_duration = 0.0f;
                float target_weight = 0.0f;
                for (const auto &phase : sim_phases) {
                    total_duration += phase.duration_s;
                    if (phase.stop_volumetric_g > 0) {
                        target_weight = phase.stop_volumetric_g;
                    }
                }
                // Set flow rate to achieve target weight over total duration
                if (total_duration > 0 && target_weight > 0) {
                    sim_flow_rate_gps = target_weight / total_duration;
                } else {
                    sim_flow_rate_gps = 1.2f; // Default conservative flow rate
                }
                sim_begin_next_phase();
            } else {
                // Default single-phase brew if none mapped
                sim_phases = { {SimPhase::BREW, sim_brew_time, sim_target_pressure_bar, 0.0f, sim_temperature, sim_volumetric ? sim_target_weight : 0.0f} };
                sim_phase_index = -1;
                sim_flow_rate_gps = sim_target_weight / sim_brew_time;
                sim_begin_next_phase();
            }
            
            // Create or show weight label during brewing (like StatusScreen_brewVolume)
            // Position: y=0 centered in contentPanel4, matching StatusScreen exactly
            if (!brew_weight_label && ui_BrewScreen_contentPanel4) {
                brew_weight_label = lv_label_create(ui_BrewScreen_contentPanel4);
                lv_obj_set_width(brew_weight_label, 120);
                lv_obj_set_height(brew_weight_label, 30);
                lv_obj_set_x(brew_weight_label, 0);
                lv_obj_set_y(brew_weight_label, 0);  // y=0 like StatusScreen_brewVolume
                lv_obj_set_align(brew_weight_label, LV_ALIGN_CENTER);
                lv_obj_set_style_text_font(brew_weight_label, &lv_font_montserrat_24, LV_PART_MAIN);
                lv_obj_set_style_text_color(brew_weight_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                lv_obj_set_style_text_align(brew_weight_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
            }
            if (brew_weight_label) {
                lv_obj_clear_flag(brew_weight_label, LV_OBJ_FLAG_HIDDEN);
                lv_label_set_text(brew_weight_label, "0.0 g");
            }
            
            // Create or show timer label (like StatusScreen_currentDuration)
            // Position: y=70 in contentPanel4, matching StatusScreen exactly
            if (!brew_timer_label && ui_BrewScreen_contentPanel4) {
                brew_timer_label = lv_label_create(ui_BrewScreen_contentPanel4);
                lv_obj_set_width(brew_timer_label, 150);
                lv_obj_set_height(brew_timer_label, 50);
                lv_obj_set_x(brew_timer_label, 0);
                lv_obj_set_y(brew_timer_label, 70);  // y=70 like StatusScreen_currentDuration
                lv_obj_set_align(brew_timer_label, LV_ALIGN_CENTER);
                lv_obj_set_style_text_font(brew_timer_label, &lv_font_montserrat_34, LV_PART_MAIN);
                lv_obj_set_style_text_color(brew_timer_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                lv_obj_set_style_text_align(brew_timer_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
            }
            // Show timer label during brewing
            if (brew_timer_label) {
                lv_obj_clear_flag(brew_timer_label, LV_OBJ_FLAG_HIDDEN);
                lv_label_set_text(brew_timer_label, "0:00");
            }
            
            // Hide profile info during brewing (like real device switches to StatusScreen)
            if (ui_BrewScreen_controlContainer) {
                lv_obj_add_flag(ui_BrewScreen_controlContainer, LV_OBJ_FLAG_HIDDEN);
            }
            
            // Initialize timer display to 0:00 (in adjustments, for compatibility)
            if (ui_BrewScreen_targetDuration) {
                lv_label_set_text(ui_BrewScreen_targetDuration, "0:00");
            }
            // Initialize phase label
            if (ui_BrewScreen_mainLabel3 && sim_phase_index >= 0 && sim_phase_index < (int)sim_phases.size()) {
                const SimPhase &p = sim_phases[sim_phase_index];
                const char *phase_name = (p.kind == SimPhase::PREINFUSION) ? "Preinfusion" : "Brew";
                lv_label_set_text(ui_BrewScreen_mainLabel3, phase_name);
            }
            
            // Create/update a timer to simulate flow
            if (brew_timer) { lv_timer_del(brew_timer); brew_timer = NULL; }
            brew_timer = lv_timer_create([](lv_timer_t *t){
                // Advance time/weight
                float delta_t = t->period / 1000.0f;
                sim_weight += sim_flow_rate_gps * delta_t;
                sim_phase_elapsed += delta_t;
                sim_total_elapsed += delta_t;
                // Update on-screen labels
                if (ui_BrewScreen_weightLabel) {
                    char buf[16];
                    snprintf(buf, sizeof(buf), "%.1f g", sim_weight);
                    lv_label_set_text(ui_BrewScreen_weightLabel, buf);
                }
                // Update brew weight label (visible during brewing)
                if (brew_weight_label) {
                    char buf[16];
                    snprintf(buf, sizeof(buf), "%.1f g", sim_weight);
                    lv_label_set_text(brew_weight_label, buf);
                }
                // Phase dynamics: ramp pressure to current target over first 2s of each phase
                float ramp_time = 2.0f;
                float pfrac = sim_phase_elapsed < ramp_time ? (sim_phase_elapsed / ramp_time) : 1.0f;
                sim_pressure_bar = sim_target_pressure_bar * pfrac;
                // Nudging temperature slightly towards target
                float temp_target = sim_temperature; // use target temp as set by adjustments
                sim_temp_c += (temp_target - sim_temp_c) * 0.1f;
                // Reflect pressure/temperature to dials and text
                if (uic_BrewScreen_dials_pressureGauge) {
                    lv_arc_set_value(uic_BrewScreen_dials_pressureGauge, (int)(sim_pressure_bar));
                }
                if (uic_BrewScreen_dials_pressureText) {
                    char ptxt[16];
                    snprintf(ptxt, sizeof(ptxt), "%.1f", sim_pressure_bar);
                    lv_label_set_text(uic_BrewScreen_dials_pressureText, ptxt);
                }
                // Update pressure target arrow: follow the blue pressure gauge
                // Gauge: 298° start, 62° end, REVERSE mode, range 0-16
                // In REVERSE: value increases from start to end angle going counterclockwise
                if (uic_BrewScreen_dials_pressureTarget) {
                    float pressure_pct = sim_target_pressure_bar / 16.0f;
                    
                    // Map directly: 0 bar = 298°, 16 bar = 62°
                    // Going counterclockwise from 298° to 62° (short path through top)
                    // 298° + progress wraps through 360° to 62°
                    float start_angle = 298.0f;
                    float end_angle = 62.0f;
                    
                    // Counterclockwise span: 298 -> 360 = 62°, then 0 -> 62 = 62°, total = 124°
                    float angle_deg = start_angle + (pressure_pct * 124.0f);
                    
                    // Normalize to 0-360
                    while (angle_deg < 0.0f) angle_deg += 360.0f;
                    while (angle_deg >= 360.0f) angle_deg -= 360.0f;
                    while (angle_deg >= 360.0f) angle_deg -= 360.0f;
                    
                    // Convert to radians
                    float angle_rad = angle_deg * M_PI / 180.0f;
                    
                    // Arrow is on a circle at radius ~240 pixels from center
                    float radius = 240.0f;
                    int x_pos = (int)(radius * cosf(angle_rad));
                    int y_pos = (int)(radius * sinf(angle_rad));
                    
                    // Rotate image to point radially outward (add 90°)
                    float img_angle = angle_deg + 90.0f;
                    while (img_angle >= 360.0f) img_angle -= 360.0f;
                    int angle_cdeg = (int)(img_angle * 10.0f);
                    
                    lv_obj_set_x(uic_BrewScreen_dials_pressureTarget, x_pos);
                    lv_obj_set_y(uic_BrewScreen_dials_pressureTarget, y_pos);
                    lv_img_set_angle(uic_BrewScreen_dials_pressureTarget, angle_cdeg);
                }
                if (uic_BrewScreen_dials_tempGauge) {
                    lv_arc_set_value(uic_BrewScreen_dials_tempGauge, (int)(sim_temp_c));
                }
                // Update temperature target arrow: follow the red temperature gauge
                // Temperature gauge: 118° (0°C) to 242° (160°C) in normal mode
                // Arrow rotates clockwise as temperature increases
                if (uic_BrewScreen_dials_tempTarget) {
                    // Map temperature (0-160°C) to arc angle (118° to 242°)
                    // In normal mode, angle increases as value increases
                    float temp_pct = sim_temp_c / 160.0f;
                    float angle_deg = 118.0f + (temp_pct * 124.0f);  // 242 - 118 = 124°
                    
                    // Normalize to 0-360 range
                    while (angle_deg < 0.0f) angle_deg += 360.0f;
                    while (angle_deg >= 360.0f) angle_deg -= 360.0f;
                    
                    // Convert to centidegrees for LVGL (angle in 0.1° units)
                    int angle_cdeg = (int)(angle_deg * 10.0f);
                    lv_img_set_angle(uic_BrewScreen_dials_tempTarget, angle_cdeg);
                }
                if (uic_BrewScreen_dials_tempText) {
                    char ttxt[16];
                    snprintf(ttxt, sizeof(ttxt), "%.1f", sim_temp_c);
                    lv_label_set_text(uic_BrewScreen_dials_tempText, ttxt);
                }
                // Check current phase completion conditions
                if (sim_phase_index >= 0 && sim_phase_index < (int)sim_phases.size()) {
                    const SimPhase &p = sim_phases[sim_phase_index];
                    // Timed phase completion
                    if (sim_phase_elapsed >= p.duration_s) {
                        sim_begin_next_phase();
                    }
                    // Volumetric stop if defined on this phase
                    if (p.stop_volumetric_g > 0.0f && sim_weight >= p.stop_volumetric_g) {
                        sim_brewing = false;
                    }
                }
                // Also enforce global volumetric in UI if enabled
                if (sim_volumetric && sim_weight >= sim_target_weight) sim_brewing = false;
                // Update total elapsed time display (counting up)
                if (brew_timer_label) {
                    int minutes = (int)(sim_total_elapsed) / 60;
                    int seconds = (int)(sim_total_elapsed) % 60;
                    char tbuf[16];
                    snprintf(tbuf, sizeof(tbuf), "%d:%02d", minutes, seconds);
                    lv_label_set_text(brew_timer_label, tbuf);
                }
                // Also update the one in adjustments for compatibility
                if (ui_BrewScreen_targetDuration) {
                    int minutes = (int)(sim_total_elapsed) / 60;
                    int seconds = (int)(sim_total_elapsed) % 60;
                    char tbuf[16];
                    snprintf(tbuf, sizeof(tbuf), "%d:%02d", minutes, seconds);
                    lv_label_set_text(ui_BrewScreen_targetDuration, tbuf);
                }
                // Update phase label
                if (ui_BrewScreen_mainLabel3 && sim_phase_index >= 0 && sim_phase_index < (int)sim_phases.size()) {
                    const SimPhase &p = sim_phases[sim_phase_index];
                    const char *phase_name = (p.kind == SimPhase::PREINFUSION) ? "Preinfusion" : "Brew";
                    lv_label_set_text(ui_BrewScreen_mainLabel3, phase_name);
                }
                if (!sim_brewing) {
                    // Done: cleanup and notify
                    if (ui_BrewScreen_mainLabel3) {
                        lv_label_set_text(ui_BrewScreen_mainLabel3, "Finish");
                    }
                    // Hide timer when done
                    if (brew_timer_label) {
                        lv_obj_add_flag(brew_timer_label, LV_OBJ_FLAG_HIDDEN);
                    }
                    // Hide weight label when done
                    if (brew_weight_label) {
                        lv_obj_add_flag(brew_weight_label, LV_OBJ_FLAG_HIDDEN);
                    }
                    // Show profile info again
                    if (ui_BrewScreen_controlContainer) {
                        lv_obj_clear_flag(ui_BrewScreen_controlContainer, LV_OBJ_FLAG_HIDDEN);
                    }
                    show_toast("Shot complete");
                    lv_timer_t *self = t;
                    brew_timer = NULL;
                    lv_timer_del(self);
                }
            }, 200, NULL);
        } else {
            // Stop brewing
            if (brew_timer) { lv_timer_del(brew_timer); brew_timer = NULL; }
            // Hide timer label when stopped
            if (brew_timer_label) {
                lv_obj_add_flag(brew_timer_label, LV_OBJ_FLAG_HIDDEN);
            }
            // Hide weight label when stopped
            if (brew_weight_label) {
                lv_obj_add_flag(brew_weight_label, LV_OBJ_FLAG_HIDDEN);
            }
            // Show profile info again when stopped
            if (ui_BrewScreen_controlContainer) {
                lv_obj_clear_flag(ui_BrewScreen_controlContainer, LV_OBJ_FLAG_HIDDEN);
            }
            show_toast("Shot stopped");
        }
    }
    
    void onBrewTempLower(lv_event_t *e) {
        sim_temperature = (sim_temperature > 85.0f) ? sim_temperature - 0.5f : 85.0f;
        if (ui_BrewScreen_targetTemp) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%.1f°C", sim_temperature);
            lv_label_set_text(ui_BrewScreen_targetTemp, buf);
        }
        printf("Temperature: %.1f°C\n", sim_temperature);
    }
    
        // Add missing SquareLine event handlers referenced by generated screens
        // Navigation and screen load
        // Caret (^) back button: go back to previous screen if known, else Standby
        void onMenuClick(lv_event_t *e) {
            printf("[BUTTON] onMenuClick called\n");
            lv_obj_t *current = lv_scr_act();
            
            // If brewing, caret acts as cancel
            if (current == ui_BrewScreen && sim_brewing) {
                onBrewCancel(e);
                show_toast("Brew cancelled");
                printf("Caret/back cancelled brew\n");
                return;
            }
            
            // If on Profile screen, go back to Brew directly
            if (current == ui_ProfileScreen) {
                printf("Back from Profile to Brew\n");
                show_toast("Back to Brew");
                navigate_to(ui_BrewScreen);
                return;
            }
            
            // From main function screens (Brew/Grind/SimpleProcess), go to MenuScreen
            if (current == ui_BrewScreen || current == ui_GrindScreen || current == ui_SimpleProcessScreen) {
                printf("Back to Menu screen\n");
                show_toast("Back to Menu");
                navigate_to(ui_MenuScreen);
                return;
            }
            
            // From MenuScreen or other screens, go to Standby
            if (current == ui_MenuScreen) {
                printf("Back to Standby\n");
                show_toast("Back to Standby");
                navigate_to(ui_StandbyScreen);
                return;
            }
            
            // Otherwise pop history or fall back to Standby
            navigate_back();
            show_toast("Back");
            printf("Back navigation invoked\n");
        }
        void onStandby(lv_event_t *e) {
            printf("onStandby called - going to standby screen\n");
            onStandbyScreen(e);
        }
        void onBrewScreenLoad(lv_event_t *e) {
            // Screen just loaded: default state shows start button, not accept tick
            // Accept tick is only shown when user comes from ProfileScreen via "choose" action
            // To prevent overlap: ensure accept is hidden and start is visible
            if (sim_coming_from_profile) {
                // Coming from profile selection - show accept tick, hide start button
                if (ui_BrewScreen_acceptButton) {
                    lv_obj_clear_flag(ui_BrewScreen_acceptButton, LV_OBJ_FLAG_HIDDEN);
                }
                if (ui_BrewScreen_startButton) {
                    lv_obj_add_flag(ui_BrewScreen_startButton, LV_OBJ_FLAG_HIDDEN);
                }
                sim_coming_from_profile = false;
            } else {
                // Normal navigation - show start button, hide accept tick
                if (ui_BrewScreen_acceptButton) {
                    lv_obj_add_flag(ui_BrewScreen_acceptButton, LV_OBJ_FLAG_HIDDEN);
                }
                if (ui_BrewScreen_startButton) {
                    lv_obj_clear_flag(ui_BrewScreen_startButton, LV_OBJ_FLAG_HIDDEN);
                }
            }
            
            if (ui_BrewScreen_profileInfo) {
                lv_obj_clear_flag(ui_BrewScreen_profileInfo, LV_OBJ_FLAG_HIDDEN);
            }
            // Ensure adjustments panel is hidden by default
            if (ui_BrewScreen_adjustments) {
                lv_obj_add_flag(ui_BrewScreen_adjustments, LV_OBJ_FLAG_HIDDEN);
            }
            
            // Update pressure target arrow to show profile target
            if (uic_BrewScreen_dials_pressureTarget) {
                float pressure_pct = sim_target_pressure_bar / 16.0f;
                float start_angle = 298.0f;
                float angle_deg = start_angle + (pressure_pct * 124.0f);
                while (angle_deg < 0.0f) angle_deg += 360.0f;
                while (angle_deg >= 360.0f) angle_deg -= 360.0f;
                float angle_rad = angle_deg * M_PI / 180.0f;
                float radius = 240.0f;
                int x_pos = (int)(radius * cosf(angle_rad));
                int y_pos = (int)(radius * sinf(angle_rad));
                float img_angle = angle_deg + 90.0f;
                while (img_angle >= 360.0f) img_angle -= 360.0f;
                int angle_cdeg = (int)(img_angle * 10.0f);
                lv_obj_set_x(uic_BrewScreen_dials_pressureTarget, x_pos);
                lv_obj_set_y(uic_BrewScreen_dials_pressureTarget, y_pos);
                lv_img_set_angle(uic_BrewScreen_dials_pressureTarget, angle_cdeg);
            }
            
            // Ensure caret/back button is clickable and on top of other controls
            if (ui_BrewScreen) {
                // Find caret image button by known pointer if available
                extern lv_obj_t * ui_BrewScreen_ImgButton5;
                extern lv_obj_t * ui_BrewScreen_contentPanel4;
                if (ui_BrewScreen_ImgButton5) {
                    // Bring caret to front so it isn't occluded by content panel
                    lv_obj_move_to_index(ui_BrewScreen_ImgButton5, -1);
                    // Make sure it's not hidden and is clickable
                    lv_obj_clear_flag(ui_BrewScreen_ImgButton5, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_clear_flag(ui_BrewScreen_ImgButton5, LV_OBJ_FLAG_SCROLLABLE);
                    lv_obj_add_flag(ui_BrewScreen_ImgButton5, LV_OBJ_FLAG_CLICKABLE);
                    printf("Brew caret button: made clickable and moved to front\n");
                }
                // Ensure the central content panel doesn't intercept clicks over the caret region
                if (ui_BrewScreen_contentPanel4) {
                    lv_obj_clear_flag(ui_BrewScreen_contentPanel4, LV_OBJ_FLAG_CLICKABLE);
                    printf("Content panel: clickable flag cleared\n");
                }
            }
            printf("Brew screen loaded; start visible, accept hidden, arrows ensured\n");
        }
        void onGrindScreenLoad(lv_event_t *e) {
            // Initialize duration display
            if (ui_GrindScreen_targetDuration) {
                int minutes = (int)(sim_grind_time) / 60;
                int seconds = (int)(sim_grind_time) % 60;
                char buf[16];
                snprintf(buf, sizeof(buf), "%d:%02d", minutes, seconds);
                lv_label_set_text(ui_GrindScreen_targetDuration, buf);
            }
            printf("Grind screen loaded\n");
        }
        void onMenuScreenLoad(lv_event_t *e) { /* no-op for sim */ }
        // replaced by functional implementation below
        // void onProfileScreenLoad(lv_event_t *e) { /* no-op for sim */ }
            void onProfileScreenLoad(lv_event_t *e) {
                // Hide spinner, show details
                if (ui_ProfileScreen_loadingSpinner) lv_obj_add_flag(ui_ProfileScreen_loadingSpinner, LV_OBJ_FLAG_HIDDEN);
                if (ui_ProfileScreen_profileDetails) lv_obj_clear_flag(ui_ProfileScreen_profileDetails, LV_OBJ_FLAG_HIDDEN);
                // Ensure simple content visible initially
                if (ui_ProfileScreen_simpleContent) lv_obj_clear_flag(ui_ProfileScreen_simpleContent, LV_OBJ_FLAG_HIDDEN);
                if (ui_ProfileScreen_extendedContent) lv_obj_add_flag(ui_ProfileScreen_extendedContent, LV_OBJ_FLAG_HIDDEN);
                // Sync displayed name
                if (ui_ProfileScreen_profileName) lv_label_set_text(ui_ProfileScreen_profileName, sim_profiles[sim_profile_index]);
                printf("Profile screen loaded; details shown\n");
            }
    
        // Brewing controls
        void onVolumetricClick(lv_event_t *e) {
            sim_volumetric = !sim_volumetric;
            const char * mode = sim_volumetric ? "Volumetric" : "Timed";
            printf("Mode: %s\n", mode);
            if (ui_BrewScreen_weightLabel) {
                lv_label_set_text(ui_BrewScreen_weightLabel, sim_volumetric ? "0 g" : "-");
            }
            if (ui_BrewScreen_adjustments) {
                // In volumetric, hide duration; in timed, show duration
                if (sim_volumetric) lv_obj_add_flag(ui_BrewScreen_adjustments, LV_OBJ_FLAG_HIDDEN);
                else lv_obj_clear_flag(ui_BrewScreen_adjustments, LV_OBJ_FLAG_HIDDEN);
            }
        }
        void onVolumetricHold(lv_event_t *e) {
            printf("Volumetric button held (long press)\n");
        }
        // Settings button should open Settings/Status, not profile chooser
        void onProfileSettings(lv_event_t *e) {
            navigate_to(ui_BrewScreen);
            if (ui_BrewScreen_adjustments) {
                lv_obj_clear_flag(ui_BrewScreen_adjustments, LV_OBJ_FLAG_HIDDEN);
                // Add background to prevent bleed-through
                lv_obj_set_style_bg_opa(ui_BrewScreen_adjustments, LV_OPA_COVER, LV_PART_MAIN);
                lv_obj_set_style_bg_color(ui_BrewScreen_adjustments, lv_color_hex(0x000000), LV_PART_MAIN);
            }
            if (ui_BrewScreen_profileInfo) lv_obj_add_flag(ui_BrewScreen_profileInfo, LV_OBJ_FLAG_HIDDEN);
            show_toast("Brew settings opened");
            printf("Opened Brew settings mode on Brew screen\n");
        }
        void onProfileSave(lv_event_t *e) { printf("Profile saved: %s\n", sim_profiles[sim_profile_index]); }
        void onProfileAccept(lv_event_t *e) {
            // Apply selected profile to Brew screen and start brewing
            const char *name = sim_profiles[sim_profile_index];
            if (ui_BrewScreen_profileName) lv_label_set_text(ui_BrewScreen_profileName, name);
            // Hide profile selection panel and show main brew controls
            if (ui_BrewScreen_profileInfo) lv_obj_add_flag(ui_BrewScreen_profileInfo, LV_OBJ_FLAG_HIDDEN);
            if (ui_BrewScreen_adjustments) lv_obj_clear_flag(ui_BrewScreen_adjustments, LV_OBJ_FLAG_HIDDEN);
            // Read simple targets from profile JSON (temp, duration) if available
            const char *profile_path = sim_profile_files[sim_profile_index];
            FILE *fp = fopen(profile_path, "rb");
            if (fp) {
                fseek(fp, 0, SEEK_END);
                long len = ftell(fp);
                fseek(fp, 0, SEEK_SET);
                std::string json;
                json.resize(len);
                if (len > 0) fread(&json[0], 1, len, fp);
                fclose(fp);
                auto findNumber = [&](const char *key) -> double {
                    size_t pos = json.find(key);
                    if (pos == std::string::npos) return NAN;
                    // find colon then parse number
                    pos = json.find(':', pos);
                    if (pos == std::string::npos) return NAN;
                    // skip spaces
                    while (pos < json.size() && (json[pos] == ':' || json[pos] == ' ' || json[pos] == '\t')) pos++;
                    // collect until non-number
                    size_t end = pos;
                    while (end < json.size() && (isdigit(json[end]) || json[end] == '.' || json[end] == '-')) end++;
                    try { return std::stod(json.substr(pos, end - pos)); } catch (...) { return NAN; }
                };
                double tgtTemp = findNumber("targetTemp");
                double tgtDuration = findNumber("targetDuration");
                if (!std::isnan(tgtTemp)) {
                    sim_temperature = (float)tgtTemp;
                    if (ui_BrewScreen_targetTemp) {
                        char buf[16];
                        snprintf(buf, sizeof(buf), "%.1f°C", sim_temperature);
                        lv_label_set_text(ui_BrewScreen_targetTemp, buf);
                    }
                }
                if (!std::isnan(tgtDuration)) {
                    sim_brew_time = (float)tgtDuration;
                    if (ui_BrewScreen_targetDuration) {
                        int minutes = (int)(sim_brew_time) / 60;
                        int seconds = (int)(sim_brew_time) % 60;
                        char buf[16];
                        snprintf(buf, sizeof(buf), "%d:%02d", minutes, seconds);
                        lv_label_set_text(ui_BrewScreen_targetDuration, buf);
                    }
                }
            }
            // Load phases for the chosen profile and kick off the brew timer
            sim_load_phases_for_profile(name);
            sim_weight = 0.0f;
            sim_phase_index = -1;
            sim_phase_elapsed = 0.0f;
            sim_brewing = true;
            // Disable profile navigation while brewing
            if (ui_BrewScreen_previousProfileBtn) lv_obj_add_state(ui_BrewScreen_previousProfileBtn, LV_STATE_DISABLED);
            if (ui_BrewScreen_nextProfileBtn) lv_obj_add_state(ui_BrewScreen_nextProfileBtn, LV_STATE_DISABLED);
            // begin first phase immediately
            sim_begin_next_phase();
            if (!brew_timer) {
                brew_timer = lv_timer_create([](lv_timer_t *t){
                    // advance time
                    if (!sim_brewing) return;
                    const float dt = 0.1f; // 100ms tick
                    sim_phase_elapsed += dt;
                    // simple dynamics: pressure approaches target, temp hovers near target
                    sim_pressure_bar += (sim_target_pressure_bar - sim_pressure_bar) * 0.05f;
                    sim_temp_c += (sim_temperature - sim_temp_c) * 0.02f;
                    sim_weight += sim_flow_rate_gps * dt;
                    // update UI dials/texts if available
                    if (uic_BrewScreen_dials_pressureGauge) lv_meter_set_indicator_value(uic_BrewScreen_dials_pressureGauge, NULL, (int)(sim_pressure_bar * 10));
                    if (uic_BrewScreen_dials_tempGauge) lv_meter_set_indicator_value(uic_BrewScreen_dials_tempGauge, NULL, (int)(sim_temp_c * 10));
                    if (ui_BrewScreen_weightLabel) {
                        char buf[16];
                        snprintf(buf, sizeof(buf), "%d g", (int)sim_weight);
                        lv_label_set_text(ui_BrewScreen_weightLabel, buf);
                    }
                    // phase completion or stop conditions
                    if (sim_phase_index >= 0 && sim_phase_index < (int)sim_phases.size()) {
                        const SimPhase &p = sim_phases[sim_phase_index];
                        bool time_done = sim_phase_elapsed >= p.duration_s && p.duration_s > 0.0f;
                        bool vol_done = (p.stop_volumetric_g > 0.0f) && (sim_weight >= p.stop_volumetric_g);
                        if (time_done || vol_done) {
                            sim_begin_next_phase();
                        }
                    }
                    // finished all phases
                    if (sim_phase_index >= (int)sim_phases.size()) {
                        sim_brewing = false;
                        // Re-enable profile navigation once brewing ends
                        if (ui_BrewScreen_previousProfileBtn) lv_obj_clear_state(ui_BrewScreen_previousProfileBtn, LV_STATE_DISABLED);
                        if (ui_BrewScreen_nextProfileBtn) lv_obj_clear_state(ui_BrewScreen_nextProfileBtn, LV_STATE_DISABLED);
                    }
                }, 100, NULL);
            }
            show_toast("Profile accepted — starting brew");
            printf("Profile accepted and brew started: %s\n", name);
        }

        // Cancel brew and return to selection/settings
        void onBrewCancel(lv_event_t *e) {
            if (sim_brewing) {
                sim_brewing = false;
            }
            if (brew_timer) {
                lv_timer_del(brew_timer);
                brew_timer = NULL;
            }
            // Reset UI states
            if (ui_BrewScreen_weightLabel) lv_label_set_text(ui_BrewScreen_weightLabel, sim_volumetric ? "0 g" : "-");
            if (ui_BrewScreen_previousProfileBtn) lv_obj_clear_state(ui_BrewScreen_previousProfileBtn, LV_STATE_DISABLED);
            if (ui_BrewScreen_nextProfileBtn) lv_obj_clear_state(ui_BrewScreen_nextProfileBtn, LV_STATE_DISABLED);
            // Show profile panel again
            if (ui_BrewScreen_profileInfo) lv_obj_clear_flag(ui_BrewScreen_profileInfo, LV_OBJ_FLAG_HIDDEN);
            show_toast("Brew cancelled");
            printf("Brew cancelled and UI reset\n");
        }
        void onProfileSaveAsNew(lv_event_t *e) { printf("Profile saved as new based on: %s\n", sim_profiles[sim_profile_index]); }
        void onNextProfile(lv_event_t *e) {
            sim_profile_index = (sim_profile_index + 1) % (sizeof(sim_profiles)/sizeof(sim_profiles[0]));
            lv_obj_t *scr = lv_scr_act();
            if (scr == ui_ProfileScreen) {
                if (ui_ProfileScreen_profileName) lv_label_set_text(ui_ProfileScreen_profileName, sim_profiles[sim_profile_index]);
            } else if (ui_BrewScreen_profileName) {
                lv_label_set_text(ui_BrewScreen_profileName, sim_profiles[sim_profile_index]);
            }
            printf("Next profile: %s\n", sim_profiles[sim_profile_index]);
            show_toast("Next profile");
        }
        void onPreviousProfile(lv_event_t *e) {
            sim_profile_index = (sim_profile_index - 1);
            if (sim_profile_index < 0) sim_profile_index = (sizeof(sim_profiles)/sizeof(sim_profiles[0])) - 1;
            lv_obj_t *scr = lv_scr_act();
            if (scr == ui_ProfileScreen) {
                if (ui_ProfileScreen_profileName) lv_label_set_text(ui_ProfileScreen_profileName, sim_profiles[sim_profile_index]);
            } else if (ui_BrewScreen_profileName) {
                lv_label_set_text(ui_BrewScreen_profileName, sim_profiles[sim_profile_index]);
            }
            printf("Previous profile: %s\n", sim_profiles[sim_profile_index]);
            show_toast("Previous profile");
        }
    
    void onBrewTempRaise(lv_event_t *e) {
        sim_temperature = (sim_temperature < 100.0f) ? sim_temperature + 0.5f : 100.0f;
        if (ui_BrewScreen_targetTemp) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%.1f°C", sim_temperature);
            lv_label_set_text(ui_BrewScreen_targetTemp, buf);
        }
        printf("Temperature: %.1f°C\n", sim_temperature);
    }
    
    void onBrewTimeLower(lv_event_t *e) {
        sim_brew_time = (sim_brew_time > 10.0f) ? sim_brew_time - 1.0f : 10.0f;
        if (ui_BrewScreen_targetDuration) {
            int minutes = (int)(sim_brew_time) / 60;
            int seconds = (int)(sim_brew_time) % 60;
            char buf[16];
            snprintf(buf, sizeof(buf), "%d:%02d", minutes, seconds);
            lv_label_set_text(ui_BrewScreen_targetDuration, buf);
        }
        printf("Brew time: %.1fs\n", sim_brew_time);
    }
    
    void onBrewTimeRaise(lv_event_t *e) {
        sim_brew_time = (sim_brew_time < 60.0f) ? sim_brew_time + 1.0f : 60.0f;
        if (ui_BrewScreen_targetDuration) {
            int minutes = (int)(sim_brew_time) / 60;
            int seconds = (int)(sim_brew_time) % 60;
            char buf[16];
            snprintf(buf, sizeof(buf), "%d:%02d", minutes, seconds);
            lv_label_set_text(ui_BrewScreen_targetDuration, buf);
        }
        printf("Brew time: %.1fs\n", sim_brew_time);
    }

    // Grinding controls
    void onGrindToggle(lv_event_t *e) {
        sim_grind_active = !sim_grind_active;
        
        if (sim_grind_active) {
            show_toast("Grinding...");
            printf("Grind started\n");
            
            // Hide start button
            if (ui_GrindScreen_startButton) {
                lv_obj_add_flag(ui_GrindScreen_startButton, LV_OBJ_FLAG_HIDDEN);
            }
            
            // Hide target container
            if (ui_GrindScreen_targetContainer) {
                lv_obj_add_flag(ui_GrindScreen_targetContainer, LV_OBJ_FLAG_HIDDEN);
            }
            
            // Reset grind weight
            sim_grind_weight = 0.0f;
            sim_total_elapsed = 0.0f;
            
            // Calculate weight rate: target weight / target time
            float target_weight = 18.0f; // default 18g
            float grind_rate_gps = target_weight / sim_grind_time; // grams per second
            
            // Create timer label if needed
            if (!brew_timer_label && ui_GrindScreen_contentPanel7) {
                brew_timer_label = lv_label_create(ui_GrindScreen_contentPanel7);
                lv_obj_set_width(brew_timer_label, 150);
                lv_obj_set_height(brew_timer_label, 50);
                lv_obj_set_y(brew_timer_label, 0);
                lv_obj_set_align(brew_timer_label, LV_ALIGN_CENTER);
                lv_obj_set_style_text_font(brew_timer_label, &lv_font_montserrat_34, LV_PART_MAIN);
                lv_obj_set_style_text_color(brew_timer_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                lv_obj_set_style_text_align(brew_timer_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
            }
            
            // Show timer
            if (brew_timer_label) {
                lv_obj_clear_flag(brew_timer_label, LV_OBJ_FLAG_HIDDEN);
                lv_label_set_text(brew_timer_label, "0:00");
            }
            
            // Update weight label to show 0g
            if (ui_GrindScreen_weightLabel) {
                lv_label_set_text(ui_GrindScreen_weightLabel, "0.0 g");
            }
            
            // Start grind timer
            if (brew_timer) { lv_timer_del(brew_timer); brew_timer = NULL; }
            brew_timer = lv_timer_create([](lv_timer_t *t){
                float delta_t = t->period / 1000.0f;
                sim_total_elapsed += delta_t;
                sim_grind_weight += (18.0f / sim_grind_time) * delta_t;
                
                // Update timer
                if (brew_timer_label) {
                    int minutes = (int)(sim_total_elapsed) / 60;
                    int seconds = (int)(sim_total_elapsed) % 60;
                    char tbuf[32];
                    snprintf(tbuf, sizeof(tbuf), "%d:%02d", minutes, seconds);
                    lv_label_set_text(brew_timer_label, tbuf);
                }
                
                // Update weight
                if (ui_GrindScreen_weightLabel) {
                    char wbuf[32];
                    snprintf(wbuf, sizeof(wbuf), "%.1f g", sim_grind_weight);
                    lv_label_set_text(ui_GrindScreen_weightLabel, wbuf);
                }
                
                // Auto-stop when reaching target time
                if (sim_total_elapsed >= sim_grind_time) {
                    sim_grind_active = false;
                    if (brew_timer) {
                        lv_timer_t *self = t;
                        brew_timer = NULL;
                        lv_timer_del(self);
                    }
                    show_toast("Grind complete");
                    
                    // Hide timer
                    if (brew_timer_label) {
                        lv_obj_add_flag(brew_timer_label, LV_OBJ_FLAG_HIDDEN);
                    }
                    
                    // Show controls again
                    if (ui_GrindScreen_startButton) {
                        lv_obj_clear_flag(ui_GrindScreen_startButton, LV_OBJ_FLAG_HIDDEN);
                    }
                    if (ui_GrindScreen_targetContainer) {
                        lv_obj_clear_flag(ui_GrindScreen_targetContainer, LV_OBJ_FLAG_HIDDEN);
                    }
                }
            }, 200, NULL);
        } else {
            show_toast("Stopped");
            printf("Grind stopped\n");
            
            // Stop timer
            if (brew_timer) {
                lv_timer_del(brew_timer);
                brew_timer = NULL;
            }
            
            // Hide timer label
            if (brew_timer_label) {
                lv_obj_add_flag(brew_timer_label, LV_OBJ_FLAG_HIDDEN);
            }
            
            // Show controls
            if (ui_GrindScreen_startButton) {
                lv_obj_clear_flag(ui_GrindScreen_startButton, LV_OBJ_FLAG_HIDDEN);
            }
            if (ui_GrindScreen_targetContainer) {
                lv_obj_clear_flag(ui_GrindScreen_targetContainer, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
    
    void onSteamTempLower(lv_event_t *e) {
        sim_temperature = (sim_temperature > 100.0f) ? sim_temperature - 1.0f : 100.0f;
        if (ui_SimpleProcessScreen_targetTemp) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%.0f°C", sim_temperature);
            lv_label_set_text(ui_SimpleProcessScreen_targetTemp, buf);
        }
        show_toast("-");
        printf("Steam temperature: %.1f°C\n", sim_temperature);
    }
    
    void onSteamTempRaise(lv_event_t *e) {
        sim_temperature = (sim_temperature < 140.0f) ? sim_temperature + 1.0f : 140.0f;
        if (ui_SimpleProcessScreen_targetTemp) {
            char buf[16];
            snprintf(buf, sizeof(buf), "%.0f°C", sim_temperature);
            lv_label_set_text(ui_SimpleProcessScreen_targetTemp, buf);
        }
        show_toast("+");
        printf("Steam temperature: %.1f°C\n", sim_temperature);
    }
    
    void onGrindTimeLower(lv_event_t *e) {
        sim_grind_time = (sim_grind_time > 5.0f) ? sim_grind_time - 0.5f : 5.0f;
        if (ui_GrindScreen_targetDuration) {
            int minutes = (int)(sim_grind_time) / 60;
            int seconds = (int)(sim_grind_time) % 60;
            char buf[16];
            snprintf(buf, sizeof(buf), "%d:%02d", minutes, seconds);
            lv_label_set_text(ui_GrindScreen_targetDuration, buf);
        }
        show_toast("-");
        printf("Grind time: %.1fs\n", sim_grind_time);
    }
    
    void onGrindTimeRaise(lv_event_t *e) {
        sim_grind_time = (sim_grind_time < 30.0f) ? sim_grind_time + 0.5f : 30.0f;
        if (ui_GrindScreen_targetDuration) {
            int minutes = (int)(sim_grind_time) / 60;
            int seconds = (int)(sim_grind_time) % 60;
            char buf[16];
            snprintf(buf, sizeof(buf), "%d:%02d", minutes, seconds);
            lv_label_set_text(ui_GrindScreen_targetDuration, buf);
        }
        show_toast("+");
        printf("Grind time: %.1fs\n", sim_grind_time);
    }
    
    void onBrewScreen(lv_event_t *e) {
        navigate_to(ui_BrewScreen);
        sim_brewing = false;
        show_toast("Brew");
        printf("Navigated to Brew Screen from Menu\n");
    }
    
    void onWaterScreen(lv_event_t *e) {
        sim_is_steam = false;
        navigate_to(ui_SimpleProcessScreen);
        show_toast("Water");
        printf("Navigated to Water Screen\n");
    }
    
    void onSteamScreen(lv_event_t *e) {
        sim_is_steam = true;
        navigate_to(ui_SimpleProcessScreen);
        show_toast("Steam");
        printf("Navigated to Steam Screen\n");
    }
    
    void onWakeup(lv_event_t *e) {
        navigate_to(ui_MenuScreen);
        show_toast("Wake up");
        printf("Wake up - navigated to Menu Screen\n");
    }
    
    void onFlush(lv_event_t *e) {
        show_toast("Flush");
        printf("Flush activated\n");
    }
    
    void onGrindScreen(lv_event_t *e) {
        navigate_to(ui_GrindScreen);
        show_toast("Grind");
        printf("Navigated to Grind Screen\n");
    }
    
    void onGrindStart(lv_event_t *e) {
        printf("Grind started for %.1fs\n", sim_grind_time);
        if (ui_SimpleProcessScreen) lv_scr_load(ui_SimpleProcessScreen);
    }
    
    void onMenuScreen(lv_event_t *e) {
        navigate_to(ui_MenuScreen);
        printf("Navigated to Menu Screen\n");
    }
    
    void onProfileScreen(lv_event_t *e) {
        navigate_to(ui_ProfileScreen);
        printf("Navigated to Profile Screen\n");
    }
    
    void onProfileSelect(lv_event_t *e) {
        navigate_to(ui_ProfileScreen);
            show_toast("Open Profile selection");
        printf("Open Profile selection\n");
    }
    
    void onSettingsScreen(lv_event_t *e) {
        navigate_to(ui_StatusScreen);
        printf("Navigated to Settings/Status Screen\n");
    }
    
    void onStandbyScreen(lv_event_t *e) {
        navigate_to(ui_StandbyScreen);
        printf("Navigated to Standby Screen\n");
    }
    
    void onStatusScreen(lv_event_t *e) {
        navigate_to(ui_StatusScreen);
        printf("Navigated to Status Screen\n");
    }
    
    void onSleep(lv_event_t *e) {
        navigate_to(ui_StandbyScreen);
        printf("Going to sleep - Standby Screen\n");
    }
}

// Additional load/toggle handlers required by generated screens
// replaced by functional implementation below
// extern "C" void onProfileLoad(lv_event_t *e) { /* no-op */ }
extern "C" void onProfileLoad(lv_event_t *e) {
    // Apply selected profile settings and return to Brew
    const char *name = sim_profiles[sim_profile_index];
    if (ui_BrewScreen_profileName) lv_label_set_text(ui_BrewScreen_profileName, name);
    // Try to read simple targets from JSON if available
    const char *file = sim_profile_files[sim_profile_index];
    FILE *fp = fopen(file, "r");
    if (fp) {
        char buf[2048];
        size_t n = fread(buf, 1, sizeof(buf)-1, fp);
        buf[n] = '\0';
        fclose(fp);
        // Naive parse: look for "targetTemp" and "targetDuration" or "target_time"
        float temp = sim_temperature;
        int seconds = (int)sim_brew_time;
        const char *tt = strstr(buf, "targetTemp");
        if (!tt) tt = strstr(buf, "target_temp");
        if (tt) {
            float v = temp;
            if (sscanf(tt, "targetTemp%*[^0-9]%f", &v) == 1 || sscanf(tt, "target_temp%*[^0-9]%f", &v) == 1) temp = v;
        }
        const char *td = strstr(buf, "targetDuration");
        if (!td) td = strstr(buf, "target_time");
        if (td) {
            int v = seconds;
            if (sscanf(td, "targetDuration%*[^0-9]%d", &v) == 1 || sscanf(td, "target_time%*[^0-9]%d", &v) == 1) seconds = v;
        }
        // Update Brew labels
        if (ui_BrewScreen_targetTemp) {
            char tbuf[16];
            snprintf(tbuf, sizeof(tbuf), "%.0f°C", temp);
            lv_label_set_text(ui_BrewScreen_targetTemp, tbuf);
            sim_temperature = temp;
        }
        if (ui_BrewScreen_targetDuration) {
            char dbuf[16];
            snprintf(dbuf, sizeof(dbuf), "%d:%02d", seconds/60, seconds%60);
            lv_label_set_text(ui_BrewScreen_targetDuration, dbuf);
            sim_brew_time = (float)seconds;
        }
        show_toast("Loaded profile from JSON");
    } else {
        // Fallback defaults per name
        if (ui_BrewScreen_targetTemp) {
            const char *temp = strcmp(name, "LM Leva") == 0 ? "90°C" : strcmp(name, "9Bar") == 0 ? "93°C" : strcmp(name, "Classic") == 0 ? "92°C" : "93°C";
            lv_label_set_text(ui_BrewScreen_targetTemp, temp);
        }
        if (ui_BrewScreen_targetDuration) {
            const char *dur = strcmp(name, "LM Leva") == 0 ? "0:25" : strcmp(name, "9Bar") == 0 ? "0:30" : strcmp(name, "Classic") == 0 ? "0:35" : "0:30";
            lv_label_set_text(ui_BrewScreen_targetDuration, dur);
        }
        show_toast("Applied default profile settings");
    }
    // Set flag before navigation so onBrewScreenLoad knows to show accept button
    sim_coming_from_profile = true;
    navigate_to(ui_BrewScreen);
    printf("Profile applied: %s — accept visible, start hidden\n", name);
}
extern "C" void onSimpleProcessScreenLoad(lv_event_t *e) {
    // Update label based on steam/water mode
    if (ui_SimpleProcessScreen_mainLabel5) {
        lv_label_set_text(ui_SimpleProcessScreen_mainLabel5, sim_is_steam ? "Steam" : "Water");
    }
    // Initialize temperature display for steam/water
    if (ui_SimpleProcessScreen_targetTemp) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.0f°C", sim_temperature);
        lv_label_set_text(ui_SimpleProcessScreen_targetTemp, buf);
    }
    printf("SimpleProcess loaded: %s\n", sim_is_steam ? "Steam" : "Water");
}
extern "C" void onSimpleProcessToggle(lv_event_t *e) {
    sim_simple_process_active = !sim_simple_process_active;
    const char *process_label = "Process";
    if (ui_SimpleProcessScreen_mainLabel5) {
        process_label = lv_label_get_text(ui_SimpleProcessScreen_mainLabel5);
    }
    
    if (sim_simple_process_active) {
        char msg[64];
        snprintf(msg, sizeof(msg), "%s started", process_label);
        show_toast(msg);
        printf("%s started\n", process_label);
        
        // Hide the go button (matches real device behavior)
        if (ui_SimpleProcessScreen_goButton) {
            lv_obj_add_flag(ui_SimpleProcessScreen_goButton, LV_OBJ_FLAG_HIDDEN);
        }
        
        // Create timer label if it doesn't exist
        if (!brew_timer_label && ui_SimpleProcessScreen_contentPanel5) {
            brew_timer_label = lv_label_create(ui_SimpleProcessScreen_contentPanel5);
            lv_obj_set_width(brew_timer_label, 150);
            lv_obj_set_height(brew_timer_label, 50);
            lv_obj_set_y(brew_timer_label, 70);  // y=70 to match BrewScreen timer position
            lv_obj_set_align(brew_timer_label, LV_ALIGN_CENTER);
            lv_obj_set_style_text_font(brew_timer_label, &lv_font_montserrat_34, LV_PART_MAIN);
            lv_obj_set_style_text_color(brew_timer_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
            lv_obj_set_style_text_align(brew_timer_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        }
        
        // Show timer label
        if (brew_timer_label) {
            lv_obj_clear_flag(brew_timer_label, LV_OBJ_FLAG_HIDDEN);
        }
        
        // Reset simulation state
        sim_total_elapsed = 0.0f;
        
        // Determine process duration based on mode
        float process_duration = 5.0f; // Default 5 seconds
        if (sim_is_steam) {
            process_duration = 5.0f; // Steam fill time ~5 seconds
        } else {
            process_duration = 10.0f; // Water dispense ~10 seconds
        }
        
        // Reset pressure/temperature for simulation
        sim_pressure_bar = 0.0f;
        sim_temp_c = sim_temperature;
        
        // Start process timer
        if (brew_timer) { lv_timer_del(brew_timer); brew_timer = NULL; }
        brew_timer = lv_timer_create([](lv_timer_t *t){
            float delta_t = t->period / 1000.0f; // 0.2 seconds
            sim_total_elapsed += delta_t;
            
            // Update timer label
            if (brew_timer_label) {
                int minutes = (int)(sim_total_elapsed) / 60;
                int seconds = (int)(sim_total_elapsed) % 60;
                char tbuf[32];
                snprintf(tbuf, sizeof(tbuf), "%d:%02d", minutes, seconds);
                lv_label_set_text(brew_timer_label, tbuf);
            }
            
            // Simulate pressure building up (for steam/water)
            if (sim_pressure_bar < 1.5f) {
                sim_pressure_bar += 0.05f; // Build to ~1.5 bar over 6 seconds
            }
            
            // Update pressure gauge and text
            if (uic_SimpleProcessScreen_dials_pressureGauge) {
                lv_arc_set_value(uic_SimpleProcessScreen_dials_pressureGauge, (int)(sim_pressure_bar * 10));
            }
            if (uic_SimpleProcessScreen_dials_pressureText) {
                char pbuf[16];
                snprintf(pbuf, sizeof(pbuf), "%.1f", sim_pressure_bar);
                lv_label_set_text(uic_SimpleProcessScreen_dials_pressureText, pbuf);
            }
            
            // Update temperature gauge
            if (uic_SimpleProcessScreen_dials_tempGauge) {
                lv_arc_set_value(uic_SimpleProcessScreen_dials_tempGauge, (int)sim_temp_c);
            }
            if (uic_SimpleProcessScreen_dials_tempText) {
                char tbuf2[16];
                snprintf(tbuf2, sizeof(tbuf2), "%.0f", sim_temp_c);
                lv_label_set_text(uic_SimpleProcessScreen_dials_tempText, tbuf2);
            }
            
        }, 200, NULL);
    } else {
        char msg[64];
        snprintf(msg, sizeof(msg), "%s stopped", process_label);
        show_toast(msg);
        printf("%s stopped\n", process_label);
        if (brew_timer) { 
            lv_timer_del(brew_timer); 
            brew_timer = NULL; 
        }
        
        // Hide timer label
        if (brew_timer_label) {
            lv_obj_add_flag(brew_timer_label, LV_OBJ_FLAG_HIDDEN);
        }
        
        // Reset pressure to 0
        sim_pressure_bar = 0.0f;
        if (uic_SimpleProcessScreen_dials_pressureGauge) {
            lv_arc_set_value(uic_SimpleProcessScreen_dials_pressureGauge, 0);
        }
        if (uic_SimpleProcessScreen_dials_pressureText) {
            lv_label_set_text(uic_SimpleProcessScreen_dials_pressureText, "0.0");
        }
        
        // Show go button again
        if (ui_SimpleProcessScreen_goButton) {
            lv_obj_clear_flag(ui_SimpleProcessScreen_goButton, LV_OBJ_FLAG_HIDDEN);
        }
    }
}
extern "C" void onStatusScreenLoad(lv_event_t *e) { /* no-op */ }

// Telemetry accessor for tests (returns JSON string)
extern "C" const char* getBrewTelemetryJSON() {
    static std::string json;
    // elapsed seconds = current phase elapsed (approx)
    json = std::string("{") +
           "\"pressure\":" + std::to_string(sim_pressure_bar) + "," +
           "\"temp\":" + std::to_string(sim_temp_c) + "," +
           "\"weight\":" + std::to_string(sim_weight) + "," +
           "\"elapsed\":" + std::to_string(sim_phase_elapsed) +
           "}";
    return json.c_str();
}
