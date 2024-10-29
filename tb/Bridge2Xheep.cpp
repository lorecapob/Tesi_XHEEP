#include <iostream>

#include "Bridge2Xheep.h"

Bridge2Xheep::Bridge2Xheep(Vtestharness *dut)
{
    this->dut = dut;
};

Bridge2Xheep::~Bridge2Xheep()
{
}

void Bridge2Xheep::writeToRAM(){

    int instrToXheep = 0;
    long long int addrToXheep = 0; 

    switch (this->state)
    {
    case 0:
        // IDLE state
        dut->req_i = 0;

        if (this->instruction_valid)
        {
            this->state = 1;
        } else {
            this->state = 0;
        }
        
        break;

    case 1:
        // REQUEST_SENT state
        dut->req_i = 1;
        dut->we_i = 1;

        addrToXheep = this->address;
        instrToXheep = this->instruction;

        dut->addr_i = addrToXheep;
        dut->wdata_i = instrToXheep;

        // Debug
        //std::cout << addrToXheep << "\t" << std::hex << instrToXheep << std::endl;
        
        if (dut->gnt_o)
        {
            this->state = 2;
        } else {
            this->state = 1;
        }

        break;

    case 2:
        dut->req_i = 0;
        // addToXheep += 4;
        dut->we_i = 0;

        this->busy = 0;
    
        this->state = 0;
        break;
    
    default:
        this->state = 0;
        break;
    }
    
};

/*
int Bridge2Xheep::endInit(){
    return 1;
}
*/

int Bridge2Xheep::isBridgeBusy()
{
    return this->busy;
}

void Bridge2Xheep::setAddress(int address) {
    this->address = address;
}

void Bridge2Xheep::setInstr(long long int instruction) {
    this->instruction = instruction;
}

void Bridge2Xheep::setInstrValid() {
    this->instruction_valid = 1;
    this->busy = 1;
};
