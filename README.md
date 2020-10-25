# HET2 System Packet Scheme
Tanner Songkakul

# Overview
This document contains the BLE packet scheme for the various configurations of the HET2 System.

# Single board
The Single Board configuration contains 1x Amperometric Channel and 1x Potentiometric Channel, as well as an accelerometer and temperature sensor.

## UUID descriptions
| **Characteristic** | **ID** | **Length (Bytes)** | **Configuration** | **Description** |
| --- | --- | --- | --- | --- |
| **SYSCFG** | ABCD | 10 | Read/Write | Commands |
| **CHAR1** | 62D2 | 10 | Notify | Info Packet |
| **CHAR2** | 44DC | 82 | Notify | Electrochemical Data |
| **CHAR3** | 3C36 | 201 | Notify | Accelerometer Data |
| **CHAR4** | 3A36 | 100 | Notify | Unused |
| **CHAR5** | 30D8 | 100 | Notify | Unused |

## Single Board Config Packet Structure
The single byte command scheme used in previous HET2 versions is not longer used on this device. A similar command scheme is available to allow for future flexibility in adding functionality, but is not currently implemented on the device. The full command structure is as follows:
| Byte | Command Prefix | Command Value | Mode | Bias | TIA Gain | Sampling Period | PGA Gain | Unused | Unused | Unused |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Byte Index | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
|     Desc.         |     Variable   Command    |     Variable   Command Value    |     Bytes [8:4]: Set data modes    Bytes [3:0]: Set   pstat mode          |     Bias   value (Vbias+128)/10    |     TIA   Gain (Preset options)    |     Sampling   period (Preset options)    |     PGA   Gain (Preset options)    |     Unused    |     Unused    |     Unused    |

To change configurations, byte 0 of the config must be set to 0x0C.

### Mode Byte

Byte 2 of the config packet will have the form 0x[Data Mode][Pstat Mode]

#### Data Modes

The device can be set to streaming and saving modes. In either mode, the indicator LED will flash once every 10 samples gathered (approximately every 30 seconds while in saving mode).

Note that entering save mode will overwrite previously saved data on the memory, so be sure to dump data from memory before sending a save command.

| **Value** | **Data Mode** |
| --- | --- |
| **0** | **Idle:** potentiostats in sleep mode |
| **1** | **Streaming** : Readings streamed over Bluetooth |
| **2** | **Saving** : Readings saved to local memory |


#### Potentiostat Modes

The device can operate in chronoamperometric mode or cyclic voltammetry mode. In either case, the device alternates between measuring amperometric and potentiometric data. In chronoamperometric mode, the bias voltage is held constant. In cyclic voltammetry mode, the bias value determines the start value and the sweep range of the cyclic voltammetry sweep (if bias voltage is set to -1V, the cyclic voltammetry sweep range will be from -1V to 1V and then back to -1V.

| **Value** | **Pstat Mode** |
| --- | --- |
| **0** | **Chronoamperometric Mode** |
| **1** | **Cyclic Voltammetry Mode** |


For example, sending 0x10 in the Mode Byte will start the device in chronoamperometric mode.

### Bias Byte

The bias byte is byte 3 of the config packet. It is an unsigned single byte which is converted to the bias voltage on the board between WE and RE. This is the constant bias for chronoamperometric mode and the sweep range for cyclic voltammetry mode. To find the value of this byte, the equation is: Bias Byte = VBias (mV)/10+128

For example, to bias the device at -1.00V, the Bias Byte should be set to (-1000/10+128) = 28

### TIA Gain Byte

Byte 4 of the config packet selects the transimpedance gain resistor on board the AD5941.

| **Value (Decimal)** | **Gain** |
| --- | --- |
| **0** | External resistor |
| **1** | 200 |
| **2** | 1k |
| **3** | 2k |
| **4** | 3k |
| **5** | 4k |
| **6** | 6k |
| **7** | 8k |
| **8** | 10k |
| **9** | 12k |
| **10** | 16k |
| **11** | 20k |
| **12** | 24k |
| **13** | 30k |
| **14** | 32k |
| **15** | 40k |
| **16** | 48k |
| **17** | 64k |
| **18** | 85k |
| **19** | 96k |
| **20** | 100k |
| **21** | 120k |
| **22** | 128k |
| **23** | 160k |
| **24** | 196k |
| **25** | 256k |
| **26** | 512k |

### Sampling Period Byte

Byte 5 of the config packet selects the sampling period of the device for each channel. Note that the device actually samples at twice this rate, alternating between the chronoamperometric and potentiometric channels.

| **Value (Decimal)** | **Period (seconds)** | **Sampling Frequency (Hz)** |
| --- | --- | --- |
| **0** | 1 | 1 |
| **1** | 0.05 | 20 |
| **2** | 0.1 | 10 |
| **3** | 0.125 | 8 |
| **4** | 0.1667 | 6 |
| **5** | 0.25 | 4 |
| **6** | 0.5 | 2 |
| **7** | 2 | 0.5 |
| **8** | 2.5 | 0.4 |
| **9** | 5 | 0.2 |
| **10** | 10 | 0.1 |
| **11** | 20 | 0.05 |
| **12** | 25 | 0.04 |
| **13** | 30 | 0.0333 |
| **14** | 50 | 0.02 |
| **15** | 60 | 0.0167 |
| **16** | 120 | 0.00833 |
| **17** | 150 | 0.00667 |
| **18** | 300 | 0.00333 |
| **19** | 600 | 0.00167 |

### PGA Gain Byte

Byte 6 of the config packet selects the programmable gain amplification for the ADC on board the AD5941.

| **Value (Decimal)** | **Gain** |
| --- | --- |
| **0** | 1 |
| **1** | 1.5 |
| **2** | 2 |
| **3** | 4 |
| **4** | 9 |

## Additional Commands

Additional functionalities can be accessed and added by using the first two bytes in the command/command value configuration. If byte 0 is set to a valid command prefix instead of 0x0C, the device will carry out the requested command.

### Variable Commands

| **Command Prefix** | **Command** | **Description** | **Notes** |
| --- | --- | --- | --- |
| **0** | Get Info | Requests info packet from device | VALUE byte doesn&#39;t matter |
| **1** | Change Data Mode | Toggle data mode between idle, streaming and saving | VALUE byte doesn&#39;t matter |
| **2** | Interval Mode Length | Number of samples before sleep | 1-255 samples |
| **3** | Interval Mode Sleep | How long to sleep between interval mode measurements | Defaults to 0If VALUE\&lt;60, sleep time is VALUE secondsIf VALUE\&gt;60, sleep time is VALUE-59 minutes |
| **B** | Blink | Toggle blinking | VALUE byte 1 to blink |
| **C** | Config | Change configuration | See config packet section |
| **F** | Memory Dump | If currently saving data, stop data collection. Dump the last saved trial measurement over Bluetooth | VALUE byte doesn&#39;t matter |


## Transmitted Packets

The device will transmit packets of two types. Info packets will be transmitted on Characteristic 1 (0x62D2), and data packets will be transmitted on Characteristic 2 (0x44DC)

### Info packet

The device will transmit a 20 byte info packet of the following structure on Characteristic 1 when it receives a command:

| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8-9 | 10-11 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Device num (1 byte) | Software ver(1 byte) | Mode(1 byte) | TIA Gain (1 byte) | Bias (1 byte) | Sampl. Period (1 byte) | PGA Gain (1 byte) | Error codes (1 byte) | Last battery reading (2 bytes) | Last temp/humidity reading(2 bytes) |

The Software version byte is of the format 0x[Version][Revision]

The Mode byte is of the format 0x[Idle/Streaming/Saving][CA/CV], where CA=0, CV=1, Idle=0, Streaming=1, Saving=2

The TIA Gain, Bias, Sampling Period, and PGA Gain bytes match the table in the command section

Error code byte:

| **Error code** | **Status** |
| --- | --- |
| **0** | No issues detected |
| **1** | Memory test failed |
| **2-255** | Not currently used |

## data packet

The device will transmit an 82 byte data packet of the following structure when in streaming mode. When dumping data from memory, the packet will have the same format.

The first 80 bytes of the packet contains 20 measurements from the amperometric and potentiostat channels in float32 format.

| [8\*n:8\*n+3] | [8\*n+4:8\*n+7] | [80] | [81] |
| --- | --- | --- | --- |
| Amperometric measurement (4 bytes) | Potentiometric measurement (4 bytes) | Data source | Counter |

Where n is 0-9 for 10 total measurements

The last byte of the packet contains a tag containing the data source as well as a counter.

The high byte of the Data source/counter is of the form 0x[Data source][Packet Count bits 11:7], and the low byte is of the form 0x[Packet Count Bytes 7:4][Packet Count Bytes 3:0]. The counter is a 12 bit counter which increments for each sample. Each sampling sequence should contain 1200 samples (300 samples/node \* 4 nodes).

# Recommended USe

For long-term chronoamperometric measurement, the following usage is recommended:

Device setup

1. Connect to device
2. Check info byte to verify device number, software version, battery level, etc
3. [optional] Blink LED to verify correct device
4. Set bias
5. Set gain
6. Set device to streaming mode
7. Set up trial verify sensor interface and operation
8. After trial has been successfully initiated, set device to saving mode
9. Disconnect from device

Ending trial

1. Connect to device
2. Check info byte to verify device number, software version, battery level, etc
3. [optional] Blink LED to verify correct device
4. Send Dump Memory command to receive data

