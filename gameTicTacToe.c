#include <LPC17xx.H>
#include <stdbool.h>
#include "string.h"
#include "GLCD.h"
#include "LED.h"
#include "KBD.h"

int q = 0;       // used to update lcd on user input & toggle led & initialize array[] & win conditions
int winner = 5;  // used for end of game screen
int LEDON_1 = 0; // used to toggle LED. default to all LED OFF when = 0

int positions[9][2] = { 
    //GLCD Positions (int 0-8 beside show where each position is on array shown below)
    {4, 3},  // 0
    {4, 9},  // 1
    {4, 15}, // 2
    {6, 3},  // 3
    {6, 9},  // 4
    {6, 15}, // 5
    {8, 3},  // 6
    {8, 9},  // 7
    {8, 15}  // 8
};

/* 
    array is shown as below 
    0 | 1 | 2
    ---------
    3 | 4 | 5
    ---------
    6 | 7 | 8         */
int array[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

// array[x]. if array[x]=0 this is an empty space, if =1 this is a *, if =2 this is a X, if =3 this is a O

// Function to create and initialize a new board
void createNewBoard() {
    GLCD_Clear(White);
    GLCD_SetBackColor(White);
    GLCD_SetTextColor(Black);
    GLCD_DisplayString (2, 0, 0, (unsigned char *)"        Use the Joystick to move the Cursor         ");
    GLCD_DisplayString (3, 0, 0, (unsigned char *)"       Select your move with Joystick Select      ");
    GLCD_SetBackColor(White);
    GLCD_SetTextColor(DarkGreen);
    GLCD_DisplayString (2, 0, 1, (unsigned char *)"      Player 1      ");
    GLCD_SetTextColor(Black);
    GLCD_DisplayString (4, 0, 1, (unsigned char *)"      |      |      ");
    GLCD_DisplayString (5, 0, 1, (unsigned char *)"------+------+------");
    GLCD_DisplayString (6, 0, 1, (unsigned char *)"      |      |      ");
    GLCD_DisplayString (7, 0, 1, (unsigned char *)"------+------+------");
    GLCD_DisplayString (8, 0, 1, (unsigned char *)"      |      |      ");  
    GLCD_DisplayString(6, 9, 1, (unsigned char *)"*"); // sets middle character to * on LCD
    // resets middle of board to a * and blank elsewhere in array[]
    for (q = 0; q < 9; q++) {
        if (q == 4) {
            array[q] = 1;
        } else {
            array[q] = 0;
        }
    }
}

// Update the current position of the cursor based on joystick input
int updatePos(int joystick_value, int currentPosition) { 
    if (array[currentPosition] == 1) { // if current position has a *
        array[currentPosition] = 0;    // remove the * and make it a blank space again
    }
    
    if (joystick_value == KBD_RIGHT) { // if user selects right
        if (currentPosition != 2 && currentPosition != 5 && currentPosition != 8) { // if not at border
            currentPosition++;         // increment the currentPosition
        }
    }
    else if (joystick_value == KBD_LEFT) { // if user selects left
        if (currentPosition != 0 && currentPosition != 3 && currentPosition != 6) { // if not at border
            currentPosition--;             // decrement the currentPosition
        }
    }
    else if (joystick_value == KBD_UP) { // if user selects up
        if (currentPosition != 0 && currentPosition != 1 && currentPosition != 2) { // if not at border
            currentPosition -= 3;        // decrement the currentPosition
        }
    }
    else if (joystick_value == KBD_DOWN) { // if user selects down
        if (currentPosition != 6 && currentPosition != 7 && currentPosition != 8) { // if not at border
            currentPosition += 3;          // increment the currentPosition
        }
    }
    
    if (array[currentPosition] == 0) { // if the position is empty (blank space)
        array[currentPosition] = 1;    // set the position to a cursor (the * symbol)
    }
    
    return currentPosition;
}

// Update the LCD with the current state of the board
void updateLCD() {
    for (q = 0; q < 9; q++) { // updates blank space with * and * with blank space
        int row = positions[q][0];
        int col = positions[q][1];
        if (array[q] == 1) {
            GLCD_DisplayString(row, col, 1, (unsigned char *)"*");
        } else if (array[q] == 0) {
            GLCD_DisplayString(row, col, 1, (unsigned char *)" ");
        }
    }
}

void ToggleLED_1() {
	if (LEDON_1 == 1) {// 1 is ON
		LEDON_1 = 0;
		for (q = 0; q < 8; q++) {
		  LED_Off(q);
	  }
	} else { // 0 is OFF
		LEDON_1 = 1;
		for (q = 0; q < 8; q++) {
		  LED_On(q);
	  }
	}
}

// Check win conditions after each move
int checkWinConditions(int counter) {
    int joystick_previous, joystick_value = 0;  
	  int selection = 0;
	  int winner = 0;
	
    
		// Check for horizontal and vertical wins
		for (q = 0; q < 3;q++) {
				// Check horizontal wins (rows) ROW at 0, 3, 6
				if (array[q * 3] == 2 && array[q * 3] == array[q * 3 + 1] && array[q * 3] == array[q * 3 + 2]) {
						// Player 1 wins horizontally
						winner = 1;
				}
				if (array[q * 3] == 3 && array[q * 3] == array[q * 3 + 1] && array[q * 3] == array[q * 3 + 2]) {
						// Player 2 wins horizontally
						winner = 2;
				}

				// Check vertical wins (columns) COLUMN at 0, 1, 2
				if (array[q] == 2 && array[q] == array[q + 3] && array[q] == array[q + 6]) {
						// Player 1 wins vertically
						winner = 1;
				}
				if (array[q] == 3 && array[q] == array[q + 3] && array[q] == array[q + 6]) {
						// Player 2 wins vertically
						winner = 2;
				}
		}

		// Check for diagonal wins (2 ways to win a diagonal)
		if ((array[0] == 2 && array[0] == array[4] && array[0] == array[8]) 
			 || (array[2] == 2 && array[2] == array[4] && array[2] == array[6])) {
				// Player 1 wins diagonally
				winner = 1;
		}
		if ((array[0] == 3 && array[0] == array[4] && array[0] == array[8]) 
			 || (array[2] == 3 && array[2] == array[4] && array[2] == array[6])) {
				// Player 2 wins diagonally
				winner = 2;
		}

		if ((winner == 1) || (winner == 2)) {
			ToggleLED_1();
			GLCD_Clear(White);
			GLCD_SetBackColor(White);
			GLCD_SetTextColor(Black);
			GLCD_DisplayString (2, 0, 0, (unsigned char *)"        Use the Joystick to move the Cursor         ");
			GLCD_DisplayString (3, 0, 0, (unsigned char *)"       Select your move with Joystick Select      ");
			if (winner == 1) {
			  GLCD_SetTextColor(DarkGreen);
				GLCD_DisplayString (2, 0, 1, (unsigned char *)"    PLAYER 1 Wins     ");
			}
			else {
			  GLCD_SetTextColor(Red);
				GLCD_DisplayString (2, 0, 1, (unsigned char *)"    PLAYER 2 Wins     ");
			}
			GLCD_SetTextColor(Black);
			GLCD_DisplayString (4, 0, 1, (unsigned char *)"     > REMATCH      ");
			GLCD_DisplayString (5, 0, 1, (unsigned char *)"       EXIT      ");
			
			for (;;) {
				joystick_value = get_button();
        if (joystick_value != joystick_previous) {
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
					else if (joystick_value == KBD_SELECT) {
						ToggleLED_1();
						return selection;
					}
					joystick_previous = joystick_value;
				}
			}
			
			
		}

    // 0 indicates a draw since no winner and counter = 9 means a full board
    if ((counter == 9) && (winner == 0)) {
			ToggleLED_1();
			GLCD_Clear(White);
			GLCD_SetBackColor(White);
			GLCD_SetTextColor(Black);
			GLCD_DisplayString (2, 0, 0, (unsigned char *)"        Use the Joystick to move the Cursor         ");
			GLCD_DisplayString (3, 0, 0, (unsigned char *)"       Select your move with Joystick Select      ");
			GLCD_SetTextColor(Magenta);
			GLCD_DisplayString (2, 0, 1, (unsigned char *)"       DRAW      ");
			GLCD_SetTextColor(Black);
			GLCD_DisplayString (4, 0, 1, (unsigned char *)"     > REMATCH      ");
			GLCD_DisplayString (5, 0, 1, (unsigned char *)"       EXIT      ");
			for (;;) {
				joystick_value = get_button();
        if (joystick_value != joystick_previous) {
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
					else if (joystick_value == KBD_SELECT) {
						ToggleLED_1();
						return selection;
					}
					joystick_previous = joystick_value;
				}
			}
    }

    // No win or draw detected, game continues
    return -1;  // Continue the game
}

// Main game loop
void gameTicTacToe() {
    int joystick_previous, joystick_value, counter = 0, turn = 0;
    int currentPosition = 4;  // Start in the middle

    createNewBoard();  // Initialize board & display the new game

    while (1) {  // Keep the game running until there's a result
        joystick_value = get_button();
        if (joystick_value != joystick_previous) {  // Check if joystick input was detected
            if (joystick_value == KBD_RIGHT || joystick_value == KBD_LEFT
                || joystick_value == KBD_UP || joystick_value == KBD_DOWN) {
                // Update the cursor position based on joystick input
                currentPosition = updatePos(joystick_value, currentPosition);
                updateLCD();  // Update LCD with new position
            }
            else if (joystick_value == KBD_SELECT) {
                // Place player's mark if it's an empty space
                if (array[currentPosition] != 2 && array[currentPosition] != 3) {
                    if (turn == 0) {
                        GLCD_SetTextColor(Red);
                        GLCD_DisplayString(2, 0, 1, (unsigned char *)"      Player 2      ");
                        GLCD_SetTextColor(DarkGreen);
                        GLCD_DisplayChar(positions[currentPosition][0], positions[currentPosition][1], 1, 'X');
                        array[currentPosition] = 2;
                    }
                    else if (turn == 1) {
                        GLCD_SetTextColor(DarkGreen);
                        GLCD_DisplayString(2, 0, 1, (unsigned char *)"      Player 1      ");
                        GLCD_SetTextColor(Red);
                        GLCD_DisplayChar(positions[currentPosition][0], positions[currentPosition][1], 1, 'O');
                        array[currentPosition] = 3;
                    }
                    counter++;  // Increase the counter to track number of moves
                    turn = turn ^ 1;  // Toggle the turn (0 for Player 1, 1 for Player 2)
                }

                // Check if there's a win or draw after the move
                winner = checkWinConditions(counter);
                if (winner != -1) {
                    // If a winner is found or it's a draw, display the result and wait for input
                    if (winner == 0) { // (SELECTED REPLAY)
											gameTicTacToe();  // Recreate the board for the next game
											break;  // End the current game loop
										} 
										else if (winner == 1) { // (SELECTED EXIT)
											break;  // End the current game 
										}
                }
            }

            joystick_previous = joystick_value;  // Update previous joystick value
        }
    }
}

