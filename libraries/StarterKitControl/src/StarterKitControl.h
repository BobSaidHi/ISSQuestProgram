#ifndef StarterKitControl_h
#define StarterKitControl_h


#include "Arduino.h"
#include <TimerOne.h>
#include <Streaming.h>  
#include <SlowSoftI2CMaster.h>
#include <SPI.h>
#include <SD.h>


//IO pin defines for the Starter Kit Hardware v1.0
#define Pin_HeaterEnable 9
#define Pin_FanEnable 6

#define Btn_Start A0

#define LED_Running 8
#define LED_Stopped 7

#define LED_PortA_OK 4
#define LED_PortA_FAULT 5

#define LED_PortB_OK 2
#define LED_PortB_FAULT 3


#define Pin_Soft_PortA_SCL A5
#define Pin_Soft_PortA_SDA A4

#define Pin_Soft_PortB_SCL A3
#define Pin_Soft_PortB_SDA A2

//I2C addresses
//The I2C address of the sensors can be changed. The default for each
//is the "A" address. With two different addresses, we could put a max of 
//two sensors of each type on each port
#define TMP116_A_addr 0x48
#define TMP116_B_addr 0x49

#define SHT30_A_addr  0x44
#define SHT30_B_addr  0x45


class StarterKitControl
{
  public:
		StarterKitControl();
		
		void HeaterOn();
		void HeaterOff();
		void FanOn();
		void FanOff();
		
		void InitPlatform();
		void StartExperiment();
		void EndExperiment();
		bool StartButton();
		
		static volatile void IRQ_Handler();
		void delay_msec(unsigned long msec);
		void delay_sec(unsigned long sec);	

  private:
  
		void PeriodicTask();
		void Running_LED_Status(bool state);
		void PortA_LED_Status(bool state);
		void PortB_LED_Status(bool state);
			
		static volatile bool IRQ_Flag;		//set to true in the ISR. 
	   
		boolean _PreRunState;		
		
		//State of the heater and fan
		boolean _state_Heater;
		boolean _state_Fan;
		unsigned long _StartTime;
		unsigned long _timestamp;
			
		float _portA_Temperature;
		bool _portA_Valid;
		int _portA_SensorID;
		
		float _portB_Temperature;
		bool _portB_Valid;
		int _portB_SensorID;
			
		void dataToTerminal();
		void dataToSDcard();
			
		void getPortA();
		void getPortB();
		
		//class for doing software I2C on the two sensor ports
		SlowSoftI2CMaster   I2C_portA; 
		SlowSoftI2CMaster   I2C_portB; 
			
		File _logFile;
		String _logFileName;	//If the card is detected, this holds the name of the current log file
		bool _uSD_detected;		//used to indicate if we should log to the uSD card
			

};


#endif