/**
 * @file ui_stubs.cpp
 * @brief Functional implementations for UI callbacks in simulator
 */

#include "lvgl.h"
#include "ui.h"
#include <vector>
#include <string>
#include <stdio.h>

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
// Dials (from component children)
extern lv_obj_t * uic_BrewScreen_dials_tempGauge;
extern lv_obj_t * uic_BrewScreen_dials_tempTarget;
extern lv_obj_t * uic_BrewScreen_dials_pressureGauge;
extern lv_obj_t * uic_BrewScreen_dials_pressureTarget;
extern lv_obj_t * uic_BrewScreen_dials_pressureText;
extern lv_obj_t * uic_BrewScreen_dials_tempText;

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
// Simple navigation history to support "back" (caret ^)
static lv_obj_t * last_screen = NULL;
static void navigate_to(lv_obj_t *screen) {
    if (!screen) return;
    lv_obj_t *current = lv_scr_act();
    if (current && current != screen) last_screen = current;
    lv_scr_load(screen);
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

static void sim_load_phases_for_profile(const char *name) {
    sim_phases.clear();
    sim_phase_index = -1;
    sim_phase_elapsed = 0.0f;
    // Minimal mappings based on attached profiles.json
    if (strcmp(name, "9 Bar Espresso") == 0 || strcmp(name, "9Bar") == 0 || strcmp(name, "9 bar profile") == 0) {
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
    void onLoadStarted(lv_event_t * e) {
        // Transition from Init to Standby after loading
        lv_timer_t * timer = lv_timer_create([](lv_timer_t * t) {
            lv_scr_load(ui_StandbyScreen);
            lv_timer_del(t);
        }, 2000, NULL);
    }
    
    void onBrewCancel(lv_event_t *e) {
        sim_brewing = false;
        printf("Brew cancelled\n");
    }
    
    void onBrewStart(lv_event_t *e) {
        sim_brewing = !sim_brewing;
        printf("Brew %s\n", sim_brewing ? "started" : "stopped");
        // Start or stop in-place simulation on Brew screen
        if (sim_brewing) {
            // Reset state
            sim_weight = 0.0f;
            sim_phase_elapsed = 0.0f;
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
                sim_begin_next_phase();
            } else {
                // Default single-phase brew if none mapped
                sim_phases = { {SimPhase::BREW, sim_brew_time, sim_target_pressure_bar, 0.0f, sim_temperature, sim_volumetric ? sim_target_weight : 0.0f} };
                sim_phase_index = -1;
                sim_begin_next_phase();
            }
            // Create/update a timer to simulate flow
            if (brew_timer) { lv_timer_del(brew_timer); brew_timer = NULL; }
            brew_timer = lv_timer_create([](lv_timer_t *t){
                // Advance time/weight
                sim_weight += sim_flow_rate_gps * (t->period / 1000.0f);
                sim_phase_elapsed += (t->period / 1000.0f);
                // Update on-screen labels
                if (ui_BrewScreen_weightLabel) {
                    char buf[16];
                    snprintf(buf, sizeof(buf), "%.1f g", sim_weight);
                    lv_label_set_text(ui_BrewScreen_weightLabel, buf);
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
                if (uic_BrewScreen_dials_pressureTarget) {
                    lv_arc_set_value(uic_BrewScreen_dials_pressureTarget, (int)(sim_target_pressure_bar));
                }
                if (uic_BrewScreen_dials_pressureText) {
                    char ptxt[16];
                    snprintf(ptxt, sizeof(ptxt), "%.1f", sim_pressure_bar);
                    lv_label_set_text(uic_BrewScreen_dials_pressureText, ptxt);
                }
                if (uic_BrewScreen_dials_tempGauge) {
                    lv_arc_set_value(uic_BrewScreen_dials_tempGauge, (int)(sim_temp_c));
                }
                if (uic_BrewScreen_dials_tempTarget) {
                    lv_arc_set_value(uic_BrewScreen_dials_tempTarget, (int)(sim_temperature));
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
                // Timed UI label update based on remaining of current phase or total brew_time if no phases
                if (ui_BrewScreen_targetDuration) {
                    float remaining = 0.0f;
                    if (sim_phase_index >= 0 && sim_phase_index < (int)sim_phases.size()) {
                        const SimPhase &p = sim_phases[sim_phase_index];
                        remaining = p.duration_s - sim_phase_elapsed;
                    } else {
                        remaining = sim_brew_time - sim_phase_elapsed;
                    }
                    if (remaining < 0) remaining = 0;
                    int minutes = (int)(remaining) / 60;
                    int seconds = (int)(remaining) % 60;
                    char tbuf[16];
                    snprintf(tbuf, sizeof(tbuf), "%d:%02d", minutes, seconds);
                    lv_label_set_text(ui_BrewScreen_targetDuration, tbuf);
                }
                if (!sim_brewing) {
                    // Done: cleanup and notify
                    show_toast("Shot complete");
                    lv_timer_t *self = t;
                    brew_timer = NULL;
                    lv_timer_del(self);
                }
            }, 200, NULL);
        } else {
            // Stop brewing
            if (brew_timer) { lv_timer_del(brew_timer); brew_timer = NULL; }
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
            // Exit Brew settings if adjustments are visible
            if (lv_scr_act() == ui_BrewScreen && ui_BrewScreen_adjustments && !lv_obj_has_flag(ui_BrewScreen_adjustments, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_add_flag(ui_BrewScreen_adjustments, LV_OBJ_FLAG_HIDDEN);
                if (ui_BrewScreen_profileInfo) lv_obj_clear_flag(ui_BrewScreen_profileInfo, LV_OBJ_FLAG_HIDDEN);
                show_toast("Back to Brew");
                printf("Exited Brew settings\n");
                return;
            }
            if (last_screen) {
                lv_obj_t *target = last_screen;
                last_screen = NULL;
                navigate_to(target);
                printf("Back to previous screen\n");
            } else {
                onStandbyScreen(e);
            }
        }
        void onStandby(lv_event_t *e) { onStandbyScreen(e); }
        void onBrewScreenLoad(lv_event_t *e) { /* no-op for sim */ }
        void onGrindScreenLoad(lv_event_t *e) { /* no-op for sim */ }
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
            if (ui_BrewScreen_adjustments) lv_obj_clear_flag(ui_BrewScreen_adjustments, LV_OBJ_FLAG_HIDDEN);
            if (ui_BrewScreen_profileInfo) lv_obj_add_flag(ui_BrewScreen_profileInfo, LV_OBJ_FLAG_HIDDEN);
            show_toast("Brew settings opened");
            printf("Opened Brew settings mode on Brew screen\n");
        }
        void onProfileSave(lv_event_t *e) { printf("Profile saved: %s\n", sim_profiles[sim_profile_index]); }
        void onProfileAccept(lv_event_t *e) {
            // Apply selected profile to Brew screen and start brewing
            const char *name = sim_profiles[sim_profile_index];
            if (ui_BrewScreen_profileName) lv_label_set_text(ui_BrewScreen_profileName, name);
            // Load phases for the chosen profile and kick off the brew timer
            sim_load_phases_for_profile(name);
            sim_weight = 0.0f;
            sim_phase_index = -1;
            sim_phase_elapsed = 0.0f;
            sim_brewing = true;
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
                    }
                }, 100, NULL);
            }
            show_toast("Profile accepted — starting brew");
            printf("Profile accepted and brew started: %s\n", name);
        }
        void onProfileSaveAsNew(lv_event_t *e) { printf("Profile saved as new based on: %s\n", sim_profiles[sim_profile_index]); }
    
        // Grinding controls
        void onGrindToggle(lv_event_t *e) { printf("Grind toggle\n"); }
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
    
    void onSteamTempLower(lv_event_t *e) {
        sim_temperature = (sim_temperature > 100.0f) ? sim_temperature - 1.0f : 100.0f;
        printf("Steam temperature: %.1f°C\n", sim_temperature);
    }
    
    void onSteamTempRaise(lv_event_t *e) {
        sim_temperature = (sim_temperature < 140.0f) ? sim_temperature + 1.0f : 140.0f;
        printf("Steam temperature: %.1f°C\n", sim_temperature);
    }
    
    void onBrewScreen(lv_event_t *e) {
        navigate_to(ui_BrewScreen);
        sim_brewing = false;
        
        // Add arrow buttons to Container3 for profile navigation (if not already created)
        if (ui_BrewScreen_Container3 && !ui_BrewScreen_previousProfileBtn) {
            // Create Previous Profile button (left arrow)
            ui_BrewScreen_previousProfileBtn = lv_imgbtn_create(ui_BrewScreen_Container3);
            lv_imgbtn_set_src(ui_BrewScreen_previousProfileBtn, LV_IMGBTN_STATE_RELEASED, NULL, &ui_img_98036921, NULL);
            lv_obj_set_width(ui_BrewScreen_previousProfileBtn, 40);
            lv_obj_set_height(ui_BrewScreen_previousProfileBtn, 40);
            lv_obj_set_align(ui_BrewScreen_previousProfileBtn, LV_ALIGN_CENTER);
            lv_obj_set_style_img_recolor(ui_BrewScreen_previousProfileBtn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor_opa(ui_BrewScreen_previousProfileBtn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_event_cb(ui_BrewScreen_previousProfileBtn, onPreviousProfile, LV_EVENT_CLICKED, NULL);
            
            // Create Next Profile button (right arrow)
            ui_BrewScreen_nextProfileBtn = lv_imgbtn_create(ui_BrewScreen_Container3);
            lv_imgbtn_set_src(ui_BrewScreen_nextProfileBtn, LV_IMGBTN_STATE_RELEASED, NULL, &ui_img_944513416, NULL);
            lv_obj_set_width(ui_BrewScreen_nextProfileBtn, 40);
            lv_obj_set_height(ui_BrewScreen_nextProfileBtn, 40);
            lv_obj_set_align(ui_BrewScreen_nextProfileBtn, LV_ALIGN_CENTER);
            lv_obj_set_style_img_recolor(ui_BrewScreen_nextProfileBtn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_img_recolor_opa(ui_BrewScreen_nextProfileBtn, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_add_event_cb(ui_BrewScreen_nextProfileBtn, onNextProfile, LV_EVENT_CLICKED, NULL);
            
            // Move children within Container3 to get proper flex order:
            // We want: [prevBtn] [profileName] [settingsBtn] [profileSelectBtn] [nextBtn]
            // Container3 has flex row layout, so order matters
            // Move previousBtn to front, nextBtn to end
            lv_obj_move_to_index(ui_BrewScreen_previousProfileBtn, 0);
            // nextBtn should be last (will auto-position at end with flex)
        }
        
        printf("Navigated to Brew Screen\n");
    }
    
    void onWaterScreen(lv_event_t *e) {
        navigate_to(ui_SimpleProcessScreen);
        printf("Navigated to Water Screen\n");
    }
    
    void onSteamScreen(lv_event_t *e) {
        navigate_to(ui_SimpleProcessScreen);
        printf("Navigated to Steam Screen\n");
    }
    
    void onWakeup(lv_event_t *e) {
        navigate_to(ui_BrewScreen);
        printf("Wake up - navigated to Brew Screen\n");
    }
    
    void onFlush(lv_event_t *e) {
        printf("Flush activated\n");
    }
    
    void onGrindScreen(lv_event_t *e) {
        navigate_to(ui_GrindScreen);
        printf("Navigated to Grind Screen\n");
    }
    
    void onGrindStart(lv_event_t *e) {
        printf("Grind started for %.1fs\n", sim_grind_time);
        if (ui_SimpleProcessScreen) lv_scr_load(ui_SimpleProcessScreen);
    }
    
    void onGrindTimeLower(lv_event_t *e) {
        sim_grind_time = (sim_grind_time > 5.0f) ? sim_grind_time - 0.5f : 5.0f;
        printf("Grind time: %.1fs\n", sim_grind_time);
    }
    
    void onGrindTimeRaise(lv_event_t *e) {
        sim_grind_time = (sim_grind_time < 30.0f) ? sim_grind_time + 0.5f : 30.0f;
        printf("Grind time: %.1fs\n", sim_grind_time);
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
    navigate_to(ui_BrewScreen);
    printf("Profile applied: %s\n", name);
}
extern "C" void onSimpleProcessScreenLoad(lv_event_t *e) { printf("SimpleProcess loaded\n"); }
extern "C" void onSimpleProcessToggle(lv_event_t *e) { sim_brewing = !sim_brewing; printf("Process %s\n", sim_brewing ? "resume" : "pause"); }
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
