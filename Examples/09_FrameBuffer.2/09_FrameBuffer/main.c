// This program demonstrates how to initialize a frame buffer for a
// 1024 x 768 display, and how to draw on it using a simple checker board
// pattern.

// Included header files
#include "uart.h"
#include "framebuffer.h"



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
    char c;

    // Initialize the UART terminal
    uart_init();

    // Initialize the frame buffer
    initFrameBuffer();

    // Draw on the frame buffer and display it
    while(1) {
        displayFrameBuffer();
        uart_putc('.');
    }

    // Loop forever, echoing characters received from the console
    // on a separate line with : : around the character
    while (1) {
        // Wait for a character input from the console
	c = uart_getc();

        // Output the character
        uart_puts(":");
        uart_putc(c);
        uart_puts(":\n");
    }
}
