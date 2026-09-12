# EtherNode
Slim STM32H7-based Ethernet device with several IOs and signal types which can be controlled via Telnet, supports MODBUS, REST and MQTT as well as small programs running from the device directly 

The EtherNode is an industrial IO-extender that comes with different types of input- and output-signals and a wide range of communication interfaces. Thus it can be used as remote Ethernet IO-module for different other scenarios and applications. Possible usage scenarios would be applications where the EtherNode acts as slim TCP module for a PLC and where it autonomously performs distributed and remote I/O operations that are triggered via simple and easy to use commands.

The EtherNode Compact IO module can be integrated into every machine network and provides the following features:
- wide-range power supply allows input voltages from 9V to 32V
- 8 galvanically insulated digital inputs which can be operated with external power in range 5..24V
- 2 of the digital inputs optionally can be used with a quadrature encoder for counting steps (position), speed and acceleration
- 8 galvanically insulated digital outputs which can be operated with external power in range 5..24V
- 2 of the digital outputs optionally can issue freely definable frequencies of up to 500 kHz with programmable pulse-width (PWM)
- 2 of the digital outputs optionally can be used to drive control two stepper motor axes with a step frequency of up to 100 kHz, acceleration ramps and referencing
- 2 high-impedance analogue inputs in range 0..10V with a resolution of 16 bits
- Ethernet interface to connect with host (control-PC / embedded device / PLC)
- control via Telnet ASCII commands, MODBUS, MQTT (plain and JSON) or HTTP REST API (plain and JSON)
- stand-alone operation: conditions and flows can be defined and stored on the device for autonomous operation wiht and without communication via the communication interfaces
- fully operational with any control device such as a PLC or even a plain PC or a Raspberry Pi
- board designed to fit into compact housing with DIN rail mounting clamp available at https://de.rs-online.com/web/p/din-schienen-gehause/1862290 and at https://de.rs-online.com/web/p/din-schienen-gehause/0336868


 
