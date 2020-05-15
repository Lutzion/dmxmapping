/*
  dmxmapping.cpp
  Copyright (c) 2020, Lutz Hillebrand

  set MAPS in platformio.ini like

  build_flags = ...
                -D MAPS

  to use dmxmapping in your project
*/

#ifdef MAPS

#ifdef LITTLEFS
#include <LittleFS.h>
#else
#include <FS.h>
#endif

#include "dmxmapping.h"

uint16_t chan2dmxmap[DMXMAP_MAX_CHANS] ;
uint8_t dmxmaps[DMXMAP_MAX_MAPS][DMXMAP_MAX_VALUES] ;

// === dmxMapsInit() 
// inits the map arrays
bool dmxMapsInit()
{
  #ifdef VERBOSE_MAP
    Serial.println("dmxMapsInit");   
  #endif

  // Set defaults for working with maps -> all channels use mapping 0
  // Falls mit MAPS gearbeitet werden soll -> als erstes auf Standard einstellen
  memset(chan2dmxmap, 0, DMXMAP_MAX_CHANS) ;  // Alle Kanaele nutzen Mapping 0

  // init all maps
  for (uint8_t m = 0; m < DMXMAP_MAX_MAPS; m++) // Fuer alle Mappings
  {
    /*
  #ifdef VERBOSE_MAP
    Serial.print("map ") ;
    Serial.println(m);   
  #endif
    */
    // ATTENTION: v must be of type uint16_t (not uint8_t), 
    // otherwise it will not become greater 255 and we have an endless loop
    // 
    // ACHTUNG Hier muss v vom Typ uint16_t sein, denn sonst wird v nie groesser 255!
    // Dann haben wir eine Endlosschleife mit WD-Reset
    for (uint16_t v = 0; v < DMXMAP_MAX_VALUES; v++) // Fuer alle Werte
    {
      /*
  #ifdef VERBOSE_MAP
    Serial.print("value ") ;
    Serial.println(v);   
  #endif
    */
      // Default value for v in map m is v (no value change)
      dmxmaps[m][v] = v ; // Default: map des Werts v im mapping m ist v
    }
  }
  #ifdef VERBOSE_MAP
    Serial.println("dmxMapsInit end") ;
  #endif

  return true ;
}

// === isLineComment()
// checks if a line is a comment 
bool isLineComment(String line)
{
    if (line.startsWith("#") ||
        line.startsWith(";"))
    {
      return true ;
    }

    return false ;
}

// === chan2DmxMapsRead()
// reads the file mappings.txt and stores into array, which channel uses which map
bool chan2DmxMapsRead()
{
#if VERBOSE_MAP
    Serial.println("mappingdmx())");
#endif

  String sFile = String("/mappings.txt") ;
#ifdef LITTLEFS
  if (!LittleFS.exists(sFile))
#else
  if (!SPIFFS.exists(sFile))
#endif
  {
#if VERBOSE_MAP
    Serial.println("mappingdmx: no file <" + sFile + ">");
#endif
    return false;
  }

#ifdef LITTLEFS
  File f = LittleFS.open(sFile, "r");
#else
  File f = SPIFFS.open(sFile, "r");
#endif

  while (f.available())
  {
    String line = f.readStringUntil('\n') ;
#if VERBOSE_MAP
    Serial.println("mappingdmx: " + line);
#endif

    if (isLineComment(line))
    {
      // ignore comment
#if VERBOSE_MAP
      Serial.println(String("line <") + line + "> ignored") ;
#endif
      continue ;
    }
    
    uint16_t chan = line.toInt() - 1;
    uint16_t pos = line.indexOf(',') ;
    String t = line.substring(pos + 1) ;
    uint16_t map = t.toInt() ;

#if VERBOSE_MAP
    Serial.print("chan ");
    Serial.print(chan);
    Serial.print(" - ");
    Serial.println(map);
#endif
    
    if (chan >= 0 &&
        chan < DMXMAP_MAX_CHANS &&
        map > 0 &&
        map < DMXMAP_MAX_MAPS)
    {
      chan2dmxmap[chan] = map ;
    }
  }

  f.close();

  return true ;
}

// === dmxMapsRead()
// reads the map files *.map into array
bool dmxMapsRead()
{
#if VERBOSE_MAP
    Serial.println("readMaps()");
#endif

  for (uint8_t i = 1; i < DMXMAP_MAX_MAPS; i++)
  {
    String sFile = String("/") + String(i) + ".map" ;

#ifdef LITTLEFS
    if (!LittleFS.exists(sFile))
#else
    if (!SPIFFS.exists(sFile))
#endif
    {
  #if VERBOSE_MAP
      Serial.println("readMaps: no file <" + sFile + ">");
  #endif
      continue;
    }

  #if VERBOSE_MAP
      Serial.println("readMaps: file " + sFile);
  #endif

#ifdef LITTLEFS
    File f = LittleFS.open(sFile.c_str(), "r");
#else
    File f = SPIFFS.open(sFile.c_str(), "r");
#endif

    uint16_t val = 0 ;
    while (f.available())
    {
      String line = f.readStringUntil('\n') ;

      if (isLineComment(line))
      {
        // ignore comment
  #if VERBOSE_MAP
        Serial.println(String("line <") + line + "> ignored") ;
  #endif
        continue ;
      }

      uint16_t mapVal = line.toInt() ;

      if (val >= DMXMAP_MAX_VALUES)
        continue ;

      dmxmaps[i][val] = mapVal;

      val++ ;
    }

    f.close() ;
  }

  return true ;
}

// === dmxMapsExec()
// the translation of given value to mapped value
// fast by just accessing the array values 
bool dmxMapsExec(unsigned char * pData, uint16_t chans)
{
  for(uint16_t c = 0; c < chans; c++)
  {
    uint16_t m = chan2dmxmap[c] ;
    uint8_t oldVal = pData[c];
    uint8_t newVal = dmxmaps[m][oldVal] ;
    pData[c] = newVal ;

#ifdef VERBOSE_MAP
    if (m != 0 && 
        oldVal != 0)
    {
      Serial.print("MAP ");
      Serial.print(c);
      Serial.print(": ");
      Serial.print(oldVal);
      Serial.print("->");
      Serial.println(newVal);
    }
#endif
  }
  return true ;
}

#endif