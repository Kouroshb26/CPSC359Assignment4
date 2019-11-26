// This file contains C functions to handle particular kinds of exceptions.
// Only a function to handle IRQ exceptions is currently implemented.

// Header files
#include "uart.h"
#include "gpio.h"
#include "irq.h"
#include "sysreg.h"

// Reference to the global shared value
extern int sharedValue;



////////////////////////////////////////////////////////////////////////////////
//
//  Function:       IRQ_handler
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function first prints out some basic information about
//                  the state of the interrupt controller, GPIO pending
//                  interrupts, and selected system registers. It then
//                  determines the particular kind of pending interrupt (which
//                  for the moment is a rising edge event on GPIO pin 23). The
//                  interrupt is cleared, and interrupt is handled in a simple-
//                  minded way by incrementing the shared global variable.
//
////////////////////////////////////////////////////////////////////////////////

void IRQ_handler(){ //Source: 05_GPIO_PushButtonInterrupt example
    // Handle GPIO interrupts in general
    if (*IRQ_PENDING_2 == (0x1 << 20)) {
      // Handle the interrupt associated with GPIO pin 23 or 22
      if (*GPEDS0  == (0x1 << 23)){ //PIN 23 BUTTON A
        *GPEDS0 = (0x1 << 23);
        sharedValue = -1;
        uart_puts("BUTTON A");
      }
      else if(*GPEDS0  == (0x1 << 22)){ //PIN 22 BUTTON B
        *GPEDS0 = (0x1 << 22);
        sharedValue = 1;
        uart_puts("BUTTON B");
      }else  if(*GPEDS0  == (0x3 << 22)) { //In case Both are called for some reason we just do nothing
        *GPEDS0 = (0x3 << 22);
      }
    }

    // Return to the IRQ exception handler stub
    return;
}
