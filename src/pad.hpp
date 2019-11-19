#pragma once

#include <stdio.h>
#include <stdlib.h>

#define PAD_IDLE 	(unsigned short) 0
#define PAD_DOWN 	(unsigned short) 1
#define PAD_PRESSED (unsigned short) 2
#define PAD_UP 		(unsigned short) 3

struct Pad {
	unsigned short up, down, left, right;
	
	Pad(){
		clear();
	}

	void clear(){
		up = down = left = right = 0;
	}

	void print(){
		printf("pad state w:%d a:%d s:%d d:%d\n", up, left, down, right);
	}
};