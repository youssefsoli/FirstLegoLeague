#pragma config(Sensor, S1,     leftColorSensor, sensorEV3_Color)
#pragma config(Sensor, S2,     rightColorSensor, sensorEV3_Color)
#pragma config(Sensor, S3,     ultraSonic,     sensorEV3_Ultrasonic, modeEV3Ultrasonic_Listen)
#pragma config(Sensor, S4,     touchSensor,    sensorEV3_Touch)
#pragma config(Motor,  motorA,          bottomLeft,    tmotorEV3_Large, PIDControl, driveLeft, encoder)
#pragma config(Motor,  motorB,          bottomRight,   tmotorEV3_Large, PIDControl, driveRight, encoder)
#pragma config(Motor,  motorC,          topRight,      tmotorEV3_Large, PIDControl, encoder)
#pragma config(Motor,  motorD,          topLeft,       tmotorEV3_Large, PIDControl, encoder)

int distanceFromStart;
int speed = 0;
bool motorEnabled = false;
int touches = 0;

int totalModules = 4;

task motorsOn();
void motorsOff();

void calibrateMotors()
{
	if(motorEnabled)
		startTask(motorsOn);
	else {
		stopTask(motorsOn);
		motorsOff();
	}
}

bool isOnWhite()
{
	if(getColorReflected(leftColorSensor) > 75 && getColorReflected(rightColorSensor) > 65)
		return true;
	else
		return false;
}

bool isOnBlack(bool right = true, bool left = true)
{
	if((getColorReflected(leftColorSensor) < 12 || !left) && (getColorReflected(rightColorSensor) < 9 || !right))
		return true;
	else
		return false;
}

task motorsOn()
{
	motorEnabled = true;
	setMotor(bottomLeft, speed);
	setMotor(bottomRight, speed);
}

void motorsOff()
{
	motorEnabled = false;
	stopAllMotors();
}

void gotoWhiteLine()
{
	startTask(motorsOn);
	while(true)
	{
		if(getUSDistance(ultraSonic) > 55 && isOnWhite())
		{
			stopTask(motorsOn);
			motorsOff();
			break;
		}
	}
}

void gotoBlackLine(bool right = true, bool left = true)
{
	startTask(motorsOn);
	while(true)
	{
		if(getUSDistance(ultraSonic) > 55 && isOnBlack(right, left))
		{
			stopTask(motorsOn);
			motorsOff();
			break;
		}
	}
}

void carryPayload()
{
	moveMotor(topRight, 10, rotations, 50);
	forward(1.5, rotations, 50);
	moveMotor(topRight, 6, rotations, -25);
}

void dropPayload()
{
	forward(1, rotations, 50);
	moveMotor(topRight, 6, rotations, 25);
	backward(1.5, rotations, 50);
}

void monoRailStart()
{
	gotoBlackLine();
	forward(1, rotations, 50);
	moveMotor(topRight, 3, rotations, -25);
	backward(5, rotations, 100);
}

void useHammer()
{
	moveMotor(topLeft, 0.4, rotations, 100);
	moveMotor(topLeft, 0.4, rotations, -50);
}

void pushSolarPanel()
{
	forward(7.5, rotations, 100);
	useHammer();
}

void turn45Deg(bool right, float rot = 0.65)
{
	if(right)
	{
		turnRight(rot, rotations, 75);
	} else
	{
		turnLeft(rot, rotations, 75);
	}
}

void hammerTime()
{
	gotoBlackLine(true, true);
	useHammer();
}

void precisionModule()
{
	turn45Deg(false, 0.7);
	forward(3.25, rotations, 75);
	turn45Deg(true, 0.7);
	forward(3, rotations, 75);
	turn45Deg(true);
	forward(2.25, rotations, 75);
	turn45Deg(false, 0.6);
	forward(1.75, rotations, 75);
}

task taskListener()
{
	int secondsPressed = 0;
	setLEDColor(ledGreenFlash);
	while(true)
	{
		while(getTouchValue(touchSensor) == 1)
		{
			sleep(1000);
			secondsPressed++;
			if(secondsPressed >= 2)
			{
				setLEDColor(ledRedFlash);
				sleep(750);
				setLEDColor(ledRed);
				switch(touches)
				{
				case 1:
					hammerTime();
					precisionModule();
					break;
				case 2:
					gotoBlackLine();
					carryPayload();
					dropPayload();
					break;
				case 3:
					monoRailStart();
					break;
				case 4:
					pushSolarPanel();
					break;
				}
				setLEDColor(ledGreenFlash);
			}
		}

		secondsPressed = 0;
	}
}

task touchCounter()
{
	displayText(10, "Selection:");
	displayVariableValues(11, touches);
	while(true)
	{
		if(getBumpedValue(touchSensor) == 1)
		{
			touches = (touches % totalModules) + 1;
			resetBumpedValue(touchSensor);
			displayVariableValues(11, touches);
		}
	}
}

task main()
{
	resetBumpedValue(touchSensor);
	speed = 65;
	resetMotorEncoder(motorA);
	resetMotorEncoder(motorB);
	distanceFromStart = getUSDistance(ultraSonic);
	startTask(taskListener);
	startTask(touchCounter);

	while(true);
}
