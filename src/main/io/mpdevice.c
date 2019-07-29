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

#include "io/mpdevice.h"
#include "build/debug.h"

#ifdef USE_MPDEVICE

serialPort_t *mpSerialPort;
mpdeviceSwitchState_t mpSwitchStates[BOXMEDIAPLAYERVOLD - BOXMEDIAPLAYERPAUSE  + 1];

// init the mediaplayer device, it'll search the UART port with FUNCTION_RCDEVICE id
void mpdeviceInit(void) {
  serialPortFunction_e portID = FUNCTION_MPDEVICE;
  serialPortConfig_t *portConfig = findSerialPortConfig(portID);
  if (portConfig != NULL) {
    mpSerialPort = openSerialPort(portConfig->identifier, portID, NULL, NULL, 9600, MODE_RXTX, SERIAL_NOT_INVERTED);
    if (mpSerialPort != NULL) {

    }
  }
  // mediaplayerDeviceInit(playerDevice);
  for (boxId_e i = BOXMEDIAPLAYERPAUSE; i <= BOXMEDIAPLAYERVOLD; i++) {
      uint8_t switchIndex = i - BOXMEDIAPLAYERPAUSE;
      mpSwitchStates[switchIndex].isActivated = true;
  }
}

bool mpdeviceIsEnabled(void)
{
  return mpSerialPort != NULL;
}

void mpdeviceUpdate(timeUs_t currentTimeUs)
{
  mpdevicePlayerProcess(currentTimeUs);
}

void mpdevicePlayerProcess(timeUs_t currentTimeUs) {
  if (currentTimeUs) {};
  for (boxId_e i = BOXMEDIAPLAYERPAUSE; i <= BOXMEDIAPLAYERVOLD; i++) {
    uint8_t switchIndex = i - BOXMEDIAPLAYERPAUSE;

    if (IS_RC_MODE_ACTIVE(i)) {
      // check last state of this mode, if it's true, then ignore it.
      // Here is a logic to make a toggle control for this mode
      if (mpSwitchStates[switchIndex].isActivated) {
        continue;
      }

      uint8_t command = MPDEVICE_KEY_UNKNOWN;

      switch (i) {
      case BOXMEDIAPLAYERPAUSE:
        command = MPDEVICE_KEY_PAUSE;
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

        mediaplayerDevicePressButton(command);
        mpSwitchStates[switchIndex].isActivated = true;
      }
    } else {
      mpSwitchStates[switchIndex].isActivated = false;
    }
  }
}

bool mediaplayerDevicePressButton(mediaplayerDeviceKeyEvent_e command)
{
  // is this device open?
  if (!mpSerialPort) {
      return false;
  }

  uint8_t sendBuffer[MPDEVICE_SEND_LENGTH] = {0x7E, 0xFF, 06, 00, 01, 00, 00, 00, 00, 0xEF};
  sendBuffer[MPDEVICE_STACK_COMMAND] = command;
  uint16ToArray(0, sendBuffer+MPDEVICE_STACK_PARAMETER);
  uint16ToArray(calculateCheckSum(sendBuffer), sendBuffer+MPDEVICE_STACK_CHECKSUM);
  serialWriteBuf(mpSerialPort, sendBuffer, MPDEVICE_SEND_LENGTH);

  DEBUG_SET(DEBUG_MPDEVICE, 1, (uint8_t) sendBuffer[MPDEVICE_STACK_COMMAND]);
  DEBUG_SET(DEBUG_MPDEVICE, 2, (uint8_t) sendBuffer[MPDEVICE_STACK_CHECKSUM]);
  DEBUG_SET(DEBUG_MPDEVICE, 3, (uint8_t) sendBuffer[MPDEVICE_STACK_CHECKSUM+1]);
  return true;
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

#endif
