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

#include <stdbool.h>
#include <stdint.h>

#include "drivers/serial.h"
#include "common/time.h"
#include "fc/rc_modes.h"

#define MPDEVICE_EQ_NORMAL 0
#define MPDEVICE_EQ_POP 1
#define MPDEVICE_EQ_ROCK 2
#define MPDEVICE_EQ_JAZZ 3
#define MPDEVICE_EQ_CLASSIC 4
#define MPDEVICE_EQ_BASS 5

#define MPDEVICE_DEVICE_U_DISK 1
#define MPDEVICE_DEVICE_SD 2
#define MPDEVICE_DEVICE_AUX 3
#define MPDEVICE_DEVICE_SLEEP 4
#define MPDEVICE_DEVICE_FLASH 5

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
    MPDEVICE_KEY_PLAY_PAUSE = 0x0E,
    MPDEVICE_KEY_NEXT       = 0x01,
    MPDEVICE_KEY_PREV       = 0x02,
    MPDEVICE_KEY_VOL_UP     = 0x04,
    MPDEVICE_KEY_VOL_DOWN   = 0x05,
    MPDEVICE_KEY_UNKNOWN    = 0xFF,
} mediaplayerDeviceKeyEvent_e;

// end of Mediaplayer Device definition

typedef struct mediaplayerDevice_s {
    serialPort_t *serialPort;
    bool isReady;
} mediaplayerDevice_t;

extern mediaplayerDevice_t *playerDevice;

void mediaplayerDeviceSendStack(mediaplayerDevice_t *device, uint8_t command);

void uint16ToArray(uint16_t value, uint8_t *array);
uint16_t calculateCheckSum(uint8_t *buffer);

void mpdeviceInit(void);
void mediaplayerDeviceInit(mediaplayerDevice_t *device);
void mpdeviceUpdate(timeUs_t currentTimeUs);

bool mpdeviceIsEnabled(void);
void mpdevicePlayerProcess(void);

// mp3 player button simulation
bool mediaplayerDevicePressButton(mediaplayerDevice_t *device, mediaplayerDeviceKeyEvent_e command);
