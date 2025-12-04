#!/bin/bash
# Quick manual test for caret back button

echo "================================================"
echo "  Caret Back Button Verification"
echo "================================================"
echo ""
echo "MANUAL TEST STEPS:"
echo "1. Wait for InitScreen → StandbyScreen transition"
echo "2. Click anywhere to wake → MenuScreen"
echo "3. Click Brew icon (coffee cup)"
echo "4. Click the caret (^) button at bottom-center"
echo "5. ✅ SUCCESS: Should navigate back to MenuScreen"
echo "6. Click the caret (^) button again"
echo "7. ✅ SUCCESS: Should navigate to StandbyScreen"
echo ""
echo "If the caret button shows toast but doesn't navigate,"
echo "check browser console for 'navigate_to:' messages."
echo ""
echo "Opening simulator..."
echo "================================================"

open "http://localhost:8000/lvgl_sim.html?manual=$(date +%s)"
