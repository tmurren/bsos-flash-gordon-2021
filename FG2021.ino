/**************************************************************************
  BSOS FILES: 42% 53% 530 lines
  FG2021 FILES: 81% 65% 24624 lines (???% of available space)
  
  This file is part of Flash Gordon 2021.

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

#include "BallySternOS.h"
#include "FG2021-Base.h"
#include "SelfTestAndAudit.h"
#include <EEPROM.h>

#define DEBUG_MESSAGES  0 // 1 is on
#define NIGHT_TESTING   1 // 0 is on

byte MachineState = 0;
boolean MachineStateChanged = true;
#define MACHINE_STATE_ATTRACT         0
#define MACHINE_STATE_INIT_GAMEPLAY   1
#define MACHINE_STATE_INIT_NEW_BALL   2
#define MACHINE_STATE_SKILL_SHOT      3
#define MACHINE_STATE_NORMAL_GAMEPLAY 4
#define MACHINE_STATE_COUNTDOWN_BONUS 90
#define MACHINE_STATE_WIZARD_MODE     91
#define MACHINE_STATE_BALL_OVER       100
#define MACHINE_STATE_MATCH_MODE      110

unsigned long CurrentTime = 0;
unsigned long InitGameStartTime = 0;

unsigned long HighScore = 0;
byte WholeCredit = 0;
byte Credits = 0;
#define MaxCredits 25 // byte
boolean FreePlay = true;
#define BallsPerGame 3 // byte

unsigned long AttractStartAnimation = 0;
byte AttractHeadMode = 255;
byte AttractPlayfieldMode = 255;
unsigned long AttractSweepTime = 0;
unsigned long AttractSirenTime = 0;
byte AttractSweepLights = 1;
byte AttractSirenLights = 1;
byte NormalGameplayBackglassMode = 255;

byte CurrentPlayer = 0;
byte CurrentBallInPlay = 1;
byte CurrentNumPlayers = 0;
unsigned long CurrentScores[4];

byte NumTiltWarnings = 0;
#define MaxTiltWarnings 1 // byte

byte SkillShotHits = 0;
byte SkillShotState = 0; // 0 not started, 1 started, 2 collecting, 3 collected/fail
unsigned long SkillShotScoreAnimation = 0;

#define TimeToWaitForBall 1000 // const int
unsigned long BallTimeInTrough = 0;
unsigned long BallFirstSwitchHitTime = 0;
boolean PFValidated = false;
boolean BallSaveUsed = false; // not used
unsigned long BallSaveNumSeconds = 0; // not used

unsigned long InlaneLeftBlip = 0;
unsigned long InlaneRightBlip = 0;
unsigned long TargetUpperBlip = 0;
unsigned long TargetLowerTopBlip = 0;
unsigned long TargetLowerBottomBlip = 0;

byte DTarget3Lights[3] = {0, 0, 0}; // A, B, C // 0 off, 1 flashing, 2 hurry up, 3 collected
byte DTarget3Flashing = 0;

byte DTarget4Light[4] = {0, 0, 0, 0}; // A, B, C, D // 0 off, 1 flashing, 2 hurry up, 3 collected
const int DTarget4HurryUpTimer = 12500; // 10000
unsigned long WhiteHurryUpStart = 0;
unsigned long AmberHurryUpStart = 0;
// for (byte i = 0; i < 4; i++) Serial.println(F(DTarget4Light[i]));

byte DropTargetCount = 0;
boolean DTargetInlineGoal = false;
boolean DTarget3Goal = false;
boolean DTarget4Goal = false;
byte DTarget3Completions = 0;

boolean LeftSpinnerLit = false;
boolean RightSpinnerLit = false;
boolean TopPopLit = false;
boolean TopSpecialLit = false;

boolean PlayerShootsAgain = false;
byte WoodBeastXBallState[4] = {0, 0, 0, 0}; // 0 off, 1 lit, 2 collected, 3 used
byte SacuerXBallState[4] = {0, 0, 0, 0}; // ^

byte SaucerLamps = 1;
unsigned long SaucerAnimation = 0;
unsigned long SaucerHitTime = 0;
byte SaucerDirection = 0;
boolean BallInSaucer = false;

byte BonusXState = 1;
const int PlayfieldXTimer = 15000;
byte Playfield2xState = 0; // 0 off, 1 lit, 2 started
byte Playfield3xState = 0; // ^
unsigned long Playfield2XStart = 0;
unsigned long Playfield3XStart = 0;

#define MaxMiniBonus 19 // const int
#define MaxMiniDisplayBonus 19 // const int
#define MaxSuperBonus 19 // const int
#define MaxSuperDisplayBonus 19 // const int
byte MiniBonus;
byte SuperBonus;
boolean Super100kCollected = false;
boolean Mini50kCollected = false;
boolean SuperBonusReady = false;
boolean MiniBonusReady = false;
unsigned long CountdownStartTime = 0;
unsigned long LastCountdownReportTime = 0;
unsigned long BonusCountDownEndTime = 0;
boolean MiniBonusCollecting = false;
unsigned long MiniBonusCollectTimer = 0;
boolean SuperBonusCollecting = false;
unsigned long SuperBonusCollectTimer = 0;

unsigned long MatchSequenceStartTime = 0;
unsigned long MatchDelay = 150;
byte MatchDigit = 0;
byte NumMatchSpins = 0;
byte ScoreMatches = 0;

byte WizardState = 0; // 0 unqualified, 1 qualified, 2 collected, 3 started, 4 validated, 5 ming defeated
byte MingAttackProgress = 0;
boolean MingAttackReady = false;
byte MingHealth = 3;
unsigned long MingAttackAnimation = 0;
byte MingAttackLamps = 1;


void setup() {
  if (DEBUG_MESSAGES) {
    Serial.begin(115200);
  }

  // read analog pin for match sequence
  randomSeed(analogRead(A6));

  // Start out with everything tri-state, in case the original
  // CPU is running
  // Set data pins to input
  // Make pins 2-7 input
  DDRD = DDRD & 0x03;
  // Make pins 8-13 input
  DDRB = DDRB & 0xC0;
  // Set up the address lines A0-A5 as input (for now)
  DDRC = DDRC & 0xC0;

  unsigned long startTime = millis();
  boolean sawHigh = false;
  boolean sawLow = false;
  // for three seconds, look for activity on the VMA line (A5)
  // If we see anything, then the MPU is active so we shouldn't run
  while ((millis()-startTime)<1200) {
    if (digitalRead(A5)) sawHigh = true;
    else sawLow = true;
  }
  // If we saw both a high and low signal, then someone is toggling the 
  // VMA line, so we should hang here forever (until reset)
  if (sawHigh && sawLow) {
    while (1);
  }

  // Tell the OS about game-specific lights and switches
  BSOS_SetupGameSwitches(NUM_SWITCHES_WITH_TRIGGERS, NUM_PRIORITY_SWITCHES_WITH_TRIGGERS, TriggeredSwitches);

  if (DEBUG_MESSAGES) {
    Serial.println(F("Attempting to initialize the MPU"));
  }
 
  // Set up the chips and interrupts
  BSOS_InitializeMPU();
  BSOS_DisableSolenoidStack();
  BSOS_SetDisableFlippers(true);

  byte dipBank = BSOS_GetDipSwitches(0);

  // Use dip switches to set up game variables
  if (DEBUG_MESSAGES) {
    char buf[32];
    sprintf(buf, "DipBank 0 = 0x%02X\n", dipBank);
    Serial.write(buf);
  }

  HighScore = BSOS_ReadULFromEEProm(BSOS_HIGHSCORE_EEPROM_START_BYTE, 300000);
  Credits = BSOS_ReadByteFromEEProm(BSOS_CREDITS_EEPROM_BYTE);
  if (Credits>MaxCredits) Credits = MaxCredits;

  if (DEBUG_MESSAGES) {
    Serial.println(F("Done with setup"));
  }

}


void AddCredit(byte numToAdd=1) {
  if (Credits<MaxCredits) {
    WholeCredit = WholeCredit + numToAdd;
    if (WholeCredit>=2) {
      Credits+=numToAdd;
      if (Credits>MaxCredits) Credits = MaxCredits;
      BSOS_WriteByteToEEProm(BSOS_CREDITS_EEPROM_BYTE, Credits);
      WholeCredit = 0;
    }
  }
  BSOS_SetCoinLockout((Credits<MaxCredits)?false:true);
}


boolean AddPlayer(boolean resetNumPlayers = false) {
  if (Credits<1 && !FreePlay) return false;
  if (resetNumPlayers) CurrentNumPlayers = 0;
  if (CurrentNumPlayers>=4) return false;

  BSOS_PlaySoundSquawkAndTalk(18);
  CurrentNumPlayers += 1;
  BSOS_SetDisplay(CurrentNumPlayers-1, 0);
  BSOS_SetDisplayBlank(CurrentNumPlayers-1, 0x60);

  if (!FreePlay) {
    Credits -= 1;
    BSOS_WriteByteToEEProm(BSOS_CREDITS_EEPROM_BYTE, Credits);
  }

  return true;
}


int InitNewBall(bool curStateChanged, int ballNum) {

  if (curStateChanged) {
    BSOS_PlaySoundSquawkAndTalk(44);
    BSOS_TurnOffAllLamps();
    BSOS_SetDisableFlippers(false);
    BSOS_EnableSolenoidStack();
    if (NIGHT_TESTING) {
      BSOS_PushToTimedSolenoidStack(SO_DTARGET_4_RESET, 15, CurrentTime + 500);
      BSOS_PushToTimedSolenoidStack(SO_DTARGET_3_RESET, 15, CurrentTime + 500);
      BSOS_PushToTimedSolenoidStack(SO_DTARGET_INLINE_RESET, 15, CurrentTime + 500);
      BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_DOWN, 15, CurrentTime + 500);
    }

    SkillShotHits = 0;
    SkillShotState = 0;

    PFValidated = false;

    InlaneLeftBlip = 0;
    InlaneRightBlip = 0;
    TargetUpperBlip = 0;
    TargetLowerTopBlip = 0;
    TargetLowerBottomBlip = 0;

    for (byte i = 0; i < 3; i++) DTarget3Lights[i] = 0;
    DTarget3Flashing = 0;

    for (byte i = 0; i < 4; i++) DTarget4Light[i] = 0;
    WhiteHurryUpStart = 0;
    AmberHurryUpStart = 0;

    DropTargetCount = 0;
    DTargetInlineGoal = false;
    DTarget3Goal = false;
    DTarget4Goal = false;
    DTarget3Completions = 0;

    LeftSpinnerLit = false;
    RightSpinnerLit = false;
    TopPopLit = false;
    TopSpecialLit = false;

    if (WoodBeastXBallState[CurrentPlayer]==2) {
      WoodBeastXBallState[CurrentPlayer] = 3;
    } else if (WoodBeastXBallState[CurrentPlayer]==1) {
      WoodBeastXBallState[CurrentPlayer] = 0;
    }
    if (SacuerXBallState[CurrentPlayer]==1) {
      SacuerXBallState[CurrentPlayer] = 0;
    }

    SaucerLamps = 1;
    SaucerAnimation = 0;
    SaucerHitTime = 0;
    SaucerDirection = 0;

    BonusXState = 1;
    Playfield2xState = 0;
    Playfield3xState = 0;
    Playfield2XStart = 0;
    Playfield3XStart = 0;

    Super100kCollected = false;
    Mini50kCollected = false;
    SuperBonusReady = false;
    MiniBonusReady = false;

    WizardState = 0;
    MingAttackProgress = 0;
    MingAttackReady = false;
    MingHealth = 3;
 
    // BSOS_SetDisplayCredits(Credits, true);

    if (BSOS_ReadSingleSwitchState(SW_OUTHOLE)) {
      BSOS_PushToTimedSolenoidStack(SO_OUTHOLE, 4, CurrentTime + 100);
    }

    for (byte count=0; count<CurrentNumPlayers; count++) {
      BSOS_SetDisplay(count, CurrentScores[count], true, 2);
    }

    BSOS_SetDisplayBallInPlay(ballNum);
    BSOS_SetLampState(LA_BALL_IN_PLAY, 1);
    BSOS_SetLampState(LA_TILT, 0);
    if (PlayerShootsAgain==true) {
      BSOS_SetLampState(LA_SAME_PLAYER_SHOOTS_AGAIN, 1);
      BSOS_SetLampState(LA_SHOOT_AGAIN, 1);
    }

    if (BallSaveNumSeconds>0) {
      BSOS_SetLampState(LA_SAME_PLAYER_SHOOTS_AGAIN, 1, 0, 500);
    }
  }

  MiniBonus = 0;
  SuperBonus = 0;
  NumTiltWarnings = 0;

  // We should only consider the ball initialized when 
  // the ball is no longer triggering the SW_OUTHOLE
  if (BSOS_ReadSingleSwitchState(SW_OUTHOLE)) {
    return MACHINE_STATE_INIT_NEW_BALL;
  } else {
    //return MACHINE_STATE_NORMAL_GAMEPLAY;
    return MACHINE_STATE_SKILL_SHOT;
  }

}


int RunSelfTest(int curState, boolean curStateChanged) {
  int returnState = curState;
  CurrentNumPlayers = 0;
  // Any state that's greater than CHUTE_3 is handled by the Base Self-test code
  // Any that's less, is machine specific, so we handle it here.
  if (curState>=MACHINE_STATE_TEST_CHUTE_3_COINS) {
    returnState = RunBaseSelfTest(returnState, curStateChanged, CurrentTime, SW_CREDIT_BUTTON);
  } else {
    returnState = MACHINE_STATE_ATTRACT;
  }
  return returnState;
}


int RunAttractMode(int curState, boolean curStateChanged) {

  int returnState = curState;

  // If this is the first time in the attract mode loop
  if (curStateChanged) {
    BSOS_PlaySoundSquawkAndTalk(8);
    BSOS_TurnOffAllLamps();
    BSOS_DisableSolenoidStack();
    BSOS_SetDisableFlippers(true);
    if (DEBUG_MESSAGES) {
      Serial.println(F("Entering Attract Mode"));
    }
    for (byte count=0; count<5; count++) {
      BSOS_SetDisplayBlank(count, 0x00);
    }
    if (!FreePlay) {
      BSOS_SetDisplayCredits(Credits);
    }
    BSOS_SetLampState(LA_GAME_OVER, 1);
    BSOS_SetDisplayBallInPlay(0);
    AttractStartAnimation = CurrentTime+2500;
    BSOS_SetLampState(LA_MING_TOP, 1, 0, 100);
    BSOS_SetLampState(LA_MING_BOTTOM, 1, 0, 200);
    AttractHeadMode = 255;
    AttractPlayfieldMode = 255;
  }

  if (CurrentTime>=AttractStartAnimation) {
    // Alternate displays between high score and blank
    if ((CurrentTime/6000)%2==0) {

      if (AttractHeadMode!=1) {
        BSOS_SetLampState(LA_HIGH_SCORE_TO_DATE, 1);

        for (byte count=0; count<4; count++) {
          BSOS_SetDisplay(count, HighScore, true, 2);
        }
        if (!FreePlay) {
          BSOS_SetDisplayCredits(Credits, true);
        }
        BSOS_SetDisplayBallInPlay(0, true);
      }
      AttractHeadMode = 1;

    } else {
      if (AttractHeadMode!=2) {
        BSOS_SetLampState(LA_HIGH_SCORE_TO_DATE, 0);
        if (!FreePlay) {
          BSOS_SetDisplayCredits(Credits, true);
        }
        BSOS_SetDisplayBallInPlay(0, true);
        for (byte count=0; count<4; count++) {
          if (CurrentNumPlayers>0) {
            if (count<CurrentNumPlayers) {
              BSOS_SetDisplay(count, CurrentScores[count], true, 2); 
            } else {
              BSOS_SetDisplay(count, 0);
              BSOS_SetDisplayBlank(count, 0xE0);
            }
          } else {
            BSOS_SetDisplay(count, 0);
            BSOS_SetDisplayBlank(count, 0xE0);
          }
        }
      }
      AttractHeadMode = 2;
    }

    // MAIN ATTRACT MODE - 4 MODES AT 7 SECONDS
    if ((CurrentTime/7000)%4==0) {
      if (AttractPlayfieldMode!=1) {
        BSOS_TurnOffAttractLamps();
      }
      AttractRetro();
      AttractPlayfieldMode = 1;
    } else if ((CurrentTime/7000)%4==1) {
      if (AttractPlayfieldMode!=2) {
        BSOS_TurnOffAttractLamps();
      }
      if ((CurrentTime-AttractSweepTime)>25) { // 50 (15 count right now)
        AttractSweepTime = CurrentTime;
        for (byte lightcountdown=0; lightcountdown<NUM_OF_ATTRACT_LAMPS_DOWN; lightcountdown++) {
          byte dist = AttractSweepLights - AttractLampsDown[lightcountdown].rowDown;
          BSOS_SetLampState(AttractLampsDown[lightcountdown].lightNumDown, (dist<8), (dist==0/*||dist>5*/)?0:dist/3, (dist>5)?(100+AttractLampsDown[lightcountdown].lightNumDown):0);
          if (lightcountdown==(NUM_OF_ATTRACT_LAMPS_DOWN/2)) BSOS_DataRead(0);
        }
        BSOS_DataRead(0);
        for (byte lightcountup=0; lightcountup<NUM_OF_ATTRACT_LAMPS_UP; lightcountup++) {
          byte dist = AttractSweepLights - AttractLampsUp[lightcountup].rowUp;
          BSOS_SetLampState(AttractLampsUp[lightcountup].lightNumUp, (dist<8), (dist==0/*||dist>5*/)?0:dist/3, (dist>5)?(100+AttractLampsUp[lightcountup].lightNumUp):0);
          if (lightcountup==(NUM_OF_ATTRACT_LAMPS_UP/2)) BSOS_DataRead(0);
        }
        AttractSweepLights += 1;
        if (AttractSweepLights>24) AttractSweepLights = 0; // 49
      }
      backglassLampsCenterOut();
      AttractPlayfieldMode = 2;
    } else if ((CurrentTime/7000)%4==2) {
      if (AttractPlayfieldMode!=3) {
        BSOS_TurnOffAttractLamps();
      }
      AttractRetro();
      AttractPlayfieldMode = 3;
    } else {
      if (AttractPlayfieldMode!=4) {
        BSOS_TurnOffAttractLamps();
      }
      if ((CurrentTime-AttractSirenTime)>33) { // 50 (15 count right now)
        AttractSirenTime = CurrentTime;
        for (byte lightcountsiren=0; lightcountsiren<NUM_OF_ATTRACT_LAMPS_SIREN; lightcountsiren++) {
          byte dist = AttractSirenLights - AttractLampsSiren[lightcountsiren].rowSiren;
          BSOS_SetLampState(AttractLampsSiren[lightcountsiren].lightNumSiren, (dist<8), (dist==0/*||dist>5*/)?0:dist/3, (dist>5)?(100+AttractLampsSiren[lightcountsiren].lightNumSiren):0);
          if (lightcountsiren==(NUM_OF_ATTRACT_LAMPS_SIREN/2)) BSOS_DataRead(0);
        }
        BSOS_DataRead(0);
        AttractSirenLights += 1;
        if (AttractSirenLights>32) AttractSirenLights = 0; // 49
      }
      BackglassLampsKnightRider();
      AttractPlayfieldMode = 4;
    }

    // byte squawkAndTalkByte = 0; // S&T
    byte switchHit;
    while ( (switchHit=BSOS_PullFirstFromSwitchStack())!=SWITCH_STACK_EMPTY ) {
      if (switchHit==SW_CREDIT_BUTTON) {
        if (AddPlayer(true)) returnState = MACHINE_STATE_INIT_GAMEPLAY;
      }
      if (switchHit==SW_COIN_1 || switchHit==SW_COIN_2) {
        AddCredit(1);
        if (!FreePlay) {
          BSOS_SetDisplayCredits(Credits, true);
        }
      }
      if (switchHit==SW_COIN_3) {
        AddCredit(2);
        if (!FreePlay) {
          BSOS_SetDisplayCredits(Credits, true);
        }
      }
      if (switchHit==SW_SELF_TEST_SWITCH && (CurrentTime-GetLastSelfTestChangedTime())>500) {
        returnState = MACHINE_STATE_TEST_LIGHTS;
        SetLastSelfTestChangedTime(CurrentTime);
      }
      // if (DEBUG_MESSAGES) {
      //   char buf[128];
      //   sprintf(buf, "Switch = 0x%02X\n", switchHit);
      //   Serial.write(buf);
      // }
    }
  }

  return returnState;
}



int NormalGamePlay(boolean curStateChanged) {
  int returnState = MACHINE_STATE_NORMAL_GAMEPLAY;
  BSOS_SetDisplayCredits(DropTargetCount, true);

  if (curStateChanged) {
    SuperBonusReady = false;
    BSOS_SetLampState(LA_STAR_SHOOTER_BOTTOM, 0);
    BSOS_SetLampState(LA_STAR_SHOOTER_MIDDLE, 0);
    BSOS_SetLampState(LA_STAR_SHOOTER_TOP, 0);
    BSOS_SetLampState(LA_STAR_PFIELD_TOP, 0);
    BSOS_SetLampState(LA_STAR_PFIELD_BOTTOM, 0);
    BSOS_SetLampState(LA_MING_TOP, 1);
    BSOS_SetLampState(LA_MING_BOTTOM, 1);
    if (PFValidated==true) {
      if (NIGHT_TESTING) BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
    }
  }


  // handle backglass animations
  if (PFValidated==false) {
    BackglassLampsLeft2Right();
  } else if (PFValidated==true && BallInSaucer==false) {
    BackglassLampsClear();
    if ((CurrentTime/7000)%3==0) {
      if (NormalGameplayBackglassMode!=1) {
        BackglassLampsClear();
      }
      BackglassLampsKnightRider();
      NormalGameplayBackglassMode = 1;
    } else if ((CurrentTime/7000)%3==1) {
      if (NormalGameplayBackglassMode!=2) {
        BackglassLampsClear();
      }
      backglassLampsCenterOut();
      NormalGameplayBackglassMode = 2;
    } else {
      if (NormalGameplayBackglassMode!=3) {
        BackglassLampsClear();
      }
      BackglassLampsLeft2Right();
      NormalGameplayBackglassMode = 3;
    }
  }


  // handle dtarget 4 lights
  // 0 off, 1 flashing, 2 hurry up, 3 collected
  int DTarget4ToHandle = 0;
  byte lamp4Phase = (CurrentTime/2000)%2;
  for (int i = 3; i > -1; i--) {
    if (i==0) {
      DTarget4ToHandle = LA_DTARGET_4_A;
    } else if (i==1) {
      DTarget4ToHandle = LA_DTARGET_4_B;
    } else if (i==2) {
      DTarget4ToHandle = LA_DTARGET_4_C;
    } else if (i==3) {
      DTarget4ToHandle = LA_DTARGET_4_D;
    }
    if (DTarget4Light[i]<=1) {
      if (lamp4Phase==0) {
        if (i==0 || i==2) {
          BSOS_SetLampState(DTarget4ToHandle, 1, 0, 250);
          DTarget4Light[i] = 1;
        } else {
          if (DTarget4Light[0]>=2 && DTarget4Light[2]>=2 && (DTarget4Light[1]<=1 || DTarget4Light[3]<=1)) {
            if (DTarget4Light[1]<=1 && DTarget4Light[3]<=1) {
              BSOS_SetLampState(LA_DTARGET_4_B, 1, 0, 250);
              BSOS_SetLampState(LA_DTARGET_4_D, 1, 0, 250);
            } else if (DTarget4Light[1]<=1 && DTarget4Light[3]>=2) {
              BSOS_SetLampState(LA_DTARGET_4_B, 1, 0, 250);
            } else if (DTarget4Light[1]>=2 && DTarget4Light[3]<=1) {
              BSOS_SetLampState(LA_DTARGET_4_D, 1, 0, 250);
            }
          } else {
            BSOS_SetLampState(DTarget4ToHandle, 0);
            DTarget4Light[i] = 0;
          }
        }
      } else {
        if (i==1 || i==3) {
          BSOS_SetLampState(DTarget4ToHandle, 1, 0, 250);
          DTarget4Light[i] = 1;
        } else {
          if (DTarget4Light[1]>=2 && DTarget4Light[3]>=2 && (DTarget4Light[0]<=1 || DTarget4Light[2]<=1)) {
            if (DTarget4Light[0]<=1 && DTarget4Light[2]<=1) {
              BSOS_SetLampState(LA_DTARGET_4_A, 1, 0, 250);
              BSOS_SetLampState(LA_DTARGET_4_C, 1, 0, 250);
            } else if (DTarget4Light[0]<=1 && DTarget4Light[2]>=2) {
              BSOS_SetLampState(LA_DTARGET_4_A, 1, 0, 250);
            } else if (DTarget4Light[0]>=2 && DTarget4Light[2]<=1) {
              BSOS_SetLampState(LA_DTARGET_4_C, 1, 0, 250);
            }
          } else {
            BSOS_SetLampState(DTarget4ToHandle, 0);
            DTarget4Light[i] = 0;
          }
        }
      }
    } else if (DTarget4Light[i]>=2) {
      BSOS_SetLampState(DTarget4ToHandle, 1);
    }
  }


  // handle dtarget 3 lights
  unsigned long NumberOf3LampsToStrobe = 3;
  int DTarget3ToLight = 0;
  unsigned long lamp3PhaseIncrement = 0;
  if (DTarget3Lights[0]==1 || DTarget3Lights[0]==2) NumberOf3LampsToStrobe--;
  if (DTarget3Lights[1]==1 || DTarget3Lights[1]==2) NumberOf3LampsToStrobe--;
  if (DTarget3Lights[2]==1 || DTarget3Lights[2]==2) NumberOf3LampsToStrobe--;
  byte lamp3Phase = (CurrentTime/800)%NumberOf3LampsToStrobe;
  for (byte i = 0; i < 3; i++) {
    if (i==0) {
      DTarget3ToLight = LA_DTARGET_ARROW_1;
    } else if (i==1) {
      DTarget3ToLight = LA_DTARGET_ARROW_2;
    } else if (i==2) {
      DTarget3ToLight = LA_DTARGET_ARROW_3;
    }
    if (DTarget3Lights[i]==1 || DTarget3Lights[i]==2) {
      BSOS_SetLampState(DTarget3ToLight, 1);
    } else {
      BSOS_SetLampState(DTarget3ToLight, lamp3Phase==lamp3PhaseIncrement, 0, 200);
      if (lamp3Phase==lamp3PhaseIncrement) {
        DTarget3Flashing = DTarget3ToLight;
      }
      lamp3PhaseIncrement++;
    }
  }


  // handle super bonus collect lit animation
  if (SuperBonusReady!=true) {
    BSOS_SetLampState(LA_STAR_SHOOTER_BOTTOM, 0);
    BSOS_SetLampState(LA_STAR_SHOOTER_MIDDLE, 0);
    BSOS_SetLampState(LA_STAR_SHOOTER_TOP, 0);
    BSOS_SetLampState(LA_STAR_PFIELD_TOP, 0);
    BSOS_SetLampState(LA_STAR_PFIELD_BOTTOM, 0);
  } else {
    byte lamp6Phase = (CurrentTime/125)%6;
    BSOS_SetLampState(LA_STAR_SHOOTER_BOTTOM, lamp6Phase==4);
    BSOS_SetLampState(LA_STAR_SHOOTER_MIDDLE, lamp6Phase==3||lamp6Phase==4, lamp6Phase==4);
    BSOS_SetLampState(LA_STAR_SHOOTER_TOP, lamp6Phase==2||lamp6Phase==3, lamp6Phase==3);
    BSOS_SetLampState(LA_STAR_PFIELD_TOP, lamp6Phase==1||lamp6Phase==2, lamp6Phase==2);
    BSOS_SetLampState(LA_STAR_PFIELD_BOTTOM, lamp6Phase<2, lamp6Phase==1);
  }

  // handle mini bonus ring collect animation
  if (MiniBonusCollecting==true) {
    if (CurrentTime<=MiniBonusCollectTimer) {
      if (MiniBonus>=1) {
        for (byte count=0; count<10; count++) {
          BSOS_SetLampState(count, 1, 0, 150);
        }
      }
    } else {
      if (Playfield3xState==2 && Playfield2xState==2) {
        MiniBonus = MiniBonus * 5;
      } else if (Playfield3xState==2) {
        MiniBonus = MiniBonus * 3;
      } else if (Playfield2xState==2) {
        MiniBonus = MiniBonus * 2;
      }
      if (BonusXState==1) {
        int ScoreBonus = 1000 * MiniBonus;
        CurrentScores[CurrentPlayer] += ScoreBonus;
      } else if (BonusXState==2) {
        int ScoreBonus = 2000 * MiniBonus;
        CurrentScores[CurrentPlayer] += ScoreBonus;
      } else if (BonusXState==3) {
        int ScoreBonus = 3000 * MiniBonus;
        CurrentScores[CurrentPlayer] += ScoreBonus;
      } else if (BonusXState==4) {
        int ScoreBonus = 4000 * MiniBonus;
        CurrentScores[CurrentPlayer] += ScoreBonus;
      } else if (BonusXState==5) {
        int ScoreBonus = 5000 * MiniBonus;
        CurrentScores[CurrentPlayer] += ScoreBonus;
      }
      MiniBonus = 0;
      ShowMiniBonusOnLadder(MiniBonus);
      MiniBonusReady = false;
      MiniBonusCollecting = false;
    }
  }

  // handle super bonus ring collect animation
  if (SuperBonusCollecting==true) {
    if (CurrentTime<=SuperBonusCollectTimer) {
      if (SuperBonus>=1) {
        for (byte count=12; count<22; count++) {
          BSOS_SetLampState(count, 1, 0, 150);
        }
      }
    } else {
      if (Playfield3xState==2 && Playfield2xState==2) {
        SuperBonus = SuperBonus * 5;
      } else if (Playfield3xState==2) {
        SuperBonus = SuperBonus * 3;
      } else if (Playfield2xState==2) {
        SuperBonus = SuperBonus * 2;
      }
      if (BonusXState==1) {
        int ScoreBonus = 1000 * SuperBonus;
        CurrentScores[CurrentPlayer] += ScoreBonus;
      } else if (BonusXState==2) {
        int ScoreBonus = 2000 * SuperBonus;
        CurrentScores[CurrentPlayer] += ScoreBonus;
      } else if (BonusXState==3) {
        int ScoreBonus = 3000 * SuperBonus;
        CurrentScores[CurrentPlayer] += ScoreBonus;
      } else if (BonusXState==4) {
        int ScoreBonus = 4000 * SuperBonus;
        CurrentScores[CurrentPlayer] += ScoreBonus;
      } else if (BonusXState==5) {
        int ScoreBonus = 5000 * SuperBonus;
        CurrentScores[CurrentPlayer] += ScoreBonus;
      }
      SuperBonus = 0;
      ShowSuperBonusOnLadder(SuperBonus);
      SuperBonusReady = false;
      SuperBonusCollecting = false;
      PFValidated = false;
    }
  }


  // LAMP BLIP
  if (CurrentTime>=InlaneLeftBlip && (DTarget4Light[1]==0 || DTarget4Light[1]==1)) {
    BSOS_SetLampState(LA_INLANE_LEFT, 0);
  }
  if (CurrentTime>=InlaneRightBlip && (DTarget4Light[2]==0 || DTarget4Light[2]==1)) {
    BSOS_SetLampState(LA_INLANE_RIGHT, 0);
  }
  if (CurrentTime>=TargetUpperBlip && TopSpecialLit==false && MiniBonusReady==false) {
    BSOS_SetLampState(LA_TARGET_UPPER_SPECIAL, 0);
    BSOS_SetLampState(LA_TARGET_UPPER_COLLECT_BONUS, 0);
  }
  if (CurrentTime>=TargetLowerTopBlip && (DTarget4Light[0]==0 || DTarget4Light[0]==1)) {
    BSOS_SetLampState(LA_TARGET_LRIGHT_TOP, 0);
  }
  if (CurrentTime>=TargetLowerBottomBlip && (DTarget4Light[3]==0 || DTarget4Light[3]==1)) {
    BSOS_SetLampState(LA_TARGET_LRIGHT_BOTTOM, 0);
  }

  // PLAYFIELD X RUNNING
  // if ((Playfield2xState==2 && CurrentTime<Playfield2XStart) || (Playfield3xState==2 && CurrentTime<Playfield3XStart)) {
  //   BSOS_PlaySoundSquawkAndTalk(10);
  // }

  // PLAYFIELD X STOP
  if ((Playfield2xState==2 && CurrentTime>=Playfield2XStart) || (Playfield3xState==2 && CurrentTime>=Playfield3XStart)) {
    BSOS_PlaySoundSquawkAndTalk(5);
    // BSOS_PlaySoundSquawkAndTalk(11); // background sound
    if (Playfield2xState==2 && CurrentTime>=Playfield2XStart) {
      Playfield2xState = 3;
      BSOS_SetLampState(LA_CLOCK_15_SECONDS_2X, 0);
    }
    if (Playfield3xState==2 && CurrentTime>=Playfield3XStart) {
      Playfield3xState = 3;
      BSOS_SetLampState(LA_CLOCK_15_SECONDS_3X, 0);
    }
  }

  // HURRY UP
  if (DTarget4Light[0]==2 && CurrentTime>=WhiteHurryUpStart) {
    BSOS_PlaySoundSquawkAndTalk(17);
    BSOS_SetLampState(LA_DTARGET_4_A, 0);
    BSOS_SetLampState(LA_TARGET_LRIGHT_TOP, 0);
    DTarget4Light[0] = 0;
  }
  if (DTarget4Light[3]==2 && CurrentTime>=AmberHurryUpStart) {
    BSOS_PlaySoundSquawkAndTalk(17);
    BSOS_SetLampState(LA_DTARGET_4_D, 0);
    BSOS_SetLampState(LA_TARGET_LRIGHT_BOTTOM, 0);
    DTarget4Light[3] = 0;
  }


  // HANDLE SAUCER
  if (BallInSaucer==true) {
    // if (Playfield2xState==1 || Playfield3xState==1) BSOS_PlaySoundSquawkAndTalk(40);
    if (CurrentTime<=SaucerAnimation) {
      backglassLampsCenterOut();
      if ((CurrentTime-AttractSweepTime)>25) { // 50 (15 count right now)
        AttractSweepTime = CurrentTime;
        if (SaucerDirection == 0) {
          for (byte lightcountdown=0; lightcountdown<NUM_OF_SAUCER_LAMPS_DOWN; lightcountdown++) {
            byte dist = SaucerLamps - SaucerLampsDown[lightcountdown].rowSaucerDown;
            BSOS_SetLampState(SaucerLampsDown[lightcountdown].lightNumSaucerDown, (dist<8), (dist==0/*||dist>5*/)?0:dist/3, (dist>5)?(100+SaucerLampsDown[lightcountdown].lightNumSaucerDown):0);
            if (lightcountdown==(NUM_OF_SAUCER_LAMPS_DOWN/2)) BSOS_DataRead(0);
          }
        } else {
          for (byte lightcountup=0; lightcountup<NUM_OF_SAUCER_LAMPS_UP; lightcountup++) {
            byte dist = SaucerLamps - SaucerLampsUp[lightcountup].rowSaucerUp;
            BSOS_SetLampState(SaucerLampsUp[lightcountup].lightNumSaucerUp, (dist<8), (dist==0/*||dist>5*/)?0:dist/3, (dist>5)?(100+SaucerLampsUp[lightcountup].lightNumSaucerUp):0);
            if (lightcountup==(NUM_OF_SAUCER_LAMPS_UP/2)) BSOS_DataRead(0);
          }
        }
        SaucerLamps += 1;
        if (SaucerLamps>30) SaucerLamps = 0;
      }
    } else {
      if (Playfield2xState==1) {
        Playfield2xState = 2;
        Playfield2XStart = CurrentTime + PlayfieldXTimer;
        BSOS_SetLampState(LA_SAUCER_ARROW_2X, 0);
        BSOS_SetLampState(LA_CLOCK_15_SECONDS_2X, 1, 0, 100);
      }
      if (Playfield3xState==1) {
        Playfield3xState = 2;
        Playfield3XStart = CurrentTime + PlayfieldXTimer;
        BSOS_SetLampState(LA_SAUCER_ARROW_3X, 0);
        BSOS_SetLampState(LA_CLOCK_15_SECONDS_3X, 1, 0, 100);
      }
      if (SaucerDirection == 0) {
        BSOS_PushToTimedSolenoidStack(SO_SAUCER_DOWN, 5, CurrentTime);
        SaucerDirection = 1;
      } else {
        BSOS_PushToTimedSolenoidStack(SO_SAUCER_UP, 5, CurrentTime);
        SaucerDirection = 0;
      }
      SaucerLamps = 1;
      BallInSaucer = false;
      for (byte off=LA_BONUS_MINI_1K; off<=LA_BONUS_MINI_10K; off++) BSOS_SetLampState(off, 0);
      for (byte off=LA_BONUS_SUPER_1K; off<=LA_BONUS_5X; off++) BSOS_SetLampState(off, 0);
      for (byte off=LA_SAUCER_10K; off<=LA_SAUCER_XBALL; off++) BSOS_SetLampState(off, 0);
      BSOS_SetLampState(LA_SAUCER_30K, 0);
      ShowMiniBonusOnLadder(MiniBonus);
      ShowSuperBonusOnLadder(SuperBonus);
      ShowExtraBonusLights();
    }
  }


  // If the playfield hasn't been validated yet, flash score and player up num
  BSOS_SetDisplay(CurrentPlayer, CurrentScores[CurrentPlayer], true, 2);
  // if (BallFirstSwitchHitTime==0) {
  //   // BSOS_SetDisplayFlash(CurrentPlayer, CurrentScores[CurrentPlayer], CurrentTime, 500, 2);
  //   BSOS_SetDisplay(CurrentPlayer, CurrentScores[CurrentPlayer], true, 2);
  //   // if (!PlayerUpLightBlinking) {
  //   //   PlayerUpLightBlinking = true;
  //   // }
  // } else {
  //   // if (PlayerUpLightBlinking) {
  //   //   PlayerUpLightBlinking = false;
  //   // }
  // }

  // Check to see if ball is in the outhole
  if (BSOS_ReadSingleSwitchState(SW_OUTHOLE)) {
    if (BallTimeInTrough==0) {
      BallTimeInTrough = CurrentTime;
    } else {
      // Make sure the ball stays on the sensor for at least 
      // 0.5 seconds to be sure that it's not bouncing
      if ((CurrentTime-BallTimeInTrough)>500) {

        if (BallFirstSwitchHitTime==0) BallFirstSwitchHitTime = CurrentTime;
        
        // if we haven't used the ball save, and we're under the time limit, then save the ball
        if (  !BallSaveUsed && 
              ((CurrentTime-BallFirstSwitchHitTime)/1000)<((unsigned long)BallSaveNumSeconds) ) {
        
          BSOS_PushToTimedSolenoidStack(SO_OUTHOLE, 4, CurrentTime + 100);
          if (BallFirstSwitchHitTime>0) {
            BallSaveUsed = true;
            BSOS_SetLampState(LA_SAME_PLAYER_SHOOTS_AGAIN, 0);
//            BSOS_SetLampState(HEAD_SAME_PLAYER, 0);
          }
          BallTimeInTrough = CurrentTime;

          returnState = MACHINE_STATE_NORMAL_GAMEPLAY;
        } else if (NumTiltWarnings>=MaxTiltWarnings) {
          returnState = MACHINE_STATE_BALL_OVER;
        } else if (NumTiltWarnings<MaxTiltWarnings) {
          returnState = MACHINE_STATE_COUNTDOWN_BONUS;
        }
      }
    }
  } else {
    BallTimeInTrough = 0;
  }

  return returnState;
}


unsigned long InitGamePlayAnimation = 0;
int InitGamePlay(boolean curStateChanged) {
  int returnState = MACHINE_STATE_INIT_GAMEPLAY;

  if (curStateChanged) {
    InitGameStartTime = CurrentTime;
    BSOS_SetCoinLockout((Credits>=MaxCredits)?true:false);
    BSOS_SetDisableFlippers(true);
    BSOS_DisableSolenoidStack();
    BSOS_TurnOffAllLamps();
    BSOS_SetDisplayBallInPlay(1);
    InitGamePlayAnimation = CurrentTime+1500;

    // Set up general game variables
    CurrentNumPlayers = 1;
    CurrentPlayer = 0;
    CurrentBallInPlay = 1;
    PlayerShootsAgain = false;
    for (byte i = 0; i < 3; i++) WoodBeastXBallState[i] = 0;
    for (byte i = 0; i < 3; i++) SacuerXBallState[i] = 0;
    for (byte count=0; count<4; count++) CurrentScores[count] = 0;

    // if the ball is in the outhole, then we can move on
    if (BSOS_ReadSingleSwitchState(SW_OUTHOLE)) {
      if (DEBUG_MESSAGES) {
        Serial.println(F("Ball is in trough - starting new ball"));
      }
      BSOS_EnableSolenoidStack();
      BSOS_SetDisableFlippers(false);
      returnState = MACHINE_STATE_INIT_NEW_BALL;
    } else {

      if (DEBUG_MESSAGES) {
        Serial.println(F("Ball is not in trough - firing stuff and giving it a chance to come back"));
      }

      // Otherwise, let's see if it's in a spot where it could get trapped,
      // for instance, a saucer (if the game has one)
      // BSOS_PushToSolenoidStack(SOL_SAUCER, 5, true);
      if (NIGHT_TESTING) {
        BSOS_PushToSolenoidStack(SO_SAUCER_DOWN, 5, true);
      }

      // And then set a base time for when we can continue
      InitGameStartTime = CurrentTime;
    }

    for (byte count=0; count<4; count++) {
      BSOS_SetDisplay(count, 0);
      BSOS_SetDisplayBlank(count, 0x00);
    }
  }

  BackglassLampsLeft2Right();

  if (CurrentTime>=InitGamePlayAnimation) {
    // Wait for TimeToWaitForBall seconds, or until the ball appears
    // The reason to bail out after TimeToWaitForBall is just
    // in case the ball is already in the shooter lane.
    if ((CurrentTime-InitGameStartTime)>TimeToWaitForBall || BSOS_ReadSingleSwitchState(SW_OUTHOLE)) {
      BSOS_EnableSolenoidStack();
      BSOS_SetDisableFlippers(false);
      returnState = MACHINE_STATE_INIT_NEW_BALL;
    }
  } else {
    AttractRetro();
  }

  return returnState;
}


int RunGamePlayMode(int curState, boolean curStateChanged) {
  int returnState = curState;
  byte miniBonusAtTop = MiniBonus;
  byte superBonusAtTop = SuperBonus;
  unsigned long scoreAtTop = CurrentScores[CurrentPlayer];

  // Very first time into gameplay loop
  if (curState==MACHINE_STATE_INIT_GAMEPLAY) {
    returnState = InitGamePlay(curStateChanged);
  } else if (curState==MACHINE_STATE_INIT_NEW_BALL) {
    returnState = InitNewBall(curStateChanged, CurrentBallInPlay);
  } else if (curState==MACHINE_STATE_SKILL_SHOT) {
    returnState = SkillShot(curStateChanged);
  } else if (curState==MACHINE_STATE_NORMAL_GAMEPLAY) {
    returnState = NormalGamePlay(curStateChanged);
  } else if (curState==MACHINE_STATE_COUNTDOWN_BONUS) {
    returnState = CountdownBonus(curStateChanged);
  } else if (curState==MACHINE_STATE_WIZARD_MODE) {
    returnState = WizardMode(curStateChanged);
  } else if (curState==MACHINE_STATE_BALL_OVER) {    
    if (PlayerShootsAgain) {
      returnState = MACHINE_STATE_INIT_NEW_BALL;
    } else {
      CurrentPlayer+=1;
      if (CurrentPlayer>=CurrentNumPlayers) {
        CurrentPlayer = 0;
        CurrentBallInPlay+=1;
      }
        
      if (CurrentBallInPlay>BallsPerGame) {
        // CheckHighScores();
        for (byte count=0; count<CurrentNumPlayers; count++) {
          BSOS_SetDisplay(count, CurrentScores[count], true, 2);
        }

        returnState = MACHINE_STATE_MATCH_MODE;
      }
      else returnState = MACHINE_STATE_INIT_NEW_BALL;
    }
  } else if (curState==MACHINE_STATE_MATCH_MODE) {
    returnState = ShowMatchSequence(curStateChanged);    
  }

  byte switchHit;

  if (NumTiltWarnings<MaxTiltWarnings) {
    while ( (switchHit=BSOS_PullFirstFromSwitchStack())!=SWITCH_STACK_EMPTY ) {

      switch (switchHit) {
        case SW_SELF_TEST_SWITCH:
          returnState = MACHINE_STATE_TEST_LIGHTS;
          SetLastSelfTestChangedTime(CurrentTime);
          break;
        case SW_STARS_PFIELD:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (PFValidated==false) {
              if (NIGHT_TESTING) BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
              PFValidated = true;
            }
            BSOS_PlaySoundSquawkAndTalk(13);
            AddToScore(1000);
            AddToMiniBonus(1);
            if (curState==MACHINE_STATE_SKILL_SHOT) {
              SkillShotState = 3;
            }
          } else {
            BSOS_PlaySoundSquawkAndTalk(24);
            if (WizardState==3) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 4;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
        case SW_STARS_SHOOTER_LANE:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (curState==MACHINE_STATE_NORMAL_GAMEPLAY) {
              if (SuperBonusReady==true) {
                BSOS_PlaySoundSquawkAndTalk(13);
                AddToScore(1000);
              }
            } else if (curState==MACHINE_STATE_SKILL_SHOT) {
              if (SkillShotState==0) {
                SkillShotState = 1;
                BSOS_SetLampState(LA_STAR_SHOOTER_TOP, 0);
                BSOS_SetLampState(LA_STAR_SHOOTER_MIDDLE, 0);
                BSOS_SetLampState(LA_STAR_SHOOTER_BOTTOM, 0);
                if (PlayerShootsAgain==true) {
                  PlayerShootsAgain = false;
                  BSOS_SetLampState(LA_SAME_PLAYER_SHOOTS_AGAIN, 0);
                  BSOS_SetLampState(LA_SHOOT_AGAIN, 0);
                }
              }
              BSOS_PlaySoundSquawkAndTalk(13);
              SkillShotHits++;
              if (SkillShotHits>=5) SkillShotHits = 5;
            }
          }
          break;
        case SW_DTARGET_1:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (PFValidated==true) {
              SuperBonusReady = true;
              AddToScore(10000);
              BSOS_PlaySoundSquawkAndTalk(29);
              DropTargetHit();
            }
          } else  {
            BSOS_PlaySoundSquawkAndTalk(24);
            if (WizardState==4) {
              // BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
        case SW_SHOOTER_LANE_ROLL:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (curState==MACHINE_STATE_NORMAL_GAMEPLAY) {
              BSOS_PlaySoundSquawkAndTalk(18);
              if (SuperBonusReady!=true) {
                if (PFValidated==true) {
                  if (NIGHT_TESTING) BSOS_PushToSolenoidStack(SO_DTARGET_1_DOWN, 5, true);
                  PFValidated = false;
                }
              } else {
                SuperBonusCollecting = true;
                SuperBonusCollectTimer = CurrentTime + 600;
              }
            } else if (curState==MACHINE_STATE_SKILL_SHOT) {
              if (SkillShotState==0) BSOS_PlaySoundSquawkAndTalk(12);
              if (SkillShotState==1) {
                BSOS_PlaySoundSquawkAndTalk(42);
                SkillShotScoreAnimation = CurrentTime+500;
                SkillShotState = 2;
                if (SkillShotHits==1 || SkillShotHits==2) {
                  CurrentScores[CurrentPlayer] += 15000; // 10000
                } else if (SkillShotHits==3 || SkillShotHits==4) {
                  CurrentScores[CurrentPlayer] += 30000; // 25000
                } else if (SkillShotHits==5) {
                  CurrentScores[CurrentPlayer] += 75000; // 50000
                }
              }
            }
          } else {
            BSOS_PlaySoundSquawkAndTalk(50);
          }
          break;
        case SW_DTARGET_REBOUND:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (PFValidated==false) {
              if (NIGHT_TESTING) BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
              PFValidated = true;
            }
            BSOS_PlaySoundSquawkAndTalk(7);
            AddToScore(50);
            if (curState==MACHINE_STATE_SKILL_SHOT) {
              SkillShotState = 3;
              BSOS_PlaySoundSquawkAndTalk(13);
              // PFValidated = true;
            }
          } else {
            BSOS_PlaySoundSquawkAndTalk(24);
            if (WizardState==3) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 4;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
        case SW_CREDIT_BUTTON:
          if (DEBUG_MESSAGES) {
            Serial.println(F("Start game button pressed"));
          }
          if (CurrentBallInPlay<2) { // single player
            // If we haven't finished the first ball, we can add players
            AddPlayer();
          } else {
            // If the first ball is over, pressing start again resets the game
            returnState = MACHINE_STATE_INIT_GAMEPLAY;
          }
          break;
        case SW_TILT:
        case SW_SLAM:
          TiltHit();
          break;
        case SW_COIN_1:
        case SW_COIN_2:
          AddCredit(1);
          if (!FreePlay) {
            BSOS_SetDisplayCredits(Credits, true);
          }
          break;
        case SW_COIN_3:
          AddCredit(2);
          if (!FreePlay) {
            BSOS_SetDisplayCredits(Credits, true);
          }
          break;
        case SW_TARGET_LRIGHT_BOTTOM:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            // HURRY UP
            if (DTarget4Light[3]==0 || DTarget4Light[3]==1) {
              BSOS_PlaySoundSquawkAndTalk(26);
              AddToScore(5000);
              TargetLowerBottomBlip = CurrentTime + 100;
              BSOS_SetLampState(LA_TARGET_LRIGHT_BOTTOM, 1);
            } else if (DTarget4Light[3]==2) {
              BSOS_PlaySoundSquawkAndTalk(45);
              AddToScore(50000);
              AmberHurryUpStart = 0;
              DTarget4Light[3] = 3;
              BSOS_SetLampState(LA_DTARGET_4_D, 1);
              BSOS_SetLampState(LA_TARGET_LRIGHT_BOTTOM, 1);
              CheckHurryUpCompletion();
            } else if (DTarget4Light[3]==3) {
              BSOS_PlaySoundSquawkAndTalk(27);
              AddToScore(10000);
            }
            AddToSuperBonus(1);
          } else  {
            BSOS_PlaySoundSquawkAndTalk(24);
          }
          break;
        case SW_INLANE_R:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            // HURRY UP
            if (DTarget4Light[2]==0 || DTarget4Light[2]==1) {
              BSOS_PlaySoundSquawkAndTalk(26);
              AddToScore(5000);
              InlaneRightBlip = CurrentTime + 100;
              BSOS_SetLampState(LA_INLANE_RIGHT, 1);
            } else if (DTarget4Light[2]==2) {
              BSOS_PlaySoundSquawkAndTalk(27);
              AddToScore(25000);
              DTarget4Light[2] = 3;
              BSOS_SetLampState(LA_DTARGET_4_C, 1);
              BSOS_SetLampState(LA_INLANE_RIGHT, 1);
              CheckHurryUpCompletion();
            } else if (DTarget4Light[2]==3) {
              BSOS_PlaySoundSquawkAndTalk(27);
              AddToScore(10000);
            }
          } else {
            BSOS_PlaySoundSquawkAndTalk(24);
          }
          break;
        case SW_INLANE_L:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            // HURRY UP
            if (DTarget4Light[1]==0 || DTarget4Light[1]==1) {
              BSOS_PlaySoundSquawkAndTalk(26);
              AddToScore(5000);
              InlaneLeftBlip = CurrentTime + 100;
              BSOS_SetLampState(LA_INLANE_LEFT, 1);
            } else if (DTarget4Light[1]==2) {
              BSOS_PlaySoundSquawkAndTalk(27);
              AddToScore(25000);
              DTarget4Light[1] = 3;
              BSOS_SetLampState(LA_DTARGET_4_B, 1);
              BSOS_SetLampState(LA_INLANE_LEFT, 1);
              CheckHurryUpCompletion();
            } else if (DTarget4Light[1]==3) {
              BSOS_PlaySoundSquawkAndTalk(27);
              AddToScore(10000);
            }
          } else {
            BSOS_PlaySoundSquawkAndTalk(24);
          }
          break;
        case SW_TARGET_LRIGHT_TOP:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            // HURRY UP
            if (DTarget4Light[0]==0 || DTarget4Light[0]==1) {
              BSOS_PlaySoundSquawkAndTalk(26);
              AddToScore(5000);
              TargetLowerTopBlip = CurrentTime + 100;
              BSOS_SetLampState(LA_TARGET_LRIGHT_TOP, 1);
            } else if (DTarget4Light[0]==2) {
              BSOS_PlaySoundSquawkAndTalk(45);
              AddToScore(50000);
              WhiteHurryUpStart = 0;
              DTarget4Light[0] = 3;
              BSOS_SetLampState(LA_DTARGET_4_A, 1);
              BSOS_SetLampState(LA_TARGET_LRIGHT_TOP, 1);
              CheckHurryUpCompletion();
            } else if (DTarget4Light[0]==3) {
              BSOS_PlaySoundSquawkAndTalk(27);
              AddToScore(10000);
            }
            AddToSuperBonus(1);
          } else {
            BSOS_PlaySoundSquawkAndTalk(24);
          }
          break;
        case SW_DTARGET_4_A:
        case SW_DTARGET_4_B:
        case SW_DTARGET_4_C:
        case SW_DTARGET_4_D:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            AddToSuperBonus(1);
            DropTargetHit();
            if (switchHit==SW_DTARGET_4_A) {
              if (DTarget4Light[0]==0) {
                BSOS_PlaySoundSquawkAndTalk(24);
                AddToScore(5000);
              } else if (DTarget4Light[0]==1) {
                WhiteHurryUpStart = CurrentTime + DTarget4HurryUpTimer;
                BSOS_SetLampState(LA_TARGET_LRIGHT_TOP, 1, 0, 100);
                DTarget4Light[0] = 2;
                BSOS_PlaySoundSquawkAndTalk(25);
                AddToScore(10000);
              } else if (DTarget4Light[0]==2 || DTarget4Light[0]==3) {
                BSOS_PlaySoundSquawkAndTalk(25);
                AddToScore(10000);
              }
            }
            if (switchHit==SW_DTARGET_4_B) {
              if (DTarget4Light[1]==0) {
                BSOS_PlaySoundSquawkAndTalk(24);
                AddToScore(5000);
              } else if (DTarget4Light[1]==1) {
                BSOS_SetLampState(LA_INLANE_LEFT, 1, 0, 250);
                DTarget4Light[1] = 2;
                BSOS_PlaySoundSquawkAndTalk(25);
                AddToScore(10000);
              } else if (DTarget4Light[1]==2 || DTarget4Light[1]==3) {
                BSOS_PlaySoundSquawkAndTalk(25);
                AddToScore(10000);
              }
            }
            if (switchHit==SW_DTARGET_4_C) {
              if (DTarget4Light[2]==0) {
                BSOS_PlaySoundSquawkAndTalk(24);
                AddToScore(5000);
              } else if (DTarget4Light[2]==1) {
                BSOS_SetLampState(LA_INLANE_RIGHT, 1, 0, 250);
                DTarget4Light[2] = 2;
                BSOS_PlaySoundSquawkAndTalk(25);
                AddToScore(10000);
              } else if (DTarget4Light[2]==2 || DTarget4Light[2]==3) {
                BSOS_PlaySoundSquawkAndTalk(25);
                AddToScore(10000);
              }
            }
            if (switchHit==SW_DTARGET_4_D) {
              if (DTarget4Light[3]==0) {
                BSOS_PlaySoundSquawkAndTalk(24);
                AddToScore(5000);
              } else if (DTarget4Light[3]==1) {
                AmberHurryUpStart = CurrentTime + DTarget4HurryUpTimer;
                BSOS_SetLampState(LA_TARGET_LRIGHT_BOTTOM, 1, 0, 100);
                DTarget4Light[3] = 2;
                BSOS_PlaySoundSquawkAndTalk(25);
                AddToScore(10000);
              } else if (DTarget4Light[3]==2 || DTarget4Light[3]==3) {
                BSOS_PlaySoundSquawkAndTalk(25);
                AddToScore(10000);
              }
            }
            if (CheckIfDTargets4Down()) {
              AddToScore(15000);
              if (LeftSpinnerLit==false) {
                LeftSpinnerLit = true;
                BSOS_SetLampState(LA_SPINNER_LEFT, 1);
              } else {
                RightSpinnerLit = true;
                BSOS_SetLampState(LA_SPINNER_RIGHT, 1);
              }
              DTarget4Goal = true;
              BSOS_SetLampState(LA_SAUCER_10K, 1);
              CheckSaucerDTargetGoal();
              if (BonusXState==4) {
                BonusXState = 5;
                ShowExtraBonusLights();
              }
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_4_RESET, 15, CurrentTime + 500);
            }
          } else {
            BSOS_PlaySoundSquawkAndTalk(24);
            // BSOS_PushToTimedSolenoidStack(SO_DTARGET_4_RESET, 15, CurrentTime);
          }
          break;
        case SW_DTARGET_3_A:
        case SW_DTARGET_3_B:
        case SW_DTARGET_3_C:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (PFValidated==false) {
              if (NIGHT_TESTING) BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
              PFValidated = true;
            }
            AddToMiniBonus(1);
            DropTargetHit();
            if (switchHit==SW_DTARGET_3_A) {
              if (DTarget3Lights[0]==0 && DTarget3Flashing==LA_DTARGET_ARROW_1) {
                BSOS_SetLampState(LA_DTARGET_ARROW_1, 1);
                DTarget3Lights[0] = 1;
                BSOS_PlaySoundSquawkAndTalk(24);
                AddToScore(10000);
              } else if (DTarget3Lights[0]==1) {
                BSOS_PlaySoundSquawkAndTalk(25);
                AddToScore(10000);
              } else {
                BSOS_PlaySoundSquawkAndTalk(25);
                AddToScore(5000);
              }
            }
            if (switchHit==SW_DTARGET_3_B) {
              if (DTarget3Lights[1]==0 && DTarget3Flashing==LA_DTARGET_ARROW_2) {
                BSOS_SetLampState(LA_DTARGET_ARROW_2, 1);
                DTarget3Lights[1] = 1;
                BSOS_PlaySoundSquawkAndTalk(24);
                AddToScore(10000);
              } else if (DTarget3Lights[1]==1) {
                BSOS_PlaySoundSquawkAndTalk(25);
                AddToScore(10000);
              } else {
                BSOS_PlaySoundSquawkAndTalk(25);
                AddToScore(5000);
              }
            }
            if (switchHit==SW_DTARGET_3_C) {
              if (DTarget3Lights[2]==0 && DTarget3Flashing==LA_DTARGET_ARROW_3) {
                BSOS_SetLampState(LA_DTARGET_ARROW_3, 1);
                DTarget3Lights[2] = 1;
                BSOS_PlaySoundSquawkAndTalk(24);
                AddToScore(10000);
              } else if (DTarget3Lights[2]==1) {
                BSOS_PlaySoundSquawkAndTalk(25);
                AddToScore(10000);
              } else {
                BSOS_PlaySoundSquawkAndTalk(25);
                AddToScore(5000);
              }
            }
            if (CheckIfDTargets3Down()) {
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_3_RESET, 15, CurrentTime + 500);
              DTarget3Completions++;
              if (DTarget3Completions==1) {
                BSOS_SetLampState(LA_POP_TOP, 1);
                TopPopLit = true;
              } else if (DTarget3Completions==2) {
                BSOS_SetLampState(LA_TARGET_UPPER_COLLECT_BONUS, 1);
                MiniBonusReady = true;
              } else if (DTarget3Completions==3) {
                BSOS_SetLampState(LA_TARGET_UPPER_SPECIAL, 1);
                TopSpecialLit = true;
              }
              AddToScore(15000);
              BSOS_SetLampState(LA_SAUCER_30K, 1);
              DTarget3Goal = true;
              CheckSaucerDTargetGoal();
              if (BonusXState==3) {
                BonusXState = 4;
                ShowExtraBonusLights();
              }
            }
            if (DTarget3Lights[0]==1 && DTarget3Lights[1]==1 && DTarget3Lights[2]==1 && Playfield2xState==0) {
              Mini50kCollected = true;
              Playfield2xState = 1;
              BSOS_PlaySoundSquawkAndTalk(42);
              BSOS_SetLampState(LA_SAUCER_ARROW_2X, 1, 0, 150);
              BSOS_SetLampState(LA_BONUS_MINI_50K, 1);
            }
            if (curState==MACHINE_STATE_SKILL_SHOT) {
              SkillShotState = 3;
              BSOS_PlaySoundSquawkAndTalk(13);
              // PFValidated = true;
            }
          } else  {
            BSOS_PlaySoundSquawkAndTalk(24);
            // BSOS_PushToTimedSolenoidStack(SO_DTARGET_3_RESET, 15, CurrentTime);
            if (WizardState==3) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 4;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
        case SW_TARGET_TOP:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (PFValidated==false) {
              if (NIGHT_TESTING) BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
              PFValidated = true;
            }
            AddToScore(5000);
            AddToMiniBonus(1);
            if (MiniBonusReady==false && TopSpecialLit==false) {
              TargetUpperBlip = CurrentTime + 100;
              BSOS_SetLampState(LA_TARGET_UPPER_SPECIAL, 1);
              BSOS_SetLampState(LA_TARGET_UPPER_COLLECT_BONUS, 1);
              BSOS_PlaySoundSquawkAndTalk(27);
            }
            if (MiniBonusReady==true) {
              BSOS_SetLampState(LA_TARGET_UPPER_COLLECT_BONUS, 0);
              MiniBonusCollecting = true;
              MiniBonusCollectTimer = CurrentTime + 600;
            }
            if (TopSpecialLit==true) {
              AddToScore(50000);
              BSOS_SetLampState(LA_TARGET_UPPER_SPECIAL, 0);
              AddCredit(1);
              if (NIGHT_TESTING) BSOS_PushToSolenoidStack(SO_KNOCKER, 5, true);
              TopSpecialLit = false;
              BSOS_PlaySoundSquawkAndTalk(27);
            }
            if (curState==MACHINE_STATE_SKILL_SHOT) {
              SkillShotState = 3;
              BSOS_PlaySoundSquawkAndTalk(13);
              // PFValidated = true;
            }
          } else  {
            BSOS_PlaySoundSquawkAndTalk(24);
            if (WizardState==3) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 4;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
        case SW_DTARGET_INLINE_1ST:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            AddToScore(10000);
            AddToSuperBonus(2);
            DropTargetHit();
            BSOS_PlaySoundSquawkAndTalk(25);
          } else  {
            // BSOS_PushToTimedSolenoidStack(SO_DTARGET_INLINE_RESET, 15, CurrentTime);
          }
          break;
        case SW_DTARGET_INLINE_2ND:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            AddToScore(10000);
            DropTargetHit();
            BSOS_PlaySoundSquawkAndTalk(25);
            if (BonusXState==1) {
              BonusXState = 2;
              ShowExtraBonusLights();
            }
          }
          break;
        case SW_DTARGET_INLINE_3RD:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            AddToScore(10000);
            DTargetInlineGoal = true;
            // RightSpinnerLit = true;
            // BSOS_SetLampState(LA_SPINNER_RIGHT, 1);
            BSOS_SetLampState(LA_SAUCER_20K, 1);
            CheckSaucerDTargetGoal();
            BSOS_PlaySoundSquawkAndTalk(25);
            if (WoodBeastXBallState[CurrentPlayer]==0) {
              WoodBeastXBallState[CurrentPlayer] = 1;
              BSOS_SetLampState(LA_TARGET_WOOD_BEAST_XBALL, 1);
            }
            DropTargetHit();
            if (BonusXState==2) {
              BonusXState = 3;
              ShowExtraBonusLights();
            }
          }
          break;
        case SW_TARGET_WOOD_BEAST:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (WoodBeastXBallState[CurrentPlayer]==1) {
              WoodBeastXBallState[CurrentPlayer] = 2;
              PlayerShootsAgain = true;
              BSOS_SetLampState(LA_TARGET_WOOD_BEAST_XBALL, 0);
              BSOS_SetLampState(LA_SHOOT_AGAIN, 1);
              BSOS_SetLampState(LA_SAME_PLAYER_SHOOTS_AGAIN, 1);
            }
            BSOS_PushToTimedSolenoidStack(SO_DTARGET_INLINE_RESET, 15, CurrentTime + 750);
            BSOS_PlaySoundSquawkAndTalk(27);
            AddToScore(25000);
          } else {
            BSOS_PlaySoundSquawkAndTalk(24);
          }
          break;
        case SW_REBOUND:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (PFValidated==false) {
              if (NIGHT_TESTING) BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
              PFValidated = true;
            }
            BSOS_PlaySoundSquawkAndTalk(7);
            AddToScore(10);
            if (curState==MACHINE_STATE_SKILL_SHOT) {
              SkillShotState = 3;
              BSOS_PlaySoundSquawkAndTalk(13);
              // PFValidated = true;
            }
          } else {
            BSOS_PlaySoundSquawkAndTalk(24);
            if (WizardState==3) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 4;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
        case SW_SAUCER:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (PFValidated==false) {
              if (NIGHT_TESTING) BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
              PFValidated = true;
            }
            if (SaucerHitTime==0 || (CurrentTime-SaucerHitTime)>500) {
              SaucerHitTime = CurrentTime;
              SaucerAnimation = CurrentTime+1650;
              if (DTarget4Goal==true) AddToScore(10000);
              if (DTargetInlineGoal==true) AddToScore(20000);
              if (DTarget3Goal==true) AddToScore(30000);
              if (SacuerXBallState[CurrentPlayer]==1) {
                SacuerXBallState[CurrentPlayer] = 2;
                PlayerShootsAgain = true;
                BSOS_SetLampState(LA_SAUCER_XBALL, 0);
                BSOS_SetLampState(LA_SHOOT_AGAIN, 1);
                BSOS_SetLampState(LA_SAME_PLAYER_SHOOTS_AGAIN, 1);
              }
              BallInSaucer = true;
            }
            BSOS_PlaySoundSquawkAndTalk(22);
            if (curState==MACHINE_STATE_SKILL_SHOT) {
              SkillShotState = 3;
              BSOS_PlaySoundSquawkAndTalk(13);
              // PFValidated = true;
            }
          } else {
            if (WizardState==3) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 4;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
            if (SaucerHitTime==0 || (CurrentTime-SaucerHitTime)>500) {
              SaucerHitTime = CurrentTime;
              if (MingAttackReady==false) {
                BSOS_PlaySoundSquawkAndTalk(17);
                AddToScore(3500);
                MingAttackProgress += 35;

                if (MingAttackProgress>=140) {
                  MingAttackProgress = 140;
                  MingAttackReady = true;
                }
              }
              if (MingAttackReady==true) {
                MingHealth--;
                if (MingHealth==0) {
                  BSOS_PlaySoundSquawkAndTalk(43);
                  BSOS_PushToTimedSolenoidStack(SO_SAUCER_DOWN, 5, CurrentTime+1600);
                  BSOS_PushToTimedSolenoidStack(SO_KNOCKER, 5, CurrentTime+1600);
                  MingAttackAnimation = CurrentTime+2475;
                } else {
                  BSOS_PlaySoundSquawkAndTalk(42);
                  MingAttackAnimation = CurrentTime+1650;
                }
              }
              BallInSaucer = true;
            }
          }
          break;
        case SW_OUTLANE_RIGHT:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            AddToScore(2000);
            AddToSuperBonus(2);
            if (WizardState==0 || WizardState==4) BSOS_PlaySoundSquawkAndTalk(28);
            if (WizardState==1) {
              WizardState = 2;
              BSOS_PlaySoundSquawkAndTalk(50);
            }
          }
          break;
        case SW_OUTLANE_LEFT:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            AddToScore(2000);
            AddToSuperBonus(2);
            if (WizardState==0 || WizardState==4) BSOS_PlaySoundSquawkAndTalk(28);
            if (WizardState==1) {
              WizardState = 2;
              BSOS_PlaySoundSquawkAndTalk(50);
            }
          }
          break;
        case SW_SPINNER_RIGHT:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            // if (PFValidated==false) {
            //   if (NIGHT_TESTING) BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
            //   PFValidated = true;
            // }
            if (RightSpinnerLit==false) {
              BSOS_PlaySoundSquawkAndTalk(8);
              AddToScore(100);
            } else {
              BSOS_PlaySoundSquawkAndTalk(9);
              AddToScore(1000);
            }
            if (curState==MACHINE_STATE_SKILL_SHOT) {
              SkillShotState = 3;
              BSOS_PlaySoundSquawkAndTalk(13);
              // PFValidated = true;
            }
          } else {
            BSOS_PlaySoundSquawkAndTalk(7);
            AddToScore(1000);
            MingAttackProgress++;
            if (WizardState==3) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 4;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
        case SW_SPINNER_LEFT:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            // if (PFValidated==false) {
            //   if (NIGHT_TESTING) BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
            //   PFValidated = true;
            // }
            if (LeftSpinnerLit==false) {
              BSOS_PlaySoundSquawkAndTalk(8);
              AddToScore(100);
            } else {
              BSOS_PlaySoundSquawkAndTalk(9);
              AddToScore(1000);
            }
            if (curState==MACHINE_STATE_SKILL_SHOT) {
              SkillShotState = 3;
              BSOS_PlaySoundSquawkAndTalk(13);
              // PFValidated = true;
            }
          } else {
            BSOS_PlaySoundSquawkAndTalk(7);
            AddToScore(1000);
            MingAttackProgress++;
            if (WizardState==3) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 4;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
        case SW_SLING_RIGHT:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            AddToScore(50);
            BSOS_PlaySoundSquawkAndTalk(21);
          } else {
            BSOS_PlaySoundSquawkAndTalk(24);
          }
          break;
        case SW_SLING_LEFT:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            AddToScore(50);
            BSOS_PlaySoundSquawkAndTalk(21);
          } else {
            BSOS_PlaySoundSquawkAndTalk(24);
          }
          break;
        case SW_POP_TOP:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (PFValidated==false) {
              if (NIGHT_TESTING) BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
              PFValidated = true;
            }
            if (TopPopLit==false) {
              BSOS_PlaySoundSquawkAndTalk(20);
              AddToScore(1000);
            } else {
              BSOS_PlaySoundSquawkAndTalk(13);
              AddToScore(5000);
            }
            if (curState==MACHINE_STATE_SKILL_SHOT) {
              SkillShotState = 3;
              BSOS_PlaySoundSquawkAndTalk(13);
              // PFValidated = true;
            }
          } else {
            BSOS_PlaySoundSquawkAndTalk(27);
            AddToScore(2000);
            MingAttackProgress += 20;
            if (WizardState==3) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 4;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
        case SW_POP_RIGHT:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            BSOS_PlaySoundSquawkAndTalk(20);
            AddToScore(1000);
          } else  {
            BSOS_PlaySoundSquawkAndTalk(27);
            AddToScore(2000);
            MingAttackProgress += 20;
            if (WizardState==3) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 4;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
        case SW_POP_LEFT:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            BSOS_PlaySoundSquawkAndTalk(20);
            AddToScore(1000);
          } else  {
            BSOS_PlaySoundSquawkAndTalk(27);
            AddToScore(2000);
            MingAttackProgress += 20;
            if (WizardState==3) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 4;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
      }
      // if (DEBUG_MESSAGES) {
      //   char buf[128];
      //   sprintf(buf, "Switch = 0x%02X\n", switchHit);
      //   Serial.write(buf);
      // }
    }
  } else {
    // We're tilted, so just wait for outhole
    while ( (switchHit=BSOS_PullFirstFromSwitchStack())!=SWITCH_STACK_EMPTY ) {
      switch (switchHit) {
        case SW_SELF_TEST_SWITCH:
          returnState = MACHINE_STATE_TEST_LIGHTS;
          SetLastSelfTestChangedTime(CurrentTime);
          break; 
        case SW_COIN_1:
        case SW_COIN_2:
          AddCredit(1);
          if (!FreePlay) {
            BSOS_SetDisplayCredits(Credits, true);
          }
          break;
        case SW_COIN_3:
          AddCredit(2);
          if (!FreePlay) {
            BSOS_SetDisplayCredits(Credits, true);
          }
          break;
        case SW_SAUCER:
          BSOS_PushToTimedSolenoidStack(SO_SAUCER_DOWN, 5, CurrentTime + 500);
         break;
      }
    }
  }

  if (miniBonusAtTop!=MiniBonus) {
    ShowMiniBonusOnLadder(MiniBonus);
  }

  if (superBonusAtTop!=SuperBonus) {
    ShowSuperBonusOnLadder(SuperBonus);
  }

  if (scoreAtTop!=CurrentScores[CurrentPlayer]) {
    if (DEBUG_MESSAGES) {
      // Serial.println(F("Score changed"));
    }
  }
  return returnState;
}


void loop() {
  // This line has to be in the main loop
  BSOS_DataRead(0);

  CurrentTime = millis();
  int newMachineState = MachineState;

  // Machine state is self-test/attract/game play
  // if (MachineState<0) {
  //   newMachineState = RunSelfTest(MachineState, MachineStateChanged);
  // } else if (MachineState==MACHINE_STATE_ATTRACT) {
  //   newMachineState = RunAttractMode(MachineState, MachineStateChanged);
  // } else {
  //   newMachineState = RunGamePlayMode(MachineState, MachineStateChanged);
  // }
  if (MachineState==MACHINE_STATE_ATTRACT) {
    newMachineState = RunAttractMode(MachineState, MachineStateChanged);
  } else {
    newMachineState = RunGamePlayMode(MachineState, MachineStateChanged);
  }

  if (newMachineState!=MachineState) {
    MachineState = newMachineState;
    MachineStateChanged = true;
  } else {
    MachineStateChanged = false;
  }

  BSOS_ApplyFlashToLamps(CurrentTime);
  BSOS_UpdateTimedSolenoidStack(CurrentTime);
}


void BackglassLampsClear() {
  BSOS_SetLampState(LA_FLASH_GORDON_6, 0);
  BSOS_SetLampState(LA_FLASH_GORDON_1, 0);
  BSOS_SetLampState(LA_FLASH_GORDON_5, 0);
  BSOS_SetLampState(LA_FLASH_GORDON_2, 0);
  BSOS_SetLampState(LA_FLASH_GORDON_4, 0);
  BSOS_SetLampState(LA_FLASH_GORDON_3, 0);
  BSOS_SetLampState(LA_FLASH_STROBE, 0);
}

void backglassLampsCenterOut() {
  byte lampPhase = (CurrentTime/115)%4;
  BSOS_SetLampState(LA_FLASH_GORDON_6, lampPhase==3||lampPhase==0, lampPhase==0);
  BSOS_SetLampState(LA_FLASH_GORDON_1, lampPhase==3||lampPhase==0, lampPhase==0);
  BSOS_SetLampState(LA_FLASH_GORDON_5, lampPhase==2||lampPhase==3, lampPhase==3);
  BSOS_SetLampState(LA_FLASH_GORDON_2, lampPhase==2||lampPhase==3, lampPhase==3);
  BSOS_SetLampState(LA_FLASH_GORDON_4, lampPhase==1||lampPhase==2, lampPhase==2);
  BSOS_SetLampState(LA_FLASH_GORDON_3, lampPhase==1||lampPhase==2, lampPhase==2);
  if (MachineState!=MACHINE_STATE_NORMAL_GAMEPLAY || (MachineState==MACHINE_STATE_NORMAL_GAMEPLAY && BallInSaucer==true)) BSOS_SetLampState(LA_FLASH_STROBE, lampPhase<2, lampPhase==1);
}

void BackglassLampsKnightRider() {
  byte lampPhase = (CurrentTime/80)%14;
  BSOS_SetLampState(LA_FLASH_GORDON_6, lampPhase==6||lampPhase==7, 0, 0);
  BSOS_SetLampState(LA_FLASH_GORDON_5, lampPhase==5||lampPhase==8, 0, 0);
  BSOS_SetLampState(LA_FLASH_GORDON_4, lampPhase==4||lampPhase==9, 0, 0);
  if (MachineState!=MACHINE_STATE_NORMAL_GAMEPLAY) BSOS_SetLampState(LA_FLASH_STROBE, lampPhase==3||lampPhase==10, 0, 0);
  BSOS_SetLampState(LA_FLASH_GORDON_3, lampPhase==2||lampPhase==11, 0, 0);
  BSOS_SetLampState(LA_FLASH_GORDON_2, lampPhase==1||lampPhase==12, 0, 0);
  BSOS_SetLampState(LA_FLASH_GORDON_1, lampPhase==0||lampPhase==13, 0, 0);
}

void BackglassLampsLeft2Right() {
  byte lampPhase = (CurrentTime/85)%4;
  BSOS_SetLampState(LA_FLASH_GORDON_6, lampPhase==3||lampPhase==0, lampPhase==0);
  BSOS_SetLampState(LA_FLASH_GORDON_3, lampPhase==3||lampPhase==0, lampPhase==0);
  BSOS_SetLampState(LA_FLASH_GORDON_5, lampPhase==2||lampPhase==3, lampPhase==3);
  BSOS_SetLampState(LA_FLASH_GORDON_2, lampPhase==2||lampPhase==3, lampPhase==3);
  BSOS_SetLampState(LA_FLASH_GORDON_4, lampPhase==1||lampPhase==2, lampPhase==2);
  BSOS_SetLampState(LA_FLASH_GORDON_1, lampPhase==1||lampPhase==2, lampPhase==2);
  if (MachineState!=MACHINE_STATE_NORMAL_GAMEPLAY) BSOS_SetLampState(LA_FLASH_STROBE, lampPhase<2, lampPhase==1);
}


void AttractRetro() {
  // bonus
  byte attractBonus;
  for (attractBonus=0; attractBonus<10; attractBonus++){
    if ( attractBonus % 2 == 0) { // even but odd on pf
      BSOS_SetLampState(LA_BONUS_MINI_1K+attractBonus, 1, 0, 100);
      BSOS_SetLampState(LA_BONUS_SUPER_1K+attractBonus, 1, 0, 100);
    } else { // odd even on pf
      BSOS_SetLampState(LA_BONUS_MINI_1K+attractBonus, 1, 0, 200);
      BSOS_SetLampState(LA_BONUS_SUPER_1K+attractBonus, 1, 0, 200);
    }
  }

  // sweep 4
  byte lampPhase1 = (CurrentTime/115)%4; // 125
  BSOS_SetLampState(LA_SAUCER_XBALL, lampPhase1==3||lampPhase1==0, lampPhase1==0); // SAUCER
  BSOS_SetLampState(LA_SAUCER_30K, lampPhase1==2||lampPhase1==3, lampPhase1==3);
  BSOS_SetLampState(LA_SAUCER_20K, lampPhase1==1||lampPhase1==2, lampPhase1==2);
  BSOS_SetLampState(LA_SAUCER_10K, lampPhase1<2, lampPhase1==1);
  BSOS_SetLampState(LA_DTARGET_4_A, lampPhase1==3||lampPhase1==0, lampPhase1==0); // DTARGET 4
  BSOS_SetLampState(LA_DTARGET_4_B, lampPhase1==2||lampPhase1==3, lampPhase1==3);
  BSOS_SetLampState(LA_DTARGET_4_C, lampPhase1==1||lampPhase1==2, lampPhase1==2);
  BSOS_SetLampState(LA_DTARGET_4_D, lampPhase1<2, lampPhase1==1);
  BSOS_SetLampState(LA_BONUS_5X, lampPhase1==3||lampPhase1==0, lampPhase1==0); // BONUS X
  BSOS_SetLampState(LA_BONUS_4X, lampPhase1==2||lampPhase1==3, lampPhase1==3);
  BSOS_SetLampState(LA_BONUS_3X, lampPhase1==1||lampPhase1==2, lampPhase1==2);
  BSOS_SetLampState(LA_BONUS_2X, lampPhase1<2, lampPhase1==1);
  BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, lampPhase1==3||lampPhase1==0, lampPhase1==0); // OUTLANES & PFX
  BSOS_SetLampState(LA_SAUCER_ARROW_3X, lampPhase1==2||lampPhase1==3, lampPhase1==3);
  BSOS_SetLampState(LA_CLOCK_15_SECONDS_2X, lampPhase1==2||lampPhase1==3, lampPhase1==3);
  BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, lampPhase1==1||lampPhase1==2, lampPhase1==2);
  BSOS_SetLampState(LA_SAUCER_ARROW_2X, lampPhase1<2, lampPhase1==1);
  BSOS_SetLampState(LA_CLOCK_15_SECONDS_3X, lampPhase1<2, lampPhase1==1);
  BSOS_SetLampState(LA_INLANE_LEFT, lampPhase1==3||lampPhase1==0, lampPhase1==0); // INLANES & SPINNERS
  BSOS_SetLampState(LA_SPINNER_RIGHT, lampPhase1==2||lampPhase1==3, lampPhase1==3);
  BSOS_SetLampState(LA_INLANE_RIGHT, lampPhase1==1||lampPhase1==2, lampPhase1==2);
  BSOS_SetLampState(LA_SPINNER_LEFT, lampPhase1<2, lampPhase1==1);
  BSOS_SetLampState(LA_TARGET_LRIGHT_BOTTOM, lampPhase1==3||lampPhase1==0, lampPhase1==0); // REMAINDER
  BSOS_SetLampState(LA_TARGET_WOOD_BEAST_XBALL, lampPhase1==2||lampPhase1==3, lampPhase1==3);
  BSOS_SetLampState(LA_TARGET_LRIGHT_TOP, lampPhase1==1||lampPhase1==2, lampPhase1==2);
  BSOS_SetLampState(LA_DTARGET_BONUS_5X, lampPhase1<2, lampPhase1==1);

  // sweep 5
  byte lampPhase2 = (CurrentTime/95)%5;
  BSOS_SetLampState(LA_STAR_SHOOTER_BOTTOM, lampPhase2==4||lampPhase2==0, lampPhase2==0);
  BSOS_SetLampState(LA_STAR_SHOOTER_MIDDLE, lampPhase2==3||lampPhase2==4, lampPhase2==4);
  BSOS_SetLampState(LA_STAR_SHOOTER_TOP, lampPhase2==2||lampPhase2==3, lampPhase2==3);
  BSOS_SetLampState(LA_STAR_PFIELD_TOP, lampPhase2==1||lampPhase2==2, lampPhase2==2);
  BSOS_SetLampState(LA_STAR_PFIELD_BOTTOM, lampPhase2<2, lampPhase2==1);

  // sweep 4
  byte lampPhase3 = (CurrentTime/75)%6;
  BSOS_SetLampState(LA_DTARGET_ARROW_3, lampPhase3==5||lampPhase3==0, lampPhase3==0);
  BSOS_SetLampState(LA_DTARGET_ARROW_2, lampPhase3==4||lampPhase3==5, lampPhase3==5);
  BSOS_SetLampState(LA_DTARGET_ARROW_1, lampPhase3==3, lampPhase3==4, lampPhase3==4);
  BSOS_SetLampState(LA_TARGET_UPPER_SPECIAL, lampPhase3==2||lampPhase3==3, lampPhase3==3);
  BSOS_SetLampState(LA_TARGET_UPPER_COLLECT_BONUS, lampPhase3==1||lampPhase3==2, lampPhase3==2);
  BSOS_SetLampState(LA_DTARGET_BONUS_4X, lampPhase3<2, lampPhase3==1);

  // remainder
  BSOS_DataRead(0);
  BSOS_SetLampState(LA_POP_TOP, 1, 0, 250);
  BSOS_SetLampState(LA_BONUS_MINI_50K, 1, 0, 250);
  BSOS_SetLampState(LA_BONUS_SUPER_100K, 1, 0, 250);
  BSOS_SetLampState(LA_SHOOT_AGAIN, 1, 0, 250);
  BSOS_SetLampState(LA_MING_TOP, 1, 0, 250);
  BSOS_SetLampState(LA_MING_BOTTOM, 1);

  BackglassLampsLeft2Right();
}


void AddToScore(unsigned long scoreAmount) {
  if (Playfield2xState==2) {
    scoreAmount = scoreAmount * 2;
  } else if (Playfield3xState==2) {
    scoreAmount = scoreAmount * 3;
  } else if (Playfield3xState==2 && Playfield2xState==2) {
    scoreAmount = scoreAmount * 5;
  }
  CurrentScores[CurrentPlayer] += scoreAmount;
}


void TiltHit() {
  NumTiltWarnings += 1;
  if (NumTiltWarnings>=MaxTiltWarnings) {
    BSOS_PlaySoundSquawkAndTalk(41);
    BSOS_DisableSolenoidStack();
    BSOS_SetDisableFlippers(true);
    BSOS_TurnOffAllLamps();
    if (Credits) BSOS_SetLampState(LA_CREDIT_INDICATOR, 1);
    BSOS_SetLampState(LA_TILT, 1);
  }
}


int SkillShot(boolean curStateChanged) {
  int returnState = MACHINE_STATE_SKILL_SHOT;

  if (curStateChanged) {
    BSOS_SetDisplayCredits(0, true);
    BSOS_SetLampState(LA_MING_TOP, 1, 0, 100);
    BSOS_SetLampState(LA_MING_BOTTOM, 1, 0, 200);
    // Serial.println(F("Skill Shot Started"));
  }

  // HANDLE LAMPS
  if (SkillShotState==0) {
    backglassLampsCenterOut();
    byte lampPhase = (CurrentTime/600)%3;
    BSOS_SetLampState(LA_STAR_SHOOTER_TOP, lampPhase==2, 0, 100);
    BSOS_SetLampState(LA_STAR_SHOOTER_MIDDLE, lampPhase==1||lampPhase==2, 0, 100);
    BSOS_SetLampState(LA_STAR_SHOOTER_BOTTOM, lampPhase==0||lampPhase==1||lampPhase==2, 0, 100);
  } else if (SkillShotState==1) {
    BackglassLampsClear();
    if (SkillShotHits==1 || SkillShotHits==2) {
      BSOS_SetLampState(LA_STAR_SHOOTER_BOTTOM, 1);
    } else if (SkillShotHits==3 || SkillShotHits==4) {
      BSOS_SetLampState(LA_STAR_SHOOTER_MIDDLE, 1);
      BSOS_SetLampState(LA_STAR_SHOOTER_BOTTOM, 1);
    } else if (SkillShotHits>=5) {
      BSOS_SetLampState(LA_STAR_SHOOTER_TOP, 1);
      BSOS_SetLampState(LA_STAR_SHOOTER_MIDDLE, 1);
      BSOS_SetLampState(LA_STAR_SHOOTER_BOTTOM, 1);
    }
  } else if (SkillShotState==2) {
    if (CurrentTime<=SkillShotScoreAnimation) {
      if (SkillShotHits==1 || SkillShotHits==2) {
        BSOS_SetLampState(LA_STAR_SHOOTER_BOTTOM, 1, 0, 125);
      } else if (SkillShotHits==3 || SkillShotHits==4) {
        BSOS_SetLampState(LA_STAR_SHOOTER_MIDDLE, 1, 0, 125);
        BSOS_SetLampState(LA_STAR_SHOOTER_BOTTOM, 1, 0, 125);
      } else if (SkillShotHits>=5) {
        BSOS_SetLampState(LA_STAR_SHOOTER_TOP, 1, 0, 125);
        BSOS_SetLampState(LA_STAR_SHOOTER_MIDDLE, 1, 0, 125);
        BSOS_SetLampState(LA_STAR_SHOOTER_BOTTOM, 1, 0, 125);
      }
      BSOS_SetLampState(LA_FLASH_STROBE, 1, 0, 125);
    } else {
      SkillShotState = 3;
    }
  }

  // If the skillshot hasn't been collected yet, flash score
  if (SkillShotState<=2) {
    BSOS_SetDisplayFlash(CurrentPlayer, CurrentScores[CurrentPlayer], CurrentTime, 250, 2);
  } else {
    // Serial.println(F("Skill Shot Ended"));
    BSOS_SetLampState(LA_STAR_SHOOTER_TOP, 0);
    BSOS_SetLampState(LA_STAR_SHOOTER_MIDDLE, 0);
    BSOS_SetLampState(LA_STAR_SHOOTER_BOTTOM, 0);
    BSOS_SetLampState(LA_FLASH_STROBE, 0);
    returnState = MACHINE_STATE_NORMAL_GAMEPLAY;
    for (byte count=0; count<CurrentNumPlayers; count++) {
      BSOS_SetDisplay(count, CurrentScores[count], true, 2);
    }
  }

  return returnState;
}


void DropTargetHit() {
  DropTargetCount++;
  if (DropTargetCount>=16) {
    DropTargetCount=16;
    BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 1);
    BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 1);
    BSOS_SetLampState(LA_MING_TOP, 1, 0, 100);
    BSOS_SetLampState(LA_MING_BOTTOM, 1, 0, 200);
    WizardState = 1;
    BSOS_PlaySoundSquawkAndTalk(44);
  }
}

boolean CheckIfDTargets4Down() {
  return (  BSOS_ReadSingleSwitchState(SW_DTARGET_4_A) &&
            BSOS_ReadSingleSwitchState(SW_DTARGET_4_B) &&
            BSOS_ReadSingleSwitchState(SW_DTARGET_4_C) &&
            BSOS_ReadSingleSwitchState(SW_DTARGET_4_D) );
}

boolean CheckIfDTargets3Down() {
  return (  BSOS_ReadSingleSwitchState(SW_DTARGET_3_A) &&
            BSOS_ReadSingleSwitchState(SW_DTARGET_3_B) &&
            BSOS_ReadSingleSwitchState(SW_DTARGET_3_C) );
}

void CheckHurryUpCompletion() {
  // HURRY UP
  if (DTarget4Light[0]==3 && DTarget4Light[1]==3 && DTarget4Light[2]==3 && DTarget4Light[3]==3) {
    Super100kCollected = true;
    Playfield3xState = 1;
    BSOS_PlaySoundSquawkAndTalk(48);
    BSOS_SetLampState(LA_SAUCER_ARROW_3X, 1, 0, 150);
    BSOS_SetLampState(LA_BONUS_SUPER_100K, 1);
  }
}

void CheckSaucerDTargetGoal() {
  if (DTarget4Goal==true && DTarget3Goal==true && DTargetInlineGoal==true && WoodBeastXBallState[CurrentPlayer]==3 && SacuerXBallState[CurrentPlayer]==0) {
    BSOS_SetLampState(LA_SAUCER_XBALL, 1);
    SacuerXBallState[CurrentPlayer] = 1;
  }
}


void AddToMiniBonus(byte bonusAddition) {
  MiniBonus += bonusAddition;
  if (MiniBonus>MaxMiniBonus) MiniBonus = MaxMiniBonus;
}

void AddToSuperBonus(byte bonusAddition) {
  SuperBonus += bonusAddition;
  if (SuperBonus>MaxSuperBonus) SuperBonus = MaxSuperBonus;
}

void SetMiniBonusIndicator(byte number, byte state, byte dim, int flashPeriod=0) {
  if (number==0 || number>19) return;
  BSOS_SetLampState(LA_BONUS_MINI_1K+(number-1), state, dim, flashPeriod);
}

void SetSuperBonusIndicator(byte number, byte state, byte dim, int flashPeriod=0) {
  if (number==0 || number>19) return;
  BSOS_SetLampState(LA_BONUS_SUPER_1K+(number-1), state, dim, flashPeriod);
}

void ShowMiniBonusOnLadder(byte bonus) {
  if (bonus>MaxMiniDisplayBonus) bonus = MaxMiniDisplayBonus;
  byte cap = 10;

  for (byte count=1; count<11; count++) SetMiniBonusIndicator(count, 0, 0);
  
  if (bonus==0) {
    return;
  } else if (bonus<cap) {
    SetMiniBonusIndicator(bonus, 1, 0);
    byte bottom;
    for (bottom=1; bottom<bonus; bottom++){
      SetMiniBonusIndicator(bottom, 1, 0, 0);
    }
  } else if (bonus==cap) {
    SetMiniBonusIndicator(cap, 1, 0, 0);
  } else if (bonus>cap) {
    SetMiniBonusIndicator(cap, 1, 0, 0);
    SetMiniBonusIndicator(bonus-10, 1, 0);
    byte bottom;
    for (bottom=1; bottom<bonus-10; bottom++){
      SetMiniBonusIndicator(bottom, 1, 0, 0);
    }
  }
}

void ShowSuperBonusOnLadder(byte bonus) {
  if (bonus>MaxSuperDisplayBonus) bonus = MaxSuperDisplayBonus;
  byte cap = 10;

  for (byte count=1; count<11; count++) SetSuperBonusIndicator(count, 0, 0);
  
  if (bonus==0) {
    return;
  } else if (bonus<cap) {
    SetSuperBonusIndicator(bonus, 1, 0);
    byte bottom;
    for (bottom=1; bottom<bonus; bottom++){
      SetSuperBonusIndicator(bottom, 1, 0, 0);
    }
  } else if (bonus==cap) {
    SetSuperBonusIndicator(cap, 1, 0, 0);
  } else if (bonus>cap) {
    SetSuperBonusIndicator(cap, 1, 0, 0);
    SetSuperBonusIndicator(bonus-10, 1, 0);
    byte bottom;
    for (bottom=1; bottom<bonus-10; bottom++){
      SetSuperBonusIndicator(bottom, 1, 0, 0);
    }
  }
  
//  if (bonus>MaxSuperDisplayBonus) bonus = MaxSuperDisplayBonus;
//  byte cap = 10;
//
//  for (byte count=1; count<11; count++) SetSuperBonusIndicator(count, 0, 0);
//  if (bonus==0) return;
//
//  while (bonus>cap) {
//    SetSuperBonusIndicator(cap, 1, 0, 100);
//    bonus -= cap;
//    cap -= 1;
//    if (cap==0) bonus = 0;
//  }
//
//  byte bottom;
//  for (bottom=1; bottom<bonus; bottom++){
//    SetSuperBonusIndicator(bottom, 1, 0);
//    // SetSuperBonusIndicator(bottom, 0, 1);
//  }
//
//  if (bottom<=cap) {
//    SetSuperBonusIndicator(bottom, 1, 0);
//  }
}


void ShowExtraBonusLights() {
  if (Mini50kCollected==true) {
    BSOS_SetLampState(LA_BONUS_MINI_50K, 1);
  }
  if (Super100kCollected==true) {
    BSOS_SetLampState(LA_BONUS_SUPER_100K, 1);
  }
  if (BonusXState==2) {
    BSOS_SetLampState(LA_BONUS_2X, 1);
  } else if (BonusXState==3) {
    BSOS_SetLampState(LA_BONUS_2X, 0);
    BSOS_SetLampState(LA_BONUS_3X, 1);
    BSOS_SetLampState(LA_DTARGET_BONUS_4X, 1, 0, 200);
  } else if (BonusXState==4) {
    BSOS_SetLampState(LA_DTARGET_BONUS_4X, 0);
    BSOS_SetLampState(LA_BONUS_3X, 0);
    BSOS_SetLampState(LA_BONUS_4X, 1);
    BSOS_SetLampState(LA_DTARGET_BONUS_5X, 1, 0, 200);
  } else if (BonusXState==5) {
    BSOS_SetLampState(LA_DTARGET_BONUS_5X, 0);
    BSOS_SetLampState(LA_BONUS_4X, 0);
    BSOS_SetLampState(LA_BONUS_5X, 1);
  }
}

int CountdownBonus(boolean curStateChanged) {

  // If this is the first time through the countdown loop
  if (curStateChanged) {
    BSOS_SetLampState(LA_BALL_IN_PLAY, 1, 0, 250);

    CountdownStartTime = CurrentTime;
    ShowMiniBonusOnLadder(MiniBonus);
    ShowSuperBonusOnLadder(SuperBonus);

    if (Super100kCollected==true) {
      CurrentScores[CurrentPlayer] += 100000;
      BSOS_SetLampState(LA_BONUS_SUPER_100K, 0);
    }
    if (Mini50kCollected==true) {
      CurrentScores[CurrentPlayer] += 50000;
      BSOS_SetLampState(LA_BONUS_MINI_50K, 0);
    }
    
    LastCountdownReportTime = CountdownStartTime;
    BonusCountDownEndTime = 0xFFFFFFFF;
  }

  BackglassLampsLeft2Right();

  if ((CurrentTime-LastCountdownReportTime)>100) { // adjust speed 300

    if (MiniBonus>0) {
      if (BonusXState==1) {
        CurrentScores[CurrentPlayer] += 1000;
      } else if (BonusXState==2) {
        CurrentScores[CurrentPlayer] += 2000;
      } else if (BonusXState==3) {
        CurrentScores[CurrentPlayer] += 3000;
      } else if (BonusXState==4) {
        CurrentScores[CurrentPlayer] += 4000;
      } else if (BonusXState==5) {
        CurrentScores[CurrentPlayer] += 5000;
      }
      MiniBonus -= 1;
      ShowMiniBonusOnLadder(MiniBonus);
      BSOS_PlaySoundSquawkAndTalk(12);
    } else if (SuperBonus>0) {
      if (BonusXState==1) {
        CurrentScores[CurrentPlayer] += 1000;
      } else if (BonusXState==2) {
        CurrentScores[CurrentPlayer] += 2000;
      } else if (BonusXState==3) {
        CurrentScores[CurrentPlayer] += 3000;
      } else if (BonusXState==4) {
        CurrentScores[CurrentPlayer] += 4000;
      } else if (BonusXState==5) {
        CurrentScores[CurrentPlayer] += 5000;
      }
      SuperBonus -= 1;
      ShowSuperBonusOnLadder(SuperBonus);
      BSOS_PlaySoundSquawkAndTalk(12);
    } else if (BonusCountDownEndTime==0xFFFFFFFF) {
      BonusCountDownEndTime = CurrentTime+1000;
      BSOS_SetLampState(LA_BONUS_MINI_1K, 0);
      BSOS_SetLampState(LA_BONUS_SUPER_1K, 0);
    }
    
//    if (MiniBonus>0) {
//      CurrentScores[CurrentPlayer] += 1000;
//      MiniBonus -= 1;
//      ShowMiniBonusOnLadder(MiniBonus);
//    } else if (BonusCountDownEndTime==0xFFFFFFFF) {
//      BSOS_SetLampState(LA_BONUS_MINI_1K, 0);
//      BonusCountDownEndTime = CurrentTime+1000;
//    }
//    if (SuperBonus>0) {
//      CurrentScores[CurrentPlayer] += 1000;
//      SuperBonus -= 1;
//      ShowSuperBonusOnLadder(SuperBonus);
//    } else if (BonusCountDownEndTime==0xFFFFFFFF) {
//      BSOS_SetLampState(LA_BONUS_SUPER_1K, 0);
//      BonusCountDownEndTime = CurrentTime+1000;
//    }
    LastCountdownReportTime = CurrentTime;
  }

  if (CurrentTime>BonusCountDownEndTime) {
    BonusCountDownEndTime = 0xFFFFFFFF;
    BSOS_PlaySoundSquawkAndTalk(8);
    if (WizardState==2) {
      WizardState = 3;
      return MACHINE_STATE_WIZARD_MODE;
    } else {
      return MACHINE_STATE_BALL_OVER;
    }
  }

  return MACHINE_STATE_COUNTDOWN_BONUS;
}


int ShowMatchSequence(boolean curStateChanged) {  
  if (curStateChanged) {
    MatchSequenceStartTime = CurrentTime;
    MatchDelay = 1500;
    MatchDigit = random(0,10);
    NumMatchSpins = 0;
    BSOS_SetLampState(LA_MATCH, 1, 0);
    BSOS_SetDisableFlippers();
    ScoreMatches = 0;
    BSOS_SetLampState(LA_BALL_IN_PLAY, 0);
    
    unsigned long highestScore = 0;
    // int highScorePlayerNum = 0;
    for (byte count=0; count<CurrentNumPlayers; count++) {
      if (CurrentScores[count]>highestScore) highestScore = CurrentScores[count];
      // highScorePlayerNum = count;
    }
    if (highestScore>HighScore) {
      HighScore = highestScore;
      AddCredit(3);
      BSOS_WriteULToEEProm(BSOS_HIGHSCORE_EEPROM_START_BYTE, HighScore);
      for (byte count=0; count<CurrentNumPlayers; count++) {
        // if (count==highScorePlayerNum) {
        //   BSOS_SetDisplay(count, CurrentScores[count], true, 2);
        // } else {
        //   BSOS_SetDisplayBlank(count, 0x00);
        // }
        BSOS_SetDisplay(count, CurrentScores[count], true, 2);
      }
      BSOS_PushToTimedSolenoidStack(SO_KNOCKER, 3, CurrentTime, true);
      BSOS_PushToTimedSolenoidStack(SO_KNOCKER, 3, CurrentTime+250, true);
      BSOS_PushToTimedSolenoidStack(SO_KNOCKER, 3, CurrentTime+500, true);
    }
  }

  if (NumMatchSpins<40) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      MatchDigit += 1;
      if (MatchDigit>9) MatchDigit = 0;
      // PlaySoundEffect(SOUND_EFFECT_MATCH_SPIN);
      BSOS_SetDisplayBallInPlay((int)MatchDigit*10);
      MatchDelay += 50 + 4*NumMatchSpins;
      NumMatchSpins += 1;
      BSOS_SetLampState(LA_MATCH, NumMatchSpins%2, 0);

      if (NumMatchSpins==40) {
        BSOS_SetLampState(LA_MATCH, 0);
        MatchDelay = CurrentTime - MatchSequenceStartTime;
      }
    }
  }

  if (NumMatchSpins>=40 && NumMatchSpins<=43) {
    if (CurrentTime>(MatchSequenceStartTime + MatchDelay)) {
      if ( (CurrentNumPlayers>(NumMatchSpins-40)) && ((CurrentScores[NumMatchSpins-40]/10)%10)==MatchDigit) {
        ScoreMatches |= (1<<(NumMatchSpins-40));
        AddCredit(1);
        BSOS_PushToTimedSolenoidStack(SO_KNOCKER, 3, CurrentTime, true);
        MatchDelay += 1000;
        NumMatchSpins += 1;
        BSOS_SetLampState(LA_MATCH, 1);
      } else {
        NumMatchSpins += 1;
      }
      if (NumMatchSpins==44) {
        MatchDelay += 5000;
      }
    }      
  }

  if (NumMatchSpins>43) {
    if (CurrentTime>(MatchSequenceStartTime + MatchDelay)) {
      return MACHINE_STATE_ATTRACT;
    }    
  }

  for (byte count=0; count<4; count++) {
    if ((ScoreMatches>>count)&0x01) {
      // If this score matches, we're going to flash the last two digits
      if ( (CurrentTime/200)%2 ) {
        BSOS_SetDisplayBlank(count, BSOS_GetDisplayBlank(count) & 0x1F); // 0x0F 1111 // 0x1F 11111 // 0x7f 1111111
      } else {
        BSOS_SetDisplayBlank(count, BSOS_GetDisplayBlank(count) | 0x60); // 0x30 110000 // 0x60 1100000
      }
    }
  }

  return MACHINE_STATE_MATCH_MODE;
}


int WizardMode(boolean curStateChanged) {
  int returnState = MACHINE_STATE_WIZARD_MODE;
  BSOS_SetDisplayCredits(MingAttackProgress, true);

  if (curStateChanged) {
    BSOS_PlaySoundSquawkAndTalk(43);
    WizardState = 3;
    AddToScore(50000);
    BSOS_TurnOffAllLamps();
    if (NIGHT_TESTING) {
      // BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_DOWN, 15, CurrentTime);
    }
    BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 1);
    BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 1);
    BSOS_SetLampState(LA_POP_TOP, 1);
    BSOS_SetLampState(LA_SPINNER_LEFT, 1);
    BSOS_SetLampState(LA_SPINNER_RIGHT, 1);
    BSOS_SetLampState(LA_MING_TOP, 1, 0, 100);
    BSOS_SetLampState(LA_MING_BOTTOM, 1, 0, 200);
    if (PlayerShootsAgain==true) {
      BSOS_SetLampState(LA_SAME_PLAYER_SHOOTS_AGAIN, 1);
      BSOS_SetLampState(LA_SHOOT_AGAIN, 1);
    }
    if (NIGHT_TESTING) {
      // BSOS_PushToTimedSolenoidStack(SO_DTARGET_4_RESET, 15, CurrentTime);
      // BSOS_PushToTimedSolenoidStack(SO_DTARGET_3_RESET, 15, CurrentTime + 250);
      // BSOS_PushToTimedSolenoidStack(SO_DTARGET_INLINE_RESET, 15, CurrentTime + 500);
      // BSOS_PushToTimedSolenoidStack(SO_OUTHOLE, 4, CurrentTime + 100);
    }
  }

  // validate wizard mode
  if (WizardState<4) {
    BSOS_SetDisplayFlash(CurrentPlayer, CurrentScores[CurrentPlayer], CurrentTime, 250, 2);
    BackglassLampsLeft2Right();
  } else {  
    BSOS_SetDisplay(CurrentPlayer, CurrentScores[CurrentPlayer], true, 2);
  }

  // handle lamps
  if (WizardState==4) {
    if ((MingAttackProgress==0) || (MingAttackProgress<=33)) {
      BSOS_SetLampState(LA_FLASH_GORDON_5, 0);
      BSOS_SetLampState(LA_FLASH_GORDON_2, 0);
      BSOS_SetLampState(LA_FLASH_GORDON_4, 0);
      BSOS_SetLampState(LA_FLASH_GORDON_3, 0);
      BSOS_SetLampState(LA_FLASH_STROBE, 0);

      if ((MingAttackProgress==0) || (MingAttackProgress<=11)) {
        if (MingHealth!=0) {
          BSOS_SetLampState(LA_SAUCER_10K, 1, 0, 250);
          BSOS_SetLampState(LA_FLASH_GORDON_6, 1, 0, 250);
          BSOS_SetLampState(LA_FLASH_GORDON_1, 1, 0, 250);
        }
      } else if (MingAttackProgress>=12 && MingAttackProgress<=22) {
        BSOS_SetLampState(LA_SAUCER_10K, 1, 0, 180);
        BSOS_SetLampState(LA_FLASH_GORDON_6, 1, 0, 180);
        BSOS_SetLampState(LA_FLASH_GORDON_1, 1, 0, 180);
      } else if (MingAttackProgress>=23 && MingAttackProgress<=33) {
        BSOS_SetLampState(LA_SAUCER_10K, 1, 0, 110);
        BSOS_SetLampState(LA_FLASH_GORDON_6, 1, 0, 110);
        BSOS_SetLampState(LA_FLASH_GORDON_1, 1, 0, 110);
      }
    } else if (MingAttackProgress>=34 && MingAttackProgress<=69) {
      BSOS_SetLampState(LA_SAUCER_10K, 1);
      BSOS_SetLampState(LA_FLASH_GORDON_6, 1);
      BSOS_SetLampState(LA_FLASH_GORDON_1, 1);
      if (MingAttackProgress>=34 && MingAttackProgress<=46) {
        BSOS_SetLampState(LA_SAUCER_20K, 1, 0, 250);
        BSOS_SetLampState(LA_FLASH_GORDON_5, 1, 0, 250);
        BSOS_SetLampState(LA_FLASH_GORDON_2, 1, 0, 250);
      } else if (MingAttackProgress>=47 && MingAttackProgress<=57) {
        BSOS_SetLampState(LA_SAUCER_20K, 1, 0, 180);
        BSOS_SetLampState(LA_FLASH_GORDON_5, 1, 0, 180);
        BSOS_SetLampState(LA_FLASH_GORDON_2, 1, 0, 180);
      } else if (MingAttackProgress>=58 && MingAttackProgress<=69) {
        BSOS_SetLampState(LA_SAUCER_20K, 1, 0, 110);
        BSOS_SetLampState(LA_FLASH_GORDON_5, 1, 0, 110);
        BSOS_SetLampState(LA_FLASH_GORDON_2, 1, 0, 110);
      }
    } else if (MingAttackProgress>=70 && MingAttackProgress<=104) {
      BSOS_SetLampState(LA_SAUCER_20K, 1);
      BSOS_SetLampState(LA_FLASH_GORDON_5, 1);
      BSOS_SetLampState(LA_FLASH_GORDON_2, 1);
      if (MingAttackProgress>=70 && MingAttackProgress<=81) {
        BSOS_SetLampState(LA_SAUCER_30K, 1, 0, 250);
        BSOS_SetLampState(LA_FLASH_GORDON_4, 1, 0, 250);
        BSOS_SetLampState(LA_FLASH_GORDON_3, 1, 0, 250);
      } else if (MingAttackProgress>=82 && MingAttackProgress<=92) {
        BSOS_SetLampState(LA_SAUCER_30K, 1, 0, 180);
        BSOS_SetLampState(LA_FLASH_GORDON_4, 1, 0, 180);
        BSOS_SetLampState(LA_FLASH_GORDON_3, 1, 0, 180);
      } else if (MingAttackProgress>93 && MingAttackProgress<=104) {
        BSOS_SetLampState(LA_SAUCER_30K, 1, 0, 110);
        BSOS_SetLampState(LA_FLASH_GORDON_4, 1, 0, 110);
        BSOS_SetLampState(LA_FLASH_GORDON_3, 1, 0, 110);
      }
    } else if (MingAttackProgress>=105 && MingAttackProgress<=139) {
      BSOS_SetLampState(LA_SAUCER_30K, 1);
      BSOS_SetLampState(LA_FLASH_GORDON_4, 1);
      BSOS_SetLampState(LA_FLASH_GORDON_3, 1);
      if (MingAttackProgress>=105 && MingAttackProgress<=116) {
        BSOS_SetLampState(LA_SAUCER_XBALL, 1, 0, 250);
        BSOS_SetLampState(LA_FLASH_STROBE, 1, 0, 250);
      } else if (MingAttackProgress>=117 && MingAttackProgress<=127) {
        BSOS_SetLampState(LA_SAUCER_XBALL, 1, 0, 180);
        BSOS_SetLampState(LA_FLASH_STROBE, 1, 0, 180);
      } else if (MingAttackProgress>128 && MingAttackProgress<=139) {
        BSOS_SetLampState(LA_SAUCER_XBALL, 1, 0, 110);
        BSOS_SetLampState(LA_FLASH_STROBE, 1, 0, 110);
      }
    } else if (MingAttackProgress>=140 && BallInSaucer==false) {
      MingAttackProgress = 140;
      MingAttackReady = true;
      // BSOS_SetLampState(LA_SAUCER_XBALL, 1);
      byte lampPhase = (CurrentTime/150)%4; // 250
      BSOS_SetLampState(LA_SAUCER_XBALL, lampPhase==3, lampPhase==0, lampPhase==0);
      BSOS_SetLampState(LA_SAUCER_30K, lampPhase==2||lampPhase==3, lampPhase==3);
      BSOS_SetLampState(LA_SAUCER_20K, lampPhase==1||lampPhase==2, lampPhase==2);
      BSOS_SetLampState(LA_SAUCER_10K, lampPhase<2, lampPhase==1);

      BSOS_SetLampState(LA_FLASH_STROBE, lampPhase==3, lampPhase==0, lampPhase==0);
      BSOS_SetLampState(LA_FLASH_GORDON_4, lampPhase==2||lampPhase==3, lampPhase==3);
      BSOS_SetLampState(LA_FLASH_GORDON_3, lampPhase==2||lampPhase==3, lampPhase==3);
      BSOS_SetLampState(LA_FLASH_GORDON_5, lampPhase==1||lampPhase==2, lampPhase==2);
      BSOS_SetLampState(LA_FLASH_GORDON_2, lampPhase==1||lampPhase==2, lampPhase==2);
      BSOS_SetLampState(LA_FLASH_GORDON_6, lampPhase<2, lampPhase==1);
      BSOS_SetLampState(LA_FLASH_GORDON_1, lampPhase<2, lampPhase==1);
    }
  }

  // handle ming attack animation
  if (BallInSaucer==true && MingAttackReady==true) {
    if (CurrentTime<=MingAttackAnimation) {
      backglassLampsCenterOut();
      if ((CurrentTime-AttractSweepTime)>25) {
        AttractSweepTime = CurrentTime;
        for (byte lightcountdown=0; lightcountdown<NUM_OF_ATTRACT_LAMPS_MING_ATTACK; lightcountdown++) {
          byte dist = MingAttackLamps - AttractLampsMingAttack[lightcountdown].rowMingAttack;
          BSOS_SetLampState(AttractLampsMingAttack[lightcountdown].lightNumMingAttack, (dist<8), (dist==0/*||dist>5*/)?0:dist/3, (dist>5)?(100+AttractLampsMingAttack[lightcountdown].lightNumMingAttack):0);
          if (lightcountdown==(NUM_OF_ATTRACT_LAMPS_MING_ATTACK/2)) BSOS_DataRead(0);
        }
        MingAttackLamps += 1;
        if (MingAttackLamps>30) MingAttackLamps = 0;
      }
      if (MingHealth==0 && CurrentTime>=(MingAttackAnimation-825)) {
        BSOS_DisableSolenoidStack();
      }
    } else {
      for (byte off=LA_BONUS_MINI_1K; off<=LA_TARGET_LRIGHT_TOP; off++) BSOS_SetLampState(off, 0);
      for (byte off=LA_SAUCER_10K; off<=LA_DTARGET_BONUS_5X; off++) BSOS_SetLampState(off, 0);
      for (byte off=LA_POP_TOP; off<=LA_STAR_SHOOTER_BOTTOM; off++) BSOS_SetLampState(off, 0);
      for (byte off=LA_CLOCK_15_SECONDS_3X; off<=LA_SAUCER_ARROW_2X; off++) BSOS_SetLampState(off, 0);
      BSOS_SetLampState(LA_SPINNER_RIGHT, 1);
      BSOS_SetLampState(LA_SPINNER_LEFT, 1);
      BSOS_SetLampState(LA_POP_TOP, 1);
      if (MingHealth!=0) {
        AddToScore(50000);
        BSOS_PushToTimedSolenoidStack(SO_SAUCER_DOWN, 5, CurrentTime);
      } else {
        // ming defeated
        AddToScore(150000);
        BSOS_SetDisableFlippers(true);
        byte bonusFireworks;
        for (bonusFireworks=0; bonusFireworks<10; bonusFireworks++){
          if ( bonusFireworks % 2 == 0) { // even but odd on pf
            BSOS_SetLampState(LA_BONUS_MINI_1K+bonusFireworks, 1, 0, 100);
            BSOS_SetLampState(LA_BONUS_SUPER_1K+bonusFireworks, 1, 0, 100);
          } else { // odd even on pf
            BSOS_SetLampState(LA_BONUS_MINI_1K+bonusFireworks, 1, 0, 200);
            BSOS_SetLampState(LA_BONUS_SUPER_1K+bonusFireworks, 1, 0, 200);
          }
        }
        BSOS_SetLampState(LA_BONUS_MINI_50K, 1, 0, 500);
        BSOS_SetLampState(LA_BONUS_SUPER_100K, 1, 0, 500);
        BSOS_SetLampState(LA_FLASH_GORDON_1, 1, 0, 200);
        BSOS_SetLampState(LA_FLASH_GORDON_2, 1, 0, 100);
        BSOS_SetLampState(LA_FLASH_GORDON_3, 1, 0, 200);
        BSOS_SetLampState(LA_FLASH_GORDON_4, 1, 0, 200);
        BSOS_SetLampState(LA_FLASH_GORDON_5, 1, 0, 100);
        BSOS_SetLampState(LA_FLASH_GORDON_6, 1, 0, 200);
        BSOS_SetLampState(LA_FLASH_STROBE, 1, 0, 500);
      }
      MingAttackLamps = 1;
      MingAttackReady = false;
      MingAttackProgress = 0;
      BallInSaucer = false;
    }
  } else if (BallInSaucer==true && MingAttackReady==false) {
    BSOS_PushToTimedSolenoidStack(SO_SAUCER_DOWN, 5, CurrentTime + 1000);
    BallInSaucer = false;
  }

  // handle ming health animation
  if (MingHealth==3) {
    BSOS_SetLampState(LA_MING_TOP, 1, 0, 100);
    BSOS_SetLampState(LA_MING_BOTTOM, 1, 0, 200);
  } else if (MingHealth==2) {
    BSOS_SetLampState(LA_MING_TOP, 1, 0, 200);
    BSOS_SetLampState(LA_MING_BOTTOM, 1, 0, 300);
  } else if (MingHealth==1) {
    BSOS_SetLampState(LA_MING_TOP, 1, 0, 300);
    BSOS_SetLampState(LA_MING_BOTTOM, 1, 0, 400);
  } else if (MingHealth==0) {
    BSOS_SetLampState(LA_MING_TOP, 1, 0, 400);
    BSOS_SetLampState(LA_MING_BOTTOM, 1, 0, 500);
    BSOS_SetLampState(LA_SPINNER_LEFT, 0);
    BSOS_SetLampState(LA_SPINNER_RIGHT, 0);
    BSOS_SetLampState(LA_POP_TOP, 0);
    WizardState = 5;
  }

  // switch hits
  if (BSOS_ReadSingleSwitchState(SW_OUTHOLE)) {
    if (BallTimeInTrough==0) {
      BallTimeInTrough = CurrentTime;
    } else {
      BSOS_SetLampState(LA_MING_TOP, 0);
      BSOS_SetLampState(LA_MING_BOTTOM, 0);
      // Make sure the ball stays on the sensor for at least 
      // 0.5 seconds to be sure that it's not bouncing
      if ((CurrentTime-BallTimeInTrough)>3000) {

        if (BallFirstSwitchHitTime==0) BallFirstSwitchHitTime = CurrentTime;
        
        if (NumTiltWarnings>=MaxTiltWarnings) {
          returnState = MACHINE_STATE_BALL_OVER;
        } else if (NumTiltWarnings<MaxTiltWarnings) {
          if (WizardState==3) {
            returnState = MACHINE_STATE_WIZARD_MODE;
          } else if (WizardState>=4) {
            BSOS_PlaySoundSquawkAndTalk(41);
            returnState = MACHINE_STATE_BALL_OVER;
          }
        }
      }
    }
  } else {
    BallTimeInTrough = 0;
  }

  return returnState;
}
