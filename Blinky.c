#include <stdio.h>
#include <string.h>
#include "Blinky.h"
#include "LPC17xx.h"                       
#include "GLCD.h"
#include "LED.h"
#include "KBD.h"
#include "gameTicTacToe.h"
#include "gameFallingObjects.h"
#include "usbdmain.h"

#define __FI        1                      /* Font index 16x24               */
#define __USE_LCD   0										/* Uncomment to use the LCD */

struct __FILE { int handle;  };
FILE __stdout;
FILE __stdin;

int i =0; 																  // used for LEDs

extern unsigned char DRAKE_pixel_data[];    // All Photos for Photo Gallery
extern unsigned char LIQUID_pixel_data[]; 
extern unsigned char MICHAEL_pixel_data[]; 
extern unsigned char WAYNE_pixel_data[]; 
extern unsigned char NAS_pixel_data[]; 
extern unsigned char EM_pixel_data[]; 
extern unsigned char YE_pixel_data[]; 
extern unsigned char UZI_pixel_data[]; 

extern unsigned char AUDIO_pixel_data[];    // Photo for Audio Menu
extern int audio_main (void);								// Call to audio_main in usbdmain.c

unsigned char* pixelDataArray[] = {					// Array of the Photos in Photo Gallery
	NULL,
	DRAKE_pixel_data,
	LIQUID_pixel_data,
	MICHAEL_pixel_data,
	WAYNE_pixel_data,
	NAS_pixel_data,
	EM_pixel_data,
	YE_pixel_data,
	UZI_pixel_data
};

unsigned char* selectedPixelData = NULL;    // create char for Photos in Photo Gallery

int mainMenuSelector = 0;  // used to navigate the main menu where: 0=Audio, 1=Photo, 2=Game

int photoSelection = 1; // used to navigate the photo gallery menu, index starts at 1
int numberOfPhotos = 8; // how many photos in the photo gallery menu? index starts at 1 (8 total photos)

int gameSelection = 0; // used to navigate the games menu starts at 0

typedef enum { // state(s)
	MAINMENU,
	GAMES,
	PHOTOS,
	AUDIO
} Mode;

Mode currentMode = MAINMENU; /* Default state should bring user to main menu */

// Default Menu Text for Main, Audio, Photo, and Games Menus
void DefaultText(const char* text) {
	char displayBuffer[50];
	snprintf(displayBuffer, sizeof(displayBuffer), "%s Menu      ", text);
	
	GLCD_Clear(Black);                 
  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Black);
  GLCD_DisplayString(0, 0, __FI, (unsigned char *)"    Media Center    ");
  GLCD_DisplayString(1, 0, __FI, (unsigned char*)displayBuffer);
  GLCD_SetBackColor(Black);
  GLCD_SetTextColor(White);
}

// Turns all LEDs Off
void LEDsOff() {
	for ( i = 0; i < 8; i++) {
		LED_Off(i);
	}
}

// Updates the Main Menu Text Based on User Input
void updateMainMenuText() {
	GLCD_SetBackColor(Black);
  GLCD_SetTextColor(White);
	GLCD_DisplayString(27, 0, 0, (unsigned char *)"    Move the Joystick Up/Down to Navigate the Menu    ");
  GLCD_DisplayString(28, 0, 0, (unsigned char *)"  Press Select on the Joystick to Select Your Option  ");
	switch (mainMenuSelector) {
		case 0: //is audio
			GLCD_SetBackColor(White);
      GLCD_SetTextColor(Black);
      GLCD_DisplayString(4, 0, __FI, (unsigned char *)" Audio ");
		  GLCD_SetBackColor(Black);
      GLCD_SetTextColor(White);
      GLCD_DisplayString(5, 0, __FI, (unsigned char *)" Photo Gallery ");
	   	GLCD_DisplayString(6, 0, __FI, (unsigned char *)" Games ");
			break;
		case 1: //is photo
			GLCD_SetBackColor(White);
      GLCD_SetTextColor(Black);  
		  GLCD_DisplayString(5, 0, __FI, (unsigned char *)" Photo Gallery ");
		  GLCD_SetBackColor(Black);
      GLCD_SetTextColor(White);
      GLCD_DisplayString(4, 0, __FI, (unsigned char *)" Audio ");
	   	GLCD_DisplayString(6, 0, __FI, (unsigned char *)" Games ");
			break;
		case 2: //is game
			GLCD_SetBackColor(White);
      GLCD_SetTextColor(Black);  
		  GLCD_DisplayString(6, 0, __FI, (unsigned char *)" Games ");
		  GLCD_SetBackColor(Black);
      GLCD_SetTextColor(White);
      GLCD_DisplayString(4, 0, __FI, (unsigned char *)" Audio ");
		  GLCD_DisplayString(5, 0, __FI, (unsigned char *)" Photo Gallery ");
			break;
		default:
			//
			break;
	}
}

//Update Main Menu LEDs based on user Input
void updateMainMenuLED() {
	switch (mainMenuSelector) {
		case 0: //is audio
			LED_On(0);
		  LED_Off(1);
		  LED_Off(2);
			break;
		case 1: //is photo
			LED_On(1);
		  LED_Off(0);
		  LED_Off(2);
			break;
		case 2: //is game
			LED_On(2);
		  LED_Off(0);
		  LED_Off(1);
			break;
		default:
			//
			break;
	}
}

// update the mainMenuSelector based on user input (KBD_UP/KBD_DOWN)
// enter AUDIO, PHOTO, or GAMES menu based on user input + mainMenuSelector value (KBD_SELECT)
void updateMainMenuSelector() {
	int joystick_value = 0;    // stores current joystick value
	int joystick_previous = 0; // stores previous joystick value
	for (;;) {                                   // loop forever	
		joystick_value = get_button();             // obtain current joystick value
		
		if (joystick_value != joystick_previous) { // checks for an updated value from the user
			if (joystick_value == KBD_DOWN) {				 // if joystick input is down
				if (mainMenuSelector == 2) {           // we are at the bottom of the menu
					mainMenuSelector = 0;                // loop to the top of the menu
				} else {
					mainMenuSelector += 1;               // increment selector down the menu otherwise
				}

		  } else if (joystick_value == KBD_UP) {   // if joystick input is up
				if (mainMenuSelector == 0) {           // we are at the top of the menu
					mainMenuSelector = 2;                // loop to the bottom of the menu
				} else {
					mainMenuSelector -= 1;               // increment selector up the menu otherwise
				}
				
		  } else if (joystick_value == KBD_SELECT) {
				if (mainMenuSelector == 0) {        // AUDIO
					//do Audio();
					LEDsOff();
					currentMode = AUDIO;
					break;
				} else if (mainMenuSelector == 1) { // PHOTOS
					//do Photos();
					LEDsOff();
					currentMode = PHOTOS;
					break;
				} else if (mainMenuSelector == 2) { // GAMES
					//do Games();
					LEDsOff();
					currentMode = GAMES;
					break;
				}
			} else { // joystick_value is not one of (DOWN, UP, SELECT)
				//return 0;
				
			}
		joystick_previous = joystick_value; // update previous joystick value (otherwise input is too instant/fast)
		updateMainMenuText(); // update main menu option
		updateMainMenuLED();  // update main menu led
		} else {             
			//__NOP(); or return 0;
		}
		// outside of if statement
	}
}

// Audio Menu Photo and usbdmain.c audio_main() function 
// first time loading into the audio menu will connect to the pc, after connection is established if you leave and enter audio again the audio plays on the board via USB.
// after this first connection the audio connects everytime you enter the audio menu and the audio plays via USB from the desktop PC
void AudioMenu() {
	// display some instructions + a photo and call the audio_main() function from usbdmain.c
	GLCD_SetBackColor(Black);
  GLCD_SetTextColor(White);
	GLCD_DisplayString(27, 0, 0, (unsigned char *)" Rotate the POT above the LEDs to change the volume ");
	GLCD_DisplayString(28, 0, 0, (unsigned char *)" Press Left on the Joystick to exit ");
  GLCD_Bitmap (90, 70, 128,128, AUDIO_pixel_data);
	audio_main();
}

// display photo based on photoSelection value and turn on its LED.
void PhotosMenu() {
	int joystick_value = 0;    // stores current joystick value
	int joystick_previous = 0; // stores previous joystick value
	
  GLCD_SetBackColor(Black);
  GLCD_SetTextColor(White);
	GLCD_DisplayString(27, 0, 0, (unsigned char *)" Move the Joystick Up/Down to Navigate the Menu ");
  GLCD_DisplayString(28, 0, 0, (unsigned char *)" Press Left on the Joystick to exit ");
	
	for(;;) {
		joystick_value = get_button();
		if (joystick_value != joystick_previous) {  // joystick has been used
			if (joystick_value == KBD_DOWN) {         // user hits joystick down
				if (photoSelection == numberOfPhotos) { // we are at the end of the photo library so we want to display the first photo if joystick down
					photoSelection = 1; 									// displays the first photo by setting photoSelection = 1
				}
				else {																	// not at the end of the photo library so just display the next photo
					photoSelection++;											// displays the next photo by incrementing photoSelection
				}
			}
			else if (joystick_value == KBD_UP) {      // user hits joystick up
				if (photoSelection == 1) {							// we are at the beginning of the photo library so we want to display the last photo if joystick up
					photoSelection = numberOfPhotos;			// displays the last photo by setting photoSelection = numberOfPhotos
				}
				else {																	// not at the beginning of the photo library so just display the previous photo
					photoSelection--;											// displays the previous photo by decrementing photoSelection
				}
			}
			else if (joystick_value == KBD_LEFT) {    // user hits joystick left
				LEDsOff();
				currentMode = MAINMENU;									// return to the main menu
				break;
			}
		joystick_previous = joystick_value;
			
		} // end of if joystick was interacted with
		
		// whether keyboard is interacted with or not we need to do the below to initialize the array and display a default image when the library is first opened:
		
		//this initializes the selectedPixelData and pixelDataArray with photoSelection to make it easy to add more photos instead of using a ton of 'if' statements or 'switch' cases
		if (photoSelection > 0 && photoSelection < sizeof(pixelDataArray) / sizeof(pixelDataArray[0])) {
				selectedPixelData = pixelDataArray[photoSelection];
		}
		
		//since we have the photoSelection mapped with the pixelData we only need one if statement for all cases since the sizes of the photos are the same (128, 128) and at the same location (90, 70)
		if (selectedPixelData != NULL) {
			LEDsOff();
			LED_On(photoSelection - 1); // turn on specific led's (can only have values 0-7 here) total of 8 photos. LEDs index starts at 0 so '-1' since photoSelection starts at 1
			GLCD_Bitmap (90, 70, 128,128, selectedPixelData); // display the selected photo mapped to selectedPixelData from photoSelection
		}
		
	} // end of for(;;)
}

// update the games menu text based on user input from gameSelection
void updateGameMenuText() {
	
	switch (gameSelection) {
		case 0: //is game 1
			GLCD_SetBackColor(White);
      GLCD_SetTextColor(Black);
      GLCD_DisplayString(4, 0, __FI, (unsigned char *)" TicTacToe ");
		  GLCD_SetBackColor(Black);
      GLCD_SetTextColor(White);
      GLCD_DisplayString(5, 0, __FI, (unsigned char *)" FallingObjects ");
			break;
		case 1: //is game 2
			GLCD_SetBackColor(White);
      GLCD_SetTextColor(Black);  
		  GLCD_DisplayString(5, 0, __FI, (unsigned char *)" FallingObjects ");
		  GLCD_SetBackColor(Black);
      GLCD_SetTextColor(White);
      GLCD_DisplayString(4, 0, __FI, (unsigned char *)" TicTacToe ");
			break;
		//default:
			//
			//break;
	}
}

// update games menu leds based on user input from gameSelection
void updateGameMenuLED() {
	switch (gameSelection) {
		case 0: //is game 1
			LED_On(0);
		  LED_Off(1);
			break;
		case 1: //is game 2
			LED_On(1);
		  LED_Off(0);
			break;
		default:
			//
			break;
	}
}

// update gameSelection based on user input and enter selected game if input is KBD_SELECT
void GamesMenu() {
	
	int joystick_value = 0;    // stores current joystick value
	int joystick_previous = 0; // stores previous joystick value
	updateGameMenuText();
  GLCD_SetBackColor(Black);
  GLCD_SetTextColor(White);
	GLCD_DisplayString(26, 0, 0, (unsigned char *)" Move the Joystick Up/Down to Navigate the Menu ");
  GLCD_DisplayString(27, 0, 0, (unsigned char *)" Press Select on the Joystick to Select a Game ");
	GLCD_DisplayString(28, 0, 0, (unsigned char *)" Press Left on the Joystick to go back ");
  updateGameMenuLED();
	for(;;) {
		
			joystick_value = get_button();
			if (joystick_value != joystick_previous) { // joystick has been used
				if ((joystick_value == KBD_UP) || (joystick_value == KBD_DOWN)) { // up or down
					if (gameSelection == 0) { // since only 2 games we toggle between them
						gameSelection = 1;  //if 0 go to 1
						
					} else { // if 1 go to 0
						
						gameSelection = 0; 
					}
				} else if (joystick_value == KBD_LEFT) {   // go to main menu
					currentMode = MAINMENU;
					break;
				} else if (joystick_value == KBD_SELECT) { // enter selected game
					if (gameSelection == 0) {
						// game 0
						LEDsOff();
						gameTicTacToe(); // call gameTicTacToe() from gameTicTacToe.c
					} 
					else {
						//game 1
						LEDsOff();
						gameFallingObjects(); // call gameFallingObjects() from gameFallingObjects.c
					}
					break;
				}
				joystick_previous = joystick_value;
				updateGameMenuText();
				updateGameMenuLED();
			}
		
	}
}

/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
int main (void) {

  LED_Init();    /* LED Initialization */
  KBD_Init();		 /* KDC Initialization */			
  GLCD_Init();   /* LCD Initialization */
	SysTick_Config(SystemCoreClock/100); // audio_main
	for (;;) { 									 /* Switch between the different modes/menus */
		switch (currentMode) {
			case MAINMENU:					   // If the main menu is active
		  	//
			  DefaultText("     Main");
			  updateMainMenuText();
		  	updateMainMenuLED();
		  	updateMainMenuSelector();
				break;
			case GAMES:							  // If the games menu is active
				//
			  DefaultText("     Games");
			  GamesMenu();
				break;
			case PHOTOS:              // If the photos menu is active
				//
			  DefaultText("    Photos");
			  PhotosMenu();
				break;
			case AUDIO:               // if the audio menu is active
				//
			  DefaultText("    Audio");
			  AudioMenu();
				break;
		}
	}

}

