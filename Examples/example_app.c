/*
MIT License

Copyright (c) 2019 Horizon Hobby, LLC

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// This example is not intended to compile -- it just shows an example of how to use spm_srxl.c/.h
// This example is intended to go with the example spm_srxl_config.h to provide guidance on usage.

#include "spm_srxl.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Example external functions -- imagine these are defined in uart.h and implemented in uart.c
void uartInit(uint8_t uartNum, uint32_t baudRate);
void uartSetBaud(uint8_t uartNum, uint32_t baudRate);
uint8_t uartReceiveBytes(uint8_t uartNum, uint8_t* pBuffer, uint8_t bufferSize, uint8_t timeout_ms);
uint8_t uartTransmit(uint8_t uartNum, uint8_t* pBuffer, uint8_t bytesToSend);

// Forward definitions of app-specific handling of telemetry and channel data -- see examples below
void userProvidedFillSrxlTelemetry(SrxlTelemetryData* pTelemetry);
void userProvidedReceivedChannelData(SrxlChannelData* pChannelData, bool isFailsafeData);

// UART receive buffer
uint8_t rxBuffer[2 * SRXL_MAX_BUFFER_SIZE];
uint8_t rxBufferIndex = 0;

// Sample SRXL app
int main(void)
{
    // Initialization
    uint8_t uartHandle = 1;
    uartInit(uartHandle, 115200);
    uint32_t uniqueID = (uint32_t)serialNum;  // or hashFunction(serialNum) or rand() -- just needs to be likely unique

    // Init the local SRXL device
    if(!srxlInitDevice(SRXL_DEVICE_ID, SRXL_DEVICE_PRIORITY, SRXL_DEVICE_INFO, uniqueID))
        return FAIL;

    // Init the SRXL bus: The bus index must always be < SRXL_NUM_OF_BUSES -- in this case, it can only be 0
    if(!srxlInitBus(0, uartHandle, SRXL_SUPPORTED_BAUD_RATES))
        return FAIL;

    // Sample receive loop -- this is just a simplified example of how you could receive bytes from uart.
    // You might have a much better API for receiving UART bytes with a built-in timeout on your device.
    while(1)
    {
        // Try to receive UART bytes, or timeout after 5 ms
        uint8_t bytesReceived = uartReceiveBytes(uartHandle, &rxBuffer[rxBufferIndex], SRXL_MAX_BUFFER_SIZE, 5);
        if(bytesReceived)
        {
            rxBufferIndex += bytesReceived;
            if(rxBufferIndex < 5)
                continue;

            if(rxBuffer[0] == SPEKTRUM_SRXL_ID)
            {
                uint8_t packetLength = rxBuffer[2];
                if(rxBufferIndex > packetLength)
                {
                    // Try to parse SRXL packet -- this internally calls srxlRun() after packet is parsed and reset timeout
                    if(srxlParsePacket(0, rxBuffer, packetLength))
                    {
                        // Move any remaining bytes to beginning of buffer (usually 0)
                        rxBufferIndex -= packetLength;
                        memmove(rxBuffer, &rxBuffer[packetLength], rxBufferIndex);
                    }
                    else
                    {
                        rxBufferIndex = 0;
                    }
                }
            }
        }
        else
        {
            // Tell SRXL state machine that 5 more milliseconds have passed since packet received
            srxlRun(0, 5);
            rxBufferIndex = 0;
        }

        // Check a bind button, and if pressed enter bind mode
        if(bindButtonPressed)
        {
            srxlEnterBind(DSMX_11MS);
        }
    }
}

// User-defined routine to populate telemetry data
void userProvidedFillSrxlTelemetry(SrxlTelemetryData* pTelemetry)
{
    // Copy in whatever telemetry data you wish to send back this cycle
    // You can fill it via pTelemetry...
    memset(pTelemetry->raw, 0, 16);
    // ... or directly access the global value srxlTelemData
    srxlTelemData.sensorID = 0;
}

// User-defined routine to use the provided channel data from the SRXL bus master
void userProvidedReceivedChannelData(SrxlChannelData* pChannelData, bool isFailsafeData)
{
    // Use the received channel data in whatever way desired.
    // The failsafe flag is set if the data is failsafe data instead of normal channel data.
    // You can directly access the last received packet data through pChannelData,
    // in which case the values are still packed into the beginning of the array of channel data,
    // and must be identified by the mask bits that are set (NOT recommended!).
    // This is mostly provided in case you want to know which channels were actually sent
    // during this packet:
    uint8_t currentIndex = 0;
    if(pChannelData->mask & 1)
        servoA_12bit = pChannelData->values[currentIndex++] >> 4;
    if(pChannelData->mask & 2)
        servoB_12bit = pChannelData->values[currentIndex++] >> 4;

    // The recommended method is to access all channel data through the global srxlChData var:
    servoA_12bit = srxlChData.values[0] >> 4;
    servoB_12bit = srxlChData.values[1] >> 4;

    // RSSI and frame loss data are also available:
    if(srxlChData.rssi < -85 || (srxlChData.rssi > 0 && srxlChData.rssi < 10))
        EnterLongRangeModeForExample();
}
