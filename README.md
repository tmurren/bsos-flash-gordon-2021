# Flash Gordon 2021
# Version 1.0.0
Re-imagined rules for Bally's 1981 Flash Gordon pinball machine. Based on the Bally/Stern Operating System, and implemented by adding a daughter card to the MPU's J5 connector. Refer to the PDF or Wiki here for instructions on how to build the hardware: https://github.com/BallySternOS

### To use this code
* Download the zip file (Code > Download ZIP) or clone the repository to your hard drive.  
* Unzip the FG2021 repository and name the folder that it's in as: FG2021  
* Open the FG2021.ino in Arduino's IDE and upload to microcontroller

### Updates

Initial (3/2/21): Base gameplay code and lamp animations have been implemented. Video of initial gameplay instructions here https://www.youtube.com/watch?v=chJzxOgL2Jg

Update (3/8/21): Added backglass lamp animations for "Flash Gordon" and strobe into gameplay, and fixed a bug with the 3 bank drop target completion award

Update (3/28/21): Refined wizard mode, and fixed a player addition bug after a four player game completed. After playing the wizard mode awhile, I realized that I needed to add some interest/strategy, do some balancing, and make it a little easier. I'll add a selectable option for easy, medium, and hard wizard mode later, but right now I have the medium (normal) version up on GitHub. The pop bumpers now score 20 attack power instead of 22, and I've added the saucer into the attack power mix at 35. The saucer now always fires down into the pop bumpers instead of up. Finally, the Ming attack goal is set to 140 instead of 152.

Update (7/11/21): Resolved compiler warnings that I wasn't previously able to see. I also repaired my Squawk & Talk board that had garbled voice lines, so I can continue work on adding sound to the project. This video shows some sound testing during gameplay: https://www.youtube.com/watch?v=LDPYQP0j3YY

Latest Update v1.0.0 (7/11/21): First full release of the code! Code version number now shows up when entering attract mode. Added Squawk & Talk board and sound playback/handling to code. Collecting a blinking inlane now starts a quick hurry up at the opposite spinner for 2000 a spin. Refined bonus collects so they countdown and add score as they go (including repeating for bonus multipliers). Added some EEPROM variables to track gameplay achievements. Self test modes are back, and I added a sound test and EEPROM readouts (highscore, games played, skill shot levels collected, wizard modes played, and mings defeated). Squashed some lamp animation, and saucer collect bugs. Created printable score/rule cards. Ming only laughs once after tilting now.