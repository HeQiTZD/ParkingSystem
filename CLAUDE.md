# ParkingSystem

Qt/C++ parking management system.

## Build

- Qt-based C++ project
- See `.qtc_clangd/` for clangd configuration

## Project Structure

- `UI/` — UI components (MainWindow, CameraManagement, UserManager)
- `styles/` — QSS stylesheets
- `docs/` — Documentation
- `generated/` — Generated files

## Skill routing

When the user's request matches an available skill, invoke it via the Skill tool. When in doubt, invoke the skill.

Key routing rules:
- Product ideas/brainstorming → invoke /office-hours
- Strategy/scope → invoke /plan-ceo-review
- Architecture → invoke /plan-eng-review
- Design system/plan review → invoke /design-consultation or /plan-design-review
- Full review pipeline → invoke /autoplan
- Bugs/errors → invoke /investigate
- QA/testing site behavior → invoke /qa or /qa-only
- Code review/diff check → invoke /review
- Visual polish → invoke /design-review
- Ship/deploy/PR → invoke /ship or /land-and-deploy
- Save progress → invoke /context-save
- Resume context → invoke /context-restore
