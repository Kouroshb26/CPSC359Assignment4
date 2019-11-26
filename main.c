// This program demonstrates how to initialize a frame buffer for a
// 1024 x 768 display, and how to draw on it using a simple checker board
// pattern.

// Included header files
#include "uart.h"
#include "framebuffer.h"
#include "gpio.h"
#include "systimer.h"

#define false 0
#define true 1

// Function prototypes
unsigned short get_SNES();
void init_GPIO(int pinNumber, _Bool isInput);
void set_GPIO(int pinNumber);
void clear_GPIO(int pinNumber);
unsigned get_GPIO(int pinNumber);

struct Button{
    char* name;
    int shiftValue;
};
struct Point{
    int x;
    int y;
};
struct Button createButton(char* name, int shiftValue);
struct Point createPoint(int x, int y);
void printPoint(struct Point *p);

struct Button createButton(char* name, int shiftValue){
    struct Button b;
    b.shiftValue = shiftValue;
    b.name = name;
    return b;
}

struct Point createPoint(int x, int y){
    struct Point p;
    p.x = x;
    p.y = y;
    return p;
}


////////////////////////////////////////////////////////////////////////////////
//
//  Function:       main
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function initializes the UART terminal and initializes
//                  a frame buffer for a 1024 x 768 display. Each pixel in the
//                  frame buffer is 32 bits in size, which encodes an RGB value
//                  (plus an 8-bit alpha channel that is not used). The program
//                  then draws and displays an 18 x 12 checker board pattern.
//
////////////////////////////////////////////////////////////////////////////////
void main()
{
    unsigned short data = 0xFFFF;

    // Set up the UART serial port
    uart_init();

    // Initialize the frame buffer
    initFrameBuffer();
    clearScreen();

    // Set up GPIO pin #9 for output (LATCH output)
    init_GPIO(9,false);

    // Set up GPIO pin #11 for output (CLOCK output)
    init_GPIO(11,false);

    // Set up GPIO pin #10 for input (DATA input)
    init_GPIO(10,true);

    // Clear the LATCH line (GPIO 9) to low
    clear_GPIO(9);

    // Set CLOCK line (GPIO 11) to high
    set_GPIO(11);

    struct Button buttons[6];
    buttons[0] = createButton("Start",3);
    buttons[1] = createButton("Up",4);
    buttons[2] = createButton("Down",5);
    buttons[3] = createButton("Left",6);
    buttons[4] = createButton("Right",7);
    buttons[5] = createButton("X",9);

    struct Point character = createPoint(512,384);
    printPoint(&character);


    // Print out a message to the console
    uart_puts("SNES Controller Program starting.\n");

    // Loop forever, reading from the SNES controller 30 times per second
    while (1) {
    	// Read data from the SNES controller
    	data = get_SNES();

        for(int i = 0; i < 6; i++){
            if((0x1 << buttons[i].shiftValue) & data){
                switch(buttons[i].shiftValue){
                    case 3://Start
                        uart_puts("Start\n");
                        clearScreen();
                        break;
                    case 4://UP
                        uart_puts("UP\n");
                        if(character.y > 0){
                            character.y -= 1;
                        }
                        break;
                    case 5://Down
                        uart_puts("Down\n");
                        if(character.y<767){
                            character.y += 1;
                        }
                        break;
                    case 6://Left
                        uart_puts("Left\n");
                        if(character.x > 0){
                            character.x -= 1;
                        }
                        break;
                    case 7://Right
                        uart_puts("Right\n");
                        if(character.x < 1023){
                            character.x += 1;
                        }
                        break;
                    case 9://X  FILL
                        uart_puts("X\n");
                        clearPoint(character.x,character.y);
                        floodFill(character.x,character.y);
                        break;
                    default:
                        break;
                }
            }
        }

        printPoint(&character);
        drawPoint(character.x,character.y);


    	// Delay 1/30th of a second
    	microsecond_delay(10000);
    }
}

void printPoint(struct Point *p){
    uart_puts("Position: x = ");
    uart_puthex(p->x);
    uart_puts(" y = ");
    uart_puthex(p->y);
    uart_puts("\n");
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       get_SNES
//
//  Arguments:      none
//
//  Returns:        A short integer with the button presses encoded with 16
//                  bits. 1 means pressed, and 0 means unpressed. Bit 0 is
//                  button B, Bit 1 is button Y, etc. up to Bit 11, which is
//                  button R. Bits 12-15 are always 0.
//
//  Description:    This function samples the button presses on the SNES
//                  controller, and returns an encoding of these in a 16-bit
//                  integer. We assume that the CLOCK output is already high,
//                  and set the LATCH output to high for 12 microseconds. This
//                  causes the controller to latch the values of the button
//                  presses into its internal register. We then clock this data
//                  to the CPU over the DATA line in a serial fashion, by
//                  pulsing the CLOCK line low 16 times. We read the data on
//                  the falling edge of the clock. The rising edge of the clock
//                  causes the controller to output the next bit of serial data
//                  to be place on the DATA line. The clock cycle is 12
//                  microseconds long, so the clock is low for 6 microseconds,
//                  and then high for 6 microseconds.
//
////////////////////////////////////////////////////////////////////////////////

unsigned short get_SNES()
{
    int i;
    unsigned short data = 0;
    unsigned int value;

    // Set LATCH to high for 12 microseconds. This causes the controller to
    // latch the values of button presses into its internal register. The
    // first serial bit also becomes available on the DATA line.
    set_GPIO(9);
    microsecond_delay(12);
    clear_GPIO(9);

    // Output 16 clock pulses, and read 16 bits of serial data
    for (i = 0; i < 16; i++) {
	// Delay 6 microseconds (half a cycle)
	microsecond_delay(6);

	// Clear the CLOCK line (creates a falling edge)
	clear_GPIO(11);

	// Read the value on the input DATA line
	value = get_GPIO(10);

	// Store the bit read. Note we convert a 0 (which indicates a button
	// press) to a 1 in the returned 16-bit integer. Unpressed buttons
	// will be encoded as a 0.
	if (value == 0) {
	    data |= (0x1 << i);
	}

	// Delay 6 microseconds (half a cycle)
	microsecond_delay(6);

	// Set the CLOCK to 1 (creates a rising edge). This causes the
	// controller to output the next bit, which we read half a
	// cycle later.
	set_GPIO(11);
    }

    // Return the encoded data
    return data;
}



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       init_GPIO_to_output
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function sets GPIO pinNumber to an output pin without
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
	  *GPSET0 = (0x1 << pinNumber);
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
	  *GPCLR0 = (0x1 << pinNumber);
	  return;
}


////////////////////////////////////////////////////////////////////////////////
//
//  Function:       get_GPIO
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function clears the GPIO output pin
//                  to a 0 (low) level.
//
////////////////////////////////////////////////////////////////////////////////
unsigned get_GPIO(int pinNumber){
     // Isolate pin 10, and return its value (a 0 if low, or a 1 if high)
     return (( *GPLEV0 >> pinNumber) & 0x1);
}
