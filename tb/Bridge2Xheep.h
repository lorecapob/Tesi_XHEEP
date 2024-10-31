#ifndef BRIDGE2XHEEP_H
#define BRIDGE2XHEEP_H

#include "verilated.h"
#include "Vtestharness.h"

class Bridge2Xheep{
private:
    Vtestharness *dut;
    
    int busy = 0;
    int state = 0;
    vluint32_t address = 0;
    vluint32_t instruction = 0;
    int instruction_valid = 0;
public:
    Bridge2Xheep(Vtestharness *dut);
    ~Bridge2Xheep();

    //int endInit();
    int isBridgeBusy();
    
    void setAddress(vluint32_t address);
    void setInstr(vluint32_t instruction);
    void setInstrValid();
    void writeToRAM();
};

#endif
