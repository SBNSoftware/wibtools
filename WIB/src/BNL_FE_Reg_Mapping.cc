#include <BNL_FE_Reg_Mapping.hh>
#include <iostream>
#include<cmath>
#include "trace.h"

BNL_FE_Reg_Mapping::BNL_FE_Reg_Mapping()
{
}

void BNL_FE_Reg_Mapping::set_fechn_reg(uint32_t chip, uint32_t chn, uint32_t sts, uint32_t snc, 
                                  uint32_t sg0, uint32_t sg1, uint32_t st0, uint32_t st1, uint32_t smn, uint32_t sdf){

  const std::string identification = "BNL_FE_Reg_Mapping::set_fechn_reg"; 
  
  auto chn_reg = ((sts&0x01)<<7) + ((snc&0x01)<<6) + ((sg0&0x01)<<5) + ((sg1&0x01)<<4) + 
                 ((st0&0x01)<<3) + ((st1&0x01)<<2)  + ((smn&0x01)<<1) +   ((sdf&0x01)<<0);
  std::vector<bool> chn_reg_bool;
  for (int i=0; i<8; i++){
     chn_reg_bool.push_back(bool((chn_reg>>i)%2)); 
  }
  auto start_pos = (8*16+16)*chip + (16-chn)*8;
  
  for (unsigned int i=0; i<chn_reg_bool.size(); i++){
     REGS[start_pos-8+i] = chn_reg_bool[i];
  }
}

void BNL_FE_Reg_Mapping::set_fechip_global(uint32_t chip, uint32_t slk0, uint32_t stb1, uint32_t stb, 
                                               uint32_t s16, uint32_t slk1, uint32_t sdc, uint32_t swdac1, uint32_t swdac2, uint32_t dac){

  const std::string identification = "BNL_FE_Reg_Mapping::set_fechip_global";
  
  auto global_reg = ((slk0&0x01)<<0) + ((stb1&0x01)<<1) + ((stb&0x01)<<2)+ ((s16&0x01)<<3) + ((slk1&0x01)<<4) + ((sdc&0x01)<<5) +((00&0x03)<<6);
  auto dac_reg = ((int((floor((dac&0x01)/0x01))))<<7)+((int(floor(((dac&0x02)/0x02))))<<6)+
                  ((int(floor(((dac&0x04)/0x04))))<<5)+((int(floor(((dac&0x08)/0x08))))<<4)+
                  ((int(floor(((dac&0x10)/0x10))))<<3)+((int(floor(((dac&0x20)/0x20))))<<2)+
                  (((swdac1&0x01))<<1)+(((swdac2&0x01))<<0);
  std::vector<bool> global_reg_bool;
  for (int i=0; i<8; i++){
     global_reg_bool.push_back(bool((global_reg>>i)%2));
  }  
  
  for (int i=0; i<8; i++){
     global_reg_bool.push_back(bool((dac_reg>>i)%2));
  }   
  
  auto start_pos = (8*16+16)*chip + 16*8;
  
  for (unsigned int i=0; i<global_reg_bool.size(); i++){
     REGS[start_pos+i] = global_reg_bool[i];
  }             
}

void BNL_FE_Reg_Mapping::set_fechip(uint32_t chip,uint32_t sts, uint32_t snc, uint32_t sg0, 
                                    uint32_t sg1, uint32_t st0, uint32_t st1, uint32_t smn, 
				    uint32_t sdf, uint32_t slk0, uint32_t stb1, uint32_t stb, 
				    uint32_t s16, uint32_t slk1, uint32_t sdc, uint32_t swdac1, uint32_t swdac2, uint32_t dac){
    
    const std::string identification = "BNL_FE_Reg_Mapping::set_fechip";
    
    for (int i=0; i<16; i++) set_fechn_reg(chip, i, sts, snc, sg0, sg1, st0, st1, smn, sdf);
    set_fechip_global (chip, slk0, stb1, stb, s16, slk1, sdc, swdac1, swdac2, dac);
}

void BNL_FE_Reg_Mapping::set_fe_board(uint32_t sts, uint32_t snc, uint32_t sg0, uint32_t sg1, 
                    uint32_t st0, uint32_t st1, uint32_t smn, uint32_t sdf, 
                    uint32_t slk0, uint32_t stb1, uint32_t stb, uint32_t s16, 
		    uint32_t slk1, uint32_t sdc, uint32_t swdac1, uint32_t swdac2, uint32_t dac){
   
   const std::string identification = "BNL_FE_Reg_Mapping::set_fe_board";
   
   for (int i=0; i<8; i++) set_fechip( i, sts, snc, sg0, sg1, st0, st1, smn, sdf, slk0, stb1, stb, s16, slk1, sdc, swdac1, swdac2, dac);
}

void BNL_FE_Reg_Mapping::set_collection_fe_board(uint8_t config_no, uint32_t sts, uint32_t snc, uint32_t sg0, uint32_t sg1,
                                                 uint32_t st0, uint32_t st1, uint32_t smn, uint32_t sdf, 
						 uint32_t slk0, uint32_t stb1, uint32_t stb, uint32_t s16, 
						 uint32_t slk1, uint32_t sdc, uint32_t swdac1, uint32_t swdac2, uint32_t dac){
  const std::string identification = "BNL_FE_Reg_Mapping::set_collection_fe_board";
  uint8_t channel_wire_plane[8][16];
  
  if (config_no == 0){
     for (int i=0; i<8; i++){
        for (int j=0; j<16; j++){
	   channel_wire_plane[i][j] = sbnd_channel_wire_plane_config_1[i][j];
	}
     } 
  }
     
  else if(config_no == 1){
      for (int i=0; i<8; i++){
         for (int j=0; j<16; j++){
	    channel_wire_plane[i][j] = sbnd_channel_wire_plane_config_2[i][j];
	 }
      }
  }
     
  else{
     for (int i=0; i<8; i++){
        for (int j=0; j<16; j++){
	   channel_wire_plane[i][j] = sbnd_channel_wire_plane_config_3[i][j];
	}
     }
  }
  
  for (int i=0; i<8; i++){
     for (int j=0; j<16; j++){
         if (channel_wire_plane[i][j] == 2){
	     set_fechn_reg(i, j, sts, snc, 0, 1, st0, st1, smn, sdf);
	 } // collection plane channels
     }
     set_fechip_global (i, slk0, stb1, stb, s16, slk1, sdc, swdac1, swdac2, dac);
  }
}

void BNL_FE_Reg_Mapping::set_fe_board(uint8_t config_no, uint32_t sts, uint32_t snc, uint32_t sg0, uint32_t sg1,
                                      uint32_t st0, uint32_t st1, uint32_t smn, uint32_t sdf,
				      uint32_t slk0, uint32_t stb1, uint32_t stb, uint32_t s16,
				      uint32_t slk1, uint32_t sdc, uint32_t swdac1, uint32_t swdac2, uint32_t dac){
     const std::string identification = "BNL_FE_Reg_Mapping::set_fe_board";
     uint8_t channel_wire_plane[8][16];
     if (config_no == 0){
       for (int i=0; i<8; i++){
          for (int j=0; j<16; j++){
	     channel_wire_plane[i][j] = sbnd_channel_wire_plane_config_1[i][j];
	  }
       } 
     }
     
     else if(config_no == 1){
       for (int i=0; i<8; i++){
          for (int j=0; j<16; j++){
	      channel_wire_plane[i][j] = sbnd_channel_wire_plane_config_2[i][j];
	  }
       }
     }
     
     else{
       for (int i=0; i<8; i++){
          for (int j=0; j<16; j++){
	     channel_wire_plane[i][j] = sbnd_channel_wire_plane_config_3[i][j];
	  }
       }
     }
     
     for (int i=0; i<8; i++){
        for (int j=0; j<16; j++){
	   if (channel_wire_plane[i][j] == 2) set_fechn_reg(i, j, sts, snc, 0, 1, st0, st1, smn, sdf); // change parameters here
	   else set_fechn_reg(i, j, sts, snc, 0, 0, st0, st1, smn, sdf);
	} // loop over 16 channels in each chip
	set_fechip_global(i, slk0, stb1, stb, s16, slk1, sdc, swdac1, swdac2, dac);
     } // loop over all 8 chips
}


