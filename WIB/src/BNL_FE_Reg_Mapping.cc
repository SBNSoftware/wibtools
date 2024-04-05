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
	   if (channel_wire_plane[i][j] == 2) set_fechn_reg(i, j, sts, 1, sg0, sg1, st0, st1, smn, sdf); // change parameters here
	   else set_fechn_reg(i, j, sts, 0, sg0, sg1, st0, st1, smn, sdf); // change parameters here
	} // loop over 16 channels in each chip
	set_fechip_global(i, slk0, stb1, stb, s16, slk1, sdc, swdac1, swdac2, dac);
     } // loop over all 8 chips
}

// OLD PIECE OF CODE USED FOR TESTING CHANNEL MAP
// BETWEEN FEMB CHANNELS & NEVIS LINKS
// BUT THIS IS CAPABLE OF SENDNIG TEST PULSE
// ONLY THROUHG SINGLE CHANNEL AT ONCE.
// AND ALSO FOUND, THIS IS NOT WORKING STRAIGHTWAY (NEED PLAYING WITH MULTPLE FCL PARAMETERS TO ACHIEVE THE DESIRED RESULT)
// SO DECIDED TO MODIFY THIS FUNCTION (02/23/2024, VARUNA MEDDAGE)

void BNL_FE_Reg_Mapping::set_fe_board_for_chnl_testing(uint8_t config_no, uint32_t test_chnl, uint32_t sts, uint32_t snc, uint32_t sg0, uint32_t sg1, 
                                      uint32_t st0, uint32_t st1, uint32_t smn, uint32_t sdf, 
                                      uint32_t slk0, uint32_t stb1, uint32_t stb, uint32_t s16, 
		                      uint32_t slk1, uint32_t sdc, uint32_t swdac1, uint32_t swdac2, uint32_t dac){
     const std::string identification = "BNL_FE_Reg_Mapping::set_fe_board_for_chnl_testing";
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
     
     int chip_id = test_chnl/16;
     int local_chnl_id = test_chnl%16;
     
     for (int i=0; i<8; i++){
        for (int j=0; j<16; j++){
	   if (channel_wire_plane[i][j] == 2){ 
	       if (i == chip_id && j == local_chnl_id) set_fechn_reg(i, j, sts, 1, sg0, sg1, st0, st1, smn, sdf);
	       else set_fechn_reg(i, j, 0, 1, sg0, sg1, st0, st1, smn, sdf);
	   }
	   else{
	      if (i == chip_id && j == local_chnl_id) set_fechn_reg(i, j, sts, 0, sg0, sg1, st0, st1, smn, sdf);
	      else set_fechn_reg(i, j, 0, 0, sg0, sg1, st0, st1, smn, sdf);
	   }
	} // loop over 16 channels in each chip
	set_fechip_global(i, slk0, stb1, stb, s16, slk1, sdc, swdac1, swdac2, dac);
     } // loop over all 8 chips
}

// NEW PIECE OF CODE USED FOR TESTING CHANNEL MAP
// BETWEEN FEMB CHANNELS & NEVIS LINKS
// THIS CAN SEND TEST PULSE EITHER THROUGH SINGLE
// CHANNEL OR GROUP OF CHANNELS IN A SINGLE FEMB
// INTRODUCED (02/23/2024, VARUNA MEDDAGE)


void BNL_FE_Reg_Mapping::tst_set_fe_board_for_chnl_testing(uint8_t config_no, uint32_t test_fst_chnl, uint32_t test_lst_chnl, uint32_t sts, uint32_t snc,                              uint32_t sg0, uint32_t sg1, uint32_t st0, uint32_t st1, uint32_t smn, uint32_t sdf, uint32_t slk0, uint32_t stb1, 
                             uint32_t stb, uint32_t s16, uint32_t slk1, uint32_t sdc, uint32_t swdac1, uint32_t swdac2, uint32_t dac){
     const std::string identification = "BNL_FE_Reg_Mapping::tst_set_fe_board_for_chnl_testing";
     
     // This section sets which channels
     // are collections, which are U and which are V
     // by looking into the channel map provide to each
     // FEMB. (FEMB channel map shows which channels are U, V and W in a given FEMB)
     
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
     
     //////////////////////////////////////////////////////////////////////////////////////////////////////////////
     
     // First check whether we need to test whether
     // channel mapping is correct for this FEMB (channel mapping => mapping between FEMB channels and NEVIS links)
     // For that we check parameter "test_fst_chnl" and "test_lst_chnl"
     // in this function. I am planning to set 999 for both "test_fst_chnl" and "test_lst_chnl"
     // if we don't need to test channel mapping for this FEMB.
     // if "test_fst_chnl" and "test_lst_chnl" have different values
     // that means, this FEMB needs to be tested for the accurary of 
     // channel mapping by sending a pulse in a selected set of channels or a selected single channel.
     
     // This decides whether thid FEMB needs to be tested for channel mapping
     bool needs_to_send_a_calib_signal = false;
     if ((test_fst_chnl >=0 && test_fst_chnl <= 127) && (test_lst_chnl >=0 && test_lst_chnl <= 127)) needs_to_send_a_calib_signal = true;
     
     
     if (needs_to_send_a_calib_signal){ // This checks whether this FEMB needs to be sent a calibration signal or signals
         bool single_chnl_calibration = false;
         if (test_fst_chnl == test_lst_chnl) single_chnl_calibration = true;
	 
	 // If a single channel needs to be sent a test pulse
	 // in this FEMB this block of code is executed
	 if (single_chnl_calibration){  
	     int chip_id = test_fst_chnl/16;
             int local_chnl_id = test_fst_chnl%16;
	     
	     for (int i=0; i<8; i++){
                  for (int j=0; j<16; j++){
	               if (channel_wire_plane[i][j] == 2){ 
	                   if (i == chip_id && j == local_chnl_id) set_fechn_reg(i, j, sts, 1, sg0, sg1, st0, st1, smn, sdf);
	                   else set_fechn_reg(i, j, 0, 1, sg0, sg1, st0, st1, smn, sdf);
	               }
	               else{
	                   if (i == chip_id && j == local_chnl_id) set_fechn_reg(i, j, sts, 0, sg0, sg1, st0, st1, smn, sdf);
	                   else set_fechn_reg(i, j, 0, 0, sg0, sg1, st0, st1, smn, sdf);
	               }
	           } // loop over 16 channels in each chip
	           set_fechip_global(i, slk0, stb1, stb, s16, slk1, sdc, swdac1, swdac2, dac);
              } // loop over all 8 chips
	 } // pulse is sent through only in the selected channel
	 
	 // This block is executed if a group of channels of this FEMBS
	 // needs to be sent a calibration pulse
	 else{
	     int chip_id_fst = test_fst_chnl/16;
             int local_chnl_id_fst = test_fst_chnl%16;
             int chip_id_lst = test_lst_chnl/16;
             int local_chnl_id_lst = test_lst_chnl%16;
	     
	     // next check whetehr first and last channel belongs to the same FE ASIC chip
	     
	     bool calib_wires_in_same_chip = false;
	     
	     if (chip_id_fst == chip_id_lst) calib_wires_in_same_chip = true;
	     
	     // This block of code is executed if the group of
	     // channels are in same chip.
	     if (calib_wires_in_same_chip){
	         for (int i=0; i<8; i++){
		      if (i == chip_id_fst){
		          for (int j=0; j<16; j++){
			       if (channel_wire_plane[i][j] == 2){
			           if (j >= local_chnl_id_fst && j<= chip_id_lst) set_fechn_reg(i, j, sts, 1, sg0, sg1, st0, st1, smn, sdf);
	                           else set_fechn_reg(i, j, 0, 1, sg0, sg1, st0, st1, smn, sdf);
			       } // collection plane wire baseline
			       
			       else{
			           if (j >= local_chnl_id_fst && j<= chip_id_lst) set_fechn_reg(i, j, sts, 0, sg0, sg1, st0, st1, smn, sdf);
	                           else set_fechn_reg(i, j, 0, 0, sg0, sg1, st0, st1, smn, sdf);
			       } // induction plane wire baseline
			  } // loop over 16 channels in each chip
		      } // chip having selected group of channels
		      
		      else{
		          for (int j=0; j<16; j++){
			       if (channel_wire_plane[i][j] == 2) set_fechn_reg(i, j, 0, 1, sg0, sg1, st0, st1, smn, sdf); // collection plane BL
			       else set_fechn_reg(i, j, 0, 0, sg0, sg1, st0, st1, smn, sdf); // induction plane BL
			  } // loop over 16 channels in each chip
		      } // chips having other channels
		      set_fechip_global(i, slk0, stb1, stb, s16, slk1, sdc, swdac1, swdac2, dac);
		 } // loop over all 8 chips
	     } // channels are in same chip 
	     
	     // This piece of code is executed if
	     // groups of channels are in mulitple
	     // chips.
	     else{
	         for (int i=0; i<8; i++){
		      if (i == chip_id_fst){
		          for (int j=0; j<16; j++){
			       if (j >= local_chnl_id_fst){
			           if (channel_wire_plane[i][j] == 2) set_fechn_reg(i, j, sts, 1, sg0, sg1, st0, st1, smn, sdf); // collection plane BL
				   else set_fechn_reg(i, j, sts, 0, sg0, sg1, st0, st1, smn, sdf); // induction plane BL
			       } // channels that should be tested
			       
			       else{
			           if (channel_wire_plane[i][j] == 2) set_fechn_reg(i, j, 0, 1, sg0, sg1, st0, st1, smn, sdf); // collection plane BL
				   else set_fechn_reg(i, j, 0, 0, sg0, sg1, st0, st1, smn, sdf); // induction plane BL
			       } // channels no need to check
			  } 
		      } // chip having first channel in the group
		      
		      else if ((i>chip_id_fst) && (i<chip_id_lst)){
		           for (int j=0; j<16; j++){
			        if (channel_wire_plane[i][j] == 2) set_fechn_reg(i, j, sts, 1, sg0, sg1, st0, st1, smn, sdf); // collection plane BL
				else set_fechn_reg(i, j, sts, 0, sg0, sg1, st0, st1, smn, sdf); // induction plane BL
			   }
		      } // chips between 1 st and last 
		      
		      else if (i == chip_id_lst){
		           for (int j=0; j<16; j++){
			        if (j < local_chnl_id_lst){
				    if (channel_wire_plane[i][j] == 2) set_fechn_reg(i, j, sts, 1, sg0, sg1, st0, st1, smn, sdf); // collection plane BL
				    else set_fechn_reg(i, j, sts, 0, sg0, sg1, st0, st1, smn, sdf); // induction plane BL
				} // channels that needs test pulse
				
				else{
				    if (channel_wire_plane[i][j] == 2) set_fechn_reg(i, j, 0, 1, sg0, sg1, st0, st1, smn, sdf); // collection plane BL
				    else set_fechn_reg(i, j, 0, 0, sg0, sg1, st0, st1, smn, sdf); // induction plane BL
				} // channels that don't need test pulse
			   }
		      } // chip having first channel in the group
		      
		      else{
		          for (int j=0; j<16; j++){
			       if (channel_wire_plane[i][j] == 2) set_fechn_reg(i, j, 0, 1, sg0, sg1, st0, st1, smn, sdf); // collection plane BL
			       else set_fechn_reg(i, j, 0, 0, sg0, sg1, st0, st1, smn, sdf); // induction plane BL
			  }
		      } // chips having channels which are not going to be tests with test pulse
		      set_fechip_global(i, slk0, stb1, stb, s16, slk1, sdc, swdac1, swdac2, dac);
		 } // loop over all 8 chips
	     } // channels are in different chips
	     
	 } // pulse is sent through in a selected group of channels
     } // This FEMB needs to check the accuracy of FEMB channels and NEVIS links map
     
     else{
         for (int i=0; i<8; i++){
              for (int j=0; j<16; j++){
	           if (channel_wire_plane[i][j] == 2){ 
	               set_fechn_reg(i, j, 0, 1, sg0, sg1, st0, st1, smn, sdf);
	           }
	           else{
	               set_fechn_reg(i, j, 0, 0, sg0, sg1, st0, st1, smn, sdf);
	           }
	      } // loop over 16 channels in each chip
	      set_fechip_global(i, slk0, stb1, stb, s16, slk1, sdc, swdac1, swdac2, dac);
         } // loop over all 8 chips
     } // This FEMB DOSE NOT need to check the accuracy of FEMB channels and NEVIS links map
}


