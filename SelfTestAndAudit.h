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

#ifndef SELF_TEST_H

// tests
#define MACHINE_STATE_TEST_LIGHTS           -1 // advance
#define MACHINE_STATE_TEST_DISPLAYS         -2 // advance
#define MACHINE_STATE_TEST_SOLENOIDS        -3 // advance
#define MACHINE_STATE_TEST_SWITCHES         -4
#define MACHINE_STATE_TEST_SOUNDS           -5 // advance

// stats
#define MACHINE_STATE_TEST_TOTAL_PLAYS      -6
#define MACHINE_STATE_TEST_TOTAL_REPLAYS    -7
#define MACHINE_STATE_TEST_HISCR            -8
#define MACHINE_STATE_TEST_HISCR_BEAT       -9
#define MACHINE_STATE_TEST_TOTAL_SKILL      -10
#define MACHINE_STATE_TEST_TOTAL_WIZARD     -11

// credits
#define MACHINE_STATE_TEST_CREDITS          -12 // adjustment

// score awards
#define MACHINE_STATE_TEST_SCORE_LEVEL_1    -13 // adjustment
#define MACHINE_STATE_TEST_SCORE_LEVEL_2    -14 // adjustment
#define MACHINE_STATE_TEST_SCORE_LEVEL_3    -15 // adjustment

// done
#define MACHINE_STATE_TEST_DONE             -50

// ??? future ???
// #define MACHINE_STATE_TEST_DROP_GOAL // adjustment
// #define MACHINE_STATE_TEST_MING_GOAL // adjustment
// #define MACHINE_STATE_TEST_CHUTE_1_COINS
// #define MACHINE_STATE_TEST_CHUTE_2_COINS
// #define MACHINE_STATE_TEST_CHUTE_3_COINS

unsigned long GetLastSelfTestChangedTime();
void SetLastSelfTestChangedTime(unsigned long setSelfTestChange);
int RunBaseSelfTest(int curState, boolean curStateChanged, unsigned long CurrentTime, byte resetSwitch, byte slamSwitch=0xFF);

unsigned long GetAwardScore(byte level);

#endif
