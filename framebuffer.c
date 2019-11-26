// Needed header files
#include "uart.h"
#include "mailbox.h"

// HTML RGB color codes.  These can be found at:
// https://htmlcolorcodes.com/
#define BLACK     0x00000000
#define WHITE     0x00FFFFFF
#define RED       0x00FF0000
#define LIME      0x0000FF00
#define BLUE      0x000000FF
#define AQUA      0x0000FFFF
#define FUCHSIA   0x00FF00FF
#define YELLOW    0x00FFFF00
#define GRAY      0x00808080
#define MAROON    0x00800000
#define OLIVE     0x00808000
#define GREEN     0x00008000
#define TEAL      0x00008080
#define NAVY      0x00000080
#define PURPLE    0x00800080
#define SILVER    0x00C0C0C0

// Frame buffer constants
#define FRAMEBUFFER_WIDTH      1024  // in pixels
#define FRAMEBUFFER_HEIGHT     768   // in pixels
#define FRAMEBUFFER_DEPTH      32    // bits per pixel (4 bytes per pixel)
#define FRAMEBUFFER_ALIGNMENT  4     // framebuffer address preferred alignment
#define VIRTUAL_X_OFFSET       0
#define VIRTUAL_Y_OFFSET       0
#define PIXEL_ORDER_BGR        0     // needed for the above color codes

// Frame buffer global variables
unsigned int frameBufferWidth, frameBufferHeight, frameBufferPitch;
unsigned int frameBufferDepth, frameBufferPixelOrder, frameBufferSize;
unsigned int *frameBuffer;




////////////////////////////////////////////////////////////////////////////////
//
//  Function:       initFrameBuffer
//
//  Arguments:      none
//
//  Returns:        void
//
//  Description:    This function uses the mailbox request/response protocol
//                  to allocate and set the frame buffer. This includes the
//                  width, height, and depth of the framebuffer, plus the
//                  desired pixel order (BGR). The mailbox response is used
//                  to set the frame buffer global variables that can be used
//                  later on when drawing to the screen. The most important of
//                  these is the frame buffer address.
//
////////////////////////////////////////////////////////////////////////////////

void initFrameBuffer()
{
    // Initialize the mailbox data structure.
    // It contains a series of tags that specify the
    // desired settings for the frame buffer.
    mailbox_buffer[0] = 35 * 4;
    mailbox_buffer[1] = MAILBOX_REQUEST;

    mailbox_buffer[2] = TAG_SET_PHYSICAL_WIDTH_HEIGHT;
    mailbox_buffer[3] = 8;
    mailbox_buffer[4] = 0;
    mailbox_buffer[5] = FRAMEBUFFER_WIDTH;
    mailbox_buffer[6] = FRAMEBUFFER_HEIGHT;

    mailbox_buffer[7] = TAG_SET_VIRTUAL_WIDTH_HEIGHT;
    mailbox_buffer[8] = 8;
    mailbox_buffer[9] = 0;
    mailbox_buffer[10] = FRAMEBUFFER_WIDTH;
    mailbox_buffer[11] = FRAMEBUFFER_HEIGHT;
    
    mailbox_buffer[12] = TAG_SET_VIRTUAL_OFFSET;
    mailbox_buffer[13] = 8;
    mailbox_buffer[14] = 0;
    mailbox_buffer[15] = VIRTUAL_X_OFFSET;
    mailbox_buffer[16] = VIRTUAL_Y_OFFSET;
    
    mailbox_buffer[17] = TAG_SET_DEPTH;
    mailbox_buffer[18] = 4;
    mailbox_buffer[19] = 0;
    mailbox_buffer[20] = FRAMEBUFFER_DEPTH;

    mailbox_buffer[21] = TAG_SET_PIXEL_ORDER;
    mailbox_buffer[22] = 4;
    mailbox_buffer[23] = 0;
    mailbox_buffer[24] = PIXEL_ORDER_BGR;

    mailbox_buffer[25] = TAG_ALLOCATE_BUFFER;
    mailbox_buffer[26] = 8;
    mailbox_buffer[27] = 0;
    // Request: alignment; Response: frame buffer address 
    mailbox_buffer[28] = FRAMEBUFFER_ALIGNMENT;
    mailbox_buffer[29] = 0;    // Response: Frame buffer size

    mailbox_buffer[30] = TAG_GET_PITCH;
    mailbox_buffer[31] = 4;
    mailbox_buffer[32] = 0;
    mailbox_buffer[33] = 0;    // Response: Pitch

    mailbox_buffer[34] = TAG_LAST;


    // Make a mailbox request using the above mailbox data structure
    if (mailbox_query(CHANNEL_PROPERTY_TAGS_ARMTOVC)) {
	// If here, the query succeeded, and we can check the response

	// Get the returned frame buffer address, masking out 2 upper bits
        mailbox_buffer[28] &= 0x3FFFFFFF;
        frameBuffer = (void *)((unsigned long)mailbox_buffer[28]);

	// Read the frame buffer settings from the mailbox buffer
        frameBufferWidth = mailbox_buffer[5];
        frameBufferHeight = mailbox_buffer[6];
        frameBufferPitch = mailbox_buffer[33];
	frameBufferDepth = mailbox_buffer[20];
	frameBufferPixelOrder = mailbox_buffer[24];
	frameBufferSize = mailbox_buffer[29];

	// Display frame buffer settings to the terminal
	uart_puts("Frame buffer settings:\n");

	uart_puts("    width:       0x");
	uart_puthex(frameBufferWidth);
	uart_puts(" pixels\n");

	uart_puts("    height:      0x");
	uart_puthex(frameBufferHeight);
	uart_puts(" pixels\n");

	uart_puts("    pitch:       0x");
	uart_puthex(frameBufferPitch);
	uart_puts(" bytes per row\n");

	uart_puts("    depth:       0x");
	uart_puthex(frameBufferDepth);
	uart_puts(" bits per pixel\n");

	uart_puts("    pixel order: 0x");
	uart_puthex(frameBufferPixelOrder);
	uart_puts(" (0=BGR, 1=RGB)\n");

	uart_puts("    address:     0x");
	uart_puthex(mailbox_buffer[28]);
	uart_puts("\n");

	uart_puts("    size:        0x");
	uart_puthex(frameBufferSize);
	uart_puts(" bytes\n");
	
    } else {
        uart_puts("Cannot initialize frame buffer\n");
    }
}

void drawPoint(int row, int column){
    unsigned int *pixel = frameBuffer;
    pixel[(row * frameBufferWidth) + column] = BLACK;
}


void clearScreen(){
    int row,column;
    unsigned int *pixel = frameBuffer;
    // Draw the square row by row, from the top down
    for (row = 0; row < frameBufferHeight; row++) {
        // Draw each pixel in the row from left to right
         for (column = 0; column < frameBufferWidth; column++) {
            // Draw the individual pixel by setting its
            // RGB value in the frame buffer
             pixel[(row * frameBufferWidth) + column] = WHITE;
         }
    }
}
