#ifndef BRIDGE2XHEEP_H
#define BRIDGE2XHEEP_H

#include "verilated.h"
#include "Vtestharness.h"

class Bridge2Xheep{
private:
    Vtestharness *dut;
    
    int busy = 0;
    int state = 0;
    int address = 0;
    long long int instruction = 0;
    int instruction_valid = 0;
public:
    Bridge2Xheep(Vtestharness *dut);
    ~Bridge2Xheep();

    //int endInit();
    int isBridgeBusy();
    
    void setAddress(int address);
    void setInstr(long long int instruction);
    void setInstrValid();
    void writeToRAM();
};

#endif
