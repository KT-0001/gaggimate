# Test Framework Usage Example

## Visual Interface

When you open http://localhost:8000, you'll see:

```
┌─────────────────────────────────┬────────────────────────────────┐
│   GaggiMate Display Simulator   │        Test Suite              │
│   LVGL 8.4 Browser Simulator    │                                │
│        480x480 Display           │   [▶ Run Tests]  [Clear]      │
│                                  │                                │
│  ┌───────────────────────────┐  │   Status: Ready                │
│  │                           │  │                                │
│  │                           │  │   ┌──────────────────────────┐ │
│  │       [Display            │  │   │ Total: 16                │ │
│  │        Canvas             │  │   │ Passed: 15 ✅           │ │
│  │         480x480]          │  │   │ Failed: 1 ❌            │ │
│  │                           │  │   │ Duration: 8543.20ms      │ │
│  │                           │  │   └──────────────────────────┘ │
│  │                           │  │                                │
│  └───────────────────────────┘  │   Test Results:                │
│                                  │                                │
│         [Reload]                 │   ✅ InitScreen transitions   │
│         FPS: 60                  │      234.56ms                  │
│                                  │                                │
│                                  │   ✅ Brew button navigates     │
│                                  │      123.45ms                  │
│                                  │                                │
│                                  │   ❌ Back button from Brew     │
│                                  │      Issue: Screen mismatch    │
│                                  │      Expected: StandbyScreen   │
│                                  │      Actual: BrewScreen        │
│                                  │      234.56ms                  │
│                                  │                                │
│                                  │   ✅ Temperature raise works   │
│                                  │      98.23ms                   │
│                                  │   ...                          │
└─────────────────────────────────┴────────────────────────────────┘
```

## Console Output Example

When you click "Run Tests", the browser console shows:

```
✅ Test Framework loaded
✅ Loaded 16 test cases
🧪 Starting test suite...

📋 Running: InitScreen should transition to StandbyScreen
[TEST] Simulating click: (none - automatic transition)
✅ PASS: InitScreen should transition to StandbyScreen

📋 Running: Clicking Brew button navigates to BrewScreen
[TEST] Simulating click: brew_button
[UI] onBrewScreen called - Loading BrewScreen
✅ PASS: Clicking Brew button navigates to BrewScreen

📋 Running: Clicking Grind button navigates to GrindScreen
[TEST] Simulating click: grind_button
[UI] onGrindScreen called - Loading GrindScreen
✅ PASS: Clicking Grind button navigates to GrindScreen

📋 Running: Back button from BrewScreen returns to StandbyScreen
[TEST] Simulating click: brew_button
[UI] onBrewScreen called - Loading BrewScreen
[TEST] Simulating click: back_button
[UI] onStandbyScreen called - Loading StandbyScreen
✅ PASS: Back button from BrewScreen returns to StandbyScreen

... (12 more tests)

============================================================
📊 TEST REPORT
============================================================
Total: 16 | Passed: 15 | Failed: 1
Duration: 8543.20ms
============================================================

❌ FAILED TESTS:

  Temperature adjustment maintains state
    Screen mismatch
    Expected: BrewScreen
    Actual: StandbyScreen

✅ Test suite complete
```

## Example: Test Passes

```javascript
test('Clicking Brew button navigates to BrewScreen', async (ctx) => {
    await ctx.waitForScreen('StandbyScreen', 3000);
    await ctx.clickButton('brew_button');
    await ctx.waitForScreen('BrewScreen', 1000);
    await ctx.assertScreenIs('BrewScreen');
});
```

**Console Output:**
```
📋 Running: Clicking Brew button navigates to BrewScreen
[TEST] Simulating click: brew_button
[UI] onBrewScreen called - Loading BrewScreen
✅ PASS: Clicking Brew button navigates to BrewScreen (234.56ms)
```

**UI Display:**
```
✅ Clicking Brew button navigates to BrewScreen
   234.56ms
```

## Example: Test Fails

```javascript
test('Back button returns to Standby', async (ctx) => {
    await ctx.waitForScreen('StandbyScreen', 3000);
    await ctx.clickButton('brew_button');
    await ctx.waitForScreen('BrewScreen', 1000);
    await ctx.clickButton('back_button');
    await ctx.assertScreenIs('StandbyScreen'); // FAILS if button broken
});
```

**Console Output (if fails):**
```
📋 Running: Back button returns to Standby
[TEST] Simulating click: brew_button
[UI] onBrewScreen called - Loading BrewScreen
[TEST] Simulating click: back_button
[UI] onStandbyScreen called - Loading StandbyScreen
❌ FAIL: Back button returns to Standby
   Issue: Screen mismatch
   Expected: StandbyScreen
   Actual: BrewScreen
```

**UI Display:**
```
❌ Back button returns to Standby
   Screen mismatch
   Expected: StandbyScreen
   Actual: BrewScreen
   345.67ms
```

## Real-World Testing Scenario

### Scenario: Testing a New Feature

You've added a new "Profile" button to the Menu screen. Let's write a test:

```javascript
// Add to test_cases.js
test('Profile button navigates from Menu to ProfileScreen', async (ctx) => {
    // Start at Standby
    await ctx.waitForScreen('StandbyScreen', 3000);
    
    // Go to Menu
    await ctx.clickButton('menu_button');
    await ctx.waitForScreen('MenuScreen', 1000);
    await ctx.assertScreenIs('MenuScreen', 'Should be on Menu');
    
    // Click Profile button
    await ctx.clickButton('profile_button');
    
    // Should navigate to Profile screen
    await ctx.waitForScreen('ProfileScreen', 1000);
    await ctx.assertScreenIs('ProfileScreen', 'Should navigate to Profile');
    
    // Test back button
    await ctx.clickButton('back_button');
    await ctx.waitForScreen('MenuScreen', 1000);
    await ctx.assertScreenIs('MenuScreen', 'Should return to Menu');
});
```

### If the Button ID is Wrong

**Error:**
```
📋 Running: Profile button navigates from Menu to ProfileScreen
[TEST] Simulating click: menu_button
[UI] onMenuScreen called - Loading MenuScreen
[TEST] Simulating click: profile_button
[TEST] Unknown button ID: profile_button
❌ FAIL: Profile button navigates from Menu to ProfileScreen
   Issue: Timeout waiting for screen: ProfileScreen
```

**Fix:** Check `test_helpers.c` - the button might be mapped as `profile_settings_button` instead.

### If the Event Handler is Missing

**Error:**
```
📋 Running: Profile button navigates from Menu to ProfileScreen
[TEST] Simulating click: profile_button
[UI] onProfileScreen called - Loading ProfileScreen
❌ FAIL: Profile button navigates from Menu to ProfileScreen
   Issue: Timeout waiting for screen: ProfileScreen
```

**Fix:** Check `ui_stubs.cpp` - make sure `onProfileScreen()` actually calls `lv_scr_load(ui_ProfileScreen)`.

### When Everything Works

**Success:**
```
📋 Running: Profile button navigates from Menu to ProfileScreen
[TEST] Simulating click: menu_button
[UI] onMenuScreen called - Loading MenuScreen
[TEST] Simulating click: profile_button
[UI] onProfileScreen called - Loading ProfileScreen
[TEST] Simulating click: back_button
[UI] onMenuScreen called - Loading MenuScreen
✅ PASS: Profile button navigates from Menu to ProfileScreen (456.78ms)
```

## Debugging Failed Tests

### Step-by-Step Debugging

1. **Look at the error message**
   ```
   Issue: Timeout waiting for screen: BrewScreen
   ```

2. **Check the console for the last successful action**
   ```
   [TEST] Simulating click: brew_button
   [UI] onBrewScreen called - Loading BrewScreen
   ```

3. **Verify the button ID exists in test_helpers.c**
   ```c
   } else if (strcmp(button_id, "brew_button") == 0) {
       onBrewScreen(&dummy_event);
   ```

4. **Check the event handler in ui_stubs.cpp**
   ```cpp
   void onBrewScreen(lv_event_t *e) {
       printf("[UI] onBrewScreen called - Loading BrewScreen\n");
       lv_scr_load(ui_BrewScreen);  // Make sure this line exists
   }
   ```

5. **Verify screen object exists**
   ```c
   extern lv_obj_t *ui_BrewScreen;  // In test_helpers.c
   ```

## Common Error Patterns

### Pattern 1: Screen Name Typo

```javascript
await ctx.assertScreenIs('BrewingScreen');  // ❌ Wrong name
```

**Error:**
```
Expected: BrewingScreen
Actual: BrewScreen
```

**Fix:**
```javascript
await ctx.assertScreenIs('BrewScreen');  // ✅ Correct
```

### Pattern 2: Missing Wait

```javascript
await ctx.clickButton('brew_button');
await ctx.assertScreenIs('BrewScreen');  // ❌ No wait, might fail
```

**Fix:**
```javascript
await ctx.clickButton('brew_button');
await ctx.waitForScreen('BrewScreen', 1000);  // ✅ Wait for transition
await ctx.assertScreenIs('BrewScreen');
```

### Pattern 3: Wrong Timeout

```javascript
await ctx.waitForScreen('StandbyScreen', 100);  // ❌ Too short
```

**Error:**
```
Timeout waiting for screen: StandbyScreen
```

**Fix:**
```javascript
await ctx.waitForScreen('StandbyScreen', 3000);  // ✅ 3 seconds
```

## Test Development Workflow

```
1. Write test ──→ 2. Rebuild ──→ 3. Run test ──→ 4. Test fails?
                     ./build.sh      Click "Run"         │
                                                         ├─ Yes → Debug
                                                         │        └─ Fix code
                                                         │        └─ Go to 2
                                                         │
                                                         └─ No → ✅ Done
```

## Integration Example

### Pre-Push Hook

Create `.git/hooks/pre-push`:

```bash
#!/bin/bash
echo "Running simulator tests..."

cd sim
./build.sh || exit 1

echo "Build successful. Please run tests in browser:"
echo "  1. ./serve.sh"
echo "  2. Open http://localhost:8000"
echo "  3. Click 'Run Tests'"
echo "  4. Verify all tests pass"
echo ""
read -p "All tests passed? (y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Push aborted - fix failing tests"
    exit 1
fi
```

---

This testing framework gives you **immediate feedback** on whether your UI changes work as expected, making development faster and more reliable! 🚀
