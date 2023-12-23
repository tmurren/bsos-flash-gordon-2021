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

#define NUM_OF_SAUCER_LAMPS_UP 30
struct SaucerLampsUp {
   byte lightNumSaucerUp;
   byte rowSaucerUp;
};
struct SaucerLampsUp SaucerLampsUp[] = {
   {LA_BONUS_3X, 1},
   {LA_BONUS_4X, 1},
   {LA_BONUS_2X, 2},
   {LA_BONUS_5X, 2},
   {LA_BONUS_SUPER_6K, 3},
   {LA_BONUS_SUPER_5K, 4},
   {LA_BONUS_SUPER_7K, 4},
   {LA_BONUS_SUPER_4K, 5},
   {LA_BONUS_SUPER_8K, 5},
   {LA_BONUS_SUPER_100K, 6},
   {LA_BONUS_SUPER_3K, 7},
   {LA_BONUS_SUPER_9K, 7},
   {LA_BONUS_SUPER_2K, 8},
   {LA_BONUS_SUPER_10K, 8},
   {LA_BONUS_SUPER_1K, 9},
   {LA_BONUS_MINI_6K, 10},
   {LA_BONUS_MINI_5K, 11},
   {LA_BONUS_MINI_7K, 11},
   {LA_BONUS_MINI_4K, 12},
   {LA_BONUS_MINI_8K, 12},
   {LA_BONUS_MINI_50K, 13},
   {LA_BONUS_MINI_3K, 14},
   {LA_BONUS_MINI_9K, 14},
   {LA_BONUS_MINI_2K, 15},
   {LA_BONUS_MINI_10K, 15},
   {LA_BONUS_MINI_1K, 16},
   {LA_SAUCER_10K,17},
   {LA_SAUCER_20K, 18},
   {LA_SAUCER_30K, 19},
   {LA_SAUCER_XBALL, 20}
};

#define NUM_OF_SAUCER_LAMPS_DOWN 30
struct SaucerLampsDown {
   byte lightNumSaucerDown;
   byte rowSaucerDown;
};
struct SaucerLampsDown SaucerLampsDown[] = {
   {LA_SAUCER_XBALL, 1},
   {LA_SAUCER_30K, 2},
   {LA_SAUCER_20K, 3},
   {LA_SAUCER_10K,4},
   {LA_BONUS_MINI_1K, 5},
   {LA_BONUS_MINI_10K, 6},
   {LA_BONUS_MINI_2K, 6},
   {LA_BONUS_MINI_9K, 7},
   {LA_BONUS_MINI_3K, 7},
   {LA_BONUS_MINI_50K, 8},
   {LA_BONUS_MINI_8K, 9},
   {LA_BONUS_MINI_4K, 9},
   {LA_BONUS_MINI_7K, 10},
   {LA_BONUS_MINI_5K, 10},
   {LA_BONUS_MINI_6K, 11},
   {LA_BONUS_SUPER_1K, 12},
   {LA_BONUS_SUPER_10K, 13},
   {LA_BONUS_SUPER_2K, 13},
   {LA_BONUS_SUPER_9K, 14},
   {LA_BONUS_SUPER_3K, 14},
   {LA_BONUS_SUPER_100K, 15},
   {LA_BONUS_SUPER_8K, 16},
   {LA_BONUS_SUPER_4K, 16},
   {LA_BONUS_SUPER_7K, 17},
   {LA_BONUS_SUPER_5K, 17},
   {LA_BONUS_SUPER_6K, 18},
   {LA_BONUS_5X, 19},
   {LA_BONUS_2X, 19},
   {LA_BONUS_4X, 20},
   {LA_BONUS_3X, 20}
};

#define NUM_OF_ATTRACT_LAMPS_UP 35
struct AttractLampsUp {
   byte lightNumUp;
   byte rowUp;
};
struct AttractLampsUp AttractLampsUp[] = {
   {LA_SHOOT_AGAIN, 1},
   {LA_BONUS_3X, 2},
   {LA_BONUS_4X, 2},
   {LA_BONUS_2X, 3},
   {LA_BONUS_5X, 3},
   {LA_BONUS_SUPER_6K, 4},
   {LA_BONUS_SUPER_5K, 5},
   {LA_BONUS_SUPER_7K, 5},
   {LA_BONUS_SUPER_4K, 6},
   {LA_BONUS_SUPER_8K, 6},
   {LA_BONUS_SUPER_100K, 7},
   {LA_BONUS_SUPER_3K, 8},
   {LA_BONUS_SUPER_9K, 8},
   {LA_BONUS_SUPER_2K, 9},
   {LA_BONUS_SUPER_10K, 9},
   {LA_CLOCK_15_SECONDS_2X, 10},
   {LA_CLOCK_15_SECONDS_3X, 10},
   {LA_BONUS_SUPER_1K, 11},
   {LA_BONUS_MINI_6K, 12},
   {LA_BONUS_MINI_5K, 13},
   {LA_BONUS_MINI_7K, 13},
   {LA_BONUS_MINI_4K, 14},
   {LA_BONUS_MINI_8K, 14},
   {LA_BONUS_MINI_50K, 15},
   {LA_BONUS_MINI_3K, 16},
   {LA_BONUS_MINI_9K, 16},
   {LA_BONUS_MINI_2K, 17},
   {LA_BONUS_MINI_10K, 17},
   {LA_BONUS_MINI_1K, 18},
   {LA_SAUCER_ARROW_2X, 19},
   {LA_SAUCER_ARROW_3X, 19},
   {LA_SAUCER_10K, 20},
   {LA_SAUCER_20K, 21},
   {LA_SAUCER_30K, 22},
   {LA_SAUCER_XBALL, 23}
};

#define NUM_OF_ATTRACT_LAMPS_DOWN 28
struct AttractLampsDown {
   byte lightNumDown;
   byte rowDown;
};
struct AttractLampsDown AttractLampsDown[] = {
   {LA_POP_TOP, 1},
   {LA_STAR_SHOOTER_TOP, 1},
   {LA_TARGET_UPPER_SPECIAL, 2},
   {LA_TARGET_UPPER_COLLECT_BONUS, 3},
   {LA_STAR_SHOOTER_MIDDLE, 3},
   {LA_DTARGET_ARROW_1, 4},
   {LA_DTARGET_ARROW_2, 5},
   {LA_STAR_PFIELD_TOP, 5},
   {LA_STAR_SHOOTER_BOTTOM, 5},
   {LA_DTARGET_ARROW_3, 6},
   {LA_DTARGET_BONUS_4X, 6},
   {LA_STAR_PFIELD_BOTTOM, 7},
   {LA_MING_TOP, 8},
   {LA_MING_BOTTOM, 9},
   {LA_SPINNER_LEFT, 10},
   {LA_SPINNER_RIGHT, 11},
   {LA_DTARGET_4_A, 12},
   {LA_DTARGET_4_B, 13},
   {LA_TARGET_WOOD_BEAST_XBALL, 14},
   {LA_DTARGET_BONUS_5X, 14},
   {LA_DTARGET_4_C, 15},
   {LA_TARGET_LRIGHT_TOP, 16},
   {LA_DTARGET_4_D, 17},
   {LA_TARGET_LRIGHT_BOTTOM, 18},
   {LA_INLANE_LEFT, 19},
   {LA_INLANE_RIGHT, 19},
   {LA_OUTLANE_LEFT_SPECIAL, 20},
   {LA_OUTLANE_RIGHT_SPECIAL, 20}
};

#define NUM_OF_ATTRACT_LAMPS_SIREN 63
struct AttractLampsSiren {
   byte lightNumSiren;
   byte rowSiren;
};
struct AttractLampsSiren AttractLampsSiren[] = {
   {LA_POP_TOP, 1},
   {LA_DTARGET_BONUS_4X, 2},
   {LA_TARGET_UPPER_COLLECT_BONUS, 3},
   {LA_TARGET_UPPER_SPECIAL, 4},
   {LA_DTARGET_ARROW_1, 5},
   {LA_DTARGET_ARROW_2, 6},
   {LA_SAUCER_10K, 6},
   {LA_DTARGET_ARROW_3, 7},
   {LA_SAUCER_20K, 7},
   {LA_SAUCER_30K, 8},
   {LA_SAUCER_XBALL, 9},
   {LA_MING_TOP, 10},
   {LA_MING_BOTTOM, 11},
   {LA_SPINNER_LEFT, 11},
   {LA_DTARGET_4_A, 12},
   {LA_DTARGET_4_B, 13},
   {LA_DTARGET_4_C, 14},
   {LA_DTARGET_BONUS_5X, 14},
   {LA_DTARGET_4_D, 15},
   {LA_SAUCER_ARROW_2X, 15},
   {LA_OUTLANE_LEFT_SPECIAL, 16},
   {LA_INLANE_LEFT, 16},
   {LA_BONUS_MINI_8K, 17},
   {LA_BONUS_MINI_9K, 17},
   {LA_BONUS_MINI_10K, 17},
   {LA_CLOCK_15_SECONDS_2X, 17},
   {LA_BONUS_MINI_7K, 18},
   {LA_BONUS_SUPER_8K, 18},
   {LA_BONUS_SUPER_9K, 18},
   {LA_BONUS_2X, 18},
   {LA_BONUS_SUPER_7K, 19},
   {LA_BONUS_SUPER_10K, 19},
   {LA_BONUS_3X, 19},
   {LA_BONUS_MINI_1K, 20},
   {LA_BONUS_MINI_50K, 20},
   {LA_BONUS_MINI_6K, 20},
   {LA_BONUS_SUPER_1K, 20},
   {LA_BONUS_SUPER_100K, 20},
   {LA_BONUS_SUPER_6K, 20},
   {LA_SHOOT_AGAIN, 20},
   {LA_BONUS_SUPER_2K, 21},
   {LA_BONUS_SUPER_5K, 21},
   {LA_BONUS_4X, 21},
   {LA_BONUS_MINI_5K, 22},
   {LA_BONUS_SUPER_3K, 22},
   {LA_BONUS_SUPER_4K, 22},
   {LA_BONUS_5X, 22},
   {LA_BONUS_MINI_2K, 23},
   {LA_BONUS_MINI_3K, 23},
   {LA_BONUS_MINI_4K, 23},
   {LA_CLOCK_15_SECONDS_3X, 23},
   {LA_INLANE_RIGHT, 24},
   {LA_OUTLANE_RIGHT_SPECIAL, 24},
   {LA_TARGET_LRIGHT_BOTTOM, 25},
   {LA_TARGET_LRIGHT_TOP, 26},
   {LA_TARGET_WOOD_BEAST_XBALL, 27},
   {LA_SAUCER_ARROW_3X, 28},
   {LA_SPINNER_RIGHT, 29},
   {LA_STAR_PFIELD_BOTTOM, 30},
   {LA_STAR_SHOOTER_BOTTOM, 31},
   {LA_STAR_PFIELD_TOP, 32},
   {LA_STAR_SHOOTER_MIDDLE, 32},
   {LA_STAR_SHOOTER_TOP, 33}
};

#define NUM_OF_ATTRACT_LAMPS_MING_ATTACK 60
struct AttractLampsMingAttack {
   byte lightNumMingAttack;
   byte rowMingAttack;
};
struct AttractLampsMingAttack AttractLampsMingAttack[] = {
   {LA_DTARGET_ARROW_3, 1},
   {LA_DTARGET_ARROW_2, 1},
   {LA_DTARGET_BONUS_4X, 2},
   {LA_DTARGET_ARROW_1, 2},
   {LA_SAUCER_XBALL, 2},
   {LA_SPINNER_LEFT, 2},
   {LA_SAUCER_30K, 3},
   {LA_TARGET_UPPER_COLLECT_BONUS, 3},
   {LA_TARGET_UPPER_SPECIAL, 4},
   {LA_SAUCER_20K, 4},
   {LA_SAUCER_10K, 5},
   {LA_DTARGET_4_A, 5},
   {LA_POP_TOP, 6},
   {LA_STAR_PFIELD_TOP, 6},
   {LA_STAR_PFIELD_BOTTOM, 6},
   {LA_SAUCER_ARROW_2X, 6},
   {LA_SAUCER_ARROW_3X, 6},
   {LA_DTARGET_4_B, 6},
   {LA_STAR_SHOOTER_TOP, 7},
   {LA_SPINNER_RIGHT, 7},
   {LA_DTARGET_BONUS_5X, 7},
   {LA_DTARGET_4_C, 7},
   {LA_STAR_SHOOTER_MIDDLE, 8},
   {LA_DTARGET_4_D, 8},
   {LA_BONUS_MINI_1K, 8},
   {LA_BONUS_MINI_10K, 8},
   {LA_STAR_SHOOTER_BOTTOM, 9},
   {LA_TARGET_WOOD_BEAST_XBALL, 9},
   {LA_BONUS_MINI_2K, 9},
   {LA_BONUS_MINI_9K, 9},
   {LA_TARGET_LRIGHT_TOP, 10},
   {LA_BONUS_MINI_3K, 10},
   {LA_BONUS_MINI_8K, 10},
   {LA_BONUS_MINI_50K, 10},
   {LA_TARGET_LRIGHT_BOTTOM, 11},
   {LA_BONUS_MINI_4K, 11},
   {LA_BONUS_MINI_5K, 11},
   {LA_BONUS_MINI_6K, 11},
   {LA_BONUS_MINI_7K, 11},
   {LA_INLANE_LEFT, 11},
   {LA_CLOCK_15_SECONDS_2X, 12},
   {LA_CLOCK_15_SECONDS_3X, 13},
   {LA_OUTLANE_LEFT_SPECIAL, 13},
   {LA_BONUS_SUPER_1K, 13},
   {LA_BONUS_SUPER_2K, 13},
   {LA_BONUS_SUPER_10K, 13},
   {LA_INLANE_RIGHT, 14},
   {LA_BONUS_SUPER_3K, 14},
   {LA_BONUS_SUPER_8K, 14},
   {LA_BONUS_SUPER_9K, 14},
   {LA_BONUS_SUPER_100K, 14},
   {LA_OUTLANE_RIGHT_SPECIAL, 15},
   {LA_BONUS_SUPER_4K, 15},
   {LA_BONUS_SUPER_7K, 15},
   {LA_BONUS_SUPER_5K, 16},
   {LA_BONUS_SUPER_6K, 16},
   {LA_BONUS_2X, 16},
   {LA_BONUS_4X, 17},
   {LA_BONUS_3X, 18},
   {LA_BONUS_5X, 19}
};


#define NUM_OF_SWITCHES             39 // 40 with one not used

#define SW_STARS_PFIELD             0 // 2 LEFT & RIGHT R.O. BUTTONS
#define SW_STARS_SHOOTER_LANE       1 // 3 SHOOTER LANE R.0. BUTTONS
#define SW_DTARGET_1                2 // TOP SINGLE DROP TARGET
#define SW_SHOOTER_LANE_ROLL        3 // SHOOTER LANE ROLLOVER
#define SW_DTARGET_REBOUND          4 // DROP TARGET 50 PPOINT REB. (2)
#define SW_CREDIT_BUTTON            5 // CREDIT BBUTTON
#define SW_TILT                     6 // TILT (3)
#define SW_OUTHOLE                  7 // OUTHOLE
#define SW_COIN_3                   8 // COIN III (RIGHT)
#define SW_COIN_1                   9 // COIN I (LEFT)
#define SW_COIN_2                   10 // COIN II (CENTER)
#define SW_TARGET_LRIGHT_BOTTOM     11 // LOWER RIGHT SIDE TARGET
#define SW_INLANE_R                 12 // FLIP FEED LANE (RIGHT)
#define SW_INLANE_L                 13 // FLIP FEED LANE (LEFT)
#define SW_TARGET_LRIGHT_TOP        14 // UPPER RIGHT SIDE TARGET
#define SW_SLAM                     15 // SLAM (2)
#define SW_DTARGET_4_D              16 // 4 DROP TARGET "A" (BOTTOM)
#define SW_DTARGET_4_C              17 // 4 DROP TARGET "B"
#define SW_DTARGET_4_B              18 // 4 DROP TARGET "C"
#define SW_DTARGET_4_A              19 // 4 DROP TARGET "D" (TOP)
#define SW_DTARGET_3_A              20 // 1 DROP TARGET (TOP)
#define SW_DTARGET_3_B              21 // 2 DROP TARGET (MIDDLE)
#define SW_DTARGET_3_C              22 // 3 DROP TARGET (BOTTOM)
#define SW_TARGET_TOP               23 // TOP TARGGET
#define SW_DTARGET_INLINE_1ST       24 // 1ST INLINE DROP TARGET
#define SW_DTARGET_INLINE_2ND       25 // 2ND INLINE DROP TARGET
#define SW_DTARGET_INLINE_3RD       26 // 3RD INLINE DROP TARGET
#define SW_TARGET_WOOD_BEAST        27 // INLINE BACK TARGET
#define SW_REBOUND                  28 // 10 POINT REBOUND (2)
#define SW_SAUCER                   29 // SAUCER
#define SW_OUTLANE_RIGHT            30 // RIGHT OUTLANE
#define SW_OUTLANE_LEFT             31 // LEFT OUTLANE
#define SW_SPINNER_RIGHT            32 // RIGHT SPINNER
#define SW_SPINNER_LEFT             33 // LEFT SPINNER
#define SW_SLING_RIGHT              34 // RIGHT SLINGSHOT
#define SW_SLING_LEFT               35 // LEFT SLINGSHOT
#define SW_POP_TOP                  36 // TOP THUMPER BUMPER
// #define N/A                      37 // not used
#define SW_POP_RIGHT                38 // RIGHT THUMPER BUMPER
#define SW_POP_LEFT                 39 // LEFT THUMPER BUMPER


// Defines for solenoids
#define SO_DTARGET_4_RESET          0 // 4 DROP TARGET
#define SO_DTARGET_3_RESET          1 // 3 DROP TARGET
#define SO_DTARGET_INLINE_RESET     2 // INLINE DROP TARGET
#define SO_SAUCER_DOWN              3 // SAUCER KICK DOWN
#define SO_KNOCKER                  5 // KNOCKER
// #define N/A                      4 // not used
#define SO_OUTHOLE                  6 // OUTHOLE KICKER
#define SO_SAUCER_UP                7 // SAUCER KICK UP
#define SO_DTARGET_1_RESET          8 // SINGLE DROP TARGET RESET
#define SO_POP_LEFT                 9 // LEFT THUMPER BUMPER
#define SO_POP_RIGHT                10 // RIGHT THUMPER BBUMPER
#define SO_DTARGET_1_DOWN           11 // SINGLE DROP TARGET PULL DOWN
#define SO_POP_TOP                  12 // TOP THUMPER BUMPER
#define SO_SLING_LEFT               13 // LEFT SLINGSHOT
#define SO_SLING_RIGHT              14 // RIGHT SLINGSHOT
// #define SO_COIN_LOCKOUT          ?? // COIN LOCKOUT DOOR
// #define SO_FLIPPER_ENABLE        ?? // KI RELAY (FLIPPER ENAMBLE)
// #define N/A                      ?? // not used
// #define N/A                      ?? // not used


// SWITCHES_WITH_TRIGGERS are for switches that will automatically
// activate a solenoid (like in the case of a chime that rings on a rollover)
// but SWITCHES_WITH_TRIGGERS are fully debounced before being activated
#define NUM_SWITCHES_WITH_TRIGGERS 5

// PRIORITY_SWITCHES_WITH_TRIGGERS are switches that trigger immediately
// (like for pop bumpers or slings) - they are not debounced completely
#define NUM_PRIORITY_SWITCHES_WITH_TRIGGERS 5


// Define automatic solenoid triggers (switch, solenoid, number of 1/120ths of a second to fire)
struct PlayfieldAndCabinetSwitch TriggeredSwitches[] = {
   { SW_SLING_RIGHT, SO_SLING_RIGHT, 4},
   { SW_SLING_LEFT, SO_SLING_LEFT, 4},
   { SW_POP_RIGHT, SO_POP_RIGHT, 3},
   { SW_POP_LEFT, SO_POP_LEFT, 3},
   { SW_POP_TOP, SO_POP_TOP, 3}
};



// Squawk & Talk Timings
byte SoundTimings[52] = { // byte or float
   0,  // 0         off/strange
   0,  // 1         off/strange
   0,  // 2         off/strange
   0,  // 3         off/strange
   0,  // 4  ****** 2.060s humm low (same as 9 ???)
   0,  // 5         sound off
   1,  // 6         background sound 1
   0,  // 7  ****** 0.173s rebound hit
   0,  // 8  ****** 1.550s spinner humm medium
   0,  // 9  ****** 1.971s spinner humm low (same as 4 ???)
   1,  // 10 0.500s or 11.820s (multiply) timer sound 
   0,  // 11        background sound 2
   0,  // 12 ****** 0.411s boink up high
   0,  // 13 ****** 0.467s boink up low
   0,  // 14        background sound 3
   0,  // 15        background sound 4
   2,  // 16 0.400s (multiply) alarm
   6,  // 17 1.360s low grumble (new)
   0,  // 18 3.240s saucer (repeat of 22 & 23)
   0,  // 19        background sound 5
   0,  // 20 ****** 0.844s drop target hit
   0,  // 21 ****** 0.889s beep beep hit
   13, // 22 3.240s saucer (repeat of 18 & 23)
   13, // 23 3.240s saucer OR sound off (repeat of 18 & 22)
   0,  // 24 ****** 0.643s bong bounce down
   0,  // 25 ****** 1.260s bong bounce up
   0,  // 26 ****** 1.760s crash bounce down
   0,  // 27 ****** 1.760s crash bounce up
   0,  // 28 3.638s outlanes
   0,  // 29 ****** 1.497s crash
   0,  // 30        background sound 6 OR sound off
   0,  // 31        background sound 7
   0,  // 32 ****** 0.463s ding 1
   0,  // 33 ****** 0.463s ding 2
   0,  // 34 ****** 0.463s ding 3
   0,  // 35 ****** 0.463s ding 4
   0,  // 36        background sound 8
   0,  // 37        background sound 9
   0,  // 38        background sound 10 OR sound off
   0,  // 39        background sound 11
   15, // 40 3.721s "ignite deathray, 15 seconds" ming
   52, // 41 12.940s ming laugh five times
   10, // 42 2.321s "lucky shot there for you" ming
   8,  // 43 1.830s "miserable earthling" ming OR sound off
   8,  // 44 2.058s "emperor ming awaits" ming
   6,  // 45 1.500s "flash" ming
   10, // 46 2.471s "Trying? Get out, you're embarassing" ming
   8,  // 47 1.856s "15 seconds" ming
   7,  // 48 1.770s "miserable earthling" ming
   6,  // 49 1.500s "flash" ming
   10, // 50 2.527s ming laugh single OR ding ?
   8   // 51 1.803s "15 seconds" ming
};