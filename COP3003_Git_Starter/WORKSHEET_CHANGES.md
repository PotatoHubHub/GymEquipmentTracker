# Small changes for the worksheet

Choose one different change per member. Two-person teams choose any two; three-person teams can use all three. These short Git practice edits do not replace the separately assessed individual small-change activity in November.

Before editing, inspect the current code and record a prediction. Later members must pull earlier changes before starting, so expected results must match the current shared version.

## 1. Change the loan period

New rule: equipment may be borrowed for 10 days instead of 7. Update the program's loan-period setting. Check that checkout day 10 now gives due day 20, and that checkout day 365 gives due day 375.

Suggested commit message: `Change standard loan period to ten days`

## 2. Change the first allowed checkout day

New rule: checkout days start at 1 instead of 0. Keep the upper limit at 365. Ensure the prompt and range error match the rule. Check that 0 is rejected and 1 succeeds. The due day for 1 depends on the current loan period: 8 with seven days, or 11 with ten days.

Suggested commit message: `Reject checkout day zero`

## 3. Show the loan length in the summary

Add a summary line showing the loan length in days, computed from the due day and checkout day. Do not print an unrelated fixed number. Keep the existing two summary lines. Check that the displayed length equals the current loan period and that invalid input still produces no summary.

Suggested commit message: `Display calculated loan length in summary`

## Evidence for each student

- Student and GitHub username:
- Changed file/function:
- Prediction before editing:
- Input and expected result:
- Observed result after rebuilding:
- Commit link:
- Teammate who checked the result:
- Assistance used and what was verified, if applicable:

Update the README checks or sample output when your change makes them outdated. Those documentation changes belong with your code change. For the October 21 follow-up, identify which teammate's change you inspected and the result you repeated.
