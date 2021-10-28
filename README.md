# Flash Gordon 2021
## BSOS Board Revision 1 w/ Arduino Nano
## Version 1.2.0
Re-imagined rules for Bally's 1981 Flash Gordon pinball machine. Based on the Bally/Stern Operating System, and implemented by adding a daughter card to the MPU's J5 connector. Refer to the hardware build instructions here: https://ballysternos.github.io/.

If you are looking for the Arduino Mega 2560 version of the code, see here: https://github.com/BallySternOS/FlashGordon2021.

### To use this code
* Download the zip file (Code > Download ZIP) or clone the repository to your hard drive.
* Unzip the FG2021 repository and make sure the parent folder is named: FG2021  
* Open FG2021.ino in Arduino's IDE and upload to microcontroller: Board - Arduino Nano, Processor - ATMega328P (maybe/maybe not old bootloader), Programmer - AVRISP mkII

### Operator game adjustments
This section at the top of the FG2021.ino file groups some variables that the operator may want to adjust, including difficutly settings like balls per game, wizard mode drop target goal, and wizard mode attack power goal.

### How to operate self test / audit
- Coindoor button: Enters self test / audit mode and advances through sections
- Credit/Reset button can be used to navigate through individual lamps, solenoids, or sounds
- Credit/Reset button can also be used to control editable features like number of credits
- Sections:
  - 1 Lights
  - 2 Displays
  - 3 Solenoids
  - 4 Switches
  - 5 Sounds
  - 6 Total Plays
  - 7 Highscore
  - 8 Highscores Beat
  - 9 Total Skill Shots (Player 1 display lowest star rollover, player 2 middle, and player 3 highest)
  - 10 Total Wizard Modes (Player 1 display modes started, and player 2 display modes completed)

### Version History
Latest Update v1.2.0 (10/21/21): Added audit for how many times the high score has been beat

v1.1.0 (8/5/21): Added attract mode speech callout. "Operator Game Adjustments" section added to the top of FG2021.ino to organize user definable variables. Also, a couple new ones were added: Turn strobe on or off (off is recommended at the moment), wizard mode drop target and attack power goals, attract mode speech callout and its timing. Updated wizard mode attack power saucer insert lamp animations so they are divided equally, and can handle operator game adjustment variables. Split up coin slot 1 & 2 use and added drop sounds, and updated credit addition to function properly. Repaired issue where 2x and 3x playfield multipliers weren't stacking. Adjusted EEPROM order numbering, variables, and reading/writing. Cleaned up some variable names.

v1.0.0 (7/11/21): First full release of the code! Code version number now appears on the player 1 display when turning on the game. Added Squawk & Talk board and sound playback/handling to code. Collecting a blinking inlane now starts a quick hurry up at the opposite spinner for 2000 a spin. Refined bonus collects so they countdown and add score as they go (including repeating for bonus multipliers). Added some EEPROM variables to track gameplay achievements. Self test modes are back, and I added a sound test and EEPROM readouts (highscore, games played, skill shot levels collected, wizard modes played, and mings defeated). Squashed some lamp animation, and saucer collect bugs. Created printable score/rule cards. Ming only laughs once after tilting now.

v0.3.1 (7/11/21): Resolved compiler warnings that I wasn't previously able to see.

v0.3.0 (3/28/21): Refined wizard mode, and fixed a player addition bug after a four player game completed. After playing the wizard mode awhile, I realized that I needed to add some interest/strategy, do some balancing, and make it a little easier. The pop bumpers now score 20 attack power instead of 22, and I've added the saucer into the attack power mix at 35. The saucer now always fires down into the pop bumpers instead of up. Finally, the Ming attack goal is set to 140 instead of 152.

v0.2.0 (3/8/21): Added backglass lamp animations for "Flash Gordon" and strobe into gameplay, and fixed a bug with the 3 bank drop target completion award.

v0.1.0 (3/2/21): Base gameplay code and lamp animations have been implemented. Video of initial gameplay instructions here https://www.youtube.com/watch?v=chJzxOgL2Jg.