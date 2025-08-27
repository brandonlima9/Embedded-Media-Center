/*------------------------------------------------------------------------------
 * Example header Blinky module
 * Copyright (c) 2019-2020 Arm Limited (or its affiliates). All rights reserved.
 *------------------------------------------------------------------------------
 * Name:    Blinky.h
 * Purpose: Blinky header 
 *----------------------------------------------------------------------------*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BLINKY_H
#define __BLINKY_H

#include <stdint.h>

/* Global variables exported */
extern volatile uint8_t clock_1s;
extern uint16_t ADC_last;

#endif /* __BLINKY_H */
