// This program sets up GPIO pin 17 as an input pin, and sets it to generate
// an interrupt whenever a rising edge is detected. The pin is assumed to
// be connected to a push button switch on a breadboard. When the button is
// pushed, a 3.3V level will be applied to the pin. The pin should otherwise
// be pulled low with a pull-down resistor of 10K Ohms.

// Include files
#include "uart.h"
#include "sysreg.h"
#include "gpio.h"
#include "irq.h"
#include "systimer.h"

#define false 0
#define true 1

// Function prototypes
void init_GPIO(int pinNumber, _Bool isInput);
void set_GPIO(int pinNumber);
void clear_GPIO(int pinNumber);
void delay(int delayTime);
void turnOnLED(int ledNumber);


// Declare a global shared variable
int sharedValue;



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       main
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function first prints out the values of some system
//                  registers for diagnostic purposes. It then initializes
//                  GPIO pin 17 to be an input pin that generates an interrupt
//                  (IRQ exception) whenever a rising edge occurs on the pin.
//                  The function then goes into an infinite loop, where the
//                  shared global variable is continually checked. If the
//                  interrupt service routine changes the shared variable,
//                  then this is detected in the loop, and the current value
//                  is printed out.
//
////////////////////////////////////////////////////////////////////////////////

void main()
{
    // Set up the UART serial port
    uart_init();    

    // Initialize the sharedValue global variable and
    // and set the local variable to be same value
    sharedValue = 1;
    unsigned int delayTime = 500000;
    

    // Set up GPIO pin #4 for output
    init_GPIO(4, false);
    init_GPIO(17, false);
    init_GPIO(27, false);
    init_GPIO(23, true);
    init_GPIO(22, true);

    // Enable IRQ Exceptions
    enableIRQ();    

    // Print out a message to the console
    uart_puts("\nAssignment 3 by Kourosh\n");

    int ledNumber = 1;

    // Loop forever, waiting for interrupts to change the shared value
    while (1) {

        //Map LED Number to Pin Number
        int pinNumber;
        if(ledNumber == 1){
            pinNumber = 4;
            uart_puts("Pin 4 is ON\n");
        }else if(ledNumber == 2){
            pinNumber = 17;
            uart_puts("Pin 17 is ON\n");
        }else if(ledNumber == 3){
            pinNumber = 27;
            uart_puts("Pin 27 is ON\n");
        }


        set_GPIO(pinNumber);// Turn on the LED
        microsecond_delay(delayTime);
        clear_GPIO(pinNumber);// Turn the LED off
          // Print a message to the console
        uart_puts("OFF\n");
        microsecond_delay(delayTime);

        ledNumber = ledNumber + sharedValue;
        if(ledNumber >= 4){
            ledNumber=1;
        }else if(ledNumber <= 0){
            ledNumber = 3;
        }
        if(sharedValue == 1){
            delayTime = 500000;
        }else{
            delayTime = 250000;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//
//  Function:       init_GPIO4_to_output
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets GPIO pin 4 to an output pin without
//                  any pull-up or pull-down resistors.
//
////////////////////////////////////////////////////////////////////////////////

void init_GPIO(int pinNumber,_Bool isInput){ //Source: 03_GPIO_PushButton example
    register unsigned int r;

    int selectNumber = pinNumber / 10;
    volatile unsigned int* GPIOSelect;

    if(selectNumber == 0){
    	GPIOSelect = GPFSEL0;
    }else if(selectNumber == 1) {
    	GPIOSelect = GPFSEL1;
    }else if(selectNumber == 2) {
    	GPIOSelect = GPFSEL2;
    }else if(selectNumber == 3) {
    	GPIOSelect = GPFSEL3;
    }else if(selectNumber == 4) {
    	GPIOSelect = GPFSEL4;
    }else if(selectNumber == 5) {
    	GPIOSelect = GPFSEL5;
    }else{
      uart_puts("INVALID PIN NUMBER\n");
      return;
    }

    r = *GPIOSelect;

    // Clear bits 12 - 14. This is the field FSEL4, which maps to GPIO pin 4.
    // We clear the bits by ANDing with a 000 bit pattern in the field.
    r &= ~(0x7 << (pinNumber%10) * 3 ); //4 mod 10 * 3

    // Set the field FSEL4 to 001, which sets pin 4 to an output pin.
    // We do so by ORing the bit pattern 001 into the field.
    if(isInput == false){
    	r |= (0x1 << (pinNumber%10) * 3 );  //4 mod 10 * 3
    }
    // Write the modified bit pattern back to the
    // GPIO Function Select Register 2
    *GPIOSelect = r;

    // Disable the pull-up/pull-down control line for GPIO pin 4. We follow the
    // procedure outlined on page 101 of the BCM2837 ARM Peripherals manual. The
    // internal pull-up and pull-down resistor isn't needed for an output pin.

    // Disable pull-up/pull-down by setting bits 0:1
    // to 00 in the GPIO Pull-Up/Down Register
    *GPPUD = 0x0;

    // Wait 150 cycles to provide the required set-up time
    // for the control signal
    r = 150;
    while (r--) {
      asm volatile("nop");
    }

    // Write to the GPIO Pull-Up/Down Clock Register 0, using a 1 on bit 4 to
    // clock in the control signal for GPIO pin 4. Note that all other pins
    // will retain their previous state.
    *GPPUDCLK0 = (0x1 << pinNumber);

    // Wait 150 cycles to provide the required hold time
    // for the control signal
    r = 150;
    while (r--) {
      asm volatile("nop");
    }

    // Clear all bits in the GPIO Pull-Up/Down Clock Register 0
    // in order to remove the clock
    *GPPUDCLK0 = 0;

    if(isInput == true){
        // Set pin 17 to so that it generates an interrupt on a rising edge.
        // We do so by setting bit 17 in the GPIO Rising Edge Detect Enable
        // Register 0 to a 1 value (p. 97 in the Broadcom manual).

        *GPREN0 = *GPREN0 | (0x1 << pinNumber); //Need to or it so we don't reset what has been done

        // Enable the GPIO IRQS for ALL the GPIO pins by setting IRQ 52
        // GPIO_int[3] in the Interrupt Enable Register 2 to a 1 value.
        // See p. 117 in the Broadcom Peripherals Manual.
        *IRQ_ENABLE_IRQS_2 = (0x1 << 20);
    }
    return;
}

////////////////////////////////////////////////////////////////////////////////
//
//  Function:       set_GPIO
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets the GPIO output pin
//                  to a 1 (high) level.
//
////////////////////////////////////////////////////////////////////////////////
void set_GPIO(int pinNumber){ //Source: 03_GPIO_PushButton example
	  register unsigned int r;

	  // Put a 1 into the SET4 field of the GPIO Pin Output Set Register 0
	  r = (0x1 << pinNumber);
	  *GPSET0 = r;
	  return;
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       clear_GPIO
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function clears the GPIO output pin
//                  to a 0 (low) level.
//
////////////////////////////////////////////////////////////////////////////////
void clear_GPIO(int pinNumber){ //Source: 03_GPIO_PushButton example
	  register unsigned int r;

	  // Put a 1 into the CLR4 field of the GPIO Pin Output Clear Register 0
	  r = (0x1 << pinNumber);
	  *GPCLR0 = r;
	  return;
}
