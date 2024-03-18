#ifndef __BNL_FE_REG_MAPPING_HH__
#define __BNL_FE_REG_MAPPING_HH__

#include <stdint.h>
#include <bitset>
#include <vector>

// This is copied from Shanshan's python scripts to configure
// WIB/FEMBs.
// The referred function from Shanshan's scripts to write this class is fe_reg_mapping.py
// The location of this function in SBND ND servers is /home/nfs/sbnd/NEW_BNL_CE_CODE_Apr27_2023/CE_LD

class BNL_FE_Reg_Mapping {
 public:
  BNL_FE_Reg_Mapping();
  
  // sec_chn_reg only sets a channel register, the other registers remains as before
  
  void set_fechn_reg(uint32_t chip=0, uint32_t chn=0, uint32_t sts=0, 
                     uint32_t snc=0, uint32_t sg0=0, uint32_t sg1=1, 
		     uint32_t st0=1, uint32_t st1=1, uint32_t smn=0, uint32_t sdf=1);
  
  // sec_chip_global only sets a chip global register, the other registers remains as before
  
  void set_fechip_global(uint32_t chip=0, uint32_t slk0=0, uint32_t stb1=0, 
                         uint32_t stb=0, uint32_t s16=0, uint32_t slk1=0, 
			 uint32_t sdc=0, uint32_t swdac1=0, uint32_t swdac2=0, uint32_t dac=0);
			 
  // sec_chip sets registers of a whole chip, registers of the other chips remains as before
  
  void set_fechip(uint32_t chip=0, uint32_t sts=0, uint32_t snc=0, uint32_t sg0=0, 
                  uint32_t sg1=1, uint32_t st0=1, uint32_t st1=1, uint32_t smn=0, 
		  uint32_t sdf=1, uint32_t slk0=0, uint32_t stb1=0, uint32_t stb=0, 
		  uint32_t s16=0, uint32_t slk1=0, uint32_t sdc=0, uint32_t swdac1=0, uint32_t swdac2=0, uint32_t dac=0);
		  
  // sec_board sets registers of a whole board
  
  void set_fe_board(uint32_t sts=0, uint32_t snc=0, uint32_t sg0=0, uint32_t sg1=1, 
                    uint32_t st0=1, uint32_t st1=1, uint32_t smn=0, uint32_t sdf=1, 
                    uint32_t slk0=0, uint32_t stb1=0, uint32_t stb=0, uint32_t s16=0, 
		    uint32_t slk1=0, uint32_t sdc=0, uint32_t swdac1=0, uint32_t swdac2=0, uint32_t dac=0);
		    
  // set_collection_fe_board sets the registers of only collection plane wires with given configuration
  
  void set_collection_fe_board(uint8_t config_no, uint32_t sts=0, uint32_t snc=0, uint32_t sg0=0, uint32_t sg1=1, 
                               uint32_t st0=1, uint32_t st1=1, uint32_t smn=0, uint32_t sdf=1, 
                               uint32_t slk0=0, uint32_t stb1=0, uint32_t stb=0, uint32_t s16=0, 
		               uint32_t slk1=0, uint32_t sdc=0, uint32_t swdac1=0, uint32_t swdac2=0, uint32_t dac=0);	
			       
  // sec_board sets channel registers to a given configuration
  
  void set_fe_board(uint8_t config_no, uint32_t sts=0, uint32_t snc=0, uint32_t sg0=0, uint32_t sg1=1, 
                    uint32_t st0=1, uint32_t st1=1, uint32_t smn=0, uint32_t sdf=1, 
                    uint32_t slk0=0, uint32_t stb1=0, uint32_t stb=0, uint32_t s16=0, 
		    uint32_t slk1=0, uint32_t sdc=0, uint32_t swdac1=0, uint32_t swdac2=0, uint32_t dac=0);
		    
  // sec_board used to test channel mapping
  
  // OLD PIECE OF CODE USED FOR TESTING CHANNEL MAP(commented out 02/23/2024)
  void set_fe_board_for_chnl_testing(uint8_t config_no, uint32_t test_chnl, uint32_t sts=0, uint32_t snc=0, uint32_t sg0=0, uint32_t sg1=1, 
                    uint32_t st0=1, uint32_t st1=1, uint32_t smn=0, uint32_t sdf=1, 
                    uint32_t slk0=0, uint32_t stb1=0, uint32_t stb=0, uint32_t s16=0, 
		    uint32_t slk1=0, uint32_t sdc=0, uint32_t swdac1=0, uint32_t swdac2=0, uint32_t dac=0);
  
  // NEW PIECE OF CODE USED FOR TESTING CHANNEL MAP (written 02/23/2024)		    
  void tst_set_fe_board_for_chnl_testing(uint8_t config_no, uint32_t test_fst_chnl, uint32_t test_lst_chnl, uint32_t sts=0, uint32_t snc=0, 
                                     uint32_t sg0=0, uint32_t sg1=1, uint32_t st0=1, uint32_t st1=1, uint32_t smn=0, uint32_t sdf=1, 
                                     uint32_t slk0=0, uint32_t stb1=0, uint32_t stb=0, uint32_t s16=0, 
		                     uint32_t slk1=0, uint32_t sdc=0, uint32_t swdac1=0, uint32_t swdac2=0, uint32_t dac=0);			       
			       		     		       
  std::vector<bool> REGS{std::vector<bool>(1152, false)}; // declare board specific registers
  
  // Following FE ASIC channel mapping is valid for following FEMBs
  //************************************************
  //*    Crate    *  WIB Slot   *     FEMB Slot    *
  //************************************************
  //*    1        *     1       *        1         *
  //*    1        *     1       *        2         *
  //*    1        *     1       *        3         *
  //*    1        *     1       *        4         *
  //*    1        *     2       *        1         *
  //*    1        *     2       *        2         *
  //*    1        *     2       *        3         *
  //*    1        *     2       *        4         *
  //*    1        *     3       *        1         *
  //*    1        *     3       *        2         *
  //*    1        *     3       *        3         *
  //*    1        *     3       *        4         *
  //*    1        *     4       *        1         *
  //*    2        *     3       *        2         *
  //*    2        *     3       *        3         *
  //*    2        *     3       *        4         *
  //*    3        *     3       *        2         *
  //*    3        *     3       *        3         *
  //*    3        *     3       *        4         *
  //*    3        *     4       *        1         *
  //*    3        *     4       *        2         *
  //*    3        *     4       *        3         *
  //*    3        *     4       *        4         *
  //*    3        *     5       *        1         *
  //*    3        *     5       *        2         *
  //*    3        *     5       *        3         *
  //*    3        *     5       *        4         *
  //*    3        *     6       *        1         *
  //*    3        *     6       *        2         *
  //*    4        *     1       *        1         *
  //*    4        *     1       *        2         *
  //*    4        *     1       *        3         *
  //*    4        *     1       *        4         *
  //*    4        *     2       *        1         *
  //*    4        *     2       *        2         *
  //*    4        *     2       *        3         *
  //*    4        *     2       *        4         *
  //*    4        *     3       *        1         *
  //*    4        *     3       *        2         *
  //*    4        *     3       *        3         *
  //*    4        *     3       *        4         *
  //*    4        *     4       *        1         *
  //************************************************            
  
  const uint8_t sbnd_channel_wire_plane_config_1[8][16] = { // U=0, V=1, W/X/Z/collection=2
        {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
	{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
	{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
  };
  
  // Following FE ASIC channel mapping is valid for following FEMBs
  //************************************************
  //*    Crate    *  WIB Slot   *     FEMB Slot    *
  //************************************************
  //*    1        *     4       *        2         *
  //*    1        *     4       *        3         *
  //*    1        *     4       *        4         *
  //*    1        *     5       *        1         *
  //*    1        *     5       *        2         *
  //*    1        *     5       *        3         *
  //*    1        *     5       *        4         *
  //*    1        *     6       *        1         *
  //*    1        *     6       *        2         *
  //*    4        *     4       *        2         *
  //*    4        *     4       *        3         *
  //*    4        *     4       *        4         *
  //*    4        *     5       *        1         *
  //*    4        *     5       *        2         *
  //*    4        *     5       *        3         *
  //*    4        *     5       *        4         *
  //*    4        *     6       *        1         *
  //*    4        *     6       *        2         *
  //************************************************          
  
  const uint8_t sbnd_channel_wire_plane_config_2[8][16] = { // U=0, V=1, W/X/Z/collection=2
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
  };
  
  // Following FE ASIC channel mapping is valid for following FEMBs
  //************************************************
  //*    Crate    *  WIB Slot   *     FEMB Slot    *
  //************************************************
  //*     2       *     1       *       1          *
  //*     2       *     1       *       2          *
  //*     2       *     1       *       3          *
  //*     2       *     1       *       4          *
  //*     2       *     2       *       1          *
  //*     2       *     2       *       2          *
  //*     2       *     2       *       3          *
  //*     2       *     2       *       4          *
  //*     2       *     3       *       1          *
  //*     2       *     4       *       1          *
  //*     2       *     4       *       2          *
  //*     2       *     4       *       3          *
  //*     2       *     4       *       4          *
  //*     2       *     5       *       1          *
  //*     2       *     5       *       2          *
  //*     2       *     5       *       3          *
  //*     2       *     5       *       4          *
  //*     2       *     6       *       1          *
  //*     2       *     6       *       2          *
  //*     3       *     1       *       1          *
  //*     3       *     1       *       2          *
  //*     3       *     1       *       3          *
  //*     3       *     1       *       4          *
  //*     3       *     2       *       1          *
  //*     3       *     2       *       2          *
  //*     3       *     2       *       3          *
  //*     3       *     2       *       4          *
  //*     3       *     3       *       1          *
  //************************************************         
  
  const uint8_t sbnd_channel_wire_plane_config_3[8][16] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
  };
  
};
#endif
