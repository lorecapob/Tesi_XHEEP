// Copyright 2022 OpenHW Group
// Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
// SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

#include "verilated.h"
#include "verilated_fst_c.h"
#include "Vtestharness.h"
#include "Vtestharness__Syms.h"

#include <stdlib.h>
#include <iostream>
// For the bridge
#include <fstream>
#include <string>

#include "XHEEP_CmdLineOptions.hh"

// -------- Bridge2Xheep --------
#include "Bridge2Xheep.h"
// ------------------------------

vluint64_t sim_time = 0;

void runCycles(unsigned int ncycles, Vtestharness *dut, VerilatedFstC *m_trace){
  for(unsigned int i = 0; i < ncycles; i++) {
    dut->clk_i ^= 1;
    dut->eval();
    m_trace->dump(sim_time);
    sim_time++;
  }
}

// ----------- Bridge2Xheep global variables ------
  int new_address = 0;
	int new_line = 0;
  int skip = 0;

	std::string instr = "";
	std::string address = "";
	int address_int = 0;
	long long int instr_int = 0x0;
	char tmp_hex;
  // int SOC_CTRL_BOOT_LOOP_REG_OFFSET = 0; -------> use the method dut->tb_set_exit_loop()

// ------------------------------------------------

// ----------- Bridge2Xheep Initialization Loop -----------------

void initRAM(Vtestharness *dut, Bridge2Xheep bridge, VerilatedFstC *m_trace, std::ifstream& hex_file){
  while (hex_file)
  {
    dut->clk_i ^= 1;

    if (!bridge.isBridgeBusy())
    {
      // If bridge is not busy with an OBI protocol, read a new hex value from the file
      hex_file.get(tmp_hex);

      // check if a new memory section is reached
      if (tmp_hex == '@')
      {
        address = "";
        new_address = 1;

        // This check is needed since the main.hex files does not contains always instructions on 32 bit
        if (instr.length() != 0)
        {
          while (instr.length() != 8)
          {
            instr += '0';
          }

          instr_int = stoll(instr, nullptr, 16);
          
          bridge.setInstr(instr_int);
          bridge.setInstrValid();

          address_int += 4;
          instr = "";
        }
        
      } else if ((tmp_hex >= '0' && tmp_hex < '10') || (tmp_hex >= 'A' && tmp_hex < 'F')) // check if tmp_hex is a value between 0 and 9 or A and F
      {
        if (new_address)
        {
          address += tmp_hex;
          
          if (address.length() == 8)
          {
            if (address == "00000000")
						{
              skip = 1;
						} else 
						{
							skip = 0;
						}

            new_address = 0;
            address_int = stoi(address, nullptr, 16);
          }
          
        } else
        {
          if (!skip)
          {
            instr += tmp_hex;

            if ((instr.length() == 8))
            {
              instr_int = stoll(instr, nullptr, 16);

              // At this point send address and instruction to the bridge
              bridge.setAddress(address_int);
              bridge.setInstr(instr_int);
              bridge.setInstrValid();

              //Cleand the instr variable and update address
              address_int += 4;
              instr = "";
            }
          
          }
        }
        
      }
    } else {
      bridge.writeToRAM();
    }
    
    dut->eval();
    m_trace->dump(sim_time);
    sim_time++;
  }

  // End of initialization. Set exit_loop to 1
  dut->tb_set_exit_loop();
  std::cout<<"Set Exit Loop"<< std::endl;
  runCycles(1, dut, m_trace);
  std::cout<<"Memory Loaded"<< std::endl;
}

// --------------------------------------------------------------


int main (int argc, char * argv[])
{

  std::string firmware;
  unsigned int max_sim_time, boot_sel, exit_val;
  bool use_openocd;
  bool run_all = false;

  Verilated::commandArgs(argc, argv);

  // Instantiate the model
  Vtestharness *dut = new Vtestharness;

  // Open VCD
  Verilated::traceEverOn (true);
  VerilatedFstC *m_trace = new VerilatedFstC;
  dut->trace (m_trace, 99);
  m_trace->open ("waveform.vcd");

  XHEEP_CmdLineOptions* cmd_lines_options = new XHEEP_CmdLineOptions(argc,argv);

  use_openocd = cmd_lines_options->get_use_openocd();
  firmware = cmd_lines_options->get_firmware();

  if(firmware.empty() && use_openocd==false){
      std::cout<<"You must specify the firmware if you are not using OpenOCD"<<std::endl;
      exit(EXIT_FAILURE);
  }

  max_sim_time = cmd_lines_options->get_max_sim_time(run_all);

  boot_sel     = cmd_lines_options->get_boot_sel();

  if(boot_sel == 1) {
    std::cout<<"[TESTBENCH]: ERROR: Executing from SPI is not supported (yet) in Verilator"<<std::endl;
    std::cout<<"exit simulation..."<<std::endl;
    exit(EXIT_FAILURE);
  }

  svSetScope(svGetScopeFromName("TOP.testharness"));
  svScope scope = svGetScope();
  if (!scope) {
    std::cout<<"Warning: svGetScope failed"<< std::endl;
    exit(EXIT_FAILURE);
  }

  // -------- For the bridge ------------------------------------
  // Bridge instantiation
	Bridge2Xheep bridge(dut);

	// File name
	std::string filename = firmware; // sistemare

	// Declaring file as read file
	std::ifstream hex_file;

	hex_file.open(filename);

	if(!hex_file){
		std::cerr << "Testbench error while opening file " << filename << std::endl;
		return 1;
	}

  // ------------------------------------------------------------

  dut->clk_i                = 0;
  dut->rst_ni               = 1;
  dut->jtag_tck_i           = 0;
  dut->jtag_tms_i           = 0;
  dut->jtag_trst_ni         = 0;
  dut->jtag_tdi_i           = 0;
  dut->execute_from_flash_i = 1; //this cause boot_sel cannot be 1 anyway
  dut->boot_select_i        = boot_sel;
  // Added for the bridge
  dut->req_i                = 0;
  dut->we_i                 = 0;
  dut->be_i                 = 0b1111;
  dut->addr_i               = 0x180;
  dut->wdata_i              = 0;

  dut->eval();
  m_trace->dump(sim_time);
  sim_time++;

  dut->rst_ni               = 1;
  //this creates the negedge
  runCycles(50, dut, m_trace);
  dut->rst_ni               = 0;
  runCycles(50, dut, m_trace);


  dut->rst_ni = 1;
  runCycles(20, dut, m_trace);
  std::cout<<"Reset Released"<< std::endl;

  // -------------------------- Bridge2Xheep module ----------------------------

  initRAM(dut, bridge, m_trace, hex_file);

  // ---------------------------------------------------------------------------

  //dont need to exit from boot loop if using OpenOCD or Boot from Flash
  /*if(use_openocd==false || boot_sel == 1) {
    dut->tb_loadHEX(firmware.c_str());
    runCycles(1, dut, m_trace);
    dut->tb_set_exit_loop();
    std::cout<<"Set Exit Loop"<< std::endl;
    runCycles(1, dut, m_trace);
    std::cout<<"Memory Loaded"<< std::endl;
  } else {
    std::cout<<"Waiting for GDB"<< std::endl;
  }*/

  if(run_all==false) {
    runCycles(max_sim_time, dut, m_trace);
  } else {
    while(dut->exit_valid_o!=1) {
      runCycles(500, dut, m_trace);
    }
  }

  if(dut->exit_valid_o==1) {
    std::cout<<"Program Finished with value "<<dut->exit_value_o<<std::endl;
    exit_val = EXIT_SUCCESS;
  } else exit_val = EXIT_FAILURE;

  m_trace->close();
  delete dut;
  delete cmd_lines_options;

  // close bridge instruction file
  hex_file.close();

  exit(exit_val);

}