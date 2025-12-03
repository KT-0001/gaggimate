/**
 * @file mouse_cursor_icon.c
 * @brief Mouse cursor icon data for simulator
 */

#include "lvgl.h"

// Simple cursor icon (optional - SDL provides its own)
// This can be used if you want a custom cursor in LVGL
const lv_img_dsc_t mouse_cursor_icon = {
    .header.always_zero = 0,
    .header.w = 0,
    .header.h = 0,
    .data_size = 0,
    .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,
    .data = NULL,
};
