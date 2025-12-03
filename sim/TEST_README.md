# 🧪 GaggiMate Test Framework

## What Is This?

An automated testing environment that lets you write test cases for the GaggiMate UI simulator. When you click a button, the framework verifies the UI responds correctly and produces detailed error reports.

## Quick Start

```bash
cd sim
./test.sh
```

This will:
1. Build the simulator (if needed)
2. Start the web server
3. Open your browser
4. Show testing instructions

Then just click **▶ Run Tests** in the browser!

## What Gets Tested?

✅ **Button clicks** - All navigation and control buttons  
✅ **Screen transitions** - Verify correct screen loads  
✅ **Navigation flow** - Multi-screen workflows  
✅ **Back button** - Returns to correct screens  
✅ **Value changes** - Temperature/time adjustments  

## Test Results

### Pass ✅
```
✅ Clicking Brew button navigates to BrewScreen
   234.56ms
```

### Fail ❌
```
❌ Back button returns to Standby
   Issue: Screen mismatch
   Expected: StandbyScreen
   Actual: BrewScreen
   345.67ms
```

Shows exactly what went wrong!

## Adding Your Own Tests

Edit `test_cases.js`:

```javascript
test('My custom test', async (ctx) => {
    // Wait for screen
    await ctx.waitForScreen('StandbyScreen', 3000);
    
    // Click button
    await ctx.clickButton('brew_button');
    
    // Verify result
    await ctx.assertScreenIs('BrewScreen');
});
```

Rebuild: `./build.sh`

## Available Buttons

### Navigation
- `brew_button`, `grind_button`, `menu_button`
- `back_button`, `profile_button`, `status_button`

### Controls
- `temp_raise_button`, `temp_lower_button`
- `time_raise_button`, `time_lower_button`
- `grind_time_raise_button`, `grind_time_lower_button`

### Actions
- `brew_start_button`, `grind_start_button`
- `flush_button`

## Available Screens

- `InitScreen` - Startup
- `StandbyScreen` - Main screen
- `BrewScreen` - Brew settings
- `GrindScreen` - Grind settings
- `MenuScreen` - Settings
- `ProfileScreen` - Profile editor
- `StatusScreen` - Status
- `SimpleProcessScreen` - Active process

## Test Methods

```javascript
// Wait for screen (with timeout)
await ctx.waitForScreen('ScreenName', 1000);

// Click button
await ctx.clickButton('button_id');

// Get current screen
const screen = ctx.getCurrentScreen();

// Verify screen
await ctx.assertScreenIs('ExpectedScreen');

// Generic assertions
ctx.assert(condition, 'Message');
ctx.assertEqual(actual, expected);

// Wait/delay
await ctx.sleep(100);
```

## Documentation

- **TESTING.md** - Quick reference
- **TEST_GUIDE.md** - Complete guide with examples
- **EXAMPLES.md** - Visual examples and debugging
- **TEST_IMPLEMENTATION.md** - Technical details

## Workflow

```bash
# 1. Make UI changes
vim src/display/ui/default/lvgl/screens/ui_BrewScreen.c

# 2. Build simulator
cd sim && ./build.sh

# 3. Run tests
./test.sh
# Click "▶ Run Tests"

# 4. Fix any failures

# 5. Commit when all pass
git commit -m "Update Brew screen"
```

## Example Test Session

```
🧪 Starting test suite...

📋 Running: InitScreen should transition to StandbyScreen
✅ PASS (234.56ms)

📋 Running: Clicking Brew button navigates to BrewScreen  
✅ PASS (123.45ms)

📋 Running: Back button from BrewScreen returns to StandbyScreen
✅ PASS (234.56ms)

... (13 more tests)

================================================
📊 TEST REPORT
================================================
Total: 16 | Passed: 16 | Failed: 0
Duration: 8543.20ms
================================================

✅ Test suite complete
```

## Benefits

✅ Catch bugs before hardware deployment  
✅ Verify button functionality automatically  
✅ Document expected UI behavior  
✅ Prevent regressions  
✅ Faster development iteration  

## Troubleshooting

### Tests Won't Run
- Check browser console (F12) for errors
- Verify files loaded (Network tab)
- Rebuild: `./build.sh`

### Test Timeout
- Increase timeout: `waitForScreen('Screen', 5000)`
- Check screen transition is implemented
- Verify button ID is correct

### Wrong Results
- Check Expected vs Actual in error message
- Verify screen name spelling (case-sensitive)
- Review console logs for details

## Support

Need help?
1. Check `TEST_GUIDE.md` for detailed documentation
2. See `EXAMPLES.md` for visual examples
3. Look at existing tests in `test_cases.js`

---

**Happy Testing! 🚀**

Write tests → Run tests → Fix bugs → Ship with confidence!
