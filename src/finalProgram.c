#pragma config(Sensor, S1,     leftColorSensor, sensorEV3_Color)
#pragma config(Sensor, S2,     rightColorSensor, sensorEV3_Color)
#pragma config(Sensor, S3,     ultraSonic,     sensorEV3_Ultrasonic, modeEV3Ultrasonic_Listen)
#pragma config(Sensor, S4,     touchSensor,    sensorEV3_Touch)
#pragma config(Motor,  motorA,          bottomLeft,    tmotorEV3_Large, PIDControl, driveLeft, encoder)
#pragma config(Motor,  motorB,          bottomRight,   tmotorEV3_Large, PIDControl, driveRight, encoder)
#pragma config(Motor,  motorC,          topRight,      tmotorEV3_Large, PIDControl, encoder)
#pragma config(Motor,  motorD,          topLeft,       tmotorEV3_Large, PIDControl, encoder)

int speed = 0;
int touches = 0;
int totalRuns = 4;

task motorsOn()
{
	setMotor(bottomLeft, speed); // Turns on the left motor
	setMotor(bottomRight, speed); // Turns on the right motor
}

bool isOnBlack(bool right = true, bool left = true)
{
	if((getColorReflected(leftColorSensor) < 12 || !left) && (getColorReflected(rightColorSensor) < 9 || !right)) // Check the light intensity for black
		return true;
	else
		return false;
}

void gotoBlackLine(bool right = true, bool left = true, int distance = 55)
{
	startTask(motorsOn); // Starts a thread that moves the robot forward
	while(true)
	{
		if(getUSDistance(ultraSonic) > distance && isOnBlack(right, left)) // Keep checking if it has reached a certain distance and is on a black line, then stop if it has
		{
			stopTask(motorsOn);
			stopAllMotors();
			break;
		}
	}
}

void turn45Deg(bool right, float rot = 0.65)
{
	if(right) // Check whether or not to turn right
	{
		turnRight(rot, rotations, 75);
	} else
	{
		turnLeft(rot, rotations, 75);
	}
}

void goDistance(int distance)
{
	startTask(motorsOn); // Starts a thread that moves the robot forward
	while(true)
	{
		if(getUSDistance(ultraSonic) >= distance) // Keep checking if it has reached a certain distance, then stop if it has
		{
			stopTask(motorsOn);
			stopAllMotors();
			break;
		}
	}
}

void useHammer(double rot = 0.4)
{
	moveMotor(topLeft, rot, rotations, 100); // Forces down hammer with motor at full speed
	moveMotor(topLeft, rot, rotations, -50); // Move the hammer back up gracefully
}

void hammerTime()
{
	gotoBlackLine(true, true, 55); // Move towards the black line in front of spaceship module
	useHammer(); // Hit the module with the hammer
}

void pushModule()
{
	turn45Deg(false); // Turn towards center of module
	forward(4, rotations, 75); // Move until robot is in front
	turn45Deg(false); // Reorient back looking forwards
	sleep(100); // Pause for a split second in order to combat jitter
	gotoBlackLine(false, true, 30); // Go towards the black line, only using the left sensor to identify it
	forward(580, degrees, 75); // Move towards it with degree precision
	backward(580, degrees, 75); // Move back to original position
}

void otherSolarPanel()
{
	turn45Deg(true); // Turn towards center of solar panel
	forward(5.5, rotations, 75); // Move until robot is right in front of panel
	turn45Deg(false); // Turn back forward
	forward(3, rotations, 75); // Go towards solar panel
	useHammer(0.3); // Hit the panel with the hammer with 0.3 rotations because of the board's edge
}

void precisionModule()
{
	backward(6, rotations, 75); // Move robot back and push the solar panel, this is the final movement
}

void monoRailStart()
{
	gotoBlackLine(); // Go to black line
	forward(1, rotations, 50); // Go 1 rotation forward to insert arm below the monorail
	moveMotor(topRight, 3, rotations, -25); // Moves arm up to raise monorail
	backward(5, rotations, 100); // Go back to base
}

void pushSolarPanel()
{
	goDistance(81);
	useHammer();
	turn45Deg(true, 0.2);
	backward(7.5, rotations, 100);
}

void coreExtraction()
{
	// Keep going forward until robot is 44 cm away from the board based on the ultraSonic
	goDistance(44);

	// Lower the hand
	moveMotor(topRight, 3, rotations, 100);

	// Move forward and push them off
	forward(3, rotations, 65);

	// Raise the hand back to original
	moveMotor(topRight, 3, rotations, -100);

	// Tilt towards corner of base
	turn45Deg(false, 0.35);

	// Reverse until within base
	backward(6, rotations, 65);
}

task taskListener()
{
	int millisecondsPressed = 0; // Holds the number of milliseconds the button has been pressed
	setLEDColor(ledGreenFlash);
	while(true)
	{
		while(getTouchValue(touchSensor) == 1) // Loops as long as it is pressed
		{
			sleep(1); // Sleep for a millisecond
			millisecondsPressed++; // Add 1 millisecond to the timer
			if(millisecondsPressed >= 2000) // If it reaches 2000 or 2 seconds, start the selected mission
			{
				setLEDColor(ledRedFlash); // Indicate module is about to start
				sleep(750); // Delay before the module starts
				setLEDColor(ledRed); // Indicate module started
				switch(touches) // See which module to run
				{
				// Final module
				case 1:
					hammerTime();
					pushModule();
					otherSolarPanel();
					precisionModule();
					break;
				// Monorail module
				case 2:
					monoRailStart();
					break;
				// Solar panel push module
				case 3:
					pushSolarPanel();
					break;
				// Core extraction module
				case 4:
					coreExtraction();
					break;
				}
				setLEDColor(ledGreenFlash); // Indicate module is done executing
			}
		}

		millisecondsPressed = 0; // Reset the number of milliseconds being pressed if the button is let go
	}
}

task touchCounter()
{
	displayText(10, "Selection:"); // Displays "Selection: " on line 10
	displayVariableValues(11, touches); // Displays the initial value of touches -> 0
	while(true) // Always run since it's a listener
	{
		if(getBumpedValue(touchSensor) == 1) // Check if the button was pressed and released
		{
			touches = (touches % totalRuns) + 1; // Add 1 to the touch counter and loop back to 1 if
			resetBumpedValue(touchSensor); // Reset touch value to 0 in order to capture the next touch
			displayVariableValues(11, touches); // Display the current module number
		}
	}
}

task main()
{
	resetBumpedValue(touchSensor); // Reset touch sensor
	speed = 65; // Set the default speed for the robot to run at
	startTask(taskListener); // Start the taskListener thread
	startTask(touchCounter); // Start the touchCounter thread
	while(true); // Keeps main thread busy while the listeners run in the background
}
