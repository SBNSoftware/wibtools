#include <MBB.hh>
#include <BNL_UDP_Exception.hh>
#include <stdio.h> //snprintf
#include <iostream>
#include <MBBException.hh>
#include <unistd.h>
#include <sstream>
#include <iomanip>
#include <bitset>

#define sleep(x) usleep((useconds_t) x * 1e6)


MBB::MBB(std::string const & address, std::string const & MBBAddressTable, bool fullStart): started(false), ContinueOnMBBRegReadError(false)
{
  mbb = new AddressTable(MBBAddressTable,address,0);

  if(fullStart){
     //Turn on write acknowledgments
     mbb->SetWriteAck(false);
     Write("UDP_EN_WR_RDBK", 1);//set to enable write echo mode.
     mbb->SetWriteAck(true);                                            
     started = true; 
     }

}

std::string MBB::GetAddress(){
  return mbb->GetRemoteAddress();
}

uint32_t MBB::Read(uint16_t address)
{
  return mbb->Read(address);    
}

uint32_t MBB::Read(std::string const & address)
{
  return mbb->Read(address);    
}

void MBB::Write(uint16_t address,uint32_t value)
{
  mbb->Write(address,value);    
}

void MBB::Write(std::string const & address,uint32_t value)
{
  mbb->Write(address,value);    
}

void MBB::FullStart()
{
  started = true;
}

void MBB::InitializeMBB()
{

}

//void MBB::ResetMBB(bool cntrlRegister=true, bool global=false, bool daq_path=false, bool udp=false){
void MBB::ResetMBB(bool reset_udp)
{
}

void MBB::EnableWIBs(uint8_t icrate,uint32_t value)
{
  WritePTC(icrate,2/*,value*/);
}

//MBB defaults to send the correct signals to the WIBs on power up.
void MBB::WritePTC(uint8_t icrate, uint16_t address/*, uint32_t value*/){
  Write("PTC_DATA_ADDRESS", address);
  Write("PTC_CRATE_ADDRESS", icrate);
  //Write("PTC_DATA", value);//should be set to 0 as per original email by Matt. If set to zero will turn ON all WIBs. 
  Write("PTC_WR_REG", 0);
  usleep(1000);
  Write("PTC_WR_REG", 1);
  usleep(1000);
  Write("PTC_WR_REG", 0);
  }
// WritePTC should only have two arguments. We should write into PTC_DATA via configMBB.

void MBB::ConfigPTC(uint8_t icrate){
  if (/*icrate < 0 || to allow for d0 PTC*/ icrate > CRATE_COUNT){
	MBBException::MBB_BAD_ARGS e;
	std::stringstream expstr;
	expstr << "ConfigPTC: icrate should be between 0 and CRATE_COUNT: "
	       << int(icrate);
	e.Append(expstr.str().c_str());
	throw e;
        }

    /*std::string reg = "PTC_DATA";
    const Item *g = GetItem(reg);
    WritePTC(icrate, 0x2, g->mask);*/ // this is what I had originally.

    //Doing configMBB and reading PTC_DATA gives different value than doing configAllPTCs and then reading it(0xffff always). This is my proposed solution.
    //uint32_t sol= Read("PTC_DATA");
  WritePTC(icrate, 0x2/*, sol*/);
}

void MBB::ConfigAllPTCs(){
          for(uint8_t icrate=1; icrate <= CRATE_COUNT; icrate++){
	      ConfigPTC(icrate); 
              }
          }

/** \Setup MBB
 *
 *  Sets up MBB
 *  mbb_config: list of options to configure the mbb:
 *          PLL clock type: 0,1 for External clock input and Internal 16MHz clock respectively. 
 *          Pulse source: 0,1 for LEMO input and MBB internal pulse generator respectively.
 *          Pulse Period: 0,1,2 for 0ns, 10ns, 20ns and so on.
 *          WIB Power: 0,1 for OFF, ON respectively.
 */
// adding a comment just to push.
void MBB::ConfigMBB(uint32_t PULSE_SOURCE, uint32_t PULSE_PERIOD, uint32_t wib_pwr1, uint32_t wib_pwr2, uint32_t wib_pwr3, uint32_t wib_pwr4, uint32_t wib_pwr5, uint32_t wib_pwr6){

     if (PULSE_SOURCE > 1){
         MBBException::MBB_BAD_ARGS e;
         std::stringstream expstr;
         expstr << "ConfigMBB: PULSE_SOURCE is allowed to be 0 (LEMO), 1 (MBB) but is: " << int(PULSE_SOURCE);
         e.Append(expstr.str().c_str());
         throw e;
         }

     // 4294967295 the decimal corresponding to 0xFFFFFFFF.
     if (PULSE_PERIOD > 4294967295){
         MBBException::MBB_BAD_ARGS e;
         std::stringstream expstr;
         expstr << "ConfigMBB: PULSE_PERIOD is allowed to be 0 (0ns), 1 (10ns) and so on.. but is: " << int(PULSE_PERIOD);
         e.Append(expstr.str().c_str());
         throw e;
         }


     if (wib_pwr1 > 1){
         MBBException::MBB_BAD_ARGS e;
         std::stringstream expstr;
         expstr << "ConfigMBB: wib_pwr1 is allowed to be 0 (OFF), 1 (ON) but is: " << int(wib_pwr1);
         e.Append(expstr.str().c_str());
         throw e;
         }

     if (wib_pwr2 > 1){
         MBBException::MBB_BAD_ARGS e;
         std::stringstream expstr;
         expstr << "ConfigMBB: wib_pwr2 is allowed to be 0 (OFF), 1 (ON) but is: " << int(wib_pwr2);
         e.Append(expstr.str().c_str());
         throw e;
         }

     if (wib_pwr3 > 1){
         MBBException::MBB_BAD_ARGS e;
         std::stringstream expstr;
         expstr << "ConfigMBB: wib_pwr3 is allowed to be 0 (OFF), 1 (ON) but is: " << int(wib_pwr3);
         e.Append(expstr.str().c_str());
         throw e;
         }
  
     if (wib_pwr4 > 1){
         MBBException::MBB_BAD_ARGS e;
         std::stringstream expstr;
         expstr << "ConfigMBB: wib_pwr4 is allowed to be 0 (OFF), 1 (ON) but is: " << int(wib_pwr4);
         e.Append(expstr.str().c_str());
         throw e;
         }

     if (wib_pwr5 > 1){
         MBBException::MBB_BAD_ARGS e;
         std::stringstream expstr;
         expstr << "ConfigMBB: wib_pwr5 is allowed to be 0 (OFF), 1 (ON) but is: " << int(wib_pwr5);
         e.Append(expstr.str().c_str());
         throw e;
         }

     if (wib_pwr6 > 1){
         MBBException::MBB_BAD_ARGS e;
         std::stringstream expstr;
         expstr << "ConfigMBB: wib_pwr6 is allowed to be 0 (OFF), 1 (ON) but is: " << int(wib_pwr6);
         e.Append(expstr.str().c_str());
         throw e;
         }

     /*if(PLL_CLOCK_TYPE==0){
        if(Read("PLL_ACTIVE_CLK")==1){
           Write("PLL_CLOCK_SELECT",0);
           Write("PLL_CLOCK_SELECT",1);
           }
        else if(Read("PLL_ACTIVE_CLK")==0){
	        Write("PLL_CLOCK_SELECT",0);
	        }
        }

     if(PLL_CLOCK_TYPE==1){
        if(Read("PLL_ACTIVE_CLK")==0){
	  Write("PLL_CLOCK_SELECT",0);
	  Write("PLL_CLOCK_SELECT",1);
          }
        else if(Read("PLL_ACTIVE_CLK")==1){
                Write("PLL_CLOCK_SELECT",1);
	        }
		}*/ 

     Write("PULSE_SRC_SELECT", PULSE_SOURCE);
     Write("PULSE_PERIOD",     PULSE_PERIOD);

     //controlling WIB power; default setting is to turn ON all WIBs

     Write("PTC_DATA", 0x3F); // turns off all the WIBs
     
     // controlling power on WIB 1
     if(wib_pwr1==1)  Write("PTC_DATA", 0x3E);//turns ON WIB 1 and all others OFF(implied).
 
     // controlling power on WIB 2 
     if(wib_pwr2==1){
           if(wib_pwr1==1)  Write("PTC_DATA", 0x3C);// turns ON WIB 1,2 and all others OFF(implied).
           if(wib_pwr1==0)  Write("PTC_DATA", 0x3D);// turns ON WIB 2.
        }
 
     // controlling power on WIB 3
     if(wib_pwr3==1){
              if(wib_pwr2==0){
	                      if(wib_pwr1==0) Write("PTC_DATA", 0x3B);// turns ON WIB 3.
                              if(wib_pwr1==1) Write("PTC_DATA", 0x3A);// turns ON WIB 1,3.
                             }
              if(wib_pwr2==1){
                              if(wib_pwr1==0) Write("PTC_DATA", 0x39);// turns ON WIB 2,3.                                          
                              if(wib_pwr1==1) Write("PTC_DATA", 0x38);// turns ON WIB 1,2,3.                                                    
                             }
          }

     // controlling power on WIB 4
     if(wib_pwr4==1){
            if(wib_pwr3==1){
	           if(wib_pwr2==1){
	                           if(wib_pwr1==1) Write("PTC_DATA", 0x30);// turns ON WIB 1,2,3,4.
	                           if(wib_pwr1==0) Write("PTC_DATA", 0x31);// turns ON WIB 2,3,4.
	                          }
         
	           if(wib_pwr2==0){
	                           if(wib_pwr1==1) Write("PTC_DATA", 0x32);// turns ON WIB 1,3,4.
	                           if(wib_pwr1==0) Write("PTC_DATA", 0x33);// turns ON WIB 3,4.
	                          }
	  }

	    if(wib_pwr3==0){
	           if(wib_pwr2==1){
	                           if(wib_pwr1==1) Write("PTC_DATA", 0x34);// turns ON WIB 1,2,4.                                      
                                   if(wib_pwr1==0) Write("PTC_DATA", 0x35);// turns ON WIB 2,4.                                          
                                  }
	  
	           if(wib_pwr2==0){
	                           if(wib_pwr1==1) Write("PTC_DATA", 0x36);// turns ON WIB 1,4.                                           
                                   if(wib_pwr1==0) Write("PTC_DATA", 0x37);// turns ON WIB 4.                                           
                                  }
	      }
        }

     // controlling power on WIB 5
     if(wib_pwr5==1){
            if(wib_pwr4==1){
	           if(wib_pwr3==1){
                          if(wib_pwr2==1){
	                                  if(wib_pwr1==1) Write("PTC_DATA", 0x20);// turns ON WIB 1,2,3,4,5.                                                 
                                          if(wib_pwr1==0) Write("PTC_DATA", 0x21);// turns ON WIB 2,3,4,5.                                 
                                          }

                          if(wib_pwr2==0){
                                          if(wib_pwr1==1) Write("PTC_DATA", 0x22);// turns ON WIB 1,3,4,5.                             
                                          if(wib_pwr1==0) Write("PTC_DATA", 0x23);// turns ON WIB 3,4,5.                                
                                         }
	               }

	           if(wib_pwr3==0){
                          if(wib_pwr2==1){
	                                  if(wib_pwr1==1) Write("PTC_DATA", 0x24);// turns ON WIB 1,2,4,5.                               
                                          if(wib_pwr1==0) Write("PTC_DATA", 0x25);// turns ON WIB 2,4,5.                                  
                                          }

                          if(wib_pwr2==0){
	                                  if(wib_pwr1==1) Write("PTC_DATA", 0x26);// turns ON WIB 1,4,5.                                  
                                          if(wib_pwr1==0) Write("PTC_DATA", 0x27);// turns ON WIB 4,5.                                  
                                         }
	              }
	    }

	    if(wib_pwr4==0){
	           if(wib_pwr3==1){
	                  if(wib_pwr2==1){
	                                  if(wib_pwr1==1) Write("PTC_DATA", 0x28);// turns ON WIB 1,2,3,5.                                                   
                                          if(wib_pwr1==0) Write("PTC_DATA", 0x29);// turns ON WIB 2,3,5.                                
                                          } 

	                  if(wib_pwr2==0){
	                                  if(wib_pwr1==1) Write("PTC_DATA", 0x2A);// turns ON WIB 1,3,5.                               
                                          if(wib_pwr1==0) Write("PTC_DATA", 0x2B);// turns ON WIB 3,5.                                 
                                         }  
	            }

	           if(wib_pwr3==0){
	                  if(wib_pwr2==1){
	                                  if(wib_pwr1==1) Write("PTC_DATA", 0x2C);// turns ON WIB 1,2,5.                                
                                          if(wib_pwr1==0) Write("PTC_DATA", 0x2D);// turns ON WIB 2,5.                                   
                                         }

	                  if(wib_pwr2==0){
	                                  if(wib_pwr1==1) Write("PTC_DATA", 0x2E);// turns ON WIB 1,5.                                 
                                          if(wib_pwr1==0) Write("PTC_DATA", 0x2F);// turns ON WIB 5.                                
                                         }
		   }
	    }
     }
             
     // controlling power on WIB 6
	if(wib_pwr6==1){
	      if(wib_pwr5==1){
	            if(wib_pwr4==1){
	                  if(wib_pwr3==1){
		                if(wib_pwr2==1){
		                                if(wib_pwr1==1) Write("PTC_DATA", 0x00);// turns ON WIB 1,2,3,4,5,6.                                    
                                                if(wib_pwr1==0) Write("PTC_DATA", 0x01);// turns ON WIB 2,3,4,5,6.                                      
                                               }

	                        if(wib_pwr2==0){
		                                if(wib_pwr1==1) Write("PTC_DATA", 0x02);// turns ON WIB 1,3,4,5,6.                                     
                                                if(wib_pwr1==0) Write("PTC_DATA", 0x03);// turns ON WIB 3,4,5,6.                                      
                                               }
		    }

	                 if(wib_pwr3==0){
		               if(wib_pwr2==1){
		                               if(wib_pwr1==1) Write("PTC_DATA", 0x04);// turns ON WIB 1,2,4,5,6.                                       
                                               if(wib_pwr1==0) Write("PTC_DATA", 0x05);// turns ON WIB 2,4,5,6.                                          
                                              }

		               if(wib_pwr2==0){
		                               if(wib_pwr1==1) Write("PTC_DATA", 0x06);// turns ON WIB 1,4,5,6.                               
                                               if(wib_pwr1==0) Write("PTC_DATA", 0x07);// turns ON WIB 4,5,6.                                        
                                              }
	           }
	      }

	            if(wib_pwr4==0){
	                  if(wib_pwr3==1){
		                if(wib_pwr2==1){
		                                if(wib_pwr1==1) Write("PTC_DATA", 0x08);// turns ON WIB 1,2,3,5,6.                                      
                                                if(wib_pwr1==0) Write("PTC_DATA", 0x09);// turns ON WIB 2,3,5,6.                                         
                                               }

		                if(wib_pwr2==0){
		                                if(wib_pwr1==1) Write("PTC_DATA", 0x0A);// turns ON WIB 1,3,5,6.                                        
                                                if(wib_pwr1==0) Write("PTC_DATA", 0x0B);// turns ON WIB 3,5,6.                                         
                                               }
                  }

	                  if(wib_pwr3==0){
		                if(wib_pwr2==1){
		                                if(wib_pwr1==1) Write("PTC_DATA", 0x0C);// turns ON WIB 1,2,5,6.                                             
                                                if(wib_pwr1==0) Write("PTC_DATA", 0x0D);// turns ON WIB 2,5,6.                                            
                                                }

		                if(wib_pwr2==0){
		                                if(wib_pwr1==1) Write("PTC_DATA", 0x0E);// turns ON WIB 1,5,6.                                        
                                                if(wib_pwr1==0) Write("PTC_DATA", 0x0F);// turns ON WIB 5,6.                                                
                                               }
				}
	      }
		    }
	

	     if(wib_pwr5==0){
	           if(wib_pwr4==1){
	                 if(wib_pwr3==1){
		               if(wib_pwr2==1){
		                               if(wib_pwr1==1) Write("PTC_DATA", 0x10);// turns ON WIB 1,2,3,4,6.                                           
                                               if(wib_pwr1==0) Write("PTC_DATA", 0x11);// turns ON WIB 2,3,4,6.                                          
                                              }

		               if(wib_pwr2==0){
		                               if(wib_pwr1==1) Write("PTC_DATA", 0x12);// turns ON WIB 1,3,4,6.                                          
                                               if(wib_pwr1==0) Write("PTC_DATA", 0x13);// turns ON WIB 3,4,6.                                          
                                              }
	           }

	                 if(wib_pwr3==0){
		               if(wib_pwr2==1){
		                               if(wib_pwr1==1) Write("PTC_DATA", 0x14);// turns ON WIB 1,2,4,6.                                            
                                               if(wib_pwr1==0) Write("PTC_DATA", 0x15);// turns ON WIB 2,4,6.                                                
                                              }

		               if(wib_pwr2==0){
		                               if(wib_pwr1==1) Write("PTC_DATA", 0x16);// turns ON WIB 1,4,6.                                         
                                               if(wib_pwr1==0) Write("PTC_DATA", 0x17);// turns ON WIB 4,6.                                                  
                                              }
	        }
	     }
          
                   if(wib_pwr4==0){
		         if(wib_pwr3==1){
		               if(wib_pwr2==1){
                                               if(wib_pwr1==1) Write("PTC_DATA", 0x18);// turns ON WIB 1,2,3,6.                                             
                                               if(wib_pwr1==0) Write("PTC_DATA", 0x19);// turns ON WIB 2,3,6.                                              
                                               }

		               if(wib_pwr2==0){
                                               if(wib_pwr1==1) Write("PTC_DATA", 0x1A);// turns ON WIB 1,3,6.                                              
                                               if(wib_pwr1==0) Write("PTC_DATA", 0x1B);// turns ON WIB 3,6.                                                  
                                              }
		   }

		         if(wib_pwr3==0){
		               if(wib_pwr2==1){
                                               if(wib_pwr1==1) Write("PTC_DATA", 0x1C);// turns ON WIB 1,2,6.                                             
                                               if(wib_pwr1==0) Write("PTC_DATA", 0x1D);// turns ON WIB 2,6.                                                  
                                              }

		               if(wib_pwr2==0){
                                               if(wib_pwr1==1) Write("PTC_DATA", 0x1E);// turns ON WIB 1,6.                                             
                                               if(wib_pwr1==0) Write("PTC_DATA", 0x1F);// turns ON WIB 6.                                                  
                                              }
			 }
		   }
	     }
	      
	}
	

	//storing the WIB Power values corresponding to each PTC.
        crate_num = Read("PTC_CRATE_ADDRESS");
	if(crate_num == 1){
	   p1 = Read("PTC_DATA");
          }
   
        if(crate_num == 2){
           p2 = Read("PTC_DATA");
	}

        if(crate_num == 3){
           p3 = Read("PTC_DATA");
	}

        if(crate_num == 4){
           p4 = Read("PTC_DATA");
	}
     

     /*if(Read("FIRMWARE_VERSION") == Read("SYS_RESET")) { // can't read register if equal
       if(ContinueOnMBBRegReadError){
           std::cout << "Error: Can't read registers from MBB"<< std::endl;
           return;
           }
        MBBException::MBB_REG_READ_ERROR e;
        std::stringstream expstr;
        expstr << " Register Read Error" << std::endl; 
	e.Append(expstr.str().c_str());
        throw e;
        }*/

     //Write("REG_RESET", 1);
     sleep(1);
    
     
}


void MBB::SetContinueOnMBBRegReadError(bool enable){
  ContinueOnMBBRegReadError = enable;
}


void MBB::TimeStampReset(){
  Write("TIMESTAMP_RESET", 1);
  sleep(1);
  Write("TIMESTAMP_RESET", 0);
  }
