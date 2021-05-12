#include <ASIC_reg_mapping.hh>
#include <iostream>
#include <iomanip>
#include <limits>
#include "trace.h"

ASIC_reg_mapping::ASIC_reg_mapping(): REGS(72)
{
}

void ASIC_reg_mapping::set_board(const FE_ASIC_reg_mapping & fe_map, const ADC_ASIC_reg_mapping & adc_map)
{
  const std::bitset<1152> fe_bits = fe_map.get_bits();
  const std::bitset<1152> adc_bits = adc_map.get_bits();
  const size_t chip_bits_len = 144;
  for (size_t iChip=0; iChip < 8; iChip++)
  {
    std::bitset<chip_bits_len*2> chip_bits;
    for (size_t iBit=0; iBit < chip_bits_len; iBit++)
    {
      chip_bits[iBit] = adc_bits[iChip*chip_bits_len+iBit];
      chip_bits[iBit+chip_bits_len] = fe_bits[iChip*chip_bits_len+iBit];
    }
    for (size_t iReg=0; iReg < 9; iReg++)
    {
      std::bitset<32> regBits;
      for (size_t iBit=0; iBit < 32; iBit++)
      {
        regBits[iBit] = chip_bits[iBit+iReg*32];
      }
      uint32_t regUInt = regBits.to_ulong();
      REGS[iChip*9+iReg] = regUInt;
    } // for iReg
  } // for iChip
} // set_board

void ASIC_reg_mapping::set_board(const FE_ASIC_reg_mapping & fe_map)
{
  const std::bitset<1152> fe_bits = fe_map.get_bits();
  //std::cout << std::hex << fe_bits << std::dec << std::endl;
  uint32_t nRegs = 36;
  for (size_t iReg=0; iReg < nRegs; iReg++)
    {
      std::bitset<32> regBits;
      for (size_t iBit=0; iBit < 32; iBit++)
	{
	  regBits[iBit] = fe_bits[iBit+iReg*32];
	}
      uint32_t regUInt = regBits.to_ulong();
      REGS[iReg] = regUInt;
      //std::cout << std::hex << REGS[iReg] << std::dec << std::endl;
    } // for iReg
} // set_board

std::vector<uint32_t> ASIC_reg_mapping::get_regs() const
{
  return REGS;
}

void ASIC_reg_mapping::print() const
{
  const std::string identification = "ASIC_reg_mapping::print";
  TLOG_INFO(identification) << "ASIC_reg_mapping (hex):" << TLOG_ENDL;

  for(uint32_t i = 0; i < REGS.size(); ++i) 
  {
    uint32_t reg = REGS[i];
    TLOG_INFO(identification) << std::hex << std::setfill('0') << std::setw(8) << reg << TLOG_ENDL;
    //std::cout << reg << std::endl;
  }
}
