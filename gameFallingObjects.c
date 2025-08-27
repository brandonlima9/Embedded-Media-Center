#include <LPC17xx.H>
#include <stdbool.h>
#include <stdio.h>
#include "string.h"
#include "GLCD.h"
#include "LED.h"
#include "KBD.h"

int LEDON_2 = 0;                   // default to all LED OFF
int currentGLCDPositionLEFT = 8;   // Stores the Left X-Player Position
int currentGLCDPositionRIGHT = 10; // Stores the Right X-Player Position
int borderLEFT = 0;								 // The Left GLCD X-Border
int borderRIGHT = 19;              // The Right GLCD X-Border
int boxGLCDPositionY = 20;         // Stores the Y-Player Position
int c = 0;                         // used for LED Loop
int a = 0;                         // used for Delay Loop
int b = 0;                         // used for Delay Loop
int score = 0;                     // Used for Player Score
int selection = 0;                 // Used for Defeat Screen Selection
int gameRunning = 1;               // Is the Game Running? (1=YES, 0=NO)
int ballHeight = 4;                // The Height Balls Drop From
char displayBuffer[50];            // Buffer Text1
char displayBuffer2[50];           // Buffer Text2
int speed = 1200;                  // Speed the Ball Falls (higher value is slower)
int randomX = 0;							     // Stores a random int from 1-18 for the X-Position of the Ball
unsigned int seed = 0;             // Used to create the random int randomX

// create random seed
void int_rand_seed(void) {
	unsigned int systick_val = (unsigned int)SysTick->VAL;
	unsigned int timer_val = (unsigned int)LPC_TIM0->TC;
	seed = systick_val ^ timer_val ^ (unsigned int)LPC_RTC->CTIME0;
}

// returns random seed
int myRandomX(void) {
	seed = (seed*1103515245+12145)&0x7FFFFFFF;
	return seed;
}

// resets/sets the default values for the below variables
void resetValues() {
	// When a new game starts these values NEED to be reset to their defaults
	currentGLCDPositionLEFT = 8;    // resets the player's position
	currentGLCDPositionRIGHT = 10;
	boxGLCDPositionY = 20;
	score = 0;											// resets the player's score
	speed = 1200;                   // resets the game speed
	selection = 0;                  // resets the DEFEAT selection
	gameRunning = 1;                // resets the game running indicator
	//randomX = 0;                  // resets the ball X-Position (not needed here since random number)
	ballHeight = 4;                 // resets the ball Y-Position (not needed here since updated after while (ballHeight < 10), here as a precaution)
}

// Function to draw the box (player's controlled object) on the screen
void drawPlayerBox() {
  GLCD_DisplayString(boxGLCDPositionY, 0, 1, (unsigned char *)"                    ");  // Clear the old box
  GLCD_DisplayChar(boxGLCDPositionY, currentGLCDPositionLEFT, 1, '|');                  // Draws new Left X-Player Position
  GLCD_DisplayChar(boxGLCDPositionY, currentGLCDPositionRIGHT, 1, '|');                 // Draws new Right X-Player Position
}

// Toggle the LEDs ON and OFF
void ToggleLED_2() {
	if (LEDON_2 == 1) {            // LEDs ARE CURRENTLY ON
		LEDON_2 = 0;                 // SET LEDs to OFF
		for (c = 0; c < 8; c++) {    // FOR ALL 8 LEDs TURN THEM OFF
		  LED_Off(c);
	  }
	} else {                       // LEDs ARE CURRENTLY OFF
		LEDON_2 = 1;                 // SET LEDs to ON
		for (c = 0; c < 8; c++) {    // FOR ALL 8 LEDs TURN THEM ON
		  LED_On(c);
	  }
	}
}

// Flash all LEDs for a short period
void flashLEDs() {
	ToggleLED_2();									 // TOGGLE LEDs
	for (c = 0; c < 5000000; c++) {  // WAIT
	__NOP(); 
	}
	ToggleLED_2();                   // TOGGLE LEDs AGAIN
}

// Function to delay the dropping speed/rate of the balls. 
// User input updated during this delay since single thread
void delay(){
	int joystick_previous, joystick_value = 0;
  for (a = 0; a < speed; a++) {   // Wait for time speed
	  for (b = 0; b < speed; b++) { // Wait for time speed
			//while the ball is dropping slowly check for user input
			joystick_value = get_button();
			if (joystick_value != joystick_previous) {         // Check if joystick input was detected
				if (joystick_value == KBD_RIGHT) {               // joystick right input detected
					if (currentGLCDPositionRIGHT != borderRIGHT) { // If not at the right border (which is 19 xPosition)
						currentGLCDPositionLEFT++;                   // increment both the left and right border and update the player box
            currentGLCDPositionRIGHT++;
            drawPlayerBox();
          }
        } 
				else if (joystick_value == KBD_LEFT) {           // joystick left input detected
					if (currentGLCDPositionLEFT != borderLEFT) {   // If not at the left border (which is 0 xPosition)
						currentGLCDPositionLEFT--;						       // decrement both the left and right border and update the player box
            currentGLCDPositionRIGHT--;
            drawPlayerBox();
          }
        }
				joystick_previous = joystick_value;              // Update previous joystick value
      }
	  }
  }
}

// Function to create the initial game screen text
void createDefaultGameText() {
	GLCD_Clear(White);
  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Black);
  GLCD_DisplayString(1, 0, 0, (unsigned char *)" Use the Joystick to Move Left and Right");
  GLCD_DisplayString(1, 0, 1, (unsigned char *)" Catch the Objects");
  GLCD_DisplayString(2, 0, 1, (unsigned char *)" Score: 0");
}

// This is the Game Logic 
void gameLogic() {
	int joystick_previous, joystick_value = 0; 
 
	// create a random value to be used for each ball's X-Position
	// This X-Position is randomX, where 1 <= randomX <= 18
	// This is because the GLCD is from X-Position 0 to 19 however, the user's basket can only catch from X-Position 1 to 18
	randomX = ((myRandomX()%18) + 1); // (myRandomX()%18 returns a value from 0-17. + 1 results in 1-18
	
	GLCD_DisplayChar(ballHeight, randomX, 1, 'O');   // drops a ball at a position (Y,X) where Y=BallHeight, X=randomX
	
	// while the ball isnt at the bottom of the LCD screen drop the ball slowly & get user input
	// value of 10 worked best for this
	while (ballHeight < 10) { 
		delay();                                       // drops ball at set speed while getting user input
		GLCD_DisplayChar(ballHeight, randomX, 1, ' '); // update previous and current ball locations
		ballHeight += 1;
		GLCD_DisplayChar(ballHeight, randomX, 1, 'O');
	}
	
	ballHeight = 4;       // reset the ball height back to 4
	
	// player has the ball INSIDE of the basket (only 1 space between LEFT and RIGHT bars)
	if (randomX > currentGLCDPositionLEFT && randomX < currentGLCDPositionRIGHT) { 
		score++;            // increment the players score value
		if (speed > 1000) { // increase speed with the score (cap speed at 1000)
			speed -= 10;
		}
		else { 							// when the user reaches max speed change color and display message
			GLCD_SetTextColor(Red);
			GLCD_DisplayString(3, 0, 1, (unsigned char *)" MAX SPEED!"); 
		}
		// when the player scores flash the LEDs and update the displayed score
		flashLEDs(); 
	  snprintf(displayBuffer, sizeof(displayBuffer), " Score: %d", score);
    GLCD_DisplayString(2, 0, 1, (unsigned char*)displayBuffer);
	}
	// player has NOT gotten the ball inside of the basket and the GAME IS OVER
	else {
		ToggleLED_2();						// Toggle LEDs
		GLCD_Clear(White);				// Display Defeat screen with Score (option of replay or exit)
		GLCD_SetBackColor(White);
		GLCD_SetTextColor(Black);
		GLCD_DisplayString (2, 0, 0, (unsigned char *)"        Use the Joystick to move the Cursor         ");
		GLCD_DisplayString (3, 0, 0, (unsigned char *)"       Select your move with Joystick Select      ");
		GLCD_SetTextColor(Red);
		GLCD_DisplayString (2, 0, 1, (unsigned char *)"      DEFEAT     ");
	  snprintf(displayBuffer2, sizeof(displayBuffer2), "   Final Score: %d", score);
    GLCD_DisplayString(3, 0, 1, (unsigned char*)displayBuffer2);
		GLCD_SetTextColor(Black);
		GLCD_DisplayString (4, 0, 1, (unsigned char *)"     > REMATCH      ");
		GLCD_DisplayString (5, 0, 1, (unsigned char *)"       EXIT      ");
		
		// Get user input and wait for KBD_SELECT
		for (;;) {
		  joystick_value = get_button();
        if (joystick_value != joystick_previous) {
					// Update LCD when KBD_UP or KBD_DOWN between REMATCH and EXIT and selection value
					if (joystick_value == KBD_DOWN || joystick_value == KBD_UP) {
						if (selection != 0) {
							selection = 0;
							GLCD_DisplayString (4, 0, 1, (unsigned char *)"     > REMATCH      ");
							GLCD_DisplayString (5, 0, 1, (unsigned char *)"       EXIT      ");
						}
						else {
							selection = 1;
							GLCD_DisplayString (4, 0, 1, (unsigned char *)"       REMATCH      ");
							GLCD_DisplayString (5, 0, 1, (unsigned char *)"     > EXIT      ");
						}
					}
					// Toggle LEDs and based on the selection value REMATCH or EXIT
					else if (joystick_value == KBD_SELECT) {
						ToggleLED_2();
						if (selection == 0) { // REMATCH
							resetValues();
							createDefaultGameText();
							drawPlayerBox();
							gameLogic();
							break;
						} else { // EXIT
							gameRunning=0;
							break;
						}
					}
					joystick_previous = joystick_value;
				}
			}
	}

	//call for new ball if gameRunning (gameRunning is only = 0 when user Exits Defeat screen)
	if (gameRunning==1) {
		gameLogic();
	}
	
}

// Method called to start first game
void gameFallingObjects() {
	resetValues();           // resets values to what they should be at the beginning of a game
  createDefaultGameText(); // displays default text at the top of LCD
  drawPlayerBox();				 // function to draw/update the player catching box
	gameLogic();             // run the game logic (calls itself recursively as long as gameRunning=1)
}

