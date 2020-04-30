// frameDescriptor.h was created by Mark Renard on 4/30/2020.
//
// This file contains the definition of a FrameDescriptor, which records
// whether a frame was referenced recently or written to since its page was
// last written to the disk. 

#ifndef FRAMEDESCRIPTOR_H
#define FRAMEDESCRIPTOR_H

typedef struct frameDescriptor{
	char reference; // Whether the frame was referenced recently
	char dirty;	// Whether frame was written to since last disk write
} FrameDescriptor;


#endif
