// frameDescriptor.c was created by Mark Reanrd on 5/5/2020.
//
// This file contains a function which initializes an array of frame descriptors
// to default values.

#include "constants.h"
#include "frameDescriptor.h"

void initFrameTable(FrameDescriptor * frameTable){
	int i = 0;
	for( ; i < NUM_FRAMES; i++){
		frameTable[i].simPid = (char) EMPTY;
		frameTable[i].pageNum = (char) EMPTY;
		frameTable[i].reference = 0;
		frameTable[i].dirty = 0;
	}
}
