export type RoastLevel = "light" | "medium" | "dark";

export interface ShotSummary {
  dose_g?: number;
  targetYield_g?: number;
  actualYield_g?: number;
  targetTime_s?: number;
  actualTime_s?: number;
  targetRatio?: number;
  hasScale: boolean;
  roastLevel?: RoastLevel; // user-set for this shot
}

export interface AdvancedShotMetrics {
  peakPressure_bar?: number;
  timeToFirstDrip_s?: number;
  suspectedChanneling?: boolean;
}

export interface FeedbackOptions {
  roastTypeTips?: boolean; // per-shot toggle
}

export interface ShotFeedback {
  message: string;              // main suggestion
  extraction?: string;          // e.g. "Likely under-extracted"
  strength?: string;            // e.g. "Watery / low strength"
  detail?: string;              // optional extra nuance
  roastNote?: string;           // optional roast-level hint
  severity: "success" | "info" | "warning";
  tasteNote: string;            // always: "Ultimately, it's down to taste – enjoy ☕️"
}

export function getShotFeedback(
  shot: ShotSummary,
  advanced?: AdvancedShotMetrics,
  options?: FeedbackOptions
): ShotFeedback | null {
  if (!shot.hasScale || !shot.actualYield_g || !shot.dose_g) return null;

  const roastAware = options?.roastTypeTips && !!shot.roastLevel;
  const { TOL_TIME, TOL_RATIO, roastNote } = getRoastAwareParams(
    roastAware ? shot.roastLevel! : undefined
  );

  const actualRatio = shot.actualYield_g / shot.dose_g;
  const targetRatio =
    shot.targetRatio ??
    (shot.targetYield_g ? shot.targetYield_g / shot.dose_g : undefined);

  const timeDiffRel =
    shot.targetTime_s && shot.actualTime_s
      ? (shot.actualTime_s - shot.targetTime_s) / shot.targetTime_s
      : 0;

  const ratioDiffRel =
    targetRatio && actualRatio
      ? (actualRatio - targetRatio) / targetRatio
      : 0;

  const withinTime =
    !shot.targetTime_s || Math.abs(timeDiffRel) <= TOL_TIME;
  const withinRatio =
    !targetRatio || Math.abs(ratioDiffRel) <= TOL_RATIO;

  const tasteNote = "Ultimately, it's down to taste – enjoy ☕️";

  // Extraction axis (-1 = under, 0 = OK, 1 = over)
  let extractionLevel: -1 | 0 | 1 = 0;
  if (shot.targetTime_s && !withinTime) {
    extractionLevel = timeDiffRel < 0 ? -1 : 1; // fast vs slow
  }

  // Strength axis (-1 = watery, 0 = OK, 1 = strong/muddy)
  let strengthLevel: -1 | 0 | 1 = 0;
  if (targetRatio && !withinRatio) {
    strengthLevel = ratioDiffRel > 0 ? -1 : 1; // higher ratio = more watery
  }

  const extractionText =
    extractionLevel === -1
      ? "Likely under-extracted (fast shot)."
      : extractionLevel === 1
      ? "Likely over-extracted (long shot)."
      : "Extraction is close to target.";

  const strengthText =
    strengthLevel === -1
      ? "On the weaker / more watery side."
      : strengthLevel === 1
      ? "On the stronger / more concentrated side."
      : "Strength is close to target.";

  // 1) Shot is close to target
  if (withinTime && withinRatio) {
    return {
      message: "Shot matched the target recipe. Nice pull!",
      extraction: "Balanced extraction.",
      strength: "Balanced strength.",
      roastNote,
      severity: "success",
      tasteNote,
    };
  }

  // 2) Grid logic: extraction × strength

  // Under-extracted + watery
  if (extractionLevel === -1 && strengthLevel === -1) {
    return {
      message:
        "Shot ran fast and high-yield (weak). Try grinding finer and stopping a bit earlier.",
      extraction: "Often perceived as sour / sharp.",
      strength: "On the watery side.",
      roastNote,
      severity: "warning",
      tasteNote,
    };
  }

  // Under-extracted + strong
  if (extractionLevel === -1 && strengthLevel === 1) {
    return {
      message:
        "Shot ran fast but quite strong. Try grinding slightly finer or allowing a bit more yield.",
      extraction: "Likely under-extracted.",
      strength: "Quite intense.",
      roastNote,
      severity: "info",
      tasteNote,
    };
  }

  // Over-extracted + very strong
  if (extractionLevel === 1 && strengthLevel === 1) {
    return {
      message:
        "Shot ran slow and low-yield (very strong). Try grinding coarser and letting it run a bit longer.",
      extraction: "Often perceived as bitter / harsh.",
      strength: "Very concentrated.",
      roastNote,
      severity: "warning",
      tasteNote,
    };
  }

  // Over-extracted + watery
  if (extractionLevel === 1 && strengthLevel === -1) {
    return {
      message:
        "Shot ran slow but still ended up fairly high-yield. Try grinding a touch coarser and aiming for a slightly lower yield.",
      extraction: "Leaning over-extracted.",
      strength: "A bit thin for the shot time.",
      roastNote,
      severity: "info",
      tasteNote,
    };
  }

  // Extraction OK, strength off
  if (strengthLevel === -1) {
    return {
      message:
        "Shot strength is on the weaker side. Try reducing yield slightly or grinding a bit finer.",
      extraction: extractionText,
      strength: "Watery / low strength.",
      roastNote,
      severity: "info",
      tasteNote,
    };
  }

  if (strengthLevel === 1) {
    return {
      message:
        "Shot is quite strong. Try increasing yield a little or grinding a touch coarser.",
      extraction: extractionText,
      strength: "High strength / muddy.",
      roastNote,
      severity: "info",
      tasteNote,
    };
  }

  // Fallback
  const detail = buildAdvancedDetail(advanced, shot, TOL_TIME);
  return {
    message:
      "Shot deviated from the target. Adjust grind or ratio and try again.",
    extraction: extractionText,
    strength: strengthText,
    detail,
    roastNote,
    severity: "info",
    tasteNote,
  };
}

function getRoastAwareParams(roast?: RoastLevel): {
  TOL_TIME: number;
  TOL_RATIO: number;
  roastNote?: string;
} {
  if (!roast) return { TOL_TIME: 0.10, TOL_RATIO: 0.10 };

  switch (roast) {
    case "light":
      return {
        TOL_TIME: 0.15,
        TOL_RATIO: 0.15,
        roastNote:
          "Light roasts often benefit from slightly longer ratios or finer grinds to tame sharp acidity.",
      };
    case "dark":
      return {
        TOL_TIME: 0.08,
        TOL_RATIO: 0.08,
        roastNote:
          "Dark roasts can over-extract quickly; shorter shots or coarser grinds often taste better.",
      };
    default:
      return {
        TOL_TIME: 0.10,
        TOL_RATIO: 0.10,
        roastNote:
          "Medium roasts are versatile; small grind or yield tweaks usually go a long way.",
      };
  }
}

function buildAdvancedDetail(
  advanced: AdvancedShotMetrics | undefined,
  shot: ShotSummary,
  tolTime: number
): string | undefined {
  if (!advanced) return undefined;

  if (
    advanced.peakPressure_bar &&
    advanced.peakPressure_bar > 10 &&
    shot.targetTime_s &&
    shot.actualTime_s &&
    shot.actualTime_s > shot.targetTime_s * (1 + tolTime)
  ) {
    return "High pressure and long shot time suggest the puck may be choking. Try a coarser grind or slightly lower dose.";
  }

  if (
    advanced.timeToFirstDrip_s &&
    shot.targetTime_s &&
    advanced.timeToFirstDrip_s < shot.targetTime_s * 0.1
  ) {
    return "Very early first drips can point to a coarse grind or uneven puck prep.";
  }

  if (advanced.suspectedChanneling) {
    return "Channeling suspected. Focus on distribution and tamping before changing grind.";
  }

  return undefined;
}
