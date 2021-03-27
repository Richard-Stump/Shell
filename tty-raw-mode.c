
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <termios.h>
#include <string.h>

/* 
 * Sets terminal into raw mode. 
 * This causes having the characters available
 * immediately instead of waiting for a newline. 
 * Also there is no automatic echo.
 */

static bool inRawMode = false;
static struct termios originalAttr;

void tty_raw_mode(void)
{
	if(inRawMode) return;

	struct termios tty_attr;
	tcgetattr(0,&tty_attr);

	originalAttr = tty_attr; //save the original attributes so that we can
							 //them later

	/* Set raw mode. */
	tty_attr.c_lflag &= (~(ICANON|ECHO));
	tty_attr.c_cc[VTIME] = 0;
	tty_attr.c_cc[VMIN] = 1;
     
	tcsetattr(0,TCSANOW,&tty_attr);

	inRawMode = true;
}

void tty_reset(void)
{
	if(inRawMode) {
		tcsetattr(0,TCSANOW,&originalAttr);
		inRawMode = false;
	}
}