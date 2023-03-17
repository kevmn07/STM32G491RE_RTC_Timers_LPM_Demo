# STM32 Demos by EK.

## Baremetall Real Time Clock, Timers, and Low Power Mode implementation for the Nucleo-G491RE evaluation board.

STM32CubeIDE ver. 1.11.2 used to build the Demo. 
VCP UART port is dedicated to the data transfer, see the board user manual.
Port setting
  BaudRate : 115200 bps
  Length   : 8 bits
  StopBits : 1 bit
  Parity   : None

This is a demo project intended to show how to enable, configure and run STM32G4 peripherals. It lacks features required by commercial-grade software and therefore cannot be used in business applications without redesign and upgrade.
__Use this software at your own risk, no support, help, or comments are provided now or in the future.__

##Pereferial

###UART
    - Polling mode is used. No complaints there, as this is a DEMO project. Also, in some cases, polling mode is an unavoidable solution.

### Timer (LPTIM1)
    - Wiveform generation mode is used to continiously run C1 GPIO pin.
    - Stays operating in LPM mode.
    - Possible waveforms, press the Blue button on the board to cycle between them:
        1. Off (Default);
        2. Duty cycle 1/3, Period 1 sec;
        3. Duty cycle 2/3, Period 1 sec;
        4. Duty cycle 1/4, Period 2 sec;
        5. Duty cycle 3/4, Period 2 sec;
        6. Duty cycle 1/5, Period 4 sec;
        7. Duty cycle 4/5, Period 4 sec;
        5. On.

### Watch Dog timer (IWDT)
    - Cannot be suspended or turned off after starting.
    - Stays operating in LPM mode.
    - Will reset the board in 20 sec if not reloaded.

### Real Time Clock
    - Clocked by crystal 32,768 Hz.
    - Stays operating in LPM mode.
    - The date and time are updated on Startup.

###Low Power (Sleep) Mode
    - Stops CPU clock.
    - Exit from LP Mode on Real Time Clock WakeUp Interrupt.

## Pseudocode
```
Startup
	Init HAL
	Config System Clock
	Init GPIO
	Init UART
	Init LPTIM1
	Init RTC
	Send "Invitation" message
	Retrieve Date and Time through UART and Setup RTC
	Switch LPTIM1 to the Default waveform
	Get and send Board Date and Time
	Send LPTIM1 waveform parameters
	Start IWDT
	Send Main Loop entry notification
END Startup

LOOP
	Get current Date and Time
	IF the minute changes
		Send Date and Time
	End IF
	
	Get “User” (Blue) button state
	On "Pressed" edge
		Cycle wiveforms and send them parameters
	END "Pressed" edge

	Check UART Input
	IF 'S' or 's' characters received
		Set "Sleep" request
	ELSE IF Input buffer contains junk data
		Send "Warning" message
	END IF

	IF "Sleep" is requested
		Start RTC WakeUp timer
		Send “GO to Sleep” message
		Go to Sleep Mode

		Sleep until an interrupt occurs
		Recovery from Sleep Mode
		Get and send Board Date and Time
		Send "WakeUP" message
		Clear "Sleep" request
	End IF

	Reset IWDT
	
END LOOP
```

## Notes
	- The goal of this project is to show how to run STM32G4 MCU hardware rather than provide an example of great coding practice, and some fun code has been added for compatibility with STM Cube IDE's auto code generation.
	 - Anyone interested is welcome to email <kevmn07@gmail.com> to report a bug, suggest improvements to the current Demo, or request a more reliable and fully documented commercial version. However, no obligation to respond.

## References:
* [STM32G491RE on the ST website](https://www.st.com/en/microcontrollers-microprocessors/stm32g491re.html)
* [UM2505 User manual STM32G4 Nucleo-64 boards (MB1367)](https://www.st.com/resource/en/user_manual/dm00556337-stm32g4-nucleo64-boards-mb1367-stmicroelectronics.pdf)
* [UM1727 User manual Getting started with STM32 Nucleo board software development tools](https://www.st.com/resource/en/user_manual/um1727-getting-started-with-stm32-nucleo-board-software-development-tools-stmicroelectronics.pdf)
* [NUCLEO64 STM32G4 Schematics (MB1367)](https://www.st.com/content/ccc/resource/technical/layouts_and_diagrams/schematic_pack/group2/73/2b/7d/ed/9d/39/4b/e1/MB1367-G491RE-C05_Schematic/files/MB1367-G491RE-C05_Schematic.PDF/jcr:content/translations/en.MB1367-G491RE-C05_Schematic.PDF)
* [PM0214 Programming manual STM32 Cortex®-M4 MCUs and MPUs programming manual](https://www.st.com/resource/en/programming_manual/pm0214-stm32-cortexm4-mcus-and-mpus-programming-manual-stmicroelectronics.pdf)
* [RM0440 Reference manual STM32G4 Series advanced Arm®-based 32-bit MCUs](
https://www.st.com/resource/en/reference_manual/rm0440-stm32g4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
