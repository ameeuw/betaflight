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
#include "pg/mpdevice.h"

#ifdef USE_MPDEVICE

serialPort_t *mpdeviceSerialPort;
mpdeviceSwitchState_t mpdeviceSwitchStates[BOXMEDIAPLAYERVOLD - BOXMEDIAPLAYERPAUSE  + 1];
bool mpdevicePlaying = false;
uint8_t mpdeviceVolume = 0;

// init the mediaplayer device, it'll search the UART port with FUNCTION_RCDEVICE id
void mpdeviceInit(void) {
  serialPortFunction_e portID = FUNCTION_MPDEVICE;
  serialPortConfig_t *portConfig = findSerialPortConfig(portID);
  if (portConfig != NULL) {
    mpdeviceSerialPort = openSerialPort(portConfig->identifier, portID, NULL, NULL, 9600, MODE_TX, SERIAL_NOT_INVERTED|SERIAL_UNIDIR);
    if (mpdeviceSerialPort != NULL) {
      // Initialise volume.
      mpdeviceSendCommand(MPDEVICE_SET_VOL, mpdeviceConfig()->lastVolume);
      // Set current volume to keep track.
      mpdeviceVolume = mpdeviceConfig()->lastVolume;
      // Set Loop All mode
      mpdeviceSendCommand(MPDEVICE_SET_LOOP_ALL, 1);
    }
  }
  // mediaplayerDeviceInit(playerDevice);
  for (boxId_e i = BOXMEDIAPLAYERPAUSE; i <= BOXMEDIAPLAYERVOLD; i++) {
      uint8_t switchIndex = i - BOXMEDIAPLAYERPAUSE;
      mpdeviceSwitchStates[switchIndex].isActivated = true;
  }
}

bool mpdeviceIsEnabled(void)
{
  return mpdeviceSerialPort != NULL;
}

void mpdeviceUpdate(timeUs_t currentTimeUs)
{
  mpdeviceProcess(currentTimeUs);
}

void mpdeviceProcess(timeUs_t currentTimeUs) {
  if (currentTimeUs) {};
  for (boxId_e i = BOXMEDIAPLAYERPAUSE; i <= BOXMEDIAPLAYERVOLD; i++) {
    uint8_t switchIndex = i - BOXMEDIAPLAYERPAUSE;

    if (IS_RC_MODE_ACTIVE(i)) {
      // check last state of this mode, if it's true, then ignore it.
      // Here is a logic to make a toggle control for this mode
      if (mpdeviceSwitchStates[switchIndex].isActivated) {
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
        mpdevicePressButton(command);
        mpdeviceSwitchStates[switchIndex].isActivated = true;
      }
    } else {
      mpdeviceSwitchStates[switchIndex].isActivated = false;
    }
  }
}

bool mpdevicePressButton(mpdeviceCommands_e command)
{
  if (command == MPDEVICE_KEY_VOL_UP && (mpdeviceVolume<30)) {
    mpdeviceVolume++;
  }

  if (command == MPDEVICE_KEY_VOL_DOWN && mpdeviceVolume>0) {
    mpdeviceVolume--;
  }

  mpdeviceSendCommand(command, 0);
  return true;
}

void mpdeviceSendCommand(mpdeviceCommands_e command, uint16_t parameter) {
  // is this device open?
  if (!mpdeviceSerialPort) {
      return false;
  }

  // Standard stack for MP3 serial command:   HEAD   VER   LEN   CMD   ACK  PARA  PARA  CHKS  CHKS   END
  uint8_t sendBuffer[MPDEVICE_SEND_LENGTH] = {0x7E, 0xFF, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0xEF};
  // Write command to stack
  sendBuffer[MPDEVICE_STACK_COMMAND] = command;

  // Write parameters to stack
  uint16ToArray(parameter, sendBuffer+MPDEVICE_STACK_PARAMETER);

  // Calculate checksum according to doc
  uint16ToArray(calculateCheckSum(sendBuffer), sendBuffer+MPDEVICE_STACK_CHECKSUM);

  // Write out to serial port
  serialWriteBuf(mpdeviceSerialPort, sendBuffer, MPDEVICE_SEND_LENGTH);
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
