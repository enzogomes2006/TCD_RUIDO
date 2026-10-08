#pragma once
// Dubles somente para testar o fluxo do sketch no computador, sem eletrica.
#include <stdint.h>
#include <stdio.h>
#include <deque>
#include <string>
#define IRAM_ATTR
constexpr int OUTPUT=1, INPUT=0, LOW=0, FALLING=2, SERIAL_8N1=0;
extern int driveLevel;
class HardwareSerial {
public:
  std::deque<uint8_t> input;
  std::string output;
  void begin(uint32_t,int=0,int=-1,int=-1) {}
  void setRxFIFOFull(int) {}
  void setRxTimeout(int) {}
  int available() {return int(input.size());}
  int read() {int value=input.front();input.pop_front();return value;}
  void println(const char* text) {output+=text;output+='\n';}
  template<class... Args> void printf(const char* format,Args... args) {
    char line[512];snprintf(line,sizeof(line),format,args...);output+=line;
  }
};
extern HardwareSerial Serial,Serial1,Serial2;
inline void pinMode(int,int) {}
inline void digitalWrite(int,int value) {driveLevel=value;}
inline int digitalPinToInterrupt(int pin) {return pin;}
inline void attachInterrupt(int,void(*)(),int) {}
