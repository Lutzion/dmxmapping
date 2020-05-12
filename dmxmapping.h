/*
  dmxmapping.h
  Copyright (c) 2020, Lutz Hillebrand
*/

#ifndef _DMXMAPPING
#define _DMXMAPPING

#ifdef MAPS

#define DMXMAP_MAX_CHANS  512
#define DMXMAP_MAX_VALUES 256
#define DMXMAP_MAX_MAPS   20

#include <stdint.h> 

bool dmxMapsInit() ;
bool chan2DmxMapsRead();
bool dmxMapsRead();
bool dmxMapsExec(unsigned char * pData, uint16_t chans);

#endif

#endif