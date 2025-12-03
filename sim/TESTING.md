# Testing Quick Reference

## Run Tests

1. Start simulator: `./serve.sh`
2. Open: http://localhost:8000
3. Click: **▶ Run Tests**

## Test Format

```javascript
test('Description', async (ctx) => {
    // Wait for screen
    await ctx.waitForScreen('ScreenName', timeout);
    
    // Click button
    await ctx.clickButton('button_id');
    
    // Verify screen
    await ctx.assertScreenIs('ExpectedScreen');
});
```

## Button IDs

**Navigation:**
- `brew_button` - Brew screen
- `grind_button` - Grind screen  
- `menu_button` - Menu screen
- `back_button` - Return to Standby
- `profile_button` - Profile screen
- `status_button` - Status screen

**Brew Controls:**
- `temp_raise_button` - Increase temp
- `temp_lower_button` - Decrease temp
- `time_raise_button` - Increase time
- `time_lower_button` - Decrease time
- `brew_start_button` - Start brewing

**Grind Controls:**
- `grind_time_raise_button` - Increase time
- `grind_time_lower_button` - Decrease time
- `grind_start_button` - Start grinding

**Actions:**
- `flush_button` - Flush

## Screen Names

- `InitScreen` - Startup
- `StandbyScreen` - Main/idle
- `BrewScreen` - Brew settings
- `GrindScreen` - Grind settings
- `MenuScreen` - Settings menu
- `ProfileScreen` - Profile editor
- `StatusScreen` - System status
- `SimpleProcessScreen` - Active process

## Common Patterns

```javascript
// Navigation test
await ctx.waitForScreen('StandbyScreen', 3000);
await ctx.clickButton('brew_button');
await ctx.assertScreenIs('BrewScreen');

// Button action
await ctx.clickButton('temp_raise_button');
await ctx.sleep(100);
await ctx.assertScreenIs('BrewScreen'); // Still on same screen

// Multi-step flow
await ctx.clickButton('brew_button');
await ctx.waitForScreen('BrewScreen', 1000);
await ctx.clickButton('brew_start_button');
await ctx.waitForScreen('SimpleProcessScreen', 1000);
```

## Add New Test

Edit `sim/test_cases.js`:

```javascript
test('My new test', async (ctx) => {
    // Test code here
});
```

Rebuild: `./build.sh`

## View Results

- **UI Panel:** Visual results with pass/fail
- **Console:** Detailed logs (F12 → Console)
- **Errors:** Shows expected vs actual values

## Debug Failed Tests

1. Check error message in test panel
2. Look at Expected vs Actual values
3. Open console (F12) for detailed logs
4. Verify button IDs and screen names match

See `TEST_GUIDE.md` for complete documentation.
