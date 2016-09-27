/*
 * tensometer.c
 *
 *  Created on: 30-04-2013
 *      Author: D. Pazderski
 */

#include "tensometer.h"

#define TENSOMETER_STATE_RESET	0
#define TENSOMETER_STATE_CONFIG_FILTER	1
#define TENSOMETER_STATE_CONFIG_MODE	2
#define TENSOMETER_STATE_WAIT	3
#define TENSOMETER_STATE_DATA	4

TTensometer tens;


#define SPI_BUF_SIZE 0x10
Uint16 SPIbuf[SPI_BUF_SIZE];

/****************************************************************************************************************************************************************************************************
 *
 * Zapytanie wysy³ane do AD7730
 *
 ***************************************************************************************************************************************************************************************************/
void TensometerRequest() {
	switch (tens.state) {

		case TENSOMETER_STATE_RESET:
			// reset AD7730
			SpiaRegs.SPIFFRX.bit.RXFFIL = 5;		// generuj przerwanie po 3 odebranych znakach
			SpiaRegs.SPITXBUF = (Uint16) 0xFF << 8;
			SpiaRegs.SPITXBUF = (Uint16) 0xFF << 8;
			SpiaRegs.SPITXBUF = (Uint16) 0xFF << 8;
			SpiaRegs.SPITXBUF = (Uint16) 0xFF << 8;
			SpiaRegs.SPITXBUF = (Uint16) 0xFF << 8;

			tens.state = TENSOMETER_STATE_CONFIG_FILTER;
		break;

		case TENSOMETER_STATE_CONFIG_FILTER:

			// ustawienie konfiguracji AD7730
			SpiaRegs.SPIFFRX.bit.RXFFIL = 4;		// generuj przerwanie po 3 odebranych znakach
			SpiaRegs.SPITXBUF = (Uint16) 0x03 << 8;
			SpiaRegs.SPITXBUF = (Uint16) 0x80 << 8;			// DP - bylo 08
			SpiaRegs.SPITXBUF = (Uint16) 0x02 << 8;
			SpiaRegs.SPITXBUF = (Uint16) 0x00 << 8;

			tens.state = TENSOMETER_STATE_CONFIG_MODE;
		break;

		case TENSOMETER_STATE_CONFIG_MODE:

			SpiaRegs.SPIFFRX.bit.RXFFIL = 3;		// generuj przerwanie po 3 odebranych znakach

			SpiaRegs.SPITXBUF = 0x02 << 8;
			// CH1, CH0 = 0
			// RN1-RN0 = 0 -> -10mV do +10mV
			// MR7 = 1 -> 5V ref
			// MR8 = 0 -> 16 bit res
			// MR9-11 = 0
			// MR12 = 0 -> bipolar
			// MD2-0 -> 001

			// 0010 0000
			SpiaRegs.SPITXBUF = (Uint16) 0x20 << 8;
			// 1000 0000
			SpiaRegs.SPITXBUF = (Uint16) 0x80 << 8;


			tens.state = TENSOMETER_STATE_WAIT;
		break;

		case TENSOMETER_STATE_WAIT:
			// wys³anie rozkazu odczytu rejestru statusowego
			SpiaRegs.SPITXBUF = (Uint16) 0x10 << 8;
			SpiaRegs.SPITXBUF = (Uint16) 0x00 << 8;
			SpiaRegs.SPIFFRX.bit.RXFFIL = 2;		// generuj przerwanie po 2 odebranych znakach
		break;

		default:
			tens.state = TENSOMETER_STATE_RESET;
		break;
	}
}

/****************************************************************************************************************************************************************************************************
 *
 * Odczyt danych z AD7730
 *
 ***************************************************************************************************************************************************************************************************/
void TensometerRead() {
	volatile Uint16 c;

	switch (tens.state) {
		case TENSOMETER_STATE_WAIT:
			c = SPIbuf[1];
			if ((c & 0x0F) != 0x0B) {
				tens.error++;
				if (tens.error > 10) {
					tens.state = TENSOMETER_STATE_RESET;
					tens.error = 0;
				}
			}
			else
			{
				if (~(c & 0x80)) {	// testuj bit BUSY
					// czujnik gotowy do odczytu
					tens.error = 0;
					GpioDataRegs.GPACLEAR.bit.GPIO19 = 1;	// zeruj ENABLE
					SpiaRegs.SPIFFRX.bit.RXFFIL = 3;		// generuj przerwanie po 3 odebranych znakach

					// wysy³aj zapytanie
					SpiaRegs.SPITXBUF = 0x11 << 8;			// odczyt danych przetwornika A/C
					SpiaRegs.SPITXBUF = 0x00 << 8;			// 16-bitowa wartosc
					SpiaRegs.SPITXBUF = 0x00 << 8;

					tens.state = TENSOMETER_STATE_DATA;
				}
	  		}
	  	break;

	  	case TENSOMETER_STATE_DATA:
	  	{
	  		c = SPIbuf[1] * 256 + SPIbuf[2];
	  	 if(c)
	  		 tens.raw_data = (c - 0x8000) + TENSOMETER_OFFSET_100;
	  	 else
	  		tens.raw_data = 0;
    		tens.state = TENSOMETER_STATE_WAIT;
	  	}
	  	break;
	  }
}

//***************************************************************************************
//
//***************************************************************************************
interrupt void SPI_RECEIVE_isr(void)    // SPI-A
{

  unsigned char s, i;

  GpioDataRegs.GPASET.bit.GPIO19 = 1;	// ustaw ENABLE

  s = SpiaRegs.SPIFFRX.bit.RXFFST;			// pobierz liczbê znaków do odczytu
  if (s > SPI_BUF_SIZE) s = SPI_BUF_SIZE;	// ograniczaj j¹ do rozmiaru bufora

  // przepisuj 8-bitowe znaki z FIFO
  for (i = 0; i < s; i++) {
    SPIbuf[i] = SpiaRegs.SPIRXBUF & ((1<<8)-1);
  }

  TensometerRead();
  SpiaRegs.SPIFFRX.bit.RXFFINTCLR = 1;		// czysc bufor odbiorczy


  PieCtrlRegs.PIEACK.all |= PIEACK_GROUP6;	// potwierdzenie obs-ugi przerwania grupy 6
}

void WriteSPI() {
  //int16 i;
  // Data length = 8 Bits, << shift (16-8) bits
  SpiaRegs.SPIFFTX.bit.TXFFST = 1;
  SpiaRegs.SPIFFRX.bit.RXFIFORESET = 1;

  TensometerRequest();
  // 6 bajtów wysy³anych
  GpioDataRegs.GPACLEAR.bit.GPIO19 = 1;	// zeruj ENABLE


}