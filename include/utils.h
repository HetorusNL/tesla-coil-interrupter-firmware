// #define DEBUG_PRINTS

#ifdef DEBUG_PRINTS
#define debugprint(obj) Serial.print(obj)
#define debugprintln(obj) Serial.println(obj)
#else
#define debugprint(obj)
#define debugprintln(obj)
#endif