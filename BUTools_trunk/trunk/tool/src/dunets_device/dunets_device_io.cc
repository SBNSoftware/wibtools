#include "dunets_device/dunets_device.hh"
#include <BUException/ExceptionBase.hh>


//
// read one or more registers
// one argument may be: address, single name or regular expression
// second argument is count to read from each address
// last argument may be "D" for doublewords (64-bits)
//


std::vector<std::string> BUTool::DUNETSDevice::myMatchRegex(std::string regex){
  return hw->getNodes(regex);  
}

uint32_t BUTool::DUNETSDevice::RegReadAddress(uint32_t addr){ 
  uhal::ValWord<uint32_t> vw; //valword for transaction
  try{
    vw = hw->getClient().read(addr); // start the transaction
    hw->getClient().dispatch(); // force the transaction
  }catch (uhal::exception::ReadAccessDenied & e){
    BUException::REG_READ_DENIED e2;    
    char str_addr[] = "0xXXXXXXXX";
    snprintf(str_addr,10,"0x%08X",addr);
    e2.Append(str_addr);
    throw e2;
  }

  return vw.value();
}
uint32_t BUTool::DUNETSDevice::RegReadRegister(std::string const & reg){
  uhal::ValWord<uint32_t> ret;
  try{
    ret = hw->getNode( reg).read() ;
    hw->dispatch();
  }catch (uhal::exception::ReadAccessDenied & e){
    BUException::REG_READ_DENIED e2;    
    e2.Append(reg);
    throw e2;
  }catch (uhal::exception::NoBranchFoundWithGivenUID & e){
    BUException::BAD_REG_NAME e2;
    e2.Append(reg);
    throw e2;
  }
  return ret.value();
}
void BUTool::DUNETSDevice::RegWriteAction(std::string const & reg){
  //This is a funky uhal thing
  try{
    uint32_t addr = hw->getNode(reg).getAddress();
    uint32_t mask = hw->getNode(reg).getMask();
    hw->getClient().write(addr, mask);
    hw->dispatch();
  }catch (uhal::exception::NoBranchFoundWithGivenUID & e){
    BUException::BAD_REG_NAME e2;
    e2.Append(reg);
    throw e2;
  }catch (uhal::exception::WriteAccessDenied & e){
    BUException::REG_WRITE_DENIED e2;
    e2.Append(reg);
    throw e2;
  }
}
void BUTool::DUNETSDevice::RegWriteAddress(uint32_t addr,uint32_t data){
  try{
    hw->getClient().write( addr, data);
    hw->getClient().dispatch() ;
  }catch (uhal::exception::WriteAccessDenied & e){
    BUException::REG_WRITE_DENIED e2;
    char str_addr[] = "0xXXXXXXXX";
    snprintf(str_addr,10,"0x%08X",addr);
    e2.Append(str_addr);
    throw e2;
  }
}
void BUTool::DUNETSDevice::RegWriteRegister(std::string const & reg, uint32_t data){
  try{
    hw->getNode( reg ).write( data );
    hw->dispatch() ;  
  }catch (uhal::exception::NoBranchFoundWithGivenUID & e){
    BUException::BAD_REG_NAME e2;
    e2.Append(reg);
    throw e2;
  }catch (uhal::exception::WriteAccessDenied & e){
    BUException::REG_WRITE_DENIED e2;
    e2.Append(reg);
    throw e2;
  }
}

uint32_t BUTool::DUNETSDevice::GetRegAddress(std::string const & reg){return hw->getNode(reg).getAddress();}
uint32_t BUTool::DUNETSDevice::GetRegMask(std::string const & reg){return hw->getNode(reg).getMask();}
uint32_t BUTool::DUNETSDevice::GetRegSize(std::string const & reg){return hw->getNode(reg).getSize();}
std::string BUTool::DUNETSDevice::GetRegMode(std::string const & reg){
  std::string ret;
  uhal::defs::BlockReadWriteMode mode = hw->getNode(reg).getMode();
  switch( mode) {
  case uhal::defs::INCREMENTAL:
    ret += " inc";
    break;
  case uhal::defs::NON_INCREMENTAL:
    ret += " non-inc";
    break;
  case uhal::defs::HIERARCHICAL:
  case uhal::defs::SINGLE:
  default:
    break;
  }
  return ret;
}
std::string BUTool::DUNETSDevice::GetRegPermissions(std::string const & reg){
  std::string ret;
  uhal::defs::NodePermission perm = hw->getNode(reg).getPermission();
  switch( perm) {
  case uhal::defs::READ:
    ret += " r";
    break;
  case uhal::defs::WRITE:
    ret += " w";
    break;
  case uhal::defs::READWRITE:
    ret += " rw";
    break;
  default:
    ;
  }
  return ret;
}

std::string BUTool::DUNETSDevice::GetRegDescription(std::string const & reg){return hw->getNode(reg).getDescription();}

std::string BUTool::DUNETSDevice::GetRegDebug(std::string const & reg){
  const boost::unordered_map<std::string,std::string> params = hw->getNode(reg).getParameters();
  std::string ret;
  for( boost::unordered_map<std::string,std::string>::const_iterator it = params.begin();
       it != params.end();
       it++) {
    ret+="   ";
    ret+=it->first.c_str();
    ret+=" = ";
    ret+=it->second.c_str();
    ret+="\n";
  }    
  return ret;
}

  
  

  
