#pragma config(Sensor, S1,     leftColorSensor, sensorEV3_Color)
#pragma config(Sensor, S2,     rightColorSensor, sensorEV3_Color)
#pragma config(Sensor, S3,     ultraSonic,     sensorEV3_Ultrasonic, modeEV3Ultrasonic_Listen)
#pragma config(Sensor, S4,     gyroScope,      sensorEV3_Gyro, modeEV3Gyro_RateAndAngle)
#pragma config(Motor,  motorA,          bottomLeft,    tmotorEV3_Large, PIDControl, driveLeft, encoder)
#pragma config(Motor,  motorB,          bottomRight,   tmotorEV3_Large, PIDControl, driveRight, encoder)
#pragma config(Motor,  motorC,          topRight,      tmotorEV3_Large, PIDControl, encoder)
#pragma config(Motor,  motorD,          topLeft,       tmotorEV3_Large, PIDControl, encoder)

int distanceFromStart;
int speed = 0;
bool motorEnabled = false;

task motorsOn();
void motorsOff();

void calibrateMotors()
{
	if(motorEnabled)
		startTask(motorsOn);
	else
		motorsOff();
}

task centerRobot()
{
	int gyroDegrees;
	while(true)
	{
		gyroDegrees = getGyroDegrees(gyroScope);
		if(gyroDegrees > 2)
		{
			turnLeft(0.1, degrees, 50);
			calibrateMotors();
		}
		else if(gyroDegrees < -2)
		{
			turnRight(0.1, degrees, 50);
			calibrateMotors();
		}
	}
}

bool isOnWhite()
{
	if(getColorReflected(leftColorSensor) > 75 && getColorReflected(rightColorSensor) > 65)
		return true;
	else
		return false;
}

bool isOnBlack()
{
	if(getColorReflected(leftColorSensor) < 12 && getColorReflected(rightColorSensor) < 9)
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

void gotoBlackLine()
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

void carryPayload()
{
	moveMotor(topRight, 3, rotations, 50);
	forward(1.5, rotations, 50);
	moveMotor(topRight, 1.5, rotations, -25);
}

void dropPayload()
{
	forward(1.5, rotations, 50);
	moveMotor(topRight, 1.5, rotations, 25);
	backward(1.5, rotations, 50);
}

void hammerTime()
{
	gotoBlackLine();
	moveMotor(topLeft, 0.4, rotations, 100);
	moveMotor(topLeft, 0.3, rotations, -100);
}

task main()
{
	speed = 100;
	resetMotorEncoder(motorA);
	resetMotorEncoder(motorB);
	distanceFromStart = getUSDistance(ultraSonic);
	resetGyro(gyroScope);
	//startTask(centerRobot);
	//gotoBlackLine();
	//carryPayload();
	hammerTime();
	//sleep(20000);
	//dropPayload();
}
