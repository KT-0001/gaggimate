# Test Framework Guide

## Overview

The GaggiMate simulator includes an automated testing framework that allows you to write test cases to verify UI functionality. Tests simulate button clicks and verify that the UI responds correctly with the expected screen transitions and state changes.

## Running Tests

### Quick Start

1. Build and start the simulator:
   ```bash
   cd sim
   ./build.sh
   ./serve.sh
   ```

2. Open http://localhost:8000 in your browser

3. Click the "▶ Run Tests" button in the Test Suite panel

4. View results in real-time as tests execute

### What Gets Tested

The test suite automatically verifies:
- ✅ Screen transitions (InitScreen → StandbyScreen)
- ✅ Navigation button functionality (Brew, Grind, Menu)
- ✅ Back button returns to correct screens
- ✅ Temperature adjustment buttons
- ✅ Time adjustment buttons
- ✅ Start/action buttons (Brew Start, Grind Start, Flush)
- ✅ Multi-screen navigation sequences

## Writing Custom Tests

### Test Structure

Tests are defined in `test_cases.js`. Each test follows this pattern:

```javascript
test('Test description', async (ctx) => {
    // Test code here
});
```

### Available Test Methods

The test context (`ctx`) provides these helper methods:

#### Navigation & Screen Testing
```javascript
// Wait for a specific screen to load (with timeout)
await ctx.waitForScreen('BrewScreen', 1000);

// Get the current screen name
const screen = ctx.getCurrentScreen();

// Assert current screen matches expected
await ctx.assertScreenIs('StandbyScreen', 'Optional error message');
```

#### Button Simulation
```javascript
// Simulate clicking a button by ID
await ctx.clickButton('brew_button');
```

Available button IDs:
- `brew_button` - Navigate to Brew screen
- `grind_button` - Navigate to Grind screen
- `menu_button` - Navigate to Menu screen
- `back_button` - Return to Standby screen
- `profile_button` - Navigate to Profile screen
- `status_button` - Navigate to Status screen
- `temp_raise_button` - Increase brew temperature
- `temp_lower_button` - Decrease brew temperature
- `time_raise_button` - Increase brew time
- `time_lower_button` - Decrease brew time
- `grind_time_raise_button` - Increase grind time
- `grind_time_lower_button` - Decrease grind time
- `brew_start_button` - Start brewing
- `grind_start_button` - Start grinding
- `flush_button` - Trigger flush action

#### Assertions
```javascript
// Generic assertion
ctx.assert(condition, 'Error message if false');

// Equality assertion
ctx.assertEqual(actual, expected, 'Optional message');

// Inequality assertion
ctx.assertNotEqual(actual, expected, 'Optional message');
```

#### Utilities
```javascript
// Wait for specified milliseconds
await ctx.sleep(500);
```

### Example Test Cases

#### Simple Navigation Test
```javascript
test('Can navigate to Brew screen', async (ctx) => {
    // Wait for app to start
    await ctx.waitForScreen('StandbyScreen', 3000);
    
    // Click Brew button
    await ctx.clickButton('brew_button');
    
    // Verify we're on Brew screen
    await ctx.assertScreenIs('BrewScreen', 'Should show Brew screen');
});
```

#### Complex Multi-Step Test
```javascript
test('Brew flow works correctly', async (ctx) => {
    // Start at Standby
    await ctx.waitForScreen('StandbyScreen', 3000);
    
    // Navigate to Brew
    await ctx.clickButton('brew_button');
    await ctx.waitForScreen('BrewScreen', 1000);
    
    // Adjust temperature
    await ctx.clickButton('temp_raise_button');
    await ctx.sleep(100);
    
    // Start brewing
    await ctx.clickButton('brew_start_button');
    
    // Should transition to process screen
    await ctx.waitForScreen('SimpleProcessScreen', 1000);
    await ctx.assertScreenIs('SimpleProcessScreen', 'Should show brewing process');
});
```

#### Testing Button State
```javascript
test('Temperature buttons work on Brew screen', async (ctx) => {
    await ctx.waitForScreen('StandbyScreen', 3000);
    await ctx.clickButton('brew_button');
    await ctx.waitForScreen('BrewScreen', 1000);
    
    // Test both temp buttons
    await ctx.clickButton('temp_raise_button');
    await ctx.sleep(50);
    await ctx.clickButton('temp_lower_button');
    await ctx.sleep(50);
    
    // Should still be on Brew screen
    await ctx.assertScreenIs('BrewScreen', 'Should remain on Brew screen');
});
```

## Test Results

### Viewing Results

Test results appear in the Test Suite panel with:
- **Total/Passed/Failed counts**
- **Execution duration**
- **Individual test status** (✅ pass / ❌ fail)
- **Error messages** for failed tests
- **Expected vs Actual values** for assertion failures

### Console Output

Detailed test execution is logged to the browser console:
```
🧪 Starting test suite...
📋 Running: InitScreen should transition to StandbyScreen
✅ PASS: InitScreen should transition to StandbyScreen
...
📊 TEST REPORT
==========================================================
Total: 16 | Passed: 15 | Failed: 1
Duration: 8543.20ms
==========================================================
```

### Failed Test Format

When a test fails, you'll see:
```
❌ FAIL: Test name
   Issue: Screen mismatch
   Expected: BrewScreen
   Actual: StandbyScreen
```

## Adding New Tests

1. Open `sim/test_cases.js`

2. Add a new test using the `test()` function:
   ```javascript
   test('Your test description', async (ctx) => {
       // Your test logic
   });
   ```

3. Rebuild the simulator:
   ```bash
   cd sim
   ./build.sh
   ```

4. Refresh the browser and click "▶ Run Tests"

## Best Practices

### ✅ DO
- Write descriptive test names
- Wait for screens to load before assertions
- Add small delays after button clicks (`await ctx.sleep(50)`)
- Use specific error messages in assertions
- Test both success and error paths
- Group related tests logically

### ❌ DON'T
- Make tests dependent on each other (each should be independent)
- Use hardcoded delays longer than necessary
- Skip assertions (always verify expected behavior)
- Test too many things in one test (keep tests focused)

## Troubleshooting

### Test Timeout
If `waitForScreen()` times out:
- Check if the screen transition is implemented
- Verify the button ID is correct
- Look for console errors
- Increase timeout value if needed

### Button Click Doesn't Work
- Verify button ID matches the mapping in `test_helpers.c`
- Check that the UI event handler exists in `ui_stubs.cpp`
- Look for errors in browser console

### Test Runs But Fails
- Check the error message in test results
- Compare Expected vs Actual values
- Review console logs for additional context
- Verify the test logic matches actual UI behavior

## Advanced Usage

### Custom Assertions

You can create custom assertion helpers:
```javascript
async function assertTemperature(ctx, expected) {
    // Custom logic to verify temperature
    const temp = getTemperatureFromUI();
    ctx.assertEqual(temp, expected, `Temperature should be ${expected}`);
}

test('Temperature adjustment test', async (ctx) => {
    await assertTemperature(ctx, 93);
});
```

### Testing Error Conditions

Test that invalid actions are handled correctly:
```javascript
test('Cannot start brew without proper setup', async (ctx) => {
    await ctx.waitForScreen('StandbyScreen', 3000);
    // Try to start without navigating to brew screen
    // Should remain on standby
    await ctx.assertScreenIs('StandbyScreen');
});
```

## Integration with Development

### Pre-Commit Testing
Add to your workflow:
```bash
# Build and test before committing
cd sim && ./build.sh && open http://localhost:8000
# Click "Run Tests"
# Only commit if all tests pass
```

### Continuous Testing
Keep the simulator open while developing:
1. Make UI changes
2. Rebuild: `cd sim && ./build.sh`
3. Refresh browser
4. Run tests
5. Verify everything still works

### Regression Prevention
When fixing a bug:
1. Write a test that reproduces the bug (should fail)
2. Fix the bug in the code
3. Run tests (should now pass)
4. Keep the test to prevent regression

## Example Test Session

```javascript
// test_cases.js - Add your custom tests

test('Flush button responds from any screen', async (ctx) => {
    await ctx.waitForScreen('StandbyScreen', 3000);
    
    // Test from Standby
    await ctx.clickButton('flush_button');
    await ctx.sleep(100);
    await ctx.assertScreenIs('StandbyScreen');
    
    // Test from Brew screen
    await ctx.clickButton('brew_button');
    await ctx.waitForScreen('BrewScreen', 1000);
    await ctx.clickButton('flush_button');
    await ctx.sleep(100);
    // Should return to Standby or stay on Brew (depending on design)
});

test('Menu navigation works correctly', async (ctx) => {
    await ctx.waitForScreen('StandbyScreen', 3000);
    await ctx.clickButton('menu_button');
    await ctx.waitForScreen('MenuScreen', 1000);
    
    // Test menu items
    // ... add menu-specific tests
    
    // Back to Standby
    await ctx.clickButton('back_button');
    await ctx.assertScreenIs('StandbyScreen');
});
```

## Support

If you encounter issues or want to add new testing capabilities:
1. Check browser console for errors
2. Review test framework code in `test_framework.js`
3. Verify C bindings in `test_helpers.c`
4. Open an issue with test output and console logs
