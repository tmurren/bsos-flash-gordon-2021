/**************************************************************************
 *     This file is part of the RPU OS for Arduino Project.

    I, Dick Hamill, the author of this program disclaim all copyright
    in order to make this program freely available in perpetuity to
    anyone who would like to use it. Dick Hamill, 6/1/2020

    RPU OS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    RPU OS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    See <https://www.gnu.org/licenses/>.
 */

#ifndef RPU_CONFIG_H

/***

  Use this file to set game-specific and hardware-specific parameters

***/
// Plug-in board architectures (0-99 is for J5, 100-199 is for CPU socket)
// Hardware Rev 1 generally uses an Arduino Nano & (optional) 74125
// Hardware Rev 2 uses an Arduino Nano, a 74155, and a 74240
// Hardware Rev 3 uses a MEGA 2560 Pro, and nothing else
// Hardware Rev 4 uses a MEGA 2560 Pro (all the pins) on a larger board (display & WIFI)
// Hardware Rev 100 (different order of magnitude because it's a different approach) plugs into the CPU socket
// Hardware Rev 101 - first RPU CPU interposer release board
// Hardware Rev 102 - second RPU (with display and WIFI socket)
#define RPU_OS_HARDWARE_REV   3

// Available Architectures (0-9 is for B/S Boards, 10-19 is for W)
//  RPU_MPU_ARCHITECTURE 1 = -17, -35, 100, 200, or compatible
//  RPU_MPU_ARCHITECTURE 11 = Sys 4, 6
//  RPU_MPU_ARCHITECTURE 13 = Sys 7
//  RPU_MPU_ARCHITECTURE 15 = Sys 11
#define RPU_MPU_ARCHITECTURE  1

// Some boards will assume a 6800 is the processor (RPU_OS_HARDWARE_REV 1 through 4)
// and some boards will try to detect the processor (RPU_OS_HARDWARE_REV 102)
// but in other cases we can specify if we're building for a 6800.
// Define RPU_MPU_BUILD_FOR_6800 with a 0 for 6802 or 6808, and with
// a 1 for 6800
#define RPU_MPU_BUILD_FOR_6800  1

// These defines allow this configuration to eliminate some functions
// to reduce program size
//#define RPU_OS_USE_DIP_SWITCHES 
#define RPU_OS_USE_S_AND_T
//#define RPU_OS_USE_DASH51
//#define RPU_OS_USE_SB100
//#define RPU_OS_USE_SB300
//#define RPU_OS_USE_WAV_TRIGGER
#define RPU_OS_USE_WAV_TRIGGER_1p3
//#define RPU_OS_DISABLE_CPC_FOR_SPACE
#define RPU_OS_USE_AUX_LAMPS
#define RPU_OS_USE_7_DIGIT_DISPLAYS
#define RPU_OS_USE_6_DIGIT_CREDIT_DISPLAY_WITH_7_DIGIT_DISPLAYS
//#define RPU_USE_EXTENDED_SWITCHES_ON_PB4
//#define RPU_USE_EXTENDED_SWITCHES_ON_PB7
//#define RPU_OS_USE_WTYPE_1_SOUND
//#define RPU_OS_USE_WTYPE_2_SOUND
//#define RPU_OS_USE_W11_SOUND




#if (RPU_MPU_ARCHITECTURE==1) 
/*******************************************************
 * This section is only for games that use the 
 * -17, -35, 100, and 200 MPU boards
 */
// Depending on the number of digits, the RPU_OS_SOFTWARE_DISPLAY_INTERRUPT_INTERVAL
// can be adjusted in order to change the refresh rate of the displays.
// The original -17 / MPU-100 boards ran at 320 Hz 
// The Alltek runs the displays at 440 Hz (probably so 7-digit displays won't flicker)
// The value below is calculated with this formula:
//       Value = (interval in ms) * (16*10^6) / (1*1024) - 1 
//          (must be <65536)
// Choose one of these values (or do whatever)
//  Value         Frequency 
//  48            318.8 Hz
//  47            325.5 Hz
//  46            332.4 Hz increments   (I use this for 6-digits displays)
//  45            339.6 Hz
//  40            381 Hz
//  35            434 Hz     (This would probably be good for 7-digit displays)
//  34            446.4 Hz      
#define RPU_OS_SOFTWARE_DISPLAY_INTERRUPT_INTERVAL  35  
//#define RPU_OS_ADJUSTABLE_DISPLAY_INTERRUPT

#ifdef RPU_OS_USE_6_DIGIT_CREDIT_DISPLAY_WITH_7_DIGIT_DISPLAYS
#define RPU_OS_MASK_SHIFT_1            0x60
#define RPU_OS_MASK_SHIFT_2            0x0C
#else
#define RPU_OS_MASK_SHIFT_1            0x30
#define RPU_OS_MASK_SHIFT_2            0x06
#endif

#ifdef RPU_OS_USE_7_DIGIT_DISPLAYS
#define RPU_OS_MAX_DISPLAY_SCORE  9999999
#define RPU_OS_NUM_DIGITS         7
#define RPU_OS_ALL_DIGITS_MASK    0x7F
#else
#define RPU_OS_MAX_DISPLAY_SCORE  999999
#define RPU_OS_NUM_DIGITS         6
#define RPU_OS_ALL_DIGITS_MASK    0x3F
#endif

#define CONTSOL_DISABLE_FLIPPERS      0x40
#define CONTSOL_DISABLE_COIN_LOCKOUT  0x20

#define RPU_OS_SWITCH_DELAY_IN_MICROSECONDS 200
#define RPU_OS_TIMING_LOOP_PADDING_IN_MICROSECONDS  70

// Fast boards might need a slower lamp strobe
#define RPU_OS_SLOW_DOWN_LAMP_STROBE  0

#ifdef RPU_OS_USE_AUX_LAMPS
#define RPU_NUM_LAMP_BANKS 11
#define RPU_MAX_LAMPS      88
#else
#define RPU_NUM_LAMP_BANKS 8
#define RPU_MAX_LAMPS      60
#endif

// Lamp Board (sort by schematics)
#define LA_BONUS_MINI_1K 0               // 1K MINI BONUS
#define LA_BONUS_MINI_2K 1               // 2K MINI BONUS
#define LA_BONUS_MINI_3K 2               // 3K MINI BONUS
#define LA_BONUS_MINI_4K 3               // 4K MINI BONUS
#define LA_BONUS_MINI_5K 4               // 5K MINI BONUS
#define LA_BONUS_MINI_6K 5               // 6K MINI BONUS
#define LA_BONUS_MINI_7K 6               // 7K MINI BONUS
#define LA_BONUS_MINI_8K 7               // 8K MINI BONUS
#define LA_BONUS_MINI_9K 8               // 9K MINI BONUS
#define LA_BONUS_MINI_10K 9              // 10K MINI BONUS
#define LA_SPINNER_RIGHT 10              // RIGHT SPINNER
#define LA_SPINNER_LEFT 11               // LEFT SPINNER
#define LA_BONUS_SUPER_1K 12             // 1K SUPER BONUS
#define LA_BONUS_SUPER_2K 13             // 2K SUPER BONUS
#define LA_BONUS_SUPER_3K 14             // 3K SUPER BONUS
#define LA_BONUS_SUPER_4K 15             // 4K SUPER BONUS
#define LA_BONUS_SUPER_5K 16             // 5K SUPER BONUS
#define LA_BONUS_SUPER_6K 17             // 6K SUPER BONUS
#define LA_BONUS_SUPER_7K 18             // 7K SUPER BONUS
#define LA_BONUS_SUPER_8K 19             // 8K SUPER BONUS
#define LA_BONUS_SUPER_9K 20             // 9K SUPER BONUS
#define LA_BONUS_SUPER_10K 21            // 10K SUPER BONUS
#define LA_BONUS_MINI_50K 22             // 50K MINI BONUS
#define LA_BONUS_SUPER_100K 23           // 100K SUPER BONUS
#define LA_BONUS_2X 24                   // 2X BONUS
#define LA_BONUS_3X 25                   // 3X BONUS
#define LA_BONUS_4X 26                   // 4X BONUS
#define LA_BONUS_5X 27                   // 5X BONUS
#define LA_DTARGET_ARROW_1 28            // #1 DROP TARGET ARROW
#define LA_DTARGET_ARROW_2 29            // #2 DROP TARGET ARROW
#define LA_DTARGET_ARROW_3 30            // #3 DROP TARGET ARROW
#define LA_DTARGET_BONUS_4X 31           // 4X 3 DROP TARGET
#define LA_DTARGET_4_D 32                // 4 DROP TARGET "A" (BOTTOM)
#define LA_DTARGET_4_C 33                // 4 DROP TARGET "B"
#define LA_DTARGET_4_B 34                // 4 DROP TARGET "C"
#define LA_DTARGET_4_A 35                // 4 DROP TARGET "D" (TOP)
#define LA_TARGET_LRIGHT_BOTTOM 36       // RT. SIDE LOWER TARGET
#define LA_INLANE_RIGHT 37               // FLIPPER FEED LANE (rt.)
#define LA_INLANE_LEFT 38                // FLIPPER FEED LANE (LEFT)
#define LA_TARGET_LRIGHT_TOP 39          // RT. SIDE UPPER TARGET
#define LA_SAME_PLAYER_SHOOTS_AGAIN 40   // SAME PLAYER SHOOTS AGAIN
#define LA_MATCH 41                      // MATCH
#define LA_SHOOT_AGAIN 42                // SHOOT AGAIN - Legacy Variable
#define LAMP_SHOOT_AGAIN 42              // SHOOT AGAIN
#define LAMP_HEAD_SAME_PLAYER_SHOOTS_AGAIN 42 // SHOOT AGAIN
#define LA_CREDIT_INDICATOR 43           // CREDIT INDICATOR
#define LA_SAUCER_10K 44                 // 10K SAUCER
#define LA_SAUCER_20K 45                 // 20K SAUCER - SAUCER_20K
#define LA_SAUCER_XBALL 46               // X-BALL SAUCCER
#define LA_DTARGET_BONUS_5X 47           // 5X 4 DROP TARGET
#define LA_BALL_IN_PLAY 48               // BALL IN PLAY
#define LA_HIGH_SCORE_TO_DATE 49         // HIGH SCORE TO DATE
#define LA_GAME_OVER 50                  // GAME OVER
#define LA_TILT 51                       // TILT
#define LA_POP_TOP 52                    // TOP THUMPER BUMPER
#define LA_TARGET_WOOD_BEAST_XBALL 53    // INLINE LA_DROP TARGET X-BALL
#define LA_SAUCER_30K 54                 // 30K SAUCER
#define LA_TARGET_UPPER_COLLECT_BONUS 55 // TOP TAR. COLLECT BONUS
#define LA_OUTLANE_RIGHT_SPECIAL 56      // RT. OUT SPECIAL
#define LA_OUTLANE_LEFT_SPECIAL 57       // LEFT OUT SPECIAL
#define LA_STAR_PFIELD_BOTTOM 58         // LOWER TOP RT. RO. BUTTON
#define LA_TARGET_UPPER_SPECIAL 59       // TOP TAR. SPECIAL

// Aux Lamp Board (sorted by schematics)
#define LA_STAR_PFIELD_TOP                60 // UPPER TOP RT. RO. BUTTON
#define LA_STAR_SHOOTER_TOP               61 // TOP SHOOTER ALLEY RO. BUTTON
#define LA_STAR_SHOOTER_MIDDLE            62 // MIDDLE SHOOTER ALLEY RO. BUTTON
#define LA_STAR_SHOOTER_BOTTOM            63 // LOWER SHOOTER ALLEY RO. BUTTON
#define LA_FLASH_GORDON_1                 64 // 1 FLASH GORDON
#define LA_FLASH_GORDON_2                 65 // 2 FLASH GORDON
#define LA_FLASH_GORDON_3                 66 // 3 FLASH GORDON
// #define N/A                            67 // not used
#define LA_FLASH_GORDON_4                 68 // 4 FLASH GORDON
#define LA_FLASH_GORDON_5                 69 // 5 FLASH GORDON
#define LA_FLASH_GORDON_6                 70 // 6 FLASH GORDON
// #define N/A                            71 // not used
#define LA_MING_BOTTOM                    72 // FACE OF MING
#define LA_MING_TOP                       73 // FACE OF MING
// #define N/A                            74 // not used
#define LA_FLASH_STROBE                   75 // BACK BOX STROBE
#define LA_CLOCK_15_SECONDS_3X            76 // 3X 15 SECOND CLOCK
#define LA_CLOCK_15_SECONDS_2X            77 // 2X 15 SECOND CLOCK
#define LA_SAUCER_ARROW_3X                78 // 3X SAUCER ARROW
#define LA_SAUCER_ARROW_2X                79 // 2X SAUCER ARROW
// #define N/A                            80 // not used
// #define N/A                            81 // not used
// #define N/A                            82 // not used
// #define N/A                            83 // not used
// #define N/A                            84 // not used
// #define N/A                            85 // not used
// #define N/A                            86 // not used
// #define N/A                            87 // not used

#define CONTSOL_DISABLE_FLIPPERS      0x40
#define CONTSOL_DISABLE_COIN_LOCKOUT  0x20

// This define needs to be set for the number of loops 
// needed to get a delay of 80 us
// So, set it to (0.000080) / (1/Clock Frequency)
// Assuming Frequency = 500kHz,  40 = (0.000080) / (1/500000)
#define RPU_OS_NUM_SWITCH_LOOPS 70
// 60 us
// So, set this to (0.000060) / (1/Clock Frequency)
#define RPU_OS_NUM_LAMP_LOOPS   20 // 30

// Fast boards might need a slower lamp strobe
#define RPU_OS_SLOW_DOWN_LAMP_STROBE  1 // 0

#define RPU_OS_CREDITS_EEPROM_BYTE                  5
#define RPU_OS_TOTAL_PLAYS_EEPROM_START_BYTE        10
#define RPU_OS_HIGHSCORE_EEPROM_START_BYTE          1
#define RPU_OS_AWARD_SCORE_1_EEPROM_START_BYTE      14
#define RPU_OS_AWARD_SCORE_2_EEPROM_START_BYTE      18
#define RPU_OS_AWARD_SCORE_3_EEPROM_START_BYTE      26
#define RPU_OS_TOTAL_REPLAYS_EEPROM_START_BYTE      30
#define RPU_OS_TOTAL_HISCORE_BEATEN_START_BYTE      34
#define RPU_OS_CHUTE_2_COINS_START_BYTE             38
#define RPU_OS_CHUTE_1_COINS_START_BYTE             42
#define RPU_OS_CHUTE_3_COINS_START_BYTE             46
#define RPU_OS_TOTAL_SKILL_1_EEPROM_BYTE            50
#define RPU_OS_TOTAL_SKILL_2_EEPROM_BYTE            54
#define RPU_OS_TOTAL_SKILL_3_EEPROM_BYTE            58
#define RPU_OS_TOTAL_WIZ_EEPROM_BYTE                62
#define RPU_OS_TOTAL_WIZ_BEAT_EEPROM_BYTE           66
#define RPU_OS_SCORE_AWARD_1_EEPROM_START_BYTE      70
#define RPU_OS_SCORE_AWARD_2_EEPROM_START_BYTE      74
#define RPU_OS_SCORE_AWARD_3_EEPROM_START_BYTE      78

#elif (RPU_MPU_ARCHITECTURE > 9) 
/*******************************************************
 * This section is only for games that use the 
 * System 4, 6, 7, 9 or 11 / other MPU boards
 */

#define RPU_NUM_LAMP_BANKS             8
#define RPU_MAX_LAMPS                  64

#define NUM_SWITCH_BYTES                8
#define MAX_NUM_SWITCHES                64
 
#if (RPU_MPU_ARCHITECTURE<13)
#define RPU_OS_MAX_DISPLAY_SCORE  999999
#define RPU_OS_NUM_DIGITS       6
#define RPU_OS_ALL_DIGITS_MASK    0x3F
#else
#define RPU_OS_MAX_DISPLAY_SCORE  9999999
#define RPU_OS_NUM_DIGITS       7
#define RPU_OS_ALL_DIGITS_MASK    0x7F
#endif

#endif


#define RPU_CREDITS_EEPROM_BYTE                   5
#define RPU_HIGHSCORE_EEPROM_START_BYTE           1
#define RPU_AWARD_SCORE_1_EEPROM_START_BYTE       10
#define RPU_AWARD_SCORE_2_EEPROM_START_BYTE       14
#define RPU_AWARD_SCORE_3_EEPROM_START_BYTE       18
#define RPU_TOTAL_PLAYS_EEPROM_START_BYTE         26
#define RPU_TOTAL_REPLAYS_EEPROM_START_BYTE       30
#define RPU_TOTAL_HISCORE_BEATEN_START_BYTE       34
#define RPU_CHUTE_2_COINS_START_BYTE              38
#define RPU_CHUTE_1_COINS_START_BYTE              42
#define RPU_CHUTE_3_COINS_START_BYTE              46
#define RPU_CPC_CHUTE_1_SELECTION_BYTE            50
#define RPU_CPC_CHUTE_2_SELECTION_BYTE            51
#define RPU_CPC_CHUTE_3_SELECTION_BYTE            52

#define RPU_CONFIG_H
#endif
