// Project  : Foosball Scoreboard

#include "RF24.h"
#include <TimerOne.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

#define redScoreUp 4  // The button input for increasing the red score.
#define blueScoreUp 8 // The button input for increasing the blue score.
#define newGame 6     // The button input for a new game.
#define data 5        // The data pin for the neoPixel
#define redGoal 0     // The value the NRF will send when red scores.
#define blueGoal 1    // The value the NRF will send when blue scores.
#define CE 7          // The CE pin for the NRF24L01.
#define CSN 10        // The CSN pin of the NRF24L01.

RF24 radio(CE, CSN);             // Initializes the NRF24l01 CE and CSN pins.
const byte address[6] = "00001"; // NRF 24l01 Addres.
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, data,
                                               NEO_MATRIX_TOP + NEO_MATRIX_RIGHT +
                                                   NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
                                               NEO_GRB + NEO_KHZ800);
const uint16_t colours[] = {
    matrix.Color(255, 0, 0), matrix.Color(0, 0, 255)};
uint32_t numbers[10]{
    // Character Map.
    0b01111110010000100100001001111110, // Map for 0
    0b01000100011111100111111001000000, // Map for 1
    0b01110100010100100100101001000100, // Map for 2
    0b10010010100100101001001011111110, // Map for 3
    0b00011100000100000001000011111110, // Map for 4
    0b10011110100100101001001011110010, // Map for 5
    0b01111000100101001001001001100010, // Map for 6
    0b01000100001001000001010000001100, // Map for 7
    0b00110110010010010100100100110110, // Map for 8
    0b10001100010100100011001000011100  // Map for 9
};
uint8_t goal = 128;
uint8_t redScore = 0;            // The red teams score.
bool redScoreUpPreviousState;    // The previos state of the red score up button.
bool newGameButtonPreviousState; // The previos state of the new game button.

uint8_t blueScore = 0;         // The blue teams score.
bool blueScoreUpPreviousState; // The previos state of the blue score up button.
volatile bool advanceText;     // A variable that goes high every 100ms.
int16_t x;                     // The matrixes x value.
bool clearGame = false;        // A boolean value that is high when the game is over.
bool scoreChanged = true;      // A boolean value that goes high when the scoreboard needs to be updated.
String winningTeam;            // A string that contains whichever team won.

void setup()
{
    radio.begin();                     // Starts comm with the NRF24L01.
    radio.openReadingPipe(0, address); // Opens pipe 0 for reading.
    radio.setPALevel(RF24_PA_MIN);     // Sets the power level of the NRF24L01.
    radio.startListening();            // Tells the NRF24L01 to start listening.
    matrix.begin();                    // Initializes the matrix.
    matrix.setTextWrap(false);         // Allows the text to go off the screen.
    matrix.setBrightness(40);          // Sets the matrix brightness.
    Timer1.initialize(100000);         // Starts an interupt that goes every 100ms.
    Timer1.attachInterrupt(ISR_AdvanceText);
    pinMode(redScoreUp, INPUT_PULLUP);
    pinMode(blueScoreUp, INPUT_PULLUP);
    pinMode(newGame, INPUT_PULLUP);
}

void scrollMessage(String message)
{ // Takes a sring and scrolls it.
    if (advanceText)
    {                           // Only happens if a goal has been scored.
        matrix.clear();         // Whipes the matrix.
        matrix.setCursor(x, 0); // Sets X at whatever stage it is at and y at 0.
        matrix.print(message);  // Prints the next bit of the message.
        if (--x < -42)
        {                        // When this is true message is done.
            x = matrix.width();  // Set X back to 8.
            if (goal == redGoal) // Checks which team has scorred.
                redScore++;
            if (goal == blueGoal)
                blueScore++;
            goal = 128;                            // Sets goal back to its default value.
            scoreChanged = true;                   // Tells the program to change the score displayed.
            if (blueScore == 10 || redScore == 10) // Has either team reached 10?
                clearGame = true;
        }
        matrix.setTextColor(colours[goal]); // Set colour to whichever team scored.
        matrix.show();
        advanceText = false; // Sets the 100ms variable to false.
    }
}

void ISR_AdvanceText()
{
    advanceText = true;
}

void displayScore(uint8_t redScore, uint8_t blueScore)
{
    if (scoreChanged)
    {                   // Only change if the score has changed.
        matrix.clear(); // Whipe the matrix.
        for (uint8_t i = 0; i < 32; i++)
        { // Go through the first half.
            if (bitRead(numbers[redScore], i))
                matrix.setPixelColor(i, colours[redGoal]);
        }
        for (uint8_t i = 32; i < 64; i++)
        {
            if (bitRead(numbers[blueScore], i - 32))
                matrix.setPixelColor(i, colours[blueScore]);
        }
        matrix.show();        // Show what has just been set.
        scoreChanged = false; // Preparing for the next pass.
    }
}

void gameOver()
{ // Clears the game.
    for (uint8_t i = 0; i < 168; i++)
    { // The string has 168 shifts.
        while (!advanceText)
            ;           // Waits here for 100ms.
        matrix.clear(); // Wipe the matrix.
        matrix.setCursor(x, 0);
        if (redScore > blueScore)
        { // Which team won?
            winningTeam = " Red Team";
            matrix.setTextColor(colours[redGoal]);
        }
        else
        {
            winningTeam = " Blue Team";
            matrix.setTextColor(colours[blueGoal]);
        }
        matrix.print("Game Over Winner:" + winningTeam);
        if (--x < -168)
        { // Check if the message is done.
            x = matrix.width();
        }
        matrix.show();
        advanceText = false;
    }
    clearGame = false; // Reset the game.
    redScore = 0;
    blueScore = 0;
    scoreChanged = true;
}

void loop()
{
    displayScore(redScore, blueScore); // Display the current score.
    if (radio.available())
    { // Checks if a message has come in.
        radio.read(&goal, sizeof(goal));
    }
    if (!digitalRead(redScoreUp) && redScoreUpPreviousState)
    {
        scoreChanged = true;
        goal = redGoal;
        redScoreUpPreviousState = false;
    }
    if (digitalRead(redScoreUp))
        redScoreUpPreviousState = true;
    if (!digitalRead(blueScoreUp) && blueScoreUpPreviousState)
    {
        scoreChanged = true;
        goal = blueGoal;
        blueScoreUpPreviousState = false;
    }
    if (digitalRead(blueScoreUp))
        blueScoreUpPreviousState = true;
    // scrollMessage("Goal!!");
    if (!digitalRead(newGame) && newGameButtonPreviousState)
    {
        scoreChanged = false;
        clearGame = true;
        newGameButtonPreviousState = false;
    }
    if (digitalRead(newGame))
        newGameButtonPreviousState = true;
    if (goal != 128)
        scrollMessage("GOAL!!"); // If someone has scored scroll goal.
    if (clearGame)
    { // Is the game done?
        gameOver();
    }
}
