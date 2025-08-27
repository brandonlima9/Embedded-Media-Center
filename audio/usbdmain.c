/*----------------------------------------------------------------------------
 * Name:    usbmain.c
 * Purpose: USB Audio Class Demo
 * Version: V1.20
 *----------------------------------------------------------------------------
 *      This software is supplied "AS IS" without any warranties, express,
 *      implied or statutory, including but not limited to the implied
 *      warranties of fitness for purpose, satisfactory quality and
 *      noninfringement. Keil extends you a royalty-free right to reproduce
 *      and distribute executable files created using this software for use
 *      on NXP Semiconductors LPC microcontroller devices only. Nothing else 
 *      gives you the right to use this software.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include "LPC17xx.h"                        /* LPC17xx definitions */
#include "type.h"
#include "usb.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "usbaudio.h"
#include "usbdmain.h"
#include "kbd.h"
#include "GLCD.h"
#include "LED.h"


extern  void SystemClockUpdate(void);
extern uint32_t SystemFrequency;  
uint8_t  Mute;                                 /* Mute State */
uint32_t Volume;                               /* Volume Level */
int rep;
int pic= 0;
#if USB_DMA
uint32_t *InfoBuf = (uint32_t *)(DMA_BUF_ADR);
short *DataBuf = (short *)(DMA_BUF_ADR + 4*P_C);
#else
uint32_t InfoBuf[P_C];
short DataBuf[B_S];                         /* Data Buffer */
#endif

uint16_t  DataOut;                              /* Data Out Index */
uint16_t  DataIn;                               /* Data In Index */

uint8_t   DataRun;                              /* Data Stream Run State */
uint16_t  PotVal;                               /* Potenciometer Value */
uint32_t  VUM;                                  /* VU Meter */
uint32_t  Tick;                                 /* Time Tick */
int l = 0;
/*
 * Get Potenciometer Value
 */

void LEDOFF() {
	for (l=0;l<8;l++) {
	  LED_Off(l);
	}
}

void get_potval (void) {
  uint32_t val;

  LPC_ADC->CR |= 0x01000000;              /* Start A/D Conversion */
  do {
    val = LPC_ADC->GDR;                   /* Read A/D Data Register */
  } while ((val & 0x80000000) == 0);      /* Wait for end of A/D Conversion */
  LPC_ADC->CR &= ~0x01000000;             /* Stop A/D Conversion */
  PotVal = ((val >> 8) & 0xF8) +          /* Extract Potenciometer Value */
           ((val >> 7) & 0x08);
	//increase from 39 in increments of 36, (255/7)
	if (PotVal == 0){ //0
		LEDOFF();
	}
	else if ((PotVal > 0) && (PotVal <=39)){ //1
		LEDOFF();
		LED_On(7);
	}
	else if (PotVal <=75){ //2
		LEDOFF();
		LED_On(6);
		LED_On(7);
	}
		else if (PotVal <=111){ //3
	
		LEDOFF();
		LED_On(5);
		LED_On(6);
		LED_On(7);
	}
		else if (PotVal <=147){ //4
		LEDOFF();
		LED_On(4);
		LED_On(5);
		LED_On(6);
		LED_On(7);
	}
	else if (PotVal <=183){ //5
		LEDOFF();
		LED_On(3);
		LED_On(4);
		LED_On(5);
		LED_On(6);
		LED_On(7);
	}
	else if (PotVal <= 219){ //6
		LEDOFF();
		LED_On(1);
		LED_On(2);
		LED_On(3);
		LED_On(4);
		LED_On(5);
		LED_On(6);
		LED_On(7);
	}
else if (PotVal <= 255) { //7
		LED_On(0);
		LED_On(1);
		LED_On(2);
		LED_On(3);
		LED_On(4);
	  LED_On(5);
		LED_On(6);
		LED_On(7);
	}

}
/*
 * Timer Counter 0 Interrupt Service Routine
 *   executed each 31.25us (32kHz frequency)
 */

void TIMER0_IRQHandler(void) 
{
	int joys;
  long  val;
  uint32_t cnt;
	rep = 1;
	while(rep){
		joys = get_button();
		switch(joys){
			case KBD_LEFT: // go left to reset
		NVIC_SystemReset(); //resets
				rep = 0;
				break;
			default:
				if (DataRun) {                            /* Data Stream is running */
					val = DataBuf[DataOut];                 /* Get Audio Sample */
					cnt = (DataIn - DataOut) & (B_S - 1);   /* Buffer Data Count */
					if (cnt == (B_S - P_C*P_S)) {           /* Too much Data in Buffer */
						DataOut++;                            /* Skip one Sample */
					}
					if (cnt > (P_C*P_S)) {                  /* Still enough Data in Buffer */
						DataOut++;                            /* Update Data Out Index */
					}
					DataOut &= B_S - 1;                     /* Adjust Buffer Out Index */
					if (val < 0) VUM -= val;                /* Accumulate Neg Value */
					else         VUM += val;                /* Accumulate Pos Value */
					val  *= Volume;                         /* Apply Volume Level */
					val >>= 16;                             /* Adjust Value */
					val  += 0x8000;                         /* Add Bias */
					val  &= 0xFFFF;                         /* Mask Value */
				} else {
					val = 0x8000;                           /* DAC Middle Point */
				}

				if (Mute) {
					val = 0x8000;                           /* DAC Middle Point */
				}

				LPC_DAC->CR = val & 0xFFC0;               /* Set Speaker Output */

				if ((Tick++ & 0x03FF) == 0) {             /* On every 1024th Tick */
					get_potval();                           /* Get Potenciometer Value */
					if (VolCur == 0x8000) {                 /* Check for Minimum Level */
						Volume = 0;                           /* No Sound */
					} else {
						Volume = VolCur * PotVal;             /* Chained Volume Level */
					}
					val = VUM >> 20;                        /* Scale Accumulated Value */
					VUM = 0;                                /* Clear VUM */
					if (val > 7) val = 7;                   /* Limit Value */
				}
				
				LPC_TIM0->IR = 1;                         /* Clear Interrupt Flag */
				rep = 0;			
				break;
			}
	}
}
/*****************************************************************************
**   Main Function  main()
******************************************************************************/
int audio_main (void)
{
	//int i,j;
	volatile uint32_t pclkdiv, pclk;
  /* SystemClockUpdate() updates the SystemFrequency variable */
  SystemClockUpdate();
	
  LPC_PINCON->PINSEL1 &=~((0x03<<18)|(0x03<<20));  
  /* P0.25, A0.0, function 01, P0.26 AOUT, function 10 */
  LPC_PINCON->PINSEL1 |= ((0x01<<18)|(0x02<<20));

  /* Enable CLOCK into ADC controller */
  LPC_SC->PCONP |= (1 << 12);

  LPC_ADC->CR = 0x00200E04;		/* ADC: 10-bit AIN2 @ 4MHz */
  LPC_DAC->CR = 0x00008000;		/* DAC Output set to Middle Point */

  /* By default, the PCLKSELx value is zero, thus, the PCLK for
  all the peripherals is 1/4 of the SystemFrequency. */
  /* Bit 2~3 is for TIMER0 */
  pclkdiv = (LPC_SC->PCLKSEL0 >> 2) & 0x03;
  switch ( pclkdiv )
  {
	case 0x00:
	default:
	  pclk = SystemFrequency/4;
	break;
	case 0x01:
	  pclk = SystemFrequency;
	break; 
	case 0x02:
	  pclk = SystemFrequency/2;
	break; 
	case 0x03:
	  pclk = SystemFrequency/8;
	break;
  
	}
	
  LPC_TIM0->MR0 = pclk/DATA_FREQ - 1;	/* TC0 Match Value 0 */
  LPC_TIM0->MCR = 3;					/* TCO Interrupt and Reset on MR0 */
  LPC_TIM0->TCR = 1;					/* TC0 Enable */

	//can edit lcd here if wanted
	
	
  NVIC_EnableIRQ(TIMER0_IRQn);
	
  USB_Init();				        /* USB Initialization */
	NVIC_EnableIRQ(USB_IRQn); /* enable USB interrupt */
	USB_Reset();
	USB_SetAddress(0);
	USB_Connect(TRUE);		    /* USB Connect */
// The main Function in blinky is an endless loop, so while(1) loop not needed
// while( 1 ); 
	return 0;
}