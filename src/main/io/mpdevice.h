/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdint.h>

#include "io/serial.h"
#include "common/time.h"
#include "fc/rc_modes.h"

#define MPDEVICE_RECEIVED_LENGTH 10
#define MPDEVICE_SEND_LENGTH 10

#define MPDEVICE_STACK_HEADER 0
#define MPDEVICE_STACK_VERSION 1
#define MPDEVICE_STACK_LENGTH 2
#define MPDEVICE_STACK_COMMAND 3
#define MPDEVICE_STACK_ACK 4
#define MPDEVICE_STACK_PARAMETER 5
#define MPDEVICE_STACK_CHECKSUM 7
#define MPDEVICE_STACK_END 9

typedef struct mpdeviceSwitchState_s {
    bool isActivated;
} mpdeviceSwitchState_t;

typedef enum {
    MPDEVICE_KEY_PAUSE    = 0x0E,
    MPDEVICE_KEY_PLAY     = 0x0D,
    MPDEVICE_KEY_NEXT     = 0x01,
    MPDEVICE_KEY_PREV     = 0x02,
    MPDEVICE_KEY_VOL_UP   = 0x04,
    MPDEVICE_KEY_VOL_DOWN = 0x05,
    MPDEVICE_SET_VOL      = 0x06,
    MPDEVICE_SET_EQ       = 0x07,
    MPDEVICE_SET_LOOP_ALL = 0x11,
    MPDEVICE_KEY_UNKNOWN  = 0xFF,
} mpdeviceCommands_e;

// end of Mediaplayer Device definition

void uint16ToArray(uint16_t value, uint8_t *array);
uint16_t calculateCheckSum(uint8_t *buffer);

void mpdeviceInit(void);
bool mpdeviceIsEnabled(void);
void mpdeviceUpdate(timeUs_t currentTimeUs);
void mpdeviceProcess(timeUs_t currentTimeUs);
void mpdeviceSendCommand(mpdeviceCommands_e command, uint16_t parameter);
bool mpdevicePressButton(mpdeviceCommands_e command);
