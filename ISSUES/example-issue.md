# Example: Timer Positioning Issue on Brew Screen

**Date Reported:** 2024-12-04  
**Reported By:** Kirk  
**Status:** Fixed

## Description
The brew timer was displaying at an incorrect vertical position, overlapping with the weight display on the Brew screen.

## Expected Behavior
The timer should be positioned at y=70 below the weight display, matching the layout of the StatusScreen exactly.

## Current Behavior
The timer was originally positioned at y=0 (center), directly overlapping the weight display which is also at y=0.

## Steps to Reproduce
1. Navigate to Brew Screen
2. Select a profile
3. Click play/start button
4. Observe timer position - it overlaps with weight label

## Affected Area
- [x] Brew Screen
- [ ] Steam Screen
- [ ] Water Screen
- [ ] Grind Screen
- [ ] Profile Screen
- [ ] Status Screen
- [ ] Menu Screen
- [ ] Simulator
- [ ] Hardware

## Severity
- [ ] Critical (blocks functionality)
- [x] High (significant issue)
- [ ] Medium (noticeable but workaround exists)
- [ ] Low (minor cosmetic issue)

## Screenshots/Evidence
Before: Timer at y=0 overlapping weight
After: Timer at y=70 below weight, clear separation

## Suggested Fix
Change the timer label's y-position from 0 to 70 in the onBrewStart event handler:
```cpp
lv_obj_set_y(brew_timer_label, 70);  // Position below weight display
```

This matches the StatusScreen_currentDuration positioning exactly.

## Additional Notes
The same positioning fix was applied to SimpleProcessScreen (steam/water) timer for consistency.
