#include <Arduino.h>
/*
  SD card file dump

 This example shows how to read a file from the SD card using the
 SD library and send it over the serial port.

 The circuit:
 * SD card attached to SPI bus as follows:
 **       SD NANO   STM32
 ** MOSI  D7   11   PA7
 ** MISO  D6   12   PA6
 ** CLK   D5   13   PA5
 ** CS -  D8    4   PA4
 
 * Fore Serial1, STM32 TX1 is PA9, RX1 is PA10. Remember to cross-over.
 
 created  22 December 2010 by Limor Fried
 modified 9 Apr 2012 by Tom Igoe
 modified 2020-08-01 by Mark Carter

 This example code is in the public domain.

 */

#include <SPI.h>
//#include <SD.h>

typedef uint8_t u8;
typedef uint32_t u32;

auto &ser = Serial1;
const int CS = PA4; // For STM32
//const int MOSI = PA7;
//const int MISO = PA6;
const int CLK = PA5;

static_assert(MOSI == PA7);
static_assert(MISO == PA6);

void CS_low() { digitalWrite(CS, 0); }
void CS_high() { digitalWrite(CS, 1); }

u8 get_response()
{
	u8 timeout = 10;
	u8 ret = 0xff;
	while (ret == 0xff && timeout-- > 0)
	{
		ret = SPI.transfer(0xff);
	}
	if (timeout == 0)
	{
		ser.println("get_response:timeout");
	}

	return ret;
}

uint8_t crc7(const uint8_t *buff, int32_t len)
{
	uint8_t crc = 0;

	while (len--)
	{
		uint8_t byte = *buff++;
		int32_t bits = 8;

		while (bits--)
		{
			crc <<= 1;
			if ((crc ^ byte) & 0x80)
			{
				crc ^= 0x89;
			}
			byte <<= 1;
		}
	}
	return (crc);
}

u8 transfer48(u8 input[6])
{
	//u8 response[6];
	CS_low();
	for (int i = 0; i < 6; i++)
	{
		SPI.transfer(input[i]);
	}
	ser.println("crc:" + String(crc7(input, 5)));

	u8 response = get_response();
	CS_high();
	return response;
}

u8 CMD(u8 cmd, u32 arg)
{
	
	u8 arr[6];
	arr[0] = 0x40 | cmd;
	for (int i = 0; i < 4; i++)
	{
		u32 b = arg & 0xFF000000;
		b = (b >> 24);
		arr[i+1] = b;
		arg = (arg << 8);
	}
	u8 crc = crc7(arr, 5);
	arr[5] = (crc<<1) + 1;

	CS_low();
	for(int i=0; i<6; i++) {
		SPI.transfer(arr[i]);
	}

	u8 response = get_response();
	CS_high();
	return response;
}

void setup()
{
	ser.begin(115200);
	ser.println("Initializing SD card... 2");
	pinMode(CS, OUTPUT);
	//pinMode(MOSI, OUTPUT);

	SPI.begin();
	SPI.beginTransaction(SPISettings(200000, MSBFIRST, SPI_MODE0));

	int attempt = 0;
	u8 response = 0;
	while (response != 1)
	{
		delay(1); // give the card time to get up to speed
		response = CMD(0, 0);
		ser.println("CMD0:" + String(++attempt) + ":" + String(response));
	}
	ser.println("Done initialisation");
	return;


#if 0
	u8 CMD8[] = {0b01001000, 0, 0, 1, 0b10101010, 0b10000111};
	response = transfer48(CMD8);
	ser.println("CMB8: " + String(response));
	#endif
}

void loop()
{
}