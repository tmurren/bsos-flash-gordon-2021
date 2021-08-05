/************************************************************************** 
  This file is part of Flash Gordon 2021

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

#include "BallySternOS.h"
#include "FG2021-Base.h"
#include "SelfTestAndAudit.h"
#include <EEPROM.h>

//============================== OPERATOR GAME ADJUSTMENTS ==============================
boolean FreePlay = true;             // false = coin drop, true = free play
#define MAX_CREDITS 25               // (25) max credits
#define BALLS_PER_GAME 3             // (3) balls per game
#define WIZARD_GOAL_DTARGET 8       // (16) set number of drop targets needed to light wizard mode at outlanes
#define WIZARD_GOAL_ATTACK 99       // (140) set attack power needed to light saucer in wizard mode
#define ATTRACT_SPEECH 1             // 0 = no attract speech, 1 = attract speect "Emperor Ming awaits!"
#define ATTRACT_SPEECH_TIMER 300000  // (300000) Ammount of time between attract mode speech callouts. 60000 = 1 minute
#define USE_STROBE 0                 // Strobe use is currently not recommended. Please message me if you would like to help! 0 = don't use backglass strobe, 1 = use backglass strobe
#define DEBUG_MESSAGES  0            // 0 = no serial monitor, 1 = serial monitor for testing
//=====================================================================================

#define VERSION_NUMBER  110

boolean FirstStartup = true;
int MachineState = 0;
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

byte Credits = 0;
unsigned long TotalPlays = 0;
unsigned long HighScore = 0;
unsigned long TotalSkill1 = 0;
unsigned long TotalSkill2 = 0;
unsigned long TotalSkill3 = 0;
unsigned long TotalWizardModes = 0;
unsigned long TotalWizardModesBeat = 0;

byte WholeCredit = 0;

unsigned long AttractStartAnimation = 0;
unsigned long InitGamePlayAnimation = 0;
unsigned long InitNewBallAnimation = 0;
unsigned long SkillShotScoreAnimation = 0;
unsigned long SaucerAnimation = 0;
unsigned long InitWizardModeAnimation = 0;
unsigned long MingAttackAnimation = 0;
unsigned long MingDefeatCelebrationBPM = 0;

byte AttractHeadMode = 255;
byte AttractPlayfieldMode = 255;
unsigned long AttractSweepTime = 0;
unsigned long AttractSirenTime = 0;
byte AttractSweepLights = 1;
byte AttractSirenLights = 1;
unsigned long AttractCalloutTimer = 0;
byte NormalGameplayBackglassMode = 255;

byte CurrentPlayer = 0;
byte CurrentBallInPlay = 1;
byte CurrentNumPlayers = 0;
unsigned long CurrentScores[4];
boolean BallDrained = false;

byte NumTiltWarnings = 0;
#define MAX_TILT_WARNINGS 1

byte SkillShotHits = 0;
byte SkillShotState = 0; // 0 not started, 1 started, 2 collecting, 3 collected/fail

#define TIME_TO_WAIT_FOR_BALL 1000
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
#define DTARGET_4_HURRY_UP_TIMER 12500;
unsigned long WhiteHurryUpStart = 0;
unsigned long AmberHurryUpStart = 0;
// for (byte i = 0; i < 4; i++) Serial.println(F(DTarget4Light[i]));

byte DropTargetCount = 0;
boolean DTargetInlineGoal = false;
boolean DTarget3Goal = false;
boolean DTarget4Goal = false;
byte DTarget3Completions = 0;

boolean LeftSpinnerHurryUpLit = false;
unsigned long LeftSpinnerHurryUpTimer = 0;
boolean RightSpinnerHurryUpLit = false;
unsigned long RightSpinnerHurryUpTimer = 0;
boolean LeftSpinnerLit = false;
boolean RightSpinnerLit = false;
boolean TopPopLit = false;
boolean TopSpecialLit = false;

boolean PlayerShootsAgain = false;
byte WoodBeastXBallState[4] = {0, 0, 0, 0}; // 0 off, 1 lit, 2 collected, 3 used
byte SaucerXBallState[4] = {0, 0, 0, 0}; // ^

byte SaucerLamps = 1;
unsigned long SaucerHitTime = 0;
byte SaucerDirection = 0;
boolean BallInSaucer = false;

byte BonusXState = 1;
#define PLAYFIELD_X_TIMER 15000;
byte Playfield2xState = 0; // 0 off, 1 lit, 2 started
byte Playfield3xState = 0; // ^
unsigned long Playfield2XStart = 0;
unsigned long Playfield3XStart = 0;

#define MAX_MINI_BONUS 19
#define MAX_MINI_DISPLAY_BONUS 19
#define MAX_SUPER_BONUS 19
#define MAX_SUPER_DISPLAY_BONUS 19
byte MiniBonus;
byte SuperBonus;
byte CountdownMini = 0;
byte CountdownSuper = 0;
byte CountdownBonusX = 0;
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

byte WizardState = 0; // 0 unqualified, 1 qualified, 2 collected, 3 initball, 4 started, 5 validated, 6 ming defeated
byte MingAttackProgress = 0;
boolean MingAttackReady = false;
byte MingHealth = 2; // 3
byte MingAttackLamps = 1;
boolean MingDefeatCelebration = false;
byte MingDefeatCelebrationIncrement = 1;

unsigned long CurrentSoundEffectTimer = 0;
// byte squawkAndTalkByte = 0; // S&T


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


  Credits = BSOS_ReadByteFromEEProm(BSOS_CREDITS_EEPROM_BYTE);
  if (Credits>MAX_CREDITS) Credits = MAX_CREDITS;
  TotalPlays = BSOS_ReadULFromEEProm(BSOS_TOTAL_PLAYS_EEPROM_BYTE, 0);
  HighScore = BSOS_ReadULFromEEProm(BSOS_HIGHSCORE_EEPROM_START_BYTE, 1000); // 300000
  TotalSkill1 = BSOS_ReadULFromEEProm(BSOS_TOTAL_SKILL_1_EEPROM_BYTE, 0);
  TotalSkill2 = BSOS_ReadULFromEEProm(BSOS_TOTAL_SKILL_2_EEPROM_BYTE, 0);
  TotalSkill3 = BSOS_ReadULFromEEProm(BSOS_TOTAL_SKILL_3_EEPROM_BYTE, 0);
  TotalWizardModes = BSOS_ReadULFromEEProm(BSOS_TOTAL_WIZ_EEPROM_BYTE, 0);
  TotalWizardModesBeat = BSOS_ReadULFromEEProm(BSOS_TOTAL_WIZ_BEAT_EEPROM_BYTE, 0);


  if (DEBUG_MESSAGES) {
    Serial.println(F("Done with setup"));
  }

}


void PlaySound(byte soundByte) {
  // don't interupt currently playing sound
  if (CurrentTime<CurrentSoundEffectTimer) return;
  // if (BSOS_ReadSingleSwitchState(SW_OUTHOLE) && (soundByte==8 || soundByte==9)) return;
  if (BallDrained==true && (soundByte==7 || soundByte==8 || soundByte==9)) return;
  if ((SuperBonusCollecting==true) && (soundByte!=12)) return;
  if ((MiniBonusCollecting==true) && (soundByte!=12)) return;

  // if no timed sound is being played, play triggered sound
  if (SoundTimings[soundByte]) CurrentSoundEffectTimer = CurrentTime + ((unsigned long)SoundTimings[soundByte] * 250); // 1000 = 1 sec
  BSOS_PlaySoundSquawkAndTalk(soundByte);
}


void AddCredit(byte numToAdd=1) {
  if (Credits<MAX_CREDITS) {
    WholeCredit = WholeCredit + numToAdd;
    if (WholeCredit==1) {
      PlaySound(32);
    } else if (WholeCredit==2) {
      PlaySound(33);
    } else if (WholeCredit==3) {
      PlaySound(34);
    } else if (WholeCredit>=4) {
      PlaySound(35);
      Credits = Credits + 1;
      BSOS_WriteByteToEEProm(BSOS_CREDITS_EEPROM_BYTE, Credits);
      WholeCredit = 0;
    }
  }
  BSOS_SetCoinLockout((Credits<MAX_CREDITS)?false:true);
}


boolean AddPlayer(boolean resetNumPlayers = false) {
  if (Credits<1 && !FreePlay) return false;
  if (resetNumPlayers) CurrentNumPlayers = 0;
  if (CurrentNumPlayers>=4) return false;

  CurrentNumPlayers += 1;
  BSOS_SetDisplay(CurrentNumPlayers-1, 0);
  BSOS_SetDisplayBlank(CurrentNumPlayers-1, 0x60);
  TotalPlays++;
  BSOS_WriteULToEEProm(BSOS_TOTAL_PLAYS_EEPROM_BYTE, TotalPlays);
  if (CurrentNumPlayers >= 2) {
    PlaySound(18); // saucer 0
  }

  if (!FreePlay) {
    Credits -= 1;
    BSOS_WriteByteToEEProm(BSOS_CREDITS_EEPROM_BYTE, Credits);
  }

  return true;
}


int InitNewBall(bool curStateChanged, int ballNum) {

  if (curStateChanged) {
    InitNewBallAnimation = CurrentTime+2058;
    PlaySound(44); // ming awaits
    BSOS_TurnOffAllLamps();
    BSOS_SetDisableFlippers(false);
    BSOS_EnableSolenoidStack();

    BSOS_PushToTimedSolenoidStack(SO_DTARGET_4_RESET, 15, CurrentTime + 500);
    BSOS_PushToTimedSolenoidStack(SO_DTARGET_3_RESET, 15, CurrentTime + 500);
    BSOS_PushToTimedSolenoidStack(SO_DTARGET_INLINE_RESET, 15, CurrentTime + 500);
    BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_DOWN, 15, CurrentTime + 500);

    BallDrained = false;

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

    LeftSpinnerHurryUpLit = false;
    LeftSpinnerHurryUpTimer = 0;
    RightSpinnerHurryUpLit = false;
    RightSpinnerHurryUpTimer = 0;
    LeftSpinnerLit = false;
    RightSpinnerLit = false;
    TopPopLit = false;
    TopSpecialLit = false;

    SkillShotScoreAnimation = 0;
    SaucerAnimation = 0;
    MingAttackAnimation = 0;
    MingDefeatCelebrationBPM = 0;
    MingDefeatCelebrationIncrement = 1;
    InitWizardModeAnimation = 0;

    if (WoodBeastXBallState[CurrentPlayer]==2) {
      WoodBeastXBallState[CurrentPlayer] = 3;
    } else if (WoodBeastXBallState[CurrentPlayer]==1) {
      WoodBeastXBallState[CurrentPlayer] = 0;
    }
    if (SaucerXBallState[CurrentPlayer]==1) {
      SaucerXBallState[CurrentPlayer] = 0;
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

    CountdownMini = 0;
    CountdownSuper = 0;
    CountdownBonusX = 0;
    Super100kCollected = false;
    Mini50kCollected = false;
    SuperBonusReady = false;
    MiniBonusReady = false;

    WizardState = 0;
    MingAttackProgress = 0;
    MingAttackReady = false;
    MingHealth = 3;
    MingDefeatCelebration = false;
 
    // BSOS_SetDisplayCredits(Credits, true);

    // if (BSOS_ReadSingleSwitchState(SW_OUTHOLE)) {
    //   BSOS_PushToTimedSolenoidStack(SO_OUTHOLE, 4, CurrentTime + 100);
    // }
    // BSOS_PushToTimedSolenoidStack(SO_OUTHOLE, 4, CurrentTime + 100);

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
  // if (BSOS_ReadSingleSwitchState(SW_OUTHOLE)) {
  //   return MACHINE_STATE_INIT_NEW_BALL;
  // } else {
  //   //return MACHINE_STATE_NORMAL_GAMEPLAY;
  //   if (CurrentTime>=InitNewBallAnimation) {
  //     PlaySound(6);
  //     return MACHINE_STATE_SKILL_SHOT;
  //   } else {
  //     return MACHINE_STATE_INIT_NEW_BALL;
  //   }
  // }
  if (CurrentTime>=InitNewBallAnimation) {
    PlaySound(6);
    BSOS_PushToTimedSolenoidStack(SO_OUTHOLE, 4, CurrentTime + 100);
    return MACHINE_STATE_SKILL_SHOT;
  } else {
    return MACHINE_STATE_INIT_NEW_BALL;
  }

}


int RunSelfTest(int curState, boolean curStateChanged) {
  int returnState = curState;
  CurrentNumPlayers = 0;
  // Any state that's greater than CHUTE_3 is handled by the Base Self-test code
  // Any that's less, is machine specific, so we handle it here.
  if (curState>=MACHINE_STATE_TEST_TOTAL_WIZ) {
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
    // reset eeproms
    // BSOS_WriteByteToEEProm(BSOS_CREDITS_EEPROM_BYTE, 0);
    // BSOS_WriteULToEEProm(BSOS_TOTAL_PLAYS_EEPROM_BYTE, 0);
    // BSOS_WriteULToEEProm(BSOS_HIGHSCORE_EEPROM_START_BYTE, 0);
    // BSOS_WriteULToEEProm(BSOS_TOTAL_SKILL_1_EEPROM_BYTE, 0);
    // BSOS_WriteULToEEProm(BSOS_TOTAL_SKILL_2_EEPROM_BYTE, 0);
    // BSOS_WriteULToEEProm(BSOS_TOTAL_SKILL_3_EEPROM_BYTE, 0);
    // BSOS_WriteULToEEProm(BSOS_TOTAL_WIZ_EEPROM_BYTE, 0);
    // BSOS_WriteULToEEProm(BSOS_TOTAL_WIZ_BEAT_EEPROM_BYTE, 0);

    if (ATTRACT_SPEECH) AttractCalloutTimer = ATTRACT_SPEECH_TIMER;
    PlaySound(8);
    BSOS_TurnOffAllLamps();
    BSOS_DisableSolenoidStack();
    BSOS_SetDisableFlippers(true);
    if (DEBUG_MESSAGES) {
      Serial.println(F("Entering Attract Mode"));
    }
    for (byte count=0; count<5; count++) {
      BSOS_SetDisplayBlank(count, 0x00);
    }
    if (FirstStartup==true) {
      BSOS_SetDisplay(0, VERSION_NUMBER, true, 2);
      FirstStartup = false;
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

  if (ATTRACT_SPEECH) {
    if (CurrentTime>=AttractCalloutTimer) {
      PlaySound(44);
      AttractCalloutTimer = CurrentTime + ATTRACT_SPEECH_TIMER;
    }
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
        if (AttractSirenLights>33) AttractSirenLights = 0; // 49
      }
      BackglassLampsKnightRider();
      AttractPlayfieldMode = 4;
    }

    byte switchHit;
    while ( (switchHit=BSOS_PullFirstFromSwitchStack())!=SWITCH_STACK_EMPTY ) {
      if (switchHit==SW_CREDIT_BUTTON) {
        if (AddPlayer(true)) returnState = MACHINE_STATE_INIT_GAMEPLAY;
      }
      if (switchHit==SW_COIN_1) {
        if (!FreePlay) {
          AddCredit(2);
          BSOS_SetDisplayCredits(Credits, true);
        }
      }
      if (switchHit==SW_COIN_2) {
        if (!FreePlay) {
          AddCredit(2);
          BSOS_SetDisplayCredits(Credits, true);
        }
      }
      if (switchHit==SW_COIN_3) {
        if (!FreePlay) {
          AddCredit(4);
          BSOS_SetDisplayCredits(Credits, true);
        }
      }
      if (switchHit==SW_SELF_TEST_SWITCH && (CurrentTime-GetLastSelfTestChangedTime())>500) {
        returnState = MACHINE_STATE_TEST_LIGHTS;
        SetLastSelfTestChangedTime(CurrentTime);
      }
      if (switchHit==SW_REBOUND) {
        PlaySound(5);
      }
      // if (switchHit==SW_TARGET_LRIGHT_BOTTOM) { // S&T
      //   PlaySound(5); // sound off
      //   BSOS_PlaySoundSquawkAndTalk(squawkAndTalkByte);
      //   BSOS_SetDisplayCredits(squawkAndTalkByte, true);
      //   squawkAndTalkByte++;
      // }
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
      BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
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
  if (((CurrentTime-LastCountdownReportTime)>25) && (MiniBonusCollecting==true)) { // adjust speed 300
    if (MiniBonus>0) {
      CurrentScores[CurrentPlayer] += 1000;
      MiniBonus -= 1;
      ShowMiniBonusOnLadder(MiniBonus);
      PlaySound(12);
      if (BonusXState>1 && MiniBonus==0) {
        MiniBonus = CountdownMini;
        BonusXState -= 1;
      }
    } else {
      MiniBonusCollecting = false;
      MiniBonusReady = false;
      BonusXState = CountdownBonusX;
    }
    LastCountdownReportTime = CurrentTime;
  }

  // handle super bonus ring collect animation
  if (((CurrentTime-LastCountdownReportTime)>25) && (SuperBonusCollecting==true)) { // adjust speed 300
    if (SuperBonus>0) {
      CurrentScores[CurrentPlayer] += 1000;
      SuperBonus -= 1;
      ShowSuperBonusOnLadder(SuperBonus);
      PlaySound(12);
      if (BonusXState>1 && SuperBonus==0) {
        SuperBonus = CountdownSuper;
        BonusXState -= 1;
      }
    } else {
      SuperBonusCollecting = false;
      SuperBonusReady = false;
      BonusXState = CountdownBonusX;
      PFValidated = false;
    }
    LastCountdownReportTime = CurrentTime;
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
  // }

  // PLAYFIELD X STOP
  if ((Playfield2xState==2 && CurrentTime>=Playfield2XStart) || (Playfield3xState==2 && CurrentTime>=Playfield3XStart)) {
    PlaySound(11); // background sound 2 (restart)
    if (Playfield2xState==2 && CurrentTime>=Playfield2XStart) {
      Playfield2xState = 3;
      BSOS_SetLampState(LA_CLOCK_15_SECONDS_2X, 0);
    }
    if (Playfield3xState==2 && CurrentTime>=Playfield3XStart) {
      Playfield3xState = 3;
      BSOS_SetLampState(LA_CLOCK_15_SECONDS_3X, 0);
    }
  }

  // TARGET HURRY UPS
  if (DTarget4Light[0]==2 && CurrentTime>=WhiteHurryUpStart) {
    PlaySound(17);
    BSOS_SetLampState(LA_DTARGET_4_A, 0);
    BSOS_SetLampState(LA_TARGET_LRIGHT_TOP, 0);
    DTarget4Light[0] = 0;
  }
  if (DTarget4Light[3]==2 && CurrentTime>=AmberHurryUpStart) {
    PlaySound(17);
    BSOS_SetLampState(LA_DTARGET_4_D, 0);
    BSOS_SetLampState(LA_TARGET_LRIGHT_BOTTOM, 0);
    DTarget4Light[3] = 0;
  }


  // SPINNER HURRY UPS
  if (LeftSpinnerHurryUpLit==true) {
    if (CurrentTime>=LeftSpinnerHurryUpTimer) {
      if (LeftSpinnerLit==true) {
        BSOS_SetLampState(LA_SPINNER_LEFT, 1);
      } else {
        BSOS_SetLampState(LA_SPINNER_LEFT, 0);
      }
      LeftSpinnerHurryUpLit = false;
    }
  }

  if (RightSpinnerHurryUpLit==true) {
    if (CurrentTime>=RightSpinnerHurryUpTimer) {
      if (RightSpinnerLit==true) {
        BSOS_SetLampState(LA_SPINNER_RIGHT, 1);
      } else {
        BSOS_SetLampState(LA_SPINNER_RIGHT, 0);
      }
      RightSpinnerHurryUpLit = false;
    }
  }


  // HANDLE SAUCER
  if (BallInSaucer==true) {
    // if (Playfield2xState==1 || Playfield3xState==1) PlaySound(40);
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
        Playfield2XStart = CurrentTime + PLAYFIELD_X_TIMER;
        BSOS_SetLampState(LA_SAUCER_ARROW_2X, 0);
        BSOS_SetLampState(LA_CLOCK_15_SECONDS_2X, 1, 0, 100);
        PlaySound(10);
      }
      if (Playfield3xState==1) {
        Playfield3xState = 2;
        Playfield3XStart = CurrentTime + PLAYFIELD_X_TIMER;
        BSOS_SetLampState(LA_SAUCER_ARROW_3X, 0);
        BSOS_SetLampState(LA_CLOCK_15_SECONDS_3X, 1, 0, 100);
        PlaySound(10);
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
        BallDrained = true;
        PlaySound(5); // sound off
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
        } else if (NumTiltWarnings>=MAX_TILT_WARNINGS) {
          returnState = MACHINE_STATE_BALL_OVER;
        } else if (NumTiltWarnings<MAX_TILT_WARNINGS) {
          returnState = MACHINE_STATE_COUNTDOWN_BONUS;
        }
      }
    }
  } else {
    BallTimeInTrough = 0;
  }

  return returnState;
}


int InitGamePlay(boolean curStateChanged) {
  int returnState = MACHINE_STATE_INIT_GAMEPLAY;

  if (curStateChanged) {
    InitGameStartTime = CurrentTime;
    BSOS_SetCoinLockout((Credits>=MAX_CREDITS)?true:false);
    BSOS_SetDisableFlippers(true);
    BSOS_DisableSolenoidStack();
    BSOS_TurnOffAllLamps();
    BSOS_SetDisplayBallInPlay(1);
    InitGamePlayAnimation = CurrentTime+3240;
    PlaySound(18); // saucer 0
    BSOS_EnableSolenoidStack();
    BSOS_PushToSolenoidStack(SO_SAUCER_DOWN, 5, true);

    // Set up general game variables
    CurrentNumPlayers = 1;
    CurrentPlayer = 0;
    CurrentBallInPlay = 1;
    PlayerShootsAgain = false;
    for (byte i = 0; i < 3; i++) WoodBeastXBallState[i] = 0;
    for (byte i = 0; i < 3; i++) SaucerXBallState[i] = 0;
    for (byte count=0; count<4; count++) CurrentScores[count] = 0;

    for (byte count=0; count<4; count++) {
      BSOS_SetDisplay(count, 0);
      BSOS_SetDisplayBlank(count, 0x00);
    }
  }

  BackglassLampsLeft2Right();

  if (CurrentTime>=InitGamePlayAnimation) {
    BSOS_EnableSolenoidStack();
    BSOS_SetDisableFlippers(false);
    returnState = MACHINE_STATE_INIT_NEW_BALL;
    // if the ball is in the outhole, then we can move on
    // if (BSOS_ReadSingleSwitchState(SW_OUTHOLE)) {
    //   if (DEBUG_MESSAGES) {
    //     Serial.println(F("Ball is in trough - starting new ball"));
    //   }
    //   BSOS_EnableSolenoidStack();
    //   BSOS_SetDisableFlippers(false);
    //   returnState = MACHINE_STATE_INIT_NEW_BALL;
    // } else {

    //   if (DEBUG_MESSAGES) {
    //     Serial.println(F("Ball is not in trough - firing stuff and giving it a chance to come back"));
    //   }

    //   // And then set a base time for when we can continue
    //   InitGameStartTime = CurrentTime;
    // }

    // // Wait for TIME_TO_WAIT_FOR_BALL seconds, or until the ball appears
    // // The reason to bail out after TIME_TO_WAIT_FOR_BALL is just
    // // in case the ball is already in the shooter lane.
    // if ((CurrentTime-InitGameStartTime)>TIME_TO_WAIT_FOR_BALL || BSOS_ReadSingleSwitchState(SW_OUTHOLE)) {
    //   BSOS_EnableSolenoidStack();
    //   BSOS_SetDisableFlippers(false);
    //   returnState = MACHINE_STATE_INIT_NEW_BALL;
    // }
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
        
      if (CurrentBallInPlay>BALLS_PER_GAME) {
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

  if (NumTiltWarnings<MAX_TILT_WARNINGS) {
    while ( (switchHit=BSOS_PullFirstFromSwitchStack())!=SWITCH_STACK_EMPTY ) {

      switch (switchHit) {
        case SW_SELF_TEST_SWITCH:
          returnState = MACHINE_STATE_TEST_LIGHTS;
          SetLastSelfTestChangedTime(CurrentTime);
          break;
        case SW_STARS_PFIELD:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (PFValidated==false) {
              BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
              PFValidated = true;
            }
            PlaySound(13);
            AddToScore(1000);
            AddToMiniBonus(1);
            if (curState==MACHINE_STATE_SKILL_SHOT) {
              SkillShotState = 3;
            }
          } else {
            PlaySound(24); // wizsound bong bounce down
            if (WizardState==4) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 5;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
        case SW_STARS_SHOOTER_LANE:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (curState==MACHINE_STATE_NORMAL_GAMEPLAY) {
              if (SuperBonusReady==true) {
                PlaySound(13);
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
              PlaySound(13);
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
              PlaySound(29);
              DropTargetHit();
            }
          } else  {
            PlaySound(24); // wizsound bong bounce down
            if (WizardState==5) {
              // BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
        case SW_SHOOTER_LANE_ROLL:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (curState==MACHINE_STATE_NORMAL_GAMEPLAY) {
              PlaySound(18);
              if (SuperBonusReady!=true) {
                if (PFValidated==true) {
                  BSOS_PushToSolenoidStack(SO_DTARGET_1_DOWN, 5, true);
                  PFValidated = false;
                }
              } else {
                SuperBonusCollecting = true;
                CountdownSuper = SuperBonus;
                CountdownBonusX = BonusXState;
                LastCountdownReportTime = CurrentTime;
              }
            } else if (curState==MACHINE_STATE_SKILL_SHOT) {
              if (SkillShotState==0) PlaySound(12);
              if (SkillShotState==1) {
                PlaySound(42); // wizsound lucky shot earthling
                SkillShotScoreAnimation = CurrentTime+500;
                SkillShotState = 2;
                if (SkillShotHits==1 || SkillShotHits==2) {
                  CurrentScores[CurrentPlayer] += 15000; // 10000
                  TotalSkill1++;
                  BSOS_WriteULToEEProm(BSOS_TOTAL_SKILL_1_EEPROM_BYTE, TotalSkill1);
                } else if (SkillShotHits==3 || SkillShotHits==4) {
                  CurrentScores[CurrentPlayer] += 30000; // 25000
                  TotalSkill2++;
                  BSOS_WriteULToEEProm(BSOS_TOTAL_SKILL_2_EEPROM_BYTE, TotalSkill2);
                } else if (SkillShotHits==5) {
                  CurrentScores[CurrentPlayer] += 75000; // 50000
                  TotalSkill3++;
                  BSOS_WriteULToEEProm(BSOS_TOTAL_SKILL_3_EEPROM_BYTE, TotalSkill3);
                }
              }
            }
          } else {
            PlaySound(26); // wizsound crash bounce up
          }
          break;
        case SW_DTARGET_REBOUND:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (PFValidated==false) {
              BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
              PFValidated = true;
            }
            PlaySound(7);
            AddToScore(50);
            if (curState==MACHINE_STATE_SKILL_SHOT) {
              SkillShotState = 3;
              PlaySound(13);
              // PFValidated = true;
            }
          } else {
            PlaySound(24); // wizsound bong bounce down
            if (WizardState==4) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 5;
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
          if (!FreePlay) {
            AddCredit(2);
            BSOS_SetDisplayCredits(Credits, true);
          }
          break;
        case SW_COIN_2:
          if (!FreePlay) {
            AddCredit(2);
            BSOS_SetDisplayCredits(Credits, true);
          }
          break;
        case SW_COIN_3:
          if (!FreePlay) {
            AddCredit(4);
            BSOS_SetDisplayCredits(Credits, true);
          }
          break;
        case SW_TARGET_LRIGHT_BOTTOM:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            // HURRY UP
            if (DTarget4Light[3]==0 || DTarget4Light[3]==1) {
              PlaySound(26);
              AddToScore(5000);
              TargetLowerBottomBlip = CurrentTime + 100;
              BSOS_SetLampState(LA_TARGET_LRIGHT_BOTTOM, 1);
            } else if (DTarget4Light[3]==2) {
              PlaySound(45);
              AddToScore(50000);
              AmberHurryUpStart = 0;
              DTarget4Light[3] = 3;
              BSOS_SetLampState(LA_DTARGET_4_D, 1);
              BSOS_SetLampState(LA_TARGET_LRIGHT_BOTTOM, 1);
              CheckHurryUpCompletion();
            } else if (DTarget4Light[3]==3) {
              PlaySound(27);
              AddToScore(10000);
            }
            AddToSuperBonus(1);
          } else  {
            PlaySound(24); // wizsound bong bounce down
          }
          break;
        case SW_INLANE_R:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            // HURRY UP
            if (DTarget4Light[2]==0 || DTarget4Light[2]==1) {
              PlaySound(26);
              AddToScore(5000);
              InlaneRightBlip = CurrentTime + 100;
              BSOS_SetLampState(LA_INLANE_RIGHT, 1);
            } else if (DTarget4Light[2]==2) {
              PlaySound(27);
              AddToScore(25000);
              DTarget4Light[2] = 3;
              BSOS_SetLampState(LA_DTARGET_4_C, 1);
              BSOS_SetLampState(LA_INLANE_RIGHT, 1);
              LeftSpinnerHurryUpLit = true;
              LeftSpinnerHurryUpTimer = CurrentTime + 2000;
              BSOS_SetLampState(LA_SPINNER_LEFT, 1, 0, 100);
              CheckHurryUpCompletion();
            } else if (DTarget4Light[2]==3) {
              PlaySound(27);
              AddToScore(10000);
            }
          } else {
            PlaySound(24); // wizsound bong bounce down
          }
          break;
        case SW_INLANE_L:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            // HURRY UP
            if (DTarget4Light[1]==0 || DTarget4Light[1]==1) {
              PlaySound(26);
              AddToScore(5000);
              InlaneLeftBlip = CurrentTime + 100;
              BSOS_SetLampState(LA_INLANE_LEFT, 1);
            } else if (DTarget4Light[1]==2) {
              PlaySound(27);
              AddToScore(25000);
              DTarget4Light[1] = 3;
              BSOS_SetLampState(LA_DTARGET_4_B, 1);
              BSOS_SetLampState(LA_INLANE_LEFT, 1);
              RightSpinnerHurryUpLit = true;
              RightSpinnerHurryUpTimer = CurrentTime + 2000;
              BSOS_SetLampState(LA_SPINNER_RIGHT, 1, 0, 100);
              CheckHurryUpCompletion();
            } else if (DTarget4Light[1]==3) {
              PlaySound(27);
              AddToScore(10000);
            }
          } else {
            PlaySound(24); // wizsound bong bounce down
          }
          break;
        case SW_TARGET_LRIGHT_TOP:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            // HURRY UP
            if (DTarget4Light[0]==0 || DTarget4Light[0]==1) {
              PlaySound(26);
              AddToScore(5000);
              TargetLowerTopBlip = CurrentTime + 100;
              BSOS_SetLampState(LA_TARGET_LRIGHT_TOP, 1);
            } else if (DTarget4Light[0]==2) {
              PlaySound(45);
              AddToScore(50000);
              WhiteHurryUpStart = 0;
              DTarget4Light[0] = 3;
              BSOS_SetLampState(LA_DTARGET_4_A, 1);
              BSOS_SetLampState(LA_TARGET_LRIGHT_TOP, 1);
              CheckHurryUpCompletion();
            } else if (DTarget4Light[0]==3) {
              PlaySound(27);
              AddToScore(10000);
            }
            AddToSuperBonus(1);
          } else {
            PlaySound(24); // wizsound bong bounce down
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
                PlaySound(24);
                AddToScore(5000);
              } else if (DTarget4Light[0]==1) {
                WhiteHurryUpStart = CurrentTime + DTARGET_4_HURRY_UP_TIMER;
                BSOS_SetLampState(LA_TARGET_LRIGHT_TOP, 1, 0, 100);
                DTarget4Light[0] = 2;
                PlaySound(47);
                AddToScore(10000);
              } else if (DTarget4Light[0]==2 || DTarget4Light[0]==3) {
                PlaySound(25);
                AddToScore(10000);
              }
            }
            if (switchHit==SW_DTARGET_4_B) {
              if (DTarget4Light[1]==0) {
                PlaySound(24);
                AddToScore(5000);
              } else if (DTarget4Light[1]==1) {
                BSOS_SetLampState(LA_INLANE_LEFT, 1, 0, 250);
                DTarget4Light[1] = 2;
                PlaySound(25);
                AddToScore(10000);
              } else if (DTarget4Light[1]==2 || DTarget4Light[1]==3) {
                PlaySound(25);
                AddToScore(10000);
              }
            }
            if (switchHit==SW_DTARGET_4_C) {
              if (DTarget4Light[2]==0) {
                PlaySound(24);
                AddToScore(5000);
              } else if (DTarget4Light[2]==1) {
                BSOS_SetLampState(LA_INLANE_RIGHT, 1, 0, 250);
                DTarget4Light[2] = 2;
                PlaySound(25);
                AddToScore(10000);
              } else if (DTarget4Light[2]==2 || DTarget4Light[2]==3) {
                PlaySound(25);
                AddToScore(10000);
              }
            }
            if (switchHit==SW_DTARGET_4_D) {
              if (DTarget4Light[3]==0) {
                PlaySound(24);
                AddToScore(5000);
              } else if (DTarget4Light[3]==1) {
                AmberHurryUpStart = CurrentTime + DTARGET_4_HURRY_UP_TIMER;
                BSOS_SetLampState(LA_TARGET_LRIGHT_BOTTOM, 1, 0, 100);
                DTarget4Light[3] = 2;
                PlaySound(47);
                AddToScore(10000);
              } else if (DTarget4Light[3]==2 || DTarget4Light[3]==3) {
                PlaySound(25);
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
            PlaySound(24); // wizsound bong bounce down
            // BSOS_PushToTimedSolenoidStack(SO_DTARGET_4_RESET, 15, CurrentTime);
          }
          break;
        case SW_DTARGET_3_A:
        case SW_DTARGET_3_B:
        case SW_DTARGET_3_C:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (PFValidated==false) {
              BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
              PFValidated = true;
            }
            AddToMiniBonus(1);
            DropTargetHit();
            if (switchHit==SW_DTARGET_3_A) {
              if (DTarget3Lights[0]==0 && DTarget3Flashing==LA_DTARGET_ARROW_1) {
                BSOS_SetLampState(LA_DTARGET_ARROW_1, 1);
                DTarget3Lights[0] = 1;
                PlaySound(24);
                AddToScore(10000);
              } else if (DTarget3Lights[0]==1) {
                PlaySound(25);
                AddToScore(10000);
              } else {
                PlaySound(25);
                AddToScore(5000);
              }
            }
            if (switchHit==SW_DTARGET_3_B) {
              if (DTarget3Lights[1]==0 && DTarget3Flashing==LA_DTARGET_ARROW_2) {
                BSOS_SetLampState(LA_DTARGET_ARROW_2, 1);
                DTarget3Lights[1] = 1;
                PlaySound(24);
                AddToScore(10000);
              } else if (DTarget3Lights[1]==1) {
                PlaySound(25);
                AddToScore(10000);
              } else {
                PlaySound(25);
                AddToScore(5000);
              }
            }
            if (switchHit==SW_DTARGET_3_C) {
              if (DTarget3Lights[2]==0 && DTarget3Flashing==LA_DTARGET_ARROW_3) {
                BSOS_SetLampState(LA_DTARGET_ARROW_3, 1);
                DTarget3Lights[2] = 1;
                PlaySound(24);
                AddToScore(10000);
              } else if (DTarget3Lights[2]==1) {
                PlaySound(25);
                AddToScore(10000);
              } else {
                PlaySound(25);
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
              PlaySound(42);
              BSOS_SetLampState(LA_SAUCER_ARROW_2X, 1, 0, 150);
              BSOS_SetLampState(LA_BONUS_MINI_50K, 1);
            }
            if (curState==MACHINE_STATE_SKILL_SHOT) {
              SkillShotState = 3;
              PlaySound(13);
              // PFValidated = true;
            }
          } else  {
            PlaySound(24); // wizsound bong bounce down
            // BSOS_PushToTimedSolenoidStack(SO_DTARGET_3_RESET, 15, CurrentTime);
            if (WizardState==4) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 5;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
        case SW_TARGET_TOP:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (PFValidated==false) {
              BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
              PFValidated = true;
            }
            AddToScore(5000);
            AddToMiniBonus(1);
            if (MiniBonusReady==false && TopSpecialLit==false) {
              TargetUpperBlip = CurrentTime + 100;
              BSOS_SetLampState(LA_TARGET_UPPER_SPECIAL, 1);
              BSOS_SetLampState(LA_TARGET_UPPER_COLLECT_BONUS, 1);
            }
            if (MiniBonusReady==true) {
              BSOS_SetLampState(LA_TARGET_UPPER_COLLECT_BONUS, 0);
              MiniBonusCollecting = true;
              CountdownMini = MiniBonus;
              CountdownBonusX = BonusXState;
              LastCountdownReportTime = CurrentTime;
            }
            if (TopSpecialLit==true) {
              AddToScore(50000);
              BSOS_SetLampState(LA_TARGET_UPPER_SPECIAL, 0);
              AddCredit(4);
              BSOS_PushToSolenoidStack(SO_KNOCKER, 5, true);
              TopSpecialLit = false;
            }
            if (curState==MACHINE_STATE_SKILL_SHOT) {
              SkillShotState = 3;
              PlaySound(13);
              // PFValidated = true;
            } else {
              PlaySound(27);
            }
          } else  {
            PlaySound(24); // wizsound bong bounce down
            if (WizardState==4) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 5;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
        case SW_DTARGET_INLINE_1ST:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            AddToScore(10000);
            AddToSuperBonus(2);
            DropTargetHit();
            PlaySound(25);
          } else {
            PlaySound(24); // wizsound bong bounce down
            // BSOS_PushToTimedSolenoidStack(SO_DTARGET_INLINE_RESET, 15, CurrentTime);
          }
          break;
        case SW_DTARGET_INLINE_2ND:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            AddToScore(10000);
            DropTargetHit();
            PlaySound(25);
            if (BonusXState==1) {
              BonusXState = 2;
              ShowExtraBonusLights();
            }
          } else {
            PlaySound(24); // wizsound bong bounce down
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
            PlaySound(25);
            if (WoodBeastXBallState[CurrentPlayer]==0) {
              WoodBeastXBallState[CurrentPlayer] = 1;
              BSOS_SetLampState(LA_TARGET_WOOD_BEAST_XBALL, 1);
            }
            DropTargetHit();
            if (BonusXState==2) {
              BonusXState = 3;
              ShowExtraBonusLights();
            }
          } else {
            PlaySound(24); // wizsound bong bounce down
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
            PlaySound(27);
            AddToScore(25000);
          } else {
            PlaySound(24); // wizsound bong bounce down
          }
          break;
        case SW_REBOUND:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (PFValidated==false) {
              BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
              PFValidated = true;
            }
            PlaySound(7);
            AddToScore(10);
            if (curState==MACHINE_STATE_SKILL_SHOT) {
              SkillShotState = 3;
              PlaySound(13);
              // PFValidated = true;
            }
          } else {
            PlaySound(24); // wizsound bong bounce down
            if (WizardState==4) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 5;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
        case SW_SAUCER:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (PFValidated==false) {
              BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
              PFValidated = true;
            }
            if (SaucerHitTime==0 || (CurrentTime-SaucerHitTime)>500) {
              SaucerHitTime = CurrentTime;
              if (Playfield2xState==1 || Playfield3xState==1) {
                SaucerAnimation = CurrentTime+3770;
                PlaySound(40);
              } else {
                SaucerAnimation = CurrentTime+1650;
                PlaySound(22);
              }
              if (DTarget4Goal==true) AddToScore(10000);
              if (DTargetInlineGoal==true) AddToScore(20000);
              if (DTarget3Goal==true) AddToScore(30000);
              if (SaucerXBallState[CurrentPlayer]==1) {
                SaucerXBallState[CurrentPlayer] = 2;
                PlayerShootsAgain = true;
                BSOS_SetLampState(LA_SAUCER_XBALL, 0);
                BSOS_SetLampState(LA_SHOOT_AGAIN, 1);
                BSOS_SetLampState(LA_SAME_PLAYER_SHOOTS_AGAIN, 1);
              }
              DTarget4Goal = false;
              DTargetInlineGoal = false;
              DTarget3Goal = false;
              BSOS_SetLampState(LA_SAUCER_10K, 0);
              BSOS_SetLampState(LA_SAUCER_20K, 0);
              BSOS_SetLampState(LA_SAUCER_30K, 0);
              BallInSaucer = true;
            }
            if (curState==MACHINE_STATE_SKILL_SHOT) {
              SkillShotState = 3;
              // PFValidated = true;
            }
          } else {
            if (WizardState==4) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 5;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
            if (SaucerHitTime==0 || (CurrentTime-SaucerHitTime)>500) {
              SaucerHitTime = CurrentTime;
              if (MingAttackReady==false) {
                AddToScore(3500);
                MingAttackProgress += 35;
                if (MingAttackProgress>=WIZARD_GOAL_ATTACK && MingAttackReady==false) {
                  MingAttackReady = true;
                } else {
                  PlaySound(25); // wizsound crash bounce up
                }
              }
              if (MingAttackReady==true) {
                MingHealth--;
                if (MingHealth==0) {
                  PlaySound(5);
                  PlaySound(48); // wizsound defeat miserable earthling
                  BSOS_PushToTimedSolenoidStack(SO_SAUCER_DOWN, 5, CurrentTime+1600);
                  BSOS_PushToTimedSolenoidStack(SO_KNOCKER, 5, CurrentTime+1600);
                  MingAttackAnimation = CurrentTime+2475;
                } else {
                  PlaySound(5);
                  PlaySound(42); // wizsound lucky shot earthling
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
            if (WizardState==0) PlaySound(28);
            if (WizardState==1) {
              WizardState = 2;
              PlaySound(5); // sound off
              PlaySound(50);
            }
          } else {
            if (WizardState==5) PlaySound(50); // wiz sound outlane
          }
          BallDrained = true;
          break;
        case SW_OUTLANE_LEFT:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            AddToScore(2000);
            AddToSuperBonus(2);
            if (WizardState==0) PlaySound(28);
            if (WizardState==1) {
              WizardState = 2;
              PlaySound(5); // sound off
              PlaySound(50);
            }
          } else {
            if (WizardState==5) PlaySound(50); // wiz sound outlane
          }
          BallDrained = true;
          break;
        case SW_SPINNER_RIGHT:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            // if (PFValidated==false) {
            //   BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
            //   PFValidated = true;
            // }
            if (RightSpinnerLit==false) {
              PlaySound(8);
              AddToScore(100);
            } else {
              PlaySound(9);
              AddToScore(1000);
            }
            if (RightSpinnerHurryUpLit==true) {
              RightSpinnerHurryUpTimer = CurrentTime + 500;
              AddToScore(2000);
            }
            if (curState==MACHINE_STATE_SKILL_SHOT) {
              SkillShotState = 3;
              // PFValidated = true;
            }
          } else {
            AddToScore(1000);
            MingAttackProgress++;
            if (MingAttackProgress>=WIZARD_GOAL_ATTACK && MingAttackReady==false) {
              PlaySound(45); // wizsound flash
              MingAttackReady = true;
            } else {
              PlaySound(7); // wizsound rebound hit
            }
            if (WizardState==4) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 5;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
        case SW_SPINNER_LEFT:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            // if (PFValidated==false) {
            //   BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
            //   PFValidated = true;
            // }
            if (LeftSpinnerLit==false) {
              PlaySound(8);
              AddToScore(100);
            } else {
              PlaySound(9);
              AddToScore(1000);
            }
            if (LeftSpinnerHurryUpLit==true) {
              LeftSpinnerHurryUpTimer = CurrentTime + 500;
              AddToScore(2000);
            }
            if (curState==MACHINE_STATE_SKILL_SHOT) {
              SkillShotState = 3;
              PlaySound(13);
              // PFValidated = true;
            }
          } else {
            AddToScore(1000);
            MingAttackProgress++;
            if (MingAttackProgress>=WIZARD_GOAL_ATTACK && MingAttackReady==false) {
              PlaySound(45); // wizsound flash
              MingAttackReady = true;
            } else {
              PlaySound(7); // wizsound rebound hit
            }
            if (WizardState==4) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 5;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
        case SW_SLING_RIGHT:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            AddToScore(50);
            PlaySound(21);
          } else {
            PlaySound(24); // wizsound bong bounce down
          }
          break;
        case SW_SLING_LEFT:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            AddToScore(50);
            PlaySound(21);
          } else {
            PlaySound(24); // wizsound bong bounce down
          }
          break;
        case SW_POP_TOP:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            if (PFValidated==false) {
              BSOS_PushToSolenoidStack(SO_DTARGET_1_RESET, 5, true);
              PFValidated = true;
            }
            if (TopPopLit==false) {
              PlaySound(20);
              AddToScore(1000);
            } else {
              PlaySound(13);
              AddToScore(5000);
            }
            if (curState==MACHINE_STATE_SKILL_SHOT) {
              SkillShotState = 3;
              PlaySound(13);
              // PFValidated = true;
            }
          } else {
            AddToScore(2000);
            MingAttackProgress += 20;
            if (MingAttackProgress>=WIZARD_GOAL_ATTACK && MingAttackReady==false) {
              PlaySound(45); // wizsound flash
              MingAttackReady = true;
            } else {
              PlaySound(27); // wizsound crash bounce up
            }
            if (WizardState==4) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 5;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
        case SW_POP_RIGHT:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            PlaySound(20);
            AddToScore(1000);
          } else  {
            AddToScore(2000);
            MingAttackProgress += 20;
            if (MingAttackProgress>=WIZARD_GOAL_ATTACK && MingAttackReady==false) {
              PlaySound(45); // wizsound flash
              MingAttackReady = true;
            } else {
              PlaySound(27); // wizsound crash bounce up
            }
            if (WizardState==4) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 5;
              BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_RESET, 15, CurrentTime);
            }
          }
          break;
        case SW_POP_LEFT:
          if (curState!=MACHINE_STATE_WIZARD_MODE) {
            PlaySound(20);
            AddToScore(1000);
          } else  {
            AddToScore(2000);
            MingAttackProgress += 20;
            if (MingAttackProgress>=WIZARD_GOAL_ATTACK && MingAttackReady==false) {
              MingAttackReady = true;
              PlaySound(45); // wizsound flash
              MingAttackReady = true;
            } else {
              PlaySound(27); // wizsound crash bounce up
            }
            if (WizardState==4) {
              BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 0);
              BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 0);
              WizardState = 5;
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
          if (!FreePlay) {
            AddCredit(2);
            BSOS_SetDisplayCredits(Credits, true);
          }
          break;
        case SW_COIN_2:
          if (!FreePlay) {
            AddCredit(2);
            BSOS_SetDisplayCredits(Credits, true);
          }
          break;
        case SW_COIN_3:
          if (!FreePlay) {
            AddCredit(4);
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
  if (MachineState<0) {
    newMachineState = RunSelfTest(MachineState, MachineStateChanged);
  } else if (MachineState==MACHINE_STATE_ATTRACT) {
    newMachineState = RunAttractMode(MachineState, MachineStateChanged);
  } else {
    newMachineState = RunGamePlayMode(MachineState, MachineStateChanged);
  }
  // if (MachineState==MACHINE_STATE_ATTRACT) {
  //   newMachineState = RunAttractMode(MachineState, MachineStateChanged);
  // } else {
  //   newMachineState = RunGamePlayMode(MachineState, MachineStateChanged);
  // }

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
  if (USE_STROBE) {
    byte lampPhase = (CurrentTime/115)%4;
    BSOS_SetLampState(LA_FLASH_GORDON_6, lampPhase==3||lampPhase==0, lampPhase==0);
    BSOS_SetLampState(LA_FLASH_GORDON_1, lampPhase==3||lampPhase==0, lampPhase==0);
    BSOS_SetLampState(LA_FLASH_GORDON_5, lampPhase==2||lampPhase==3, lampPhase==3);
    BSOS_SetLampState(LA_FLASH_GORDON_2, lampPhase==2||lampPhase==3, lampPhase==3);
    BSOS_SetLampState(LA_FLASH_GORDON_4, lampPhase==1||lampPhase==2, lampPhase==2);
    BSOS_SetLampState(LA_FLASH_GORDON_3, lampPhase==1||lampPhase==2, lampPhase==2);
    if (MachineState!=MACHINE_STATE_NORMAL_GAMEPLAY || (MachineState==MACHINE_STATE_NORMAL_GAMEPLAY && BallInSaucer==true)) BSOS_SetLampState(LA_FLASH_STROBE, lampPhase<2, lampPhase==1);
  } else {
    byte lampPhase = (CurrentTime/115)%3;
    BSOS_SetLampState(LA_FLASH_GORDON_6, lampPhase==2||lampPhase==0, lampPhase==0);
    BSOS_SetLampState(LA_FLASH_GORDON_1, lampPhase==2||lampPhase==0, lampPhase==0);
    BSOS_SetLampState(LA_FLASH_GORDON_5, lampPhase==1||lampPhase==2, lampPhase==2);
    BSOS_SetLampState(LA_FLASH_GORDON_2, lampPhase==1||lampPhase==2, lampPhase==2);
    BSOS_SetLampState(LA_FLASH_GORDON_4, lampPhase<2, lampPhase==1);
    BSOS_SetLampState(LA_FLASH_GORDON_3, lampPhase<2, lampPhase==1);
  }
}

void BackglassLampsKnightRider() {
  if (USE_STROBE) {
    byte lampPhase = (CurrentTime/80)%14;
    BSOS_SetLampState(LA_FLASH_GORDON_6, lampPhase==6||lampPhase==7, 0, 0);
    BSOS_SetLampState(LA_FLASH_GORDON_5, lampPhase==5||lampPhase==8, 0, 0);
    BSOS_SetLampState(LA_FLASH_GORDON_4, lampPhase==4||lampPhase==9, 0, 0);
    if (MachineState!=MACHINE_STATE_NORMAL_GAMEPLAY) BSOS_SetLampState(LA_FLASH_STROBE, lampPhase==3||lampPhase==10, 0, 0);
    BSOS_SetLampState(LA_FLASH_GORDON_3, lampPhase==2||lampPhase==11, 0, 0);
    BSOS_SetLampState(LA_FLASH_GORDON_2, lampPhase==1||lampPhase==12, 0, 0);
    BSOS_SetLampState(LA_FLASH_GORDON_1, lampPhase==0||lampPhase==13, 0, 0);
  } else {
    byte lampPhase = (CurrentTime/80)%12;
    BSOS_SetLampState(LA_FLASH_GORDON_6, lampPhase==5||lampPhase==6, 0, 0);
    BSOS_SetLampState(LA_FLASH_GORDON_5, lampPhase==4||lampPhase==7, 0, 0);
    BSOS_SetLampState(LA_FLASH_GORDON_4, lampPhase==3||lampPhase==8, 0, 0);
    BSOS_SetLampState(LA_FLASH_GORDON_3, lampPhase==2||lampPhase==9, 0, 0);
    BSOS_SetLampState(LA_FLASH_GORDON_2, lampPhase==1||lampPhase==10, 0, 0);
    BSOS_SetLampState(LA_FLASH_GORDON_1, lampPhase==0||lampPhase==11, 0, 0);
  }
}

void BackglassLampsLeft2Right() {
  if (USE_STROBE) {
    byte lampPhase = (CurrentTime/85)%4;
    BSOS_SetLampState(LA_FLASH_GORDON_6, lampPhase==3||lampPhase==0, lampPhase==0);
    BSOS_SetLampState(LA_FLASH_GORDON_3, lampPhase==3||lampPhase==0, lampPhase==0);
    BSOS_SetLampState(LA_FLASH_GORDON_5, lampPhase==2||lampPhase==3, lampPhase==3);
    BSOS_SetLampState(LA_FLASH_GORDON_2, lampPhase==2||lampPhase==3, lampPhase==3);
    BSOS_SetLampState(LA_FLASH_GORDON_4, lampPhase==1||lampPhase==2, lampPhase==2);
    BSOS_SetLampState(LA_FLASH_GORDON_1, lampPhase==1||lampPhase==2, lampPhase==2);
    if (MachineState!=MACHINE_STATE_NORMAL_GAMEPLAY) BSOS_SetLampState(LA_FLASH_STROBE, lampPhase<2, lampPhase==1);
  } else {
    byte lampPhase = (CurrentTime/95)%3;
    BSOS_SetLampState(LA_FLASH_GORDON_6, lampPhase==2||lampPhase==0, lampPhase==0);
    BSOS_SetLampState(LA_FLASH_GORDON_3, lampPhase==2||lampPhase==0, lampPhase==0);
    BSOS_SetLampState(LA_FLASH_GORDON_5, lampPhase==1||lampPhase==2, lampPhase==2);
    BSOS_SetLampState(LA_FLASH_GORDON_2, lampPhase==1||lampPhase==2, lampPhase==2);
    BSOS_SetLampState(LA_FLASH_GORDON_4, lampPhase<2, lampPhase==1);
    BSOS_SetLampState(LA_FLASH_GORDON_1, lampPhase<2, lampPhase==1);
  }
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
  if (Playfield3xState==2 && Playfield2xState==2) {
    scoreAmount = scoreAmount * 5;
  } else if (Playfield2xState==2) {
    scoreAmount = scoreAmount * 2;
  } else if (Playfield3xState==2) {
    scoreAmount = scoreAmount * 3;
  }
  CurrentScores[CurrentPlayer] += scoreAmount;
}


void TiltHit() {
  NumTiltWarnings += 1;
  if (NumTiltWarnings>=MAX_TILT_WARNINGS) {
    PlaySound(5);
    BSOS_DisableSolenoidStack();
    BSOS_SetDisableFlippers(true);
    BSOS_TurnOffAllLamps();
    if (Credits) BSOS_SetLampState(LA_CREDIT_INDICATOR, 1);
    BSOS_SetLampState(LA_TILT, 1);
    PlaySound(50);
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
      if (USE_STROBE) BSOS_SetLampState(LA_FLASH_STROBE, 1, 0, 125);
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
    if (USE_STROBE) BSOS_SetLampState(LA_FLASH_STROBE, 0);
    returnState = MACHINE_STATE_NORMAL_GAMEPLAY;
    for (byte count=0; count<CurrentNumPlayers; count++) {
      BSOS_SetDisplay(count, CurrentScores[count], true, 2);
    }
  }

  return returnState;
}


void DropTargetHit() {
  DropTargetCount++;
  if (DropTargetCount==WIZARD_GOAL_DTARGET) {
    BSOS_SetLampState(LA_OUTLANE_RIGHT_SPECIAL, 1);
    BSOS_SetLampState(LA_OUTLANE_LEFT_SPECIAL, 1);
    BSOS_SetLampState(LA_MING_TOP, 1, 0, 100);
    BSOS_SetLampState(LA_MING_BOTTOM, 1, 0, 200);
    WizardState = 1;
    PlaySound(44);
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
    PlaySound(48);
    BSOS_SetLampState(LA_SAUCER_ARROW_3X, 1, 0, 150);
    BSOS_SetLampState(LA_BONUS_SUPER_100K, 1);
  }
}

void CheckSaucerDTargetGoal() {
  if (DTarget4Goal==true && DTarget3Goal==true && DTargetInlineGoal==true && WoodBeastXBallState[CurrentPlayer]==3 && SaucerXBallState[CurrentPlayer]==0) {
    BSOS_SetLampState(LA_SAUCER_XBALL, 1);
    SaucerXBallState[CurrentPlayer] = 1;
  }
}


void AddToMiniBonus(byte bonusAddition) {
  MiniBonus += bonusAddition;
  if (MiniBonus>MAX_MINI_BONUS) MiniBonus = MAX_MINI_BONUS;
}

void AddToSuperBonus(byte bonusAddition) {
  SuperBonus += bonusAddition;
  if (SuperBonus>MAX_SUPER_BONUS) SuperBonus = MAX_SUPER_BONUS;
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
  if (bonus>MAX_MINI_DISPLAY_BONUS) bonus = MAX_MINI_DISPLAY_BONUS;
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
  if (bonus>MAX_SUPER_DISPLAY_BONUS) bonus = MAX_SUPER_DISPLAY_BONUS;
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
  
//  if (bonus>MAX_SUPER_DISPLAY_BONUS) bonus = MAX_SUPER_DISPLAY_BONUS;
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

    CountdownMini = MiniBonus;
    CountdownSuper = SuperBonus;
    CountdownBonusX = BonusXState;

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
      CurrentScores[CurrentPlayer] += 1000;
      MiniBonus -= 1;
      ShowMiniBonusOnLadder(MiniBonus);
      PlaySound(12);
      if (BonusXState>1 && MiniBonus==0) {
        MiniBonus = CountdownMini;
        BonusXState -= 1;
      }
      if ((BonusXState==1) && (MiniBonus==0)) BonusXState = CountdownBonusX;
    } else if (SuperBonus>0) {
      CurrentScores[CurrentPlayer] += 1000;
      SuperBonus -= 1;
      ShowSuperBonusOnLadder(SuperBonus);
      PlaySound(12);
      if (BonusXState>1 && SuperBonus==0) {
        SuperBonus = CountdownSuper;
        BonusXState -= 1;
      }
    } else if (BonusCountDownEndTime==0xFFFFFFFF) {
      BonusCountDownEndTime = CurrentTime+1000;
      BSOS_SetLampState(LA_BONUS_MINI_1K, 0);
      BSOS_SetLampState(LA_BONUS_SUPER_1K, 0);
    }
    LastCountdownReportTime = CurrentTime;
    BSOS_SetDisplay(CurrentPlayer, CurrentScores[CurrentPlayer], true, 2);
  }

  if (CurrentTime>BonusCountDownEndTime) { 
    BonusCountDownEndTime = 0xFFFFFFFF;
    PlaySound(8);
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
      AddCredit(12);
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

  if (NumMatchSpins<10) {
    if (CurrentTime > (MatchSequenceStartTime + MatchDelay)) {
      MatchDigit += 1;
      if (MatchDigit>9) MatchDigit = 0;
      PlaySound(28);
      BSOS_SetDisplayBallInPlay((int)MatchDigit*10);
      MatchDelay += 200 + 4*NumMatchSpins;
      NumMatchSpins += 1;
      BSOS_SetLampState(LA_MATCH, NumMatchSpins%2, 0);

      if (NumMatchSpins==10) {
        BSOS_SetLampState(LA_MATCH, 0);
        MatchDelay = CurrentTime - MatchSequenceStartTime;
      }
    }
  }

  if (NumMatchSpins>=10 && NumMatchSpins<=13) {
    if (CurrentTime>(MatchSequenceStartTime + MatchDelay)) {
      if ( (CurrentNumPlayers>(NumMatchSpins-10)) && ((CurrentScores[NumMatchSpins-10]/10)%10)==MatchDigit) {
        ScoreMatches |= (1<<(NumMatchSpins-10));
        AddCredit(4);
        BSOS_PushToTimedSolenoidStack(SO_KNOCKER, 3, CurrentTime, true);
        MatchDelay += 1000;
        NumMatchSpins += 1;
        BSOS_SetLampState(LA_MATCH, 1);
      } else {
        NumMatchSpins += 1;
      }
      if (NumMatchSpins==14) {
        MatchDelay += 5000;
      }
    }      
  }

  if (NumMatchSpins>13) {
    if (CurrentTime>(MatchSequenceStartTime + MatchDelay)) {
      PlaySound(5); // sound off
      PlaySound(46);
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
    InitWizardModeAnimation = CurrentTime+1770;
    TotalWizardModes++;
    BSOS_WriteULToEEProm(BSOS_TOTAL_WIZ_EEPROM_BYTE, TotalWizardModes);
    BallDrained = false;
    PlaySound(48); // wizsound miserable earthling
    WizardState = 3;
    AddToScore(50000);
    BSOS_TurnOffAllLamps();
    BSOS_PushToTimedSolenoidStack(SO_DTARGET_1_DOWN, 15, CurrentTime);
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
    BSOS_PushToTimedSolenoidStack(SO_DTARGET_4_RESET, 15, CurrentTime);
    BSOS_PushToTimedSolenoidStack(SO_DTARGET_3_RESET, 15, CurrentTime + 250);
    BSOS_PushToTimedSolenoidStack(SO_DTARGET_INLINE_RESET, 15, CurrentTime + 500);
    // BSOS_PushToTimedSolenoidStack(SO_OUTHOLE, 4, CurrentTime + 100);
  }

  // validate wizard mode
  if (WizardState<4) {
    BSOS_SetDisplayFlash(CurrentPlayer, CurrentScores[CurrentPlayer], CurrentTime, 250, 2);
    BackglassLampsLeft2Right();
  } else {  
    BSOS_SetDisplay(CurrentPlayer, CurrentScores[CurrentPlayer], true, 2);
  }

  // handle lamps // commented numbers based on 140 ming attack
  if (WizardState==5) {
    if (MingAttackProgress==0 || MingAttackProgress<=WIZARD_GOAL_ATTACK/4-1) { // 0-34
      BSOS_SetLampState(LA_FLASH_GORDON_5, 0);
      BSOS_SetLampState(LA_FLASH_GORDON_2, 0);
      BSOS_SetLampState(LA_FLASH_GORDON_4, 0);
      BSOS_SetLampState(LA_FLASH_GORDON_3, 0);
      if (USE_STROBE) BSOS_SetLampState(LA_FLASH_STROBE, 0);

      if (MingAttackProgress==0 || MingAttackProgress<=(WIZARD_GOAL_ATTACK/4)/3-1) { // 0-10 : 11
        if (MingHealth!=0) {
          BSOS_SetLampState(LA_SAUCER_10K, 1, 0, 250);
          BSOS_SetLampState(LA_FLASH_GORDON_6, 1, 0, 250);
          BSOS_SetLampState(LA_FLASH_GORDON_1, 1, 0, 250);
        }
      } else if (MingAttackProgress>=(WIZARD_GOAL_ATTACK/4)/3 && MingAttackProgress<=((WIZARD_GOAL_ATTACK/4)/3)*2-1) { // 11-22 : 12
        BSOS_SetLampState(LA_SAUCER_10K, 1, 0, 180);
        BSOS_SetLampState(LA_FLASH_GORDON_6, 1, 0, 180);
        BSOS_SetLampState(LA_FLASH_GORDON_1, 1, 0, 180);
      } else if (MingAttackProgress>=((WIZARD_GOAL_ATTACK/4)/3)*2 && MingAttackProgress<=WIZARD_GOAL_ATTACK/4-1) { // 23-34 : 12
        BSOS_SetLampState(LA_SAUCER_10K, 1, 0, 110);
        BSOS_SetLampState(LA_FLASH_GORDON_6, 1, 0, 110);
        BSOS_SetLampState(LA_FLASH_GORDON_1, 1, 0, 110);
      }

    } else if (MingAttackProgress>=WIZARD_GOAL_ATTACK/4 && MingAttackProgress<=(WIZARD_GOAL_ATTACK/4)*2-1) { // 35-69
      BSOS_SetLampState(LA_SAUCER_10K, 1);
      BSOS_SetLampState(LA_FLASH_GORDON_6, 1);
      BSOS_SetLampState(LA_FLASH_GORDON_1, 1);

      if (MingAttackProgress>=WIZARD_GOAL_ATTACK/4 && MingAttackProgress<=(WIZARD_GOAL_ATTACK/4)+(((WIZARD_GOAL_ATTACK/4)/3)-1)) { // 35-45
        BSOS_SetLampState(LA_SAUCER_20K, 1, 0, 250);
        BSOS_SetLampState(LA_FLASH_GORDON_5, 1, 0, 250);
        BSOS_SetLampState(LA_FLASH_GORDON_2, 1, 0, 250);
      } else if (MingAttackProgress>=(WIZARD_GOAL_ATTACK/4)+((WIZARD_GOAL_ATTACK/4)/3) && MingAttackProgress<=(WIZARD_GOAL_ATTACK/4)+((((WIZARD_GOAL_ATTACK/4)/3)*2)-1)) { // 46-57
        BSOS_SetLampState(LA_SAUCER_20K, 1, 0, 180);
        BSOS_SetLampState(LA_FLASH_GORDON_5, 1, 0, 180);
        BSOS_SetLampState(LA_FLASH_GORDON_2, 1, 0, 180);
      } else if (MingAttackProgress>=(WIZARD_GOAL_ATTACK/4)+(((WIZARD_GOAL_ATTACK/4)/3)*2) && MingAttackProgress<=(((WIZARD_GOAL_ATTACK/4)*2)-1)) { // 58-69
        BSOS_SetLampState(LA_SAUCER_20K, 1, 0, 110);
        BSOS_SetLampState(LA_FLASH_GORDON_5, 1, 0, 110);
        BSOS_SetLampState(LA_FLASH_GORDON_2, 1, 0, 110);
      }

    } else if (MingAttackProgress>=(WIZARD_GOAL_ATTACK/4)*2 && MingAttackProgress<=(WIZARD_GOAL_ATTACK/4)*3-1) { // 70-104
      BSOS_SetLampState(LA_SAUCER_10K, 1);
      BSOS_SetLampState(LA_SAUCER_20K, 1);
      BSOS_SetLampState(LA_FLASH_GORDON_5, 1);
      BSOS_SetLampState(LA_FLASH_GORDON_2, 1);

      if (MingAttackProgress>=(WIZARD_GOAL_ATTACK/4)*2 && MingAttackProgress<=((WIZARD_GOAL_ATTACK/4)*2)+((WIZARD_GOAL_ATTACK/4)/3-1)) { // 70-80
        BSOS_SetLampState(LA_SAUCER_30K, 1, 0, 250);
        BSOS_SetLampState(LA_FLASH_GORDON_4, 1, 0, 250);
        BSOS_SetLampState(LA_FLASH_GORDON_3, 1, 0, 250);
      } else if (MingAttackProgress>=((WIZARD_GOAL_ATTACK/4)*2)+((WIZARD_GOAL_ATTACK/4)/3) && MingAttackProgress<=((WIZARD_GOAL_ATTACK/4)*2)+(((WIZARD_GOAL_ATTACK/4)/3)*2-1)) { // 81-92
        BSOS_SetLampState(LA_SAUCER_30K, 1, 0, 180);
        BSOS_SetLampState(LA_FLASH_GORDON_4, 1, 0, 180);
        BSOS_SetLampState(LA_FLASH_GORDON_3, 1, 0, 180);
      } else if (MingAttackProgress<=((WIZARD_GOAL_ATTACK/4)*2)+(((WIZARD_GOAL_ATTACK/4)/3)*2) && MingAttackProgress<=((WIZARD_GOAL_ATTACK/4)*3-1)) { // 91-104
        BSOS_SetLampState(LA_SAUCER_30K, 1, 0, 110);
        BSOS_SetLampState(LA_FLASH_GORDON_4, 1, 0, 110);
        BSOS_SetLampState(LA_FLASH_GORDON_3, 1, 0, 110);
      }

    } else if (MingAttackProgress>=(WIZARD_GOAL_ATTACK/4*3) && MingAttackProgress<=((WIZARD_GOAL_ATTACK/4)*4-1)) { // 105-139
      BSOS_SetLampState(LA_SAUCER_10K, 1);
      BSOS_SetLampState(LA_SAUCER_20K, 1);
      BSOS_SetLampState(LA_SAUCER_30K, 1);
      BSOS_SetLampState(LA_FLASH_GORDON_4, 1);
      BSOS_SetLampState(LA_FLASH_GORDON_3, 1);

      if (MingAttackProgress>=(WIZARD_GOAL_ATTACK/4)*3 && MingAttackProgress<=((WIZARD_GOAL_ATTACK/4)*3)+((WIZARD_GOAL_ATTACK/4)/3-1)) { // 105-115
        BSOS_SetLampState(LA_SAUCER_XBALL, 1, 0, 250);
        if (USE_STROBE) BSOS_SetLampState(LA_FLASH_STROBE, 1, 0, 250);
      } else if (MingAttackProgress>=((WIZARD_GOAL_ATTACK/4)*3)+((WIZARD_GOAL_ATTACK/4)/3) && MingAttackProgress<=((WIZARD_GOAL_ATTACK/4)*3)+(((WIZARD_GOAL_ATTACK/4)/3)*2-1)) { // 116-127
        BSOS_SetLampState(LA_SAUCER_XBALL, 1, 0, 180);
        if (USE_STROBE) BSOS_SetLampState(LA_FLASH_STROBE, 1, 0, 180);
      } else if (MingAttackProgress<=((WIZARD_GOAL_ATTACK/4)*3)+(((WIZARD_GOAL_ATTACK/4)/3)*2) && MingAttackProgress<=((WIZARD_GOAL_ATTACK/4)*3)+((WIZARD_GOAL_ATTACK/4)-1)) { // 128-139
        BSOS_SetLampState(LA_SAUCER_XBALL, 1, 0, 110);
        if (USE_STROBE) BSOS_SetLampState(LA_FLASH_STROBE, 1, 0, 110);
      }

    } else if (MingAttackProgress>=WIZARD_GOAL_ATTACK && BallInSaucer==false) { // 140
      MingAttackProgress = WIZARD_GOAL_ATTACK;
      // MingAttackReady = true;
      // BSOS_SetLampState(LA_SAUCER_XBALL, 1);

      byte lampPhaseSaucer = (CurrentTime/150)%4; // 250
      BSOS_SetLampState(LA_SAUCER_XBALL, lampPhaseSaucer==3, lampPhaseSaucer==0, lampPhaseSaucer==0);
      BSOS_SetLampState(LA_SAUCER_30K, lampPhaseSaucer==2||lampPhaseSaucer==3, lampPhaseSaucer==3);
      BSOS_SetLampState(LA_SAUCER_20K, lampPhaseSaucer==1||lampPhaseSaucer==2, lampPhaseSaucer==2);
      BSOS_SetLampState(LA_SAUCER_10K, lampPhaseSaucer<2, lampPhaseSaucer==1);
      if (USE_STROBE) {
        byte lampPhaseFlash = (CurrentTime/150)%4; // 250
        BSOS_SetLampState(LA_FLASH_STROBE, lampPhaseFlash==3, lampPhaseFlash==0, lampPhaseFlash==0);
        BSOS_SetLampState(LA_FLASH_GORDON_4, lampPhaseFlash==2||lampPhaseFlash==3, lampPhaseFlash==3);
        BSOS_SetLampState(LA_FLASH_GORDON_3, lampPhaseFlash==2||lampPhaseFlash==3, lampPhaseFlash==3);
        BSOS_SetLampState(LA_FLASH_GORDON_5, lampPhaseFlash==1||lampPhaseFlash==2, lampPhaseFlash==2);
        BSOS_SetLampState(LA_FLASH_GORDON_2, lampPhaseFlash==1||lampPhaseFlash==2, lampPhaseFlash==2);
        BSOS_SetLampState(LA_FLASH_GORDON_6, lampPhaseFlash<2, lampPhaseFlash==1);
        BSOS_SetLampState(LA_FLASH_GORDON_1, lampPhaseFlash<2, lampPhaseFlash==1);
      } else {
        byte lampPhaseFlash = (CurrentTime/150)%3; // 250
        BSOS_SetLampState(LA_FLASH_GORDON_4, lampPhaseFlash==2||lampPhaseFlash==0, lampPhaseFlash==0);
        BSOS_SetLampState(LA_FLASH_GORDON_3, lampPhaseFlash==2||lampPhaseFlash==3, lampPhaseFlash==3);
        BSOS_SetLampState(LA_FLASH_GORDON_5, lampPhaseFlash==1||lampPhaseFlash==2, lampPhaseFlash==2);
        BSOS_SetLampState(LA_FLASH_GORDON_2, lampPhaseFlash==1||lampPhaseFlash==2, lampPhaseFlash==2);
        BSOS_SetLampState(LA_FLASH_GORDON_6, lampPhaseFlash<2, lampPhaseFlash==1);
        BSOS_SetLampState(LA_FLASH_GORDON_1, lampPhaseFlash<2, lampPhaseFlash==1);
      }
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
        if (USE_STROBE) BSOS_SetLampState(LA_FLASH_STROBE, 1, 0, 500);
        TotalWizardModesBeat++;
        BSOS_WriteULToEEProm(BSOS_TOTAL_WIZ_BEAT_EEPROM_BYTE, TotalWizardModesBeat);
        MingDefeatCelebration = true;
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
    WizardState = 6;
  }

  // handle ming defeat celebration sound
  if (MingDefeatCelebration==true) {
    if (CurrentTime>=MingDefeatCelebrationBPM) {
      MingDefeatCelebrationBPM = CurrentTime + 182; // 329 from 182 bpm
      if (MingDefeatCelebrationIncrement==1) {
        PlaySound(32); // wizsound ding 1
      } else if (MingDefeatCelebrationIncrement==2) {
        PlaySound(33); // wizsound ding 2
      } else if (MingDefeatCelebrationIncrement==3) {
        PlaySound(34); // wizsound ding 3
      } else if (MingDefeatCelebrationIncrement==4) {
        PlaySound(33); // wizsound ding 2
      } else if (MingDefeatCelebrationIncrement==5) {
        PlaySound(35); // wizsound ding 4
      }
      MingDefeatCelebrationIncrement += 1;
      if (MingDefeatCelebrationIncrement > 5) MingDefeatCelebrationIncrement = 1;
    }
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
        
        if (NumTiltWarnings>=MAX_TILT_WARNINGS) {
          returnState = MACHINE_STATE_BALL_OVER;
        } else if (NumTiltWarnings<MAX_TILT_WARNINGS) {
          if (WizardState==3) {
            if (CurrentTime>=InitWizardModeAnimation) {
              PlaySound(6);
              BSOS_PushToTimedSolenoidStack(SO_OUTHOLE, 4, CurrentTime + 100);
              WizardState = 4;
            }
            returnState = MACHINE_STATE_WIZARD_MODE;
          } else if (WizardState>=5) {
            PlaySound(5);
            BallDrained = true;
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