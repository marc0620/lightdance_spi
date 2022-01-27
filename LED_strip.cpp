/****************************************************************************
  FileName     [ LED_strip.cpp ]
  PackageName  [ clientApp ]
  Synopsis     [ LED strip control ]
  Author       [  ]
  Copyright    [ Copyleft(c) , NTUEE, Taiwan ]
****************************************************************************/
#include "LED_strip.h"
#include <time.h>
#include <iostream>
#include <vector>
//#include "definition.h"

/*from definition.h*/
#define LED_BRIGHTNESS_SCALE 0.8
#define LEDS_0 10
#define LEDS_1 96
#define LEDS_2 42
#define LEDS {LEDS_0, LEDS_1, LEDS_2}
/**/
using namespace std;
/*
  @brief constructor of LED strip, initialize SPI communication.
  @param nStrips number of LED strips
  @param nLEDs number of LEDs in each strip
*/
LED_Strip::LED_Strip(const uint8_t &nStrips, const uint16_t *nLEDs) : _nStrips(nStrips)
{
	// SPI init
    if (!bcm2835_init())
	{
		printf("bcm2835_init failed. Are you running as root??\n");
		exit(-1);
	}
	if (!bcm2835_spi_begin())
	{
		printf("bcm2835_spi_begin failed. Are you running as root??\n");
		exit(-1);
	}

	_nLEDs = new uint16_t[nStrips];
	for (uint8_t i = 0; i < nStrips; ++i)
		_nLEDs[i] = nLEDs[i];

  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);
}

LED_Strip::~LED_Strip()
{
	bcm2835_spi_end();
	bcm2835_close();

    delete _nLEDs;
}

/*!
  @brief Send pixel secquence to LED strip via SPI.
  @param id Destination strip ID
  @param color pixel secquence to send

  @note strip ID from 0 to nStrips - 1
*/
void LED_Strip::send (const uint8_t &id, std::vector<int>& color){
  uint16_t dataLen = 3 * _nLEDs[id] + 2;
	char buf[dataLen] = {0};
	const char stripInfo[2] = {id,0};
	getSeq(id, buf, color);
	bcm2835_spi_writenb(stripInfo, 2);
	bcm2835_spi_writenb(buf, dataLen);	
	for (int a = 0; a < color.size()+1; a++) std::cout << (short)buf[a] << " ";
		std::cout << std::endl;
	
}

void LED_Strip::sendToStrip(const uint8_t &id, std::vector<int>& color)
{
	send(id,color);

}


/*!
  @brief 	   Convert pixel secquence to special dataframe for SPI communication.
  @param id    Destination strip ID
  @param len   Data length of pixel secquence
  @param seq   Pixel secquence to send
  @param color Output dataframe
*/
void LED_Strip::getSeq(const uint8_t &id, char *seq, std:: vector<int>& color)
{
	seq[0] = START_BYTE;
	for (uint16_t i = 0 ; i < color.size(); ++i)
	{
		seq[i + DATA_OFFSET] = color[i]*LED_BRIGHTNESS_SCALE ;
	}
	seq[color.size()+1] = '\0';
}
