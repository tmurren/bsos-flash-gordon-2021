# Flash Gordon 2021
## Retro Pin Upgrade - Board Revision 3 w/ Arduino Mega 2560
## Version 1.0.0
Re-imagined rules for Bally's 1981 Flash Gordon pinball machine. Based on the Retro Pin Upgrade (RPU) operating system, and implemented by adding a daughter card to the MPU's J5 connector.

If you are looking for the Arduino Nano version of the code, see here: https://github.com/tmurren/bsos-flash-gordon-2021.

### To use this code
* Download this zip file (Code > Download ZIP) or clone the repository to your hard drive.
* Go to https://github.com/RetroPinUpgrade/ExampleMachine and download the latest version of RPU
* Unzip the FG2021 repository and make sure the parent folder is named: FG2021
* Unzip the RPU repository and copy RPU.cpp and RPU.h to your FG2021 folder
* Open FG2021.ino in your FG2021 folder with Arduino's IDE and upload to microcontroller: Board - Arduino Mega or Mega 2560, Processor - ATmega2560 (Mega 2560), Programmer - ArduinoISP

### Operator game adjustments
This section at the top of the FG2021.ino file groups some variables that the operator may want to adjust, including difficulty settings like balls per game, wizard mode drop target goal, and wizard mode attack power goal.

### Adjustments on first startup
Score award thresholds to award replays can be set in self test / audit (may have defaulted to zero)

### How to operate self test / audit
- Coindoor button: Enters self test / audit mode and advances through sections
- Credit/Reset button can be used to navigate through individual lamps, solenoids, or sounds
- Credit/Reset button can also be used to control editable features like number of credits, and score award thresholds. Press button to advance number, hold button to advance number rapidly (speed increases the longer you hold), and double press button quickly to reset to zero
- Sections:
  - 1 Lights
  - 2 Displays
  - 3 Solenoids
  - 4 Switches
  - 5 Sounds
  - 6 Total Plays
  - 7 Total Replays
  - 8 Highscore
  - 9 Highscores Beat
  - 10 Total Skill Shots (Player 1 display lowest star rollover, player 2 middle, and player 3 highest)
  - 11 Total Wizard Modes (Player 1 display modes started, and player 2 display modes completed)
  - 12 Credits
  - Score Award Threshold 1
  - Score Award Threshold 2
  - Score Award Threshold 3

### Version History
Latest Update v1.1.0 (4/20/23):
- Naming changed to Retro Pin Upgrade

v1.0.0 (10/21/21):
- Created this alternate version of FG2021 that supports the latest BSOS version and is meant to be use with the revision 3 board (Arduino Mega 2560)
- Added audit for how many times the high score has been beat
- Added total replays to self test / audit
- Added score award thresholds to give credits that are adjustable in machine self test / audit
- Added current credits that are adjustable in machine self test / audit