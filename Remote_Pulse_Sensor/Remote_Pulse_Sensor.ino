#include <PulseSensorPlayground.h>
#include <ALib0.h>

/* William Taylor 2017
* Read Pulse telemetry from Pulse Sensor Amped, determine beat points and record time indexs.
*/

//Constant Alias Variables
const int pulseSensorPin = A0;			//Analogue pin connected to pulse sensor.
const int visualLEDFeedbackPin = 13;	//Pin for feeding back instances of local pulse.
const int hapticFeedbackPin = 11;		//Pin for driving the haptic motor.
const int timeBetweenReads = 50;		//Pulse read rate.
const int historicalDataCount = 30;		//Amount of historical 'time between beat' records to retain.

//Data Variables
int pulseSignalValue = 0;					//Current value from pulse sensor.
float meanThreshold = 0;					//Mean of data over past 15 seconds, used for threshold.
int timeBetweenBeats[historicalDataCount];	//Stores time differences between the last 30 beats. Overwriting in a circular manner.
long previousMillis = 0;					//Stores the time of the last run through the main loop, used to run code at specified intervals.

//Function declarations
void UpdateMean15(int);
void UpdateMeanRolling(int);
void StoreBeatTime();

//Setup function, configures pinModes.
void setup()
{
	pinMode(pulseSensorPin, INPUT);
	pinMode(visualLEDFeedbackPin, OUTPUT);
	pinMode(hapticFeedbackPin, OUTPUT);
	ResetTimeBetweenBeatsArray();
	Serial.begin(9600);
}

//Main function of program.
void loop()
{
	unsigned long currentMillis = millis();
	static bool hasPeaked = false;
	if (currentMillis - previousMillis >= timeBetweenReads)
	{
		previousMillis = currentMillis;
		pulseSignalValue = analogRead(pulseSensorPin);
		UpdateMean15(pulseSignalValue);
		if (pulseSignalValue > meanThreshold && !hasPeaked) {
			StoreBeatTime();
			hasPeaked = true;
			digitalWrite(visualLEDFeedbackPin, HIGH);
		}
		else if (pulseSignalValue < meanThreshold)
		{
			hasPeaked = false;
			digitalWrite(visualLEDFeedbackPin, LOW);
		}
	}
}

//Updates the mean which determines the threshold for a heartbeat, performed using data collected over the past 15 seconds.
void UpdateMean15(int newData)
{
	static int pastReadings[(1000 / timeBetweenReads) * 15];	//Stores sensor readings from the past 15 seconds.
	static int arraySize = ((1000 / timeBetweenReads) * 15);	//Holds last index in pastReadings array.
	static unsigned long total = 0;								//Stores the sum of all values in an array.
	static int current = 0;										//Current index within pastReadings.
																//Updates 'total' field according to new data as well as updating array to contain new data.
	total -= pastReadings[current];
	pastReadings[current] = newData;
	total += pastReadings[current];
	//Updates 'meanThreshold' field based on latest data set.
	meanThreshold = ((float)total / (float)arraySize) + 2;
	//Increments current or resets to 0, creating a circular array that constantly overwrites itself.
	if (current < arraySize - 1)
	{
		current++;
	}
	else
	{
		current = 0;
	}
}

//Updates the mean which determines the threshold for a heartbeat, performed using a rolling average of all data..
void UpdateMeanRolling(int newData)
{
	static unsigned long dataQuant = 1;
	meanThreshold = meanThreshold + ((newData - meanThreshold) / dataQuant);
	dataQuant++;
}

//Rotates around timeBetweenBeats array, writing and overwriting as needed.
void StoreBeatTime()
{
	static int oldTime = 0;						//Stores time of previous beat.
	static int current = 0;						//Current index within timeBetweenBeats.
	static unsigned long timeOfLastBeat = 0;	//Records time of last beat, used to calculate interval between beats.
												//Stores time difference between this beat and the last in the array.
	timeBetweenBeats[current] = millis() - oldTime;
	//Increments current or resets to 0, creating a circular array that constantly overwrites itself.
	if (current < (historicalDataCount - 1))
	{
		current++;
	}
	else
	{
		current = 0;
		ResetTimeBetweenBeatsArray();
	}
	//Updates oldTime to be current system time. Set here so as to account for time used by the processes in this function.
	oldTime = millis();
}

//Zeros all data points in the timeBetweenBeats array.
void ResetTimeBetweenBeatsArray()
{
	for (int i = 0; i < historicalDataCount; i++)
	{
		timeBetweenBeats[i] = 0;
	}
}

