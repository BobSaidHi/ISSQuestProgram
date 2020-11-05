//====================================================================================
//
// StarterKitControl.cpp
//
// Author: Cory Duce
// Version: 1.0
// 
// This class provides control of the Quest For Space Arduino Starter Kit Shield
// This code has been tested with the Arduino Uno
//
// Change Log
// 
// August 14 2108: v1.0 Initial release.
//
//=====================================================================================


#include "StarterKitControl.h"


//Public methods

StarterKitControl::StarterKitControl()
	//initializer list
	:I2C_portA(Pin_Soft_PortA_SDA,Pin_Soft_PortA_SCL, true),I2C_portB(Pin_Soft_PortB_SDA,Pin_Soft_PortB_SCL, true)
	
{
	
	
}
//============================================================
//
// Initialize the platform
// -Start up the serial port
// -Put all the IO in a known safe state (heater off etc)
// -start the periodic timer and get interrupt setup
// -Startup the I2C sensor ports
// -Init the SD card and determine the name for the next log file
//
//============================================================
void StarterKitControl::InitPlatform()
{
  	
	
	//Serial is used for showing status to user
	Serial.begin(115200);
	Serial << F("     Quest For Space") << endl; 
	Serial << F("  Starter Kit for Arduino") << endl;
	Serial << F("---------------------------")  << endl;
	
	Serial << endl << "Starting up..." << endl;
	
	//Config all the IO directions and initial states etc
	pinMode(LED_Running, OUTPUT);
	pinMode(LED_Stopped, OUTPUT);
	pinMode(LED_PortA_OK, OUTPUT);
	pinMode(LED_PortA_FAULT, OUTPUT);
	pinMode(LED_PortB_OK, OUTPUT);
	pinMode(LED_PortB_FAULT, OUTPUT);
	
	pinMode(Btn_Start, INPUT_PULLUP);
	
	
	
	//start with the heater and fan off
	HeaterOff();
	FanOff();
		
	//Start with all the LEDs red
	Running_LED_Status(false);
	PortA_LED_Status(false);
	PortB_LED_Status(false);
		
	//flag that set in the once per sec interrupt routine.
	IRQ_Flag = false;
		
	
	//This periodic interrupt runs 1/sec and sets a flag to indicate we should log and send new data
	Timer1.initialize(1000000);
	Timer1.attachInterrupt(IRQ_Handler); // IRQ_Handler() to run every 1 second  
	
	//Start the I2C ports
	I2C_portA.i2c_init();
	I2C_portB.i2c_init();
	
	//wait for the first two interrupts 
	//For some reason, the first two happen quicker than they should
	while(!IRQ_Flag);
	IRQ_Flag = false;
		
	while(!IRQ_Flag);
	IRQ_Flag = false;
	
	
	//we start off in a pre run state where sensor data is shown but not logged
	_PreRunState = true;	
		
	
	
	//================================================================================
	//Init the SD card and get the next valid file name to use for the log file
	//open the log file and populate the header with the colum names etc
	//
	//File names are: xxxx.txt where x=0..9
	//================================================================================
	
	bool foundValidName = false;
	int fileNameNumber = 0;
	char fname [11];
	
	if (SD.begin(10)) 
	{
		Serial <<"uSD card detected. ";
		
		//Search for the next available valid file name to use for the result data
				
		while(!foundValidName && fileNameNumber < 9999)
		{		
			//Generate the next possible file name
			fileNameNumber++;
			sprintf(fname,"%04d.txt",fileNameNumber);
			
			//Check if the file does not exists
			//we will use it for our log file
			if(!SD.exists(fname))
			{
				foundValidName = true;
			}
			
		}
		
		if( fileNameNumber == 9999)
		{
			Serial << endl << "No available file names left. Erase SD card and try again." << endl;
			_uSD_detected = false;
		}
		else
		{
					
			Serial << "Using log file " << fname << endl;
			_logFileName = String(fname);	//save the name of the log file
			_logFile = SD.open(fname, FILE_WRITE);
			
			//Fill in the sensor types and colum headers
			_logFile.println(F("Time[sec],PortA[degC],PortB[degC],Heater,Fan"));
			_logFile.close();
			
			_uSD_detected = true;
		}
	}
	else
	{
		_uSD_detected = false;
		Serial <<F("No uSD card detected. Logging only to terminal.") << endl;
		
	}
 	
	
	
}

//==============================
// StartButton
//
// Returns true if the button is pressed, false if not
//
// The IRQ flag is also monitored so that the StartButton() function can be 
// continuously monitored in a loop and the periodic task will still run
//==============================
bool StarterKitControl::StartButton()
{
	if(IRQ_Flag)
	{
			IRQ_Flag = false;
			PeriodicTask();
	}
	
	return !digitalRead(Btn_Start);
}


//==============================
// Heater Control
//==============================
void StarterKitControl::HeaterOn()
{
	pinMode(Pin_HeaterEnable, OUTPUT);
	digitalWrite(Pin_HeaterEnable, HIGH);
	
	_state_Heater = true;
}

void StarterKitControl::HeaterOff()
{
	pinMode(Pin_HeaterEnable, OUTPUT);
	digitalWrite(Pin_HeaterEnable, LOW);
	
	_state_Heater = false;
}


//==============================
// Fan Control
//==============================
void StarterKitControl::FanOn()
{
	pinMode(Pin_FanEnable, OUTPUT);
	digitalWrite(Pin_FanEnable, HIGH);
	
	_state_Fan = true;
}

void StarterKitControl::FanOff()
{
	pinMode(Pin_FanEnable, OUTPUT);
	digitalWrite(Pin_FanEnable, LOW);
	
	_state_Fan = false;
}
	
//==============================
// LED Control
//==============================
void StarterKitControl::Running_LED_Status(bool state)
{
	
	if(state)
	{
		digitalWrite(LED_Running, HIGH);
		digitalWrite(LED_Stopped, LOW);		
	}
	else
	{
		digitalWrite(LED_Running, LOW);
		digitalWrite(LED_Stopped, HIGH);	
	}
}
	
void StarterKitControl::PortA_LED_Status(bool state)
{
	
	if(state)
	{
		digitalWrite(LED_PortA_OK, HIGH);
		digitalWrite(LED_PortA_FAULT, LOW);		
	}
	else
	{
		digitalWrite(LED_PortA_OK, LOW);
		digitalWrite(LED_PortA_FAULT, HIGH);	
	}
}

void StarterKitControl::PortB_LED_Status(bool state)
{
	
	if(state)
	{
		digitalWrite(LED_PortB_OK, HIGH);
		digitalWrite(LED_PortB_FAULT, LOW);		
	}
	else
	{
		digitalWrite(LED_PortB_OK, LOW);
		digitalWrite(LED_PortB_FAULT, HIGH);	
	}
}



void StarterKitControl::StartExperiment()
{
	Serial << "Starting Experiment..." << endl << endl;
	
	//we start off in a pre run state where sensor data is shown but not logged
	//Now that we are ready to run the experiment, set this to false so that data
	//gets logged.
	_PreRunState = false;
	
	Running_LED_Status(true);
	
	Serial << F("|   Time    |   PortA   |   PortB   |  Heater   |    Fan    |") << endl;
	Serial << F("|   [sec]   |  [degC]   |  [degC]   | [on/off]  | [on/off]  |") << endl;
	Serial << F("|-----------|-----------|-----------|-----------|-----------|") << endl;
	
	//syncronize to the next interrupt
	while(!IRQ_Flag);
	IRQ_Flag = false;
	
	//store the actual start time. We will use this to offset the sample time stamps which also use millis
	//This will remove the startup time from the sample timestamps
	
	delay(1000);
	_StartTime = millis();
	
	//
	
	
}

void StarterKitControl::EndExperiment()
{
	//stop the periodic task that is collecting and sending data to the SP
	Timer1.detachInterrupt();
	delayMicroseconds(5000);
	
	//Send message to terminal that we are finished
	
	//indicate on the LED
	Running_LED_Status(false);
	
	//make sure the heater is off
	HeaterOff();
	FanOff();
	
	Serial << endl << "Finished Experiment. Press Reset to run again." << endl;
	
	//Set LED to show Experiment is Finished
	Running_LED_Status(false);
	
	//Wait to be shutdown or reset
	while(1);
	
	//should never get here
	
}




//Private Methods

//====================================
// PeriodicTask
//
// This is called once per second
// by the delay routine when the 
// interrupt flag has been set
//====================================
void StarterKitControl::PeriodicTask()
{
	//Until the "Start" button is pressed we are in the PreRunState where data from the sensors is
	//displayed, but the experiment is not running and data is not being logged. This lets the user
	//check that everything is working before pressing start.
	//After the Start button is pressed, _PreRunState becomes false, and data is logged from that 
	//point on.
	//The only way to end the experiment early is to press the reset button
	
	
	//Update the time stamp and get the data from the two sensors
	
	getPortA();
	getPortB();
	_timestamp = (millis() - _StartTime);
	
	if(_PreRunState)
	{
		Running_LED_Status(false);
		if(_portA_Valid)
			Serial << F("PortA ") << _portA_Temperature << "  ";
		else
			Serial << F("PortA -----  ");
		
		if(_portB_Valid)
			Serial << F("PortB ") << _portB_Temperature;
		else
			Serial << F("PortB -----  ");
		
		Serial <<  F("   ** Press Start To Begin Experiment **") << endl;
	}
	else 
	{
		Running_LED_Status(true);
		
		//Output the data the terminal, and the SD card if present
		dataToTerminal();
		
		if(_uSD_detected)
			dataToSDcard();	
		
	}
	
	
	
}

//==========================================================================
//
// Sends the current data (time stamp, sensor temps, heater and fan states) 
// to the terminal in a nice easy to read format
//
//==========================================================================
void StarterKitControl::dataToTerminal()
{
	Serial << F("   ")<< _timestamp/1000 << "\t";
		
	//port A temp
	if(_portA_Valid)
		Serial << "\t" << _portA_Temperature << "\t";
	else
		Serial << F("\t-----,")<< "\t" ;
	
	//port B temp
	if(_portB_Valid)
		Serial << F("    ")<< _portB_Temperature<<  "\t";
	else
		Serial << F("    -----") << "\t";
	
	//heater state
	if(_state_Heater)
		Serial << F("  on\t");
	else
		Serial << F("  off\t") ;
	
	
	//fan state
	if(_state_Fan)
		Serial << F("   on");
	else
		Serial << F("   off") ;
	
	Serial << endl;

}

//==========================================================================
//
// Sends the current data (time stamp, sensor temps, heater and fan states) 
// to the memory card in a csv format
//
//==========================================================================
void StarterKitControl::dataToSDcard()
{
	File file = SD.open(_logFileName.c_str(), FILE_WRITE);
	//File file = SD.open("a.txt", FILE_WRITE);
	String line = String("");
	
	
	//If the file opened, log the data
	if(file)
	{
		//Time stamp
		line += String(_timestamp/1000);
		line += String(",");
		
		//Port A Temperature
		if(_portA_Valid)
			line += String(_portA_Temperature);
		line += String(",");
		
		//Port B Temperature
		if(_portB_Valid)
			line += String(_portB_Temperature);
		line += String(",");
		
		//Heater and Fan states
		if(_state_Heater)
			line += String("1,");
		else
			line += String("0,");
		
		
		if(_state_Fan)
			line += String("1");
		else
			line += String("0");
		
		file.println(line);
	}
	
	file.close();
}

//========================================
//
// Gets the temperature from a sensor on PortA
// Checks first for a SHT30 sensor, and then
// for a TMP116 sensor. Uses the first one
// found. The sensor is initialised every time
// since it could be unplugged and plugged back
// in at any time
//
//========================================
void StarterKitControl::getPortA()
{
	bool result = false;
	uint8_t MSB, LSB;
	uint16_t rawTemp;
	
	//Look for a SHT30 sensor
	
	result = I2C_portA.i2c_start(SHT30_A_addr <<1| I2C_WRITE);
	I2C_portA.i2c_stop();
	
  
	if(result)		//if a SHT30 is found...
	{
		_portA_Valid = true;
		PortA_LED_Status(true);
		
		//Send a soft reset
		I2C_portA.i2c_start( SHT30_A_addr<<1 | I2C_WRITE );
		I2C_portA.i2c_write(0x30);		//soft reset
		I2C_portA.i2c_write(0xA2);		//
		I2C_portA.i2c_stop();
		
		
		I2C_portA.i2c_start( SHT30_A_addr<<1 | I2C_WRITE );
		I2C_portA.i2c_write(0x24);		//no clock stretch
		//I2C_portA.i2c_write(0x16);		//low repeatability
		I2C_portA.i2c_write(0x00);		//High repeatability
		I2C_portA.i2c_stop();
	
		//wait for the conversion
		if(I2C_portA.i2c_start_wait( SHT30_A_addr<<1 | I2C_READ ))
		{
			MSB = I2C_portA.i2c_read(false);
			LSB = I2C_portA.i2c_read(true);		//send NAK
			
		}
		else
			Serial << F("PortA I2C timeout") << endl;
		
		I2C_portA.i2c_stop();
		
			
				
		rawTemp = ((uint16_t)MSB<<8) | LSB;
		
		_portA_Temperature = 175.0f * (float)rawTemp / 65535.0f - 45.0f;
		
		
		//return;
		
	}
 	else
	{
	
		//If a SHT30 was not found
		//Check for a TMP116
		
		result = I2C_portA.i2c_start(TMP116_A_addr <<1| I2C_WRITE);
		I2C_portA.i2c_write(0);		//register 0 has the temperature
		I2C_portA.i2c_stop();
		
		if(result)
		{
			_portA_Valid = true;
			PortA_LED_Status(true);
			
			I2C_portA.i2c_start( TMP116_A_addr<<1 | I2C_WRITE );
			I2C_portA.i2c_write(1);		//config register
			
			I2C_portA.i2c_rep_start( TMP116_A_addr<<1 | I2C_WRITE );	//repeated start
			I2C_portA.i2c_write(0x40);		//set one shot conversion mode
			I2C_portA.i2c_write(0x01);
			I2C_portA.i2c_stop();
			
			delay(20); //wait for the conversion
			
			I2C_portA.i2c_start( TMP116_A_addr<<1 | I2C_WRITE );
			I2C_portA.i2c_write(0);		//register 0 has the temperature
			I2C_portA.i2c_rep_start( TMP116_A_addr<<1 | I2C_READ );	//repeated start
			MSB = I2C_portA.i2c_read(false);
			LSB = I2C_portA.i2c_read(true);		//send NAK
			
			I2C_portA.i2c_stop();
			
			_portA_Temperature = 0.0078125 * ( (int)MSB<<8 | (int)LSB );
		}
		else
		{
			_portA_Valid = false;
			PortA_LED_Status(false);
		} 
	}	
	
}


//========================================
//
// Gets the temperature from a sensor on PortB
// Checks first for a SHT30 sensor, and then
// for a TMP116 sensor. Uses the first one
// found. The sensor is initialised every time
// since it could be unplugged and plugged back
// in at any time
//
//========================================
void StarterKitControl::getPortB()
{
	bool result = false;
	uint8_t MSB, LSB;
	uint16_t rawTemp;
	
	//Look for a SHT30 sensor
	
	result = I2C_portB.i2c_start(SHT30_A_addr <<1| I2C_WRITE);
	I2C_portB.i2c_stop();
	
  
	if(result)		//if a SHT30 is found...
	{
		_portB_Valid = true;
		PortB_LED_Status(true);
		
		//Send a soft reset
		I2C_portB.i2c_start( SHT30_A_addr<<1 | I2C_WRITE );
		I2C_portB.i2c_write(0x30);		//soft reset
		I2C_portB.i2c_write(0xA2);		//
		I2C_portB.i2c_stop();
		
		
		I2C_portB.i2c_start( SHT30_A_addr<<1 | I2C_WRITE );
		I2C_portB.i2c_write(0x24);		//no clock stretch
		//I2C_portB.i2c_write(0x16);		//low repeatability
		I2C_portB.i2c_write(0x00);		//High repeatability
		I2C_portB.i2c_stop();
	
		//wait for the conversion
		if(I2C_portB.i2c_start_wait( SHT30_A_addr<<1 | I2C_READ ))
		{
			MSB = I2C_portB.i2c_read(false);
			LSB = I2C_portB.i2c_read(true);		//send NAK
			
		}
		else
			Serial << F("Portb I2C timeout") << endl;
		
		I2C_portB.i2c_stop();
		
			
				
		rawTemp = ((uint16_t)MSB<<8) | LSB;
		
		_portB_Temperature = 175.0f * (float)rawTemp / 65535.0f - 45.0f;
		
		
		//return;
		
	}
 	else
	{
	
		//If a SHT30 was not found
		//Check for a TMP116
		
		result = I2C_portB.i2c_start(TMP116_A_addr <<1| I2C_WRITE);
		I2C_portB.i2c_write(0);		//register 0 has the temperature
		I2C_portB.i2c_stop();
		
		if(result)
		{
			_portB_Valid = true;
			PortB_LED_Status(true);
			
			I2C_portB.i2c_start( TMP116_A_addr<<1 | I2C_WRITE );
			I2C_portB.i2c_write(1);		//config register
			
			I2C_portB.i2c_rep_start( TMP116_A_addr<<1 | I2C_WRITE );	//repeated start
			I2C_portB.i2c_write(0x40);		//set one shot conversion mode
			I2C_portB.i2c_write(0x01);
			I2C_portB.i2c_stop();
			
			delay(20); //wait for the conversion
			
			I2C_portB.i2c_start( TMP116_A_addr<<1 | I2C_WRITE );
			I2C_portB.i2c_write(0);		//register 0 has the temperature
			I2C_portB.i2c_rep_start( TMP116_A_addr<<1 | I2C_READ );	//repeated start
			MSB = I2C_portB.i2c_read(false);
			LSB = I2C_portB.i2c_read(true);		//send NAK
			
			I2C_portB.i2c_stop();
			
			_portB_Temperature = 0.0078125 * ( (int)MSB<<8 | (int)LSB );
		}
		else
		{
			_portB_Valid = false;
			PortB_LED_Status(false);
		} 
	}	
	
}




//==============================
// Interrupt handler.
// This is called every 1000msec
// by the timer interrupt.
// A flag is set, which is monitored
// by the delay routines to tell when 
// the interrupt has fired.
//==============================
static volatile void StarterKitControl::IRQ_Handler()
{
	IRQ_Flag = true;
		
}




//==============================
// Delay Routines
// These routines monitor the IRQ flag
// while delaying.
//==============================
void StarterKitControl::delay_msec(unsigned long msec)
{
	
	unsigned long time_start = micros();
	
	while(micros() < (time_start + msec*1000) )
	{
		if(IRQ_Flag)
		{
			IRQ_Flag = false;
			PeriodicTask();
		}
	}
}


void StarterKitControl::delay_sec(unsigned long sec)
{
	
	unsigned long time_start = millis();
	
	while(millis() < (time_start + sec*1000) )
	{
		if(IRQ_Flag)
		{
			IRQ_Flag = false;
			PeriodicTask();
		}
	}
}




















