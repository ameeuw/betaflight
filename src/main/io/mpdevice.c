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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"
#include "pg/rx.h"

#include "common/crc.h"
#include "common/maths.h"
#include "common/streambuf.h"
#include "fc/rc_controls.h"
#include "fc/runtime_config.h"
#include "io/mpdevice.h"

#include "cms/cms.h"
#include "io/beeper.h"

#include "rx/rx.h"

#include "drivers/time.h"

#include "io/serial.h"

#include "mpdevice.h"

#ifdef USE_MPDEVICE

#include "build/debug.h"

static mediaplayerDevice_t mediaplayerDevice;
mediaplayerDevice_t *playerDevice = &mediaplayerDevice;
mpdeviceSwitchState_t mpSwitchStates[BOXMEDIAPLAYERVOLD - BOXMEDIAPLAYERPLAY  + 1];

bool mpdeviceIsEnabled(void)
{
  return playerDevice->serialPort != NULL;
}

void mpdeviceUpdate(timeUs_t currentTimeUs)
{
  // deviceReceive(currentTimeUs);
  mpdevicePlayerProcess();
}

void mpdevicePlayerProcess(void) {
  for (boxId_e i = BOXMEDIAPLAYERPLAY; i <= BOXMEDIAPLAYERVOLD; i++) {
    uint8_t switchIndex = i - BOXMEDIAPLAYERPLAY;

    if (IS_RC_MODE_ACTIVE(i)) {
      // check last state of this mode, if it's true, then ignore it.
      // Here is a logic to make a toggle control for this mode
      if (mpSwitchStates[switchIndex].isActivated) {
        continue;
      }

      uint8_t command = MPDEVICE_KEY_UNKNOWN;

      switch (i) {
      case BOXMEDIAPLAYERPLAY:
        command = MPDEVICE_KEY_PLAY_PAUSE;
        break;

      case BOXMEDIAPLAYERNEXT:
        command = MPDEVICE_KEY_NEXT;
        break;

      case BOXMEDIAPLAYERPREV:
        command = MPDEVICE_KEY_PREV;
        break;

      case BOXMEDIAPLAYERVOLU:
        command = MPDEVICE_KEY_VOL_UP;

        break;

      case BOXMEDIAPLAYERVOLD:
        command = MPDEVICE_KEY_VOL_DOWN;
        break;
      default:
          break;
      }

      if (command != MPDEVICE_KEY_UNKNOWN) {
        // DEBUG_SET(DEBUG_MPDEVICE, 0, command);

        mediaplayerDevicePressButton(playerDevice, command);
        mpSwitchStates[switchIndex].isActivated = true;
      }
    } else {
      mpSwitchStates[switchIndex].isActivated = false;
    }
  }
}

void uint16ToArray(uint16_t value, uint8_t *array) {
  *array = (uint8_t)(value>>8);
  *(array+1) = (uint8_t)(value);
}

uint16_t calculateCheckSum(uint8_t *buffer) {
  uint16_t sum = 0;
  for (int i=MPDEVICE_STACK_VERSION; i<MPDEVICE_STACK_CHECKSUM; i++) {
    sum += buffer[i];
  }
  return -sum;
}

void mpdeviceInit(void) {
  mediaplayerDeviceInit(playerDevice);
  for (boxId_e i = BOXMEDIAPLAYERPLAY; i <= BOXMEDIAPLAYERVOLD; i++) {
      uint8_t switchIndex = i - BOXMEDIAPLAYERPLAY;
      mpSwitchStates[switchIndex].isActivated = true;
  }
}

// init the mediaplayer device, it'll search the UART port with FUNCTION_RCDEVICE id
void mediaplayerDeviceInit(mediaplayerDevice_t *device) {
  device->isReady = false;
  serialPortFunction_e portID = FUNCTION_MPDEVICE;
  serialPortConfig_t *portConfig = findSerialPortConfig(portID);
  if (portConfig != NULL) {
    device->serialPort = openSerialPort(portConfig->identifier, portID, NULL, NULL, 9600, MODE_RXTX, SERIAL_NOT_INVERTED);
    if (device->serialPort != NULL) {
      device->isReady = true;
    }
  }
}

bool mediaplayerDevicePressButton(mediaplayerDevice_t *device, mediaplayerDeviceKeyEvent_e command)
{
  // is this device open?
  if (!device->serialPort) {
      return false;
  }

  uint8_t sendBuffer[MPDEVICE_SEND_LENGTH] = {0x7E, 0xFF, 06, 00, 01, 00, 00, 00, 00, 0xEF};
  sendBuffer[MPDEVICE_STACK_COMMAND] = command;
  uint16ToArray(0, sendBuffer+MPDEVICE_STACK_PARAMETER);
  uint16ToArray(calculateCheckSum(sendBuffer), sendBuffer+MPDEVICE_STACK_CHECKSUM);
  serialWriteBuf(device->serialPort, sendBuffer, MPDEVICE_SEND_LENGTH);

  DEBUG_SET(DEBUG_MPDEVICE, 1, (uint8_t) sendBuffer[MPDEVICE_STACK_COMMAND]);
  DEBUG_SET(DEBUG_MPDEVICE, 2, (uint8_t) sendBuffer[MPDEVICE_STACK_CHECKSUM]);
  DEBUG_SET(DEBUG_MPDEVICE, 3, (uint8_t) sendBuffer[MPDEVICE_STACK_CHECKSUM+1]);
  return true;
}

#endif
