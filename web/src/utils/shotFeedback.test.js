// Test file for shot feedback logic
// This can be reviewed manually or run with Node.js if available

// Test cases for shot feedback system

const testCases = [
  {
    name: "Perfect shot - balanced",
    shot: {
      dose_g: 18,
      targetYield_g: 36,
      actualYield_g: 36.5,
      targetTime_s: 30,
      actualTime_s: 29,
      hasScale: true,
    },
    expected: {
      severity: "success",
      contains: "matched the target",
    }
  },
  {
    name: "Under-extracted and watery (fast, high yield)",
    shot: {
      dose_g: 18,
      targetYield_g: 36,
      actualYield_g: 45,
      targetTime_s: 30,
      actualTime_s: 22,
      hasScale: true,
    },
    expected: {
      severity: "warning",
      contains: "fast and high-yield",
    }
  },
  {
    name: "Over-extracted and strong (slow, low yield)",
    shot: {
      dose_g: 18,
      targetYield_g: 36,
      actualYield_g: 28,
      targetTime_s: 30,
      actualTime_s: 40,
      hasScale: true,
    },
    expected: {
      severity: "warning",
      contains: "slow and low-yield",
    }
  },
  {
    name: "No scale - should return null",
    shot: {
      dose_g: 18,
      targetYield_g: 36,
      actualYield_g: null,
      targetTime_s: 30,
      actualTime_s: 29,
      hasScale: false,
    },
    expected: null,
  },
  {
    name: "Light roast with roast-aware tips",
    shot: {
      dose_g: 18,
      targetYield_g: 36,
      actualYield_g: 38,
      targetTime_s: 30,
      actualTime_s: 28,
      hasScale: true,
      roastLevel: "light",
    },
    options: {
      roastTypeTips: true,
    },
    expected: {
      severity: "success",
      roastNote: "Light roasts often benefit",
    }
  },
  {
    name: "Dark roast over-extraction risk",
    shot: {
      dose_g: 18,
      targetYield_g: 36,
      actualYield_g: 36,
      targetTime_s: 25,
      actualTime_s: 28,
      hasScale: true,
      roastLevel: "dark",
    },
    options: {
      roastTypeTips: true,
    },
    expected: {
      roastNote: "Dark roasts can over-extract",
    }
  },
  {
    name: "Watery strength only",
    shot: {
      dose_g: 18,
      targetYield_g: 36,
      actualYield_g: 42,
      targetTime_s: 30,
      actualTime_s: 30,
      hasScale: true,
    },
    expected: {
      severity: "info",
      contains: "weaker side",
    }
  },
  {
    name: "Strong/concentrated only",
    shot: {
      dose_g: 18,
      targetYield_g: 36,
      actualYield_g: 30,
      targetTime_s: 30,
      actualTime_s: 30,
      hasScale: true,
    },
    expected: {
      severity: "info",
      contains: "quite strong",
    }
  },
];

console.log("Shot Feedback Test Cases");
console.log("========================\n");

testCases.forEach((testCase, index) => {
  console.log(`Test ${index + 1}: ${testCase.name}`);
  console.log("Input:");
  console.log(`  Dose: ${testCase.shot.dose_g}g`);
  console.log(`  Target Yield: ${testCase.shot.targetYield_g}g`);
  console.log(`  Actual Yield: ${testCase.shot.actualYield_g}g`);
  console.log(`  Target Time: ${testCase.shot.targetTime_s}s`);
  console.log(`  Actual Time: ${testCase.shot.actualTime_s}s`);
  if (testCase.shot.roastLevel) {
    console.log(`  Roast Level: ${testCase.shot.roastLevel}`);
  }
  
  console.log("\nExpected:");
  if (testCase.expected === null) {
    console.log("  No feedback (no scale data)");
  } else {
    console.log(`  Severity: ${testCase.expected.severity || "any"}`);
    if (testCase.expected.contains) {
      console.log(`  Message contains: "${testCase.expected.contains}"`);
    }
    if (testCase.expected.roastNote) {
      console.log(`  Roast note contains: "${testCase.expected.roastNote}"`);
    }
  }
  console.log("\n---\n");
});

console.log("Manual Testing Instructions:");
console.log("============================");
console.log("1. Start the Gaggimate web UI");
console.log("2. Navigate to Shot History");
console.log("3. Expand a shot that has scale data (volume > 0)");
console.log("4. Fill in shot notes:");
console.log("   - Dose In: 18g");
console.log("   - Dose Out: should auto-fill from shot volume");
console.log("   - Roast Level: select light/medium/dark");
console.log("5. Toggle 'Show Shot Feedback' to ON");
console.log("6. Toggle 'Roast Type Tips' to see roast-specific advice");
console.log("7. Verify:");
console.log("   - Feedback card appears with color coding");
console.log("   - Message suggests specific actions (grind finer/coarser, adjust yield)");
console.log("   - Roast notes appear when roast level is set and tips enabled");
console.log("   - 'Taste note' appears at bottom: 'Ultimately, it's down to taste'");
console.log("\nTest different scenarios:");
console.log("- Perfect shot (close to target): green success card");
console.log("- Fast + watery: yellow warning, suggests grinding finer");
console.log("- Slow + strong: yellow warning, suggests grinding coarser");
console.log("- Change roast level between light/medium/dark to see different tips");
