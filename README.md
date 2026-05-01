# CustomUart

This is a software implementation of [UART](https://en.wikipedia.org/wiki/Universal_asynchronous_receiver-transmitter) for the Arduino Uno R3, made for the purpose of teaching myself more about Arduino, the ATmega328p, and UART.

The `SoftwareUART.h` header declares a class called `SoftwareUart`. This class inherits from [Print](https://docs.arduino.cc/language-reference/en/functions/communication/print/) and [Stream](https://docs.arduino.cc/language-reference/en/functions/communication/stream/), so it has all the methods you're used to like `readBytesUntil()`, `println()`, etc. It supports full duplex asynchronous communication by using the ATmega328p's hardware Timer1 to generate an interrupt per tx bit and per rx bit. Its rx pin is 8, and its tx pin is 9.

I've measured it successfully communicating up to 38400 baud when executing full duplex communication, and 57600 baud when executing half-duplex communication. I recommend sticking to a very low baud like 4800 though, otherwise you'll spend most of your cpu time transmitting data. The minimum baud is 245.

## Usage
To see this library in action, try one of the following:

Loop back mode:
1. Connect a wire directly from pin 8 to pin 9 on your Arduino Uno R3
1. Open CustomUart.ino in the Arduno IDE
1. Upload and run
1. Send messages to the serial monitor to see them echoed back to you.

Alternatively, you can use 2 Arduinos running the same program:
1. Wire pin 8 of each Arduino to pin 9 of the other Arduino
1. Connect a wire from GND of one Arduino to GND of the other Arduino
1. Open CustomUart.ino in two instances of the Arduino IDE, each connected to a different physical Arduino
1. Send messages to the serial monitor connected to one Arduino and see them appear in the serial monitor connected to the other Arduino

The tx and rx pins are also fully capable of communicating with Arduino's built-in UART pins 0 and 1, but testing that will be more complicated since it'll interfere with uploading programs and will interfere with the serial monitor.
1. Upload the program to both Arduinos before connecting any wires
1. Wire pin 8 of one Arduino (We'll call it the "main" arduino) to pin 1 of the other ("secondary") Arduino
1. Wire pin 9 of the main Arduino to pin 0 of the secondary Arduino
1. Connect a wire directly from pin 8 to pin 9 on your secondary Arduino
1. Connect a wire from GND of the main Arduino to GND of the secondary Arduino
1. Send messages to the serial monitor of the main Arduino and watch them take a circuitous route to be echoed back to you
1. Make sure to unplug the secondary Arduino's pin 0 and pin 1 before trying to upload any new programs.
