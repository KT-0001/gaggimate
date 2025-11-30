import { useState } from "preact/hooks";
import {
  getShotFeedback,
  type ShotSummary,
  type AdvancedShotMetrics,
  type ShotFeedback as ShotFeedbackType,
  type RoastLevel,
} from "../utils/shotFeedback";

interface UserPreferences {
  showShotFeedback: boolean;
}

interface ShotFeedbackProps {
  shot: ShotSummary;
  advancedMetrics?: AdvancedShotMetrics;
  userPrefs: UserPreferences;
  onUserPrefsChange?: (prefs: UserPreferences) => void;
  onRoastChange?: (roast: RoastLevel | undefined) => void;
}

// Helper to convert gaggimate shot history to ShotSummary format
export function shotHistoryToSummary(shot: any, notes: any): ShotSummary {
  const hasScale = Boolean(shot.volume && shot.volume > 0);
  
  return {
    dose_g: notes?.doseIn,
    targetYield_g: notes?.doseOut,
    actualYield_g: shot.volume,
    targetTime_s: undefined, // We don't have target time in shot data
    actualTime_s: shot.duration ? shot.duration / 1000 : undefined,
    targetRatio: notes?.ratio,
    hasScale,
    roastLevel: notes?.roastLevel,
  };
}

const feedbackAlertClass = (severity: ShotFeedbackType["severity"]) =>
  severity === "success"
    ? "alert-success"
    : severity === "warning"
    ? "alert-warning"
    : "alert-info";

export function ShotFeedback({
  shot,
  advancedMetrics,
  userPrefs,
  onUserPrefsChange,
  onRoastChange,
}: ShotFeedbackProps) {
  const [roastTypeTips, setRoastTypeTips] = useState(false);

  const feedback = userPrefs.showShotFeedback
    ? getShotFeedback(shot, advancedMetrics, { roastTypeTips })
    : null;

  return (
    <div className="space-y-6">
      {/* Global toggle */}
      <div className="flex items-center justify-between max-w-md">
        <span className="text-xs uppercase tracking-wide">
          Show Shot Feedback
        </span>
        <input
          type="checkbox"
          className="toggle toggle-xs"
          checked={userPrefs.showShotFeedback}
          onChange={(e) =>
            onUserPrefsChange?.({
              ...userPrefs,
              showShotFeedback: e.target.checked,
            })
          }
        />
      </div>

      {userPrefs.showShotFeedback && (
        <>
          {/* Roast type + Roast Type Tips */}
          <div className="flex flex-wrap items-end gap-4">
            <label className="form-control w-full max-w-xs">
              <div className="label pb-1">
                <span className="label-text text-xs uppercase">
                  Roast type
                </span>
              </div>
              <select
                className="select select-sm select-bordered"
                value={shot.roastLevel ?? ""}
                onChange={(e) =>
                  onRoastChange?.(
                    (e.target.value || undefined) as RoastLevel | undefined
                  )
                }
              >
                <option value="">Not set</option>
                <option value="light">Light roast</option>
                <option value="medium">Medium roast</option>
                <option value="dark">Dark roast</option>
              </select>
            </label>

            <label className="label cursor-pointer gap-2">
              <span className="label-text text-xs">Roast Type Tips</span>
              <input
                type="checkbox"
                className="toggle toggle-xs"
                checked={roastTypeTips}
                onChange={(e) => setRoastTypeTips(e.target.checked)}
              />
            </label>
          </div>

          {/* Feedback card */}
          {feedback && (
            <div
              className={`alert ${feedbackAlertClass(
                feedback.severity
              )} flex flex-col gap-1 items-start`}
            >
              <span className="font-medium">{feedback.message}</span>

              {feedback.extraction && (
                <span className="text-xs opacity-80">
                  {feedback.extraction}
                </span>
              )}

              {feedback.strength && (
                <span className="text-xs opacity-80">
                  {feedback.strength}
                </span>
              )}

              {feedback.roastNote && (
                <span className="text-xs opacity-80">
                  {feedback.roastNote}
                </span>
              )}

              {feedback.detail && (
                <span className="text-xs opacity-70">
                  {feedback.detail}
                </span>
              )}

              <span className="text-[11px] opacity-60 italic">
                {feedback.tasteNote}
              </span>
            </div>
          )}
        </>
      )}
    </div>
  );
}
