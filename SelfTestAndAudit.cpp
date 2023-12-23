/**************************************************************************
  This file is part of Flash Gordon 2021 - Mega

  Version: 1.0.0

  I, Tim Murren, the author of this program disclaim all copyright
  in order to make this program freely available in perpetuity to
  anyone who would like to use it. Tim Murren, 2/5/2021

  Flash Gordon 2021 is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Flash Gordon 2021 is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  See <https://www.gnu.org/licenses/>.
*/

#include <Arduino.h>
#include "SelfTestAndAudit.h"
#include "RPU_Config.h"
#include "RPU.h"

#define MACHINE_STATE_ATTRACT         0

unsigned long LastSolTestTime = 0; 
unsigned long LastSelfTestChange = 0;
unsigned long SavedValue = 0;
unsigned long ResetHold = 0;
unsigned long NextSpeedyValueChange = 0;
unsigned long NumSpeedyChanges = 0;
unsigned long LastResetPress = 0;
byte CurValue = 0;
byte CurSound = 0x01;
byte SoundPlaying = 0;
boolean SolenoidCycle = true;


int RunBaseSelfTest(int curState, boolean curStateChanged, unsigned long CurrentTime, byte resetSwitch, byte slamSwitch) {
  byte curSwitch = RPU_PullFirstFromSwitchStack();
  int returnState = curState;
  boolean resetDoubleClick = false;
  unsigned short savedScoreStartByte = 0;
  unsigned short auditNumStartByte = 0;

  if (curSwitch==resetSwitch) {
    ResetHold = CurrentTime;
    if ((CurrentTime-LastResetPress)<400) {
      resetDoubleClick = true;
      curSwitch = SWITCH_STACK_EMPTY;
    }
    LastResetPress = CurrentTime;
  }

  if (ResetHold!=0 && !RPU_ReadSingleSwitchState(resetSwitch)) {
    ResetHold = 0;
    NextSpeedyValueChange = 0;
  }

  boolean resetBeingHeld = false;
  if (ResetHold!=0 && (CurrentTime-ResetHold)>1300) {
    resetBeingHeld = true;
    if (NextSpeedyValueChange==0) {
      NextSpeedyValueChange = CurrentTime;
      NumSpeedyChanges = 0;
    }
  }

  if (slamSwitch!=0xFF && curSwitch==slamSwitch) {
    returnState = MACHINE_STATE_ATTRACT;
  }
  
  if (curSwitch==SW_SELF_TEST_SWITCH && (CurrentTime-LastSelfTestChange)>250) {
    returnState -= 1;
    if (returnState==MACHINE_STATE_TEST_DONE) returnState = MACHINE_STATE_ATTRACT;
    LastSelfTestChange = CurrentTime;
  }

  if (curStateChanged) {
    RPU_SetCoinLockout(false);
    
    for (int count=0; count<4; count++) {
      RPU_SetDisplay(count, 0);
      RPU_SetDisplayBlank(count, 0x00);        
    }
  }



  // #################### LIGHTS ####################
  if (curState==MACHINE_STATE_TEST_LIGHTS) {
    if (curStateChanged) {
      RPU_DisableSolenoidStack();
      RPU_SetDisableFlippers(true);
      RPU_SetDisplayCredits(0);
      RPU_SetDisplayBallInPlay(1);
      RPU_TurnOffAllLamps();
      for (int count=0; count<RPU_MAX_LAMPS; count++) {
        RPU_SetLampState(count, 1, 0, 500);
      }
      CurValue = 99;
      RPU_SetDisplay(0, CurValue, true);
    }
    if (curSwitch==resetSwitch || resetDoubleClick) {
      CurValue += 1;
      if (CurValue>99) CurValue = 0;
      if (CurValue==RPU_MAX_LAMPS) {
        CurValue = 99;
        for (int count=0; count<RPU_MAX_LAMPS; count++) {
          RPU_SetLampState(count, 1, 0, 500);
        }
      } else {
        RPU_TurnOffAllLamps();
        RPU_SetLampState(CurValue, 1, 0, 500);
      }
      RPU_SetDisplay(0, CurValue, true);
    }



  // #################### DISPLAYS ####################  
  } else if (curState==MACHINE_STATE_TEST_DISPLAYS) {
    if (curStateChanged) {
      RPU_TurnOffAllLamps();
      RPU_SetDisplayBlank(4, 0);
      RPU_SetDisplayBallInPlay(2);
      for (int count=0; count<4; count++) {
        RPU_SetDisplayBlank(count, 0x3F);
      }
      CurValue = 0;
    }
    if (curSwitch==resetSwitch || resetDoubleClick) {
      CurValue += 1;
      if (CurValue>34) CurValue = 0;
    }
    RPU_CycleAllDisplays(CurrentTime, CurValue);



  // #################### SOLENOIDS ####################
  } else if (curState==MACHINE_STATE_TEST_SOLENOIDS) {
    if (curStateChanged) {
      LastSolTestTime = CurrentTime;
      RPU_EnableSolenoidStack(); 
      RPU_SetDisableFlippers(false);
      RPU_SetDisplayBlank(4, 0);
      RPU_SetDisplayBallInPlay(3);
      SolenoidCycle = true;
      SavedValue = 0;
      RPU_PushToSolenoidStack(SavedValue, 5);
      RPU_SetDisplay(0, SavedValue, true);
    } 
    if (curSwitch==resetSwitch || resetDoubleClick) {
      SolenoidCycle = (SolenoidCycle) ? false : true;
    }

    if ((CurrentTime-LastSolTestTime)>1000) {
      if (SolenoidCycle) {
        SavedValue += 1;
        if (SavedValue>14) SavedValue = 0;
      }
      RPU_PushToSolenoidStack(SavedValue, 3);
      RPU_SetDisplay(0, SavedValue, true);
      LastSolTestTime = CurrentTime;
    }



  // #################### SWITCHES ####################
  } else if (curState==MACHINE_STATE_TEST_SWITCHES) {
    if (curStateChanged) {
      RPU_SetDisableFlippers(true);
      RPU_DisableSolenoidStack();
      RPU_SetDisplayBlank(4, 0);
      RPU_SetDisplayBallInPlay(4);
    }

    byte displayOutput = 0;
    for (byte switchCount=0; switchCount<40 && displayOutput<4; switchCount++) {
      if (RPU_ReadSingleSwitchState(switchCount)) {
        RPU_SetDisplay(displayOutput, switchCount, true);
        displayOutput += 1;
      }
    }

    if (displayOutput<4) {
      for (int count=displayOutput; count<4; count++) {
        RPU_SetDisplayBlank(count, 0x00);
      }
    }



  // #################### SOUNDS #################### ??? S&T NOT WORKING
  } else if (curState==MACHINE_STATE_TEST_SOUNDS) {
    if (curStateChanged) {
      RPU_SetDisplayBlank(4, 0);
      RPU_SetDisplayBallInPlay(5);
      CurValue = 0;
      RPU_SetDisplay(0, CurValue, true);
      RPU_PlaySoundSAndT(CurValue);
    }
    if (curSwitch==resetSwitch || resetDoubleClick) {
      CurValue += 1;
      if (CurValue>52) CurValue = 0;
      RPU_SetDisplay(0, CurValue, true);
      RPU_PlaySoundSAndT(CurValue);
    }



  // #################### TOTAL PLAYS ####################
  } else if (curState==MACHINE_STATE_TEST_TOTAL_PLAYS) {
    if (curStateChanged) {
      RPU_SetDisplayBlank(4, 0);
      RPU_PlaySoundSAndT(5); // sound off
      RPU_SetDisplayBallInPlay(6);
      SavedValue = RPU_ReadULFromEEProm(RPU_OS_TOTAL_PLAYS_EEPROM_START_BYTE);
      RPU_SetDisplay(0, SavedValue, true);
    }



  // #################### TOTAL REPLAYS ####################
  } else if (curState==MACHINE_STATE_TEST_TOTAL_REPLAYS) {
    if (curStateChanged) {
      RPU_SetDisplayBlank(4, 0);
      RPU_SetDisplayBallInPlay(7);
      auditNumStartByte = RPU_OS_TOTAL_REPLAYS_EEPROM_START_BYTE;
    }



  // #################### HIGHSCORE ####################
  } else if (curState==MACHINE_STATE_TEST_HISCR) {
    if (curStateChanged) {
      RPU_SetDisplayBlank(4, 0);
      RPU_SetDisplayBallInPlay(8);
      SavedValue = RPU_ReadULFromEEProm(RPU_OS_HIGHSCORE_EEPROM_START_BYTE);
      RPU_SetDisplay(0, SavedValue, true);
    }



  // #################### HIGHSCORES BEAT ####################
  } else if (curState==MACHINE_STATE_TEST_HISCR_BEAT) {
    if (curStateChanged) {
      RPU_SetDisplayBlank(4, 0);
      RPU_SetDisplayBallInPlay(9);
      auditNumStartByte = RPU_OS_TOTAL_HISCORE_BEATEN_START_BYTE;
    }



  // #################### TOTAL SKILL SHOTS ####################
  } else if (curState==MACHINE_STATE_TEST_TOTAL_SKILL) {
    if (curStateChanged) {
      RPU_SetDisplayBlank(4, 0);
      RPU_SetDisplayBallInPlay(10);
      SavedValue = RPU_ReadULFromEEProm(RPU_OS_TOTAL_SKILL_1_EEPROM_BYTE);
      RPU_SetDisplay(0, SavedValue, true);
      SavedValue = RPU_ReadULFromEEProm(RPU_OS_TOTAL_SKILL_2_EEPROM_BYTE);
      RPU_SetDisplay(1, SavedValue, true);
      SavedValue = RPU_ReadULFromEEProm(RPU_OS_TOTAL_SKILL_3_EEPROM_BYTE);
      RPU_SetDisplay(2, SavedValue, true);
    }



  // #################### TOTAL WIZARD MODES ####################
  } else if (curState==MACHINE_STATE_TEST_TOTAL_WIZARD) {
    if (curStateChanged) {
      RPU_SetDisplayBlank(4, 0);
      RPU_SetDisplayBallInPlay(11);
      SavedValue = RPU_ReadULFromEEProm(RPU_OS_TOTAL_WIZ_EEPROM_BYTE);
      RPU_SetDisplay(0, SavedValue, true);
      SavedValue = RPU_ReadULFromEEProm(RPU_OS_TOTAL_WIZ_BEAT_EEPROM_BYTE);
      RPU_SetDisplay(1, SavedValue, true);
    }



  // #################### CREDITS ####################
  } else if (curState==MACHINE_STATE_TEST_CREDITS) {
    if (curStateChanged) {
      RPU_SetDisplayBlank(4, 0);
      RPU_SetDisplayBallInPlay(12);
      SavedValue = RPU_ReadByteFromEEProm(RPU_OS_CREDITS_EEPROM_BYTE);
      RPU_SetDisplay(0, SavedValue, true);
    }
    if (curSwitch==resetSwitch || resetDoubleClick) {
      SavedValue += 1;
      if (SavedValue>20) SavedValue = 0;
      RPU_SetDisplay(0, SavedValue, true);
      RPU_WriteByteToEEProm(RPU_OS_CREDITS_EEPROM_BYTE, SavedValue & 0x000000FF);
    }



  // #################### SCORE AWARD 1 ####################
  } else if (curState==MACHINE_STATE_TEST_SCORE_LEVEL_1) {
    if (curStateChanged) {
      RPU_SetDisplayBallInPlay(13);
    }
    savedScoreStartByte = RPU_OS_SCORE_AWARD_1_EEPROM_START_BYTE;
    // if (curStateChanged) {
    //   RPU_SetDisplayBlank(4, 0);
    //   RPU_SetDisplayBallInPlay(13);
    //   SavedValue = RPU_ReadULFromEEProm(RPU_OS_SCORE_AWARD_1_EEPROM_START_BYTE);
    //   RPU_SetDisplay(0, SavedValue, true);
    // }
    // if (curSwitch==resetSwitch || resetDoubleClick) {
    //   SavedValue += 10000;
    //   if (SavedValue>9999999) SavedValue = 0;
    //   RPU_SetDisplay(0, SavedValue, true);
    //   RPU_WriteULToEEProm(RPU_OS_SCORE_AWARD_1_EEPROM_START_BYTE, SavedValue);
    // }



  // #################### SCORE AWARD 2 ####################
  } else if (curState==MACHINE_STATE_TEST_SCORE_LEVEL_2) {
    if (curStateChanged) {
      RPU_SetDisplayBallInPlay(14);
    }
    savedScoreStartByte = RPU_OS_SCORE_AWARD_2_EEPROM_START_BYTE;
    // if (curStateChanged) {
    //   RPU_SetDisplayBlank(4, 0);
    //   RPU_SetDisplayBallInPlay(14);
    //   SavedValue = RPU_ReadULFromEEProm(RPU_OS_SCORE_AWARD_2_EEPROM_START_BYTE);
    //   RPU_SetDisplay(0, SavedValue, true);
    // }
    // if (curSwitch==resetSwitch || resetDoubleClick) {
    //   SavedValue += 10000;
    //   if (SavedValue>9999999) SavedValue = 0;
    //   RPU_SetDisplay(0, SavedValue, true);
    //   RPU_WriteULToEEProm(RPU_OS_SCORE_AWARD_2_EEPROM_START_BYTE, SavedValue);
    // }



  // #################### SCORE AWARD 3 ####################
  } else if (curState==MACHINE_STATE_TEST_SCORE_LEVEL_3) {
    if (curStateChanged) {
      RPU_SetDisplayBallInPlay(15);
    }
    savedScoreStartByte = RPU_OS_SCORE_AWARD_3_EEPROM_START_BYTE;
    // if (curStateChanged) {
    //   RPU_SetDisplayBlank(4, 0);
    //   RPU_SetDisplayBallInPlay(15);
    //   SavedValue = RPU_ReadULFromEEProm(RPU_OS_SCORE_AWARD_3_EEPROM_START_BYTE);
    //   RPU_SetDisplay(0, SavedValue, true);
    // }
    // if (curSwitch==resetSwitch || resetDoubleClick) {
    //   SavedValue += 10000;
    //   if (SavedValue>9999999) SavedValue = 0;
    //   RPU_SetDisplay(0, SavedValue, true);
    //   RPU_WriteULToEEProm(RPU_OS_SCORE_AWARD_3_EEPROM_START_BYTE, SavedValue);
    // }



  // #################### CHUTE 1 COINS #################### ??? NEED TO ADD
  // } else if (curState==MACHINE_STATE_TEST_CHUTE_1_COINS) {
  //   if (curStateChanged) {
  //     RPU_SetDisplayBlank(4, 0);
  //     RPU_SetDisplayBallInPlay(16);
  //     auditNumStartByte = RPU_OS_CHUTE_1_COINS_START_BYTE;
  //   }



  // #################### CHUTE 2 COINS #################### ??? NEED TO ADD
  // } else if (curState==MACHINE_STATE_TEST_CHUTE_2_COINS) {
  //   if (curStateChanged) {
  //     RPU_SetDisplayBlank(4, 0);
  //     RPU_SetDisplayBallInPlay(17);
  //     auditNumStartByte = RPU_OS_CHUTE_2_COINS_START_BYTE;
  //   }



  // #################### CHUTE 3 COINS #################### ??? NEED TO ADD
  // } else if (curState==MACHINE_STATE_TEST_CHUTE_3_COINS) {
  //   if (curStateChanged) {
  //     RPU_SetDisplayBlank(4, 0);
  //     RPU_SetDisplayBallInPlay(18);
  //     auditNumStartByte = RPU_OS_CHUTE_3_COINS_START_BYTE;
  //   }
  }



  if (savedScoreStartByte) {
    if (curStateChanged) {
      SavedValue = RPU_ReadULFromEEProm(savedScoreStartByte);
      RPU_SetDisplay(0, SavedValue, true);  
    }

    if (curSwitch==resetSwitch) {
      SavedValue += 10000;
      RPU_SetDisplay(0, SavedValue, true);  
      RPU_WriteULToEEProm(savedScoreStartByte, SavedValue);
    }

    if (resetBeingHeld && (CurrentTime>=NextSpeedyValueChange)) {
      SavedValue += 10000;
      RPU_SetDisplay(0, SavedValue, true);  
      if (NumSpeedyChanges<6) NextSpeedyValueChange = CurrentTime + 400;
      else if (NumSpeedyChanges<50) NextSpeedyValueChange = CurrentTime + 50;
      else NextSpeedyValueChange = CurrentTime + 10;
      NumSpeedyChanges += 1;
    }

    if (!resetBeingHeld && NumSpeedyChanges>0) {
      RPU_WriteULToEEProm(savedScoreStartByte, SavedValue);
      NumSpeedyChanges = 0;
    }
    
    if (resetDoubleClick) {
      SavedValue = 0;
      RPU_SetDisplay(0, SavedValue, true);  
    }
  }

  if (auditNumStartByte) {
    if (curStateChanged) {
      SavedValue = RPU_ReadULFromEEProm(auditNumStartByte);
      RPU_SetDisplay(0, SavedValue, true);
    }

    if (resetDoubleClick) {
      SavedValue = 0;
      RPU_SetDisplay(0, SavedValue, true);  
      RPU_WriteULToEEProm(auditNumStartByte, SavedValue);
    }
    
  }
  
  return returnState;
}

unsigned long GetLastSelfTestChangedTime() {
  return LastSelfTestChange;
}


void SetLastSelfTestChangedTime(unsigned long setSelfTestChange) {
  LastSelfTestChange = setSelfTestChange;
}
