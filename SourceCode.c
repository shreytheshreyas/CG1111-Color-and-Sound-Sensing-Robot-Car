#include <MeMCore.h>
#define RGBWait 200
#define LDRWait 10
#define MOTOR1_TUNE -1
#define MOTOR2_TUNE 1
#define FWDSPEEDL 133 // default speed of left motor
#define FWDSPEEDR 155 // default speed of right motor
/*Below, we declare objects of a particular class type in order
* to control the respective components of the mbot
*/
MeLightSensor lightSensor(PORT_8);
MeLineFollower lineFinder(PORT_2);
MePort Sound(PORT_3);
MePort irSensor(PORT_4);
MeDCMotor motor1(M1);
MeDCMotor motor2(M2);
MeRGBLed led( 0 , 30 );
MeBuzzer buzzer;
// Define colour sensor LED pins
int ledArray[ 3 ][ 3 ] = {
{ 255 , 0 , 0 },
{ 0 , 255 , 0 },
{ 0 , 0 , 255 }
};
//placeholders for colour detected
int red = 0 , green = 0 , blue = 0 ;
int rightSensor = 0 , leftSensor = 0 ;
int actionBasedOnColour = 0 ;
//floats to hold colour arrays
float colourArray[] = { 0 , 0 , 0 };
float whiteArray[] = { 526 , 368 , 401 };
float blackArray[] = { 339 , 231 , 255 };
float greyDiff[] = { 187 , 137 , 146 };
void setup (){
Serial . begin ( 9600 );
led.setpin( 13 );
led.setColor( 0 , 0 , 0 );
setBalance();
}
void loop () {
// check if im on a line finder then i need to read colour or sound
if (iAmOnALine()) {
stopMoving();
readColour();
actOnColour(colourArray);
} else {
moveForward();
}
}
/*The below function is used to print the whiteArray and blackArray.
*Also used in the readColour function so as to print out the RGB values
corresponding to a
*particular colour.
*/
void printArray( float arr[], int size ) {
int i;
for (i = 0 ; i < size ; i++) {
if (i == 0 )
Serial . println ( "R" );
if (i == 1 )
Serial . println ( "G" );
if (i == 2 )
Serial . println ( "B" );
Serial . println (arr[i]);
}
}
/* The function below is used to get the calibrated values for the colour
sensor
* for different lighting conditions. This function is commented during the
race so as
* to not interfere with the mbot's decision making.
*/
void setBalance(){
//Scanning white sample
Serial . println ( "Put White Sample For Calibration ..." );
delay ( 5000 ); // so that we have time for put in the sample
led.setColor( 0 , 0 , 0 ); // switch OFF during Calibration (beginning of the
process)
//Storing the maximum values for the RGB values in the white array
for ( int i = 0 ;i<= 2 ;i++){
led.setColor(ledArray[i][ 0 ], ledArray[i][ 1 ], ledArray[i][ 2 ]);
led.show();
delay (RGBWait);
whiteArray[i] = getAvgReading( 5 ); //scan 5 times and return the
average, (for accuracy)
led.setColor( 0 , 0 , 0 );
led.show();
delay (RGBWait);
}
//Scanning black sample
Serial . println ( "Put Black Sample For Calibration ..." );
delay ( 5000 ); //delay for five seconds for getting sample ready
//Storing the minimum values for the RGB values in the black array
for ( int i = 0 ;i<= 2 ;i++){
led.setColor(ledArray[i][ 0 ], ledArray[i][ 1 ], ledArray[i][ 2 ]);
led.show();
delay (RGBWait);
blackArray[i] = getAvgReading( 5 );
led.setColor( 0 , 0 , 0 );
led.show();
delay (RGBWait);
//the difference between the maximum and the minimum gives the range
for RGB values for that lighting condition
greyDiff[i] = whiteArray[i] - blackArray[i];
}
//delay another 5 seconds for getting ready colour objects
Serial . println ( "Colour Sensor Is Ready." );
delay ( 5000 );
printArray(whiteArray, 3 ); // Prints the RGB values of the white array
printArray(blackArray, 3 ); // Prints the RGB values of the black array
}
/*We use this function to calculate the RGB values
*in that particular light condition.
*/
void readColour() {
for ( int c = 0 ;c<= 2 ;c++){
led.setColor(ledArray[c][ 0 ], ledArray[c][ 1 ], ledArray[c][ 2 ]);
led.show();
delay (RGBWait);
colourArray[c] = getAvgReading( 5 );
colourArray[c] = (colourArray[c] - blackArray[c])/(greyDiff[c])* 255 ;
led.setColor( 0 , 0 , 0 );
led.show();
delay (RGBWait);
}
delay (LDRWait);
printArray(colourArray, 3 );
}
/*The function below is used to get the average value of for each colour
for the respective
while calibrating.
*/
int getAvgReading( int times){
//find the average reading for the requested number of times of scanning
LDR
int total = 0 ;
int reading = 0 ;
for ( int i = 0 ;i < times;i++){
reading = lightSensor. read ();
total = reading + total;
delay (LDRWait);
}
//calculate the average and return it
return total/times;
}
//The function below will make the mbot perform the required challenge to
the corresponding colour.
void actOnColour( float colourArray[]) {
if (isLightBlue(colourArray))
twoSuccessiveRight();
else if (isYellow(colourArray))
uTurn();
else if (isRed(colourArray))
turnLeft(FWDSPEEDR * 3 );
else if (isPurple(colourArray))
twoSuccessiveLeft();
else if (isGreen(colourArray))
turnRight(FWDSPEEDL * 3.5 );
else {
delay ( 100 );
if (soundSensor() == 1 )
turnRight(FWDSPEEDL * 3.5 );
if (soundSensor() == 2 )
turnLeft(FWDSPEEDR * 3 );
if (soundSensor == 0 )
victoryTone();
}
}
/*Function to activate the IR sensors and make sure the
* mbot doesn't crash into a maze wall.
* We take the difference between the left and the right
* IR sensor values because, when a maze wall was placed in front
* of the right sensor, the values of both the right IR sensor and the left
IR
* sensor were affected. So we noticed a pattern, if the difference between
the IR sensor
* values is less than -300 then the mbot will be close to a left maze
wall, so we shift it
* to the right and if the difference between the IR sensor values is more
than 200 it will be closer
* to a right maze wall, so we shift it to the left.
*/
void courseCorrect() {
int difference = 0 ;
rightSensor = irSensor.aRead2();
leftSensor = irSensor.aRead1();
difference = leftSensor - rightSensor;
if (difference < -300 ) {
turnRight(FWDSPEEDL);
}
if (difference > 200 ) {
turnLeft(FWDSPEEDR);
}
}
//Performs left turn.
void turnLeft( int delayTime){
motor1. run (MOTOR1_TUNE * FWDSPEEDL * -1 );
motor2. run (FWDSPEEDR);
delay (delayTime);
}
//Performs right turn.
void turnRight( int delayTime){
motor1. run (MOTOR1_TUNE * FWDSPEEDL);
motor2. run (MOTOR2_TUNE * FWDSPEEDR * -1 );
delay (delayTime);
}
//Performs two successive right turns.
int twoSuccessiveRight() {
turnRight(FWDSPEEDL * 3.5 );
moveForward();
delay ( 4 *(FWDSPEEDL + FWDSPEEDR));
turnRight(FWDSPEEDL * 3.5 );
}
// Performs two successive left turns.
int twoSuccessiveLeft() {
turnLeft(FWDSPEEDL * 3.5 );
moveForward();
delay ( 4 *(FWDSPEEDL + FWDSPEEDR));
turnLeft(FWDSPEEDL * 3.5 );
}
// Performs u-turn.
int uTurn() {
turnRight(FWDSPEEDL * 7.0 );
}
// Makes sure mbot is on the black line.
int iAmOnALine() {
int sensorState = lineFinder.readSensors();
if (sensorState == S1_IN_S2_IN)
return 1 ;
return 0 ;
}
// Makes the mbot stop.
void stopMoving() {
motor1. run ( 0 );
motor2. run ( 0 );
}
// Makes mbot move forward without hitting a wall.
void moveForward() {
courseCorrect(); //Activates IR sensors to makes sure mbot does not hit
the wall
motor1. run (MOTOR1_TUNE * FWDSPEEDL);
motor2. run (MOTOR2_TUNE * FWDSPEEDR);
}
//check if colour is red
int isRed( float arr[]) {
if (arr[ 0 ] > 215 && arr[ 1 ] > 45 && arr[ 2 ] > 35 )
return 1 ;
return 0 ;
}
//check if colour is yellow
int isYellow( float arr[]) {
if (arr[ 0 ] > 250 pp&& arr[ 1 ] > 200 && arr[ 2 ] > 70 )
return 1 ;
return 0 ;
}
//check if colour is green
int isGreen( float arr[]){
if (arr[ 0 ] > 60 && arr[ 1 ] > 130 && arr[ 2 ] > 55 )
return 1 ;
return 0 ;
}
//check if colour is purple
int isPurple( float arr[]){
if (arr[ 0 ] > 125 && arr[ 1 ] > 115 && arr[ 2 ] > 170 )
return 1 ;
return 0 ;
}
//check if colour is light blue
int isLightBlue( float arr[]) {
if (arr[ 0 ] > 150 && arr[ 1 ] > 230 && arr[ 2 ] > 220 )
return 1 ;
return 0 ;
}
/*This function will check whether the sound played during the sound
challenges
*are of high frequency or low frequency. if the sound played has a high
frequency
*the value of the highpass filter goes beyond 700 and it returns 1. if the
sound played
*has a low frequency the value of the highpass filter will be below 600
and the value of
*the lowpass filter goes beyond 600 and as a result it returns 2. If none
of the conditions
*are met the function returns a value of 0.
*/
int soundSensor(){
int highpass = Sound.aRead1();
int lowpass = Sound.aRead2();
if (highpass > 700 )
return 1 ;
else if (lowpass > 600 && highpass < 600 )
return 2 ;
return 0 ;
}
/*This function plays the victory tone when the mbot
* reaches the end of the maze.
*/
void victoryTone() {
// Array for the frequencies of every note in the song.
int melody[] =
{ 587 , 587 , 587 ,
587 , 523 , 440 , 349 ,
349 , 587 , 587 , 523 ,
466 , 523 , 523 , 587 ,
622 , 622 , 622 , 622 ,
622 , 587 , 523 , 466 ,
466 , 587 , 587 , 523 ,
466 , 587 };
// Array for the duration of every note.
int noteDurations[] =
{ 100 , 100 , 150 ,
50 , 100 , 50 , 150 ,
50 , 100 , 100 , 50 ,
50 , 250 , 100 , 150 ,
50 , 50 , 100 , 100 ,
50 , 100 , 100 , 100 ,
50 , 100 , 50 , 100 ,
50 , 100 };
// Playing the song.
for ( int note = 0 ; note < sizeof (melody) / sizeof ( int ); note++) {
buzzer. tone (melody[note] * 12 , noteDurations[note] * 1.5 );
delay (noteDurations[note]);
buzzer. noTone ();
}
stopMoving();
// Infinite loop - end of program.
while ( 1 ) {
}
}