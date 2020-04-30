// randomGen.h was created by Mark Renard on 3/26/2020
//
// This file contains prototypes for functions related to random number
// generation, which should be called after srand has been called.

#ifndef RANDOMGEN_H
#define RANDOMGEN_H

unsigned int randUnsigned(unsigned int min, unsigned int max);
int randInt(int min, int max);
int randBinary(double probability);

#endif
