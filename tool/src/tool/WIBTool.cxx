#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
//TCLAP parser
#include <tclap/CmdLine.h>
#include "tool/Launcher.hh"
#include "tool/CLI.hh"
#include "tool/CommandReturn.hh"
#include "tool/DeviceFactory.hh"

#include <WIBException/ExceptionBase.hh>
#include <WIB.hh>

#include <readline/readline.h> //for rl_delete_text
#include <signal.h> //signals                                                                                                                                                                                                 
using namespace WIBTool;                                                 

#define DevFac WIBTool::DeviceFactory::Instance()


volatile bool running = true;

void signal_handler(int sig){
  if(sig == SIGINT){
    //remove SIGINT from signal_handler
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);    
    
    
    //Set an alarm for a second from now
    alarm(1);
    
    //cleanup line
    rl_delete_text(0,rl_end);
    printf("\n");
    //    rl_on_new_line();
    rl_forced_update_display();
    rl_done=1;
    
  }else if (sig == SIGALRM){    
    //re-enable SIGINT capture
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);    
  }
}



std::string LimitStringLines(std::string source,size_t beginLineCount = 5,size_t endLineCount = 2) {
  //Load the first beginLineCount lines.
  if((source.size() > 0)&&(source.find('\n') == std::string::npos)){
    source=source+'\n';
  }
  std::string beginString;
  while( beginLineCount && !source.empty()) {
    //Find the next new line
    size_t pos = source.find('\n');
    if(pos == std::string::npos) {
      source.clear();
      break;
    }
    
    //append the line associated with it to our begin string with a tab at the beginning
    beginString += std::string("\t") + source.substr(0,pos) + std::string("\n");
    //Move past the newline
    pos++;
    //trim string
    source = source.substr(pos,source.size()-pos);
    
    beginLineCount--;
  }

  std::string endString;
  while(endLineCount && !source.empty()) {
    //Find the next new line
    size_t pos = source.rfind('\n');
    
    if(pos == std::string::npos) {
      //We didn't find a newline, so this is the last line
      pos = 0;
    } else if(++pos == source.size()) { //Move past the null line, but catch if it was the last char.
      source.resize(source.size()-1);
      continue;
    }
    
    //reverse append the line associated with it to our begin string with a tab at the beginning
    endString = std::string("\t") + source.substr(pos,source.size()-pos) + 
      std::string("\n") + endString;
    
    //trim source string
    if(pos >0) {
      pos--; //Move back to the newline
      source = source.substr(0,pos); //trim up to the newline
    } else { // nothing left, so clear
      source.clear();
    }
    
    endLineCount--;
  }
  
  //Build final string
  if(!source.empty()) {
    //Count the number of skipped lines if non-zero
    size_t skippedLineCount = 1;
    for(size_t iStr = 0; iStr < source.size();iStr++) {
      if(source[iStr] == '\n')
	skippedLineCount++;
    }
    std::ostringstream s;
    s << "*** Skipping " << skippedLineCount << " lines! ***\n";
    beginString += s.str();
  }
  beginString += endString;
  return beginString;
}

int main(int argc, char* argv[]) 
{
  //signal handling
  struct sigaction sa;
  sa.sa_handler = signal_handler;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGALRM,&sa, NULL);

  //Create semaphores
  int sem_timeout_ms=10000;
  std::cout << "Acquiring semaphores.\n";
  sem_t *sem_wib_lck = sem_open(WIB::SEMNAME_WIBLCK, O_CREAT, 0666, 1);
  sem_t *sem_wib_yld = sem_open(WIB::SEMNAME_WIBYLD, O_CREAT, 0666, 1);
  if (sem_wib_lck == SEM_FAILED || sem_wib_yld == SEM_FAILED) {
    std::cout << "Failed to create either " << WIB::SEMNAME_WIBLCK << " or " << WIB::SEMNAME_WIBYLD << ", exiting...\n";
    sem_close(sem_wib_lck);
    sem_close(sem_wib_yld);
    return -1;
  }

  struct timespec timeout;
  clock_gettime(CLOCK_REALTIME, &timeout);
  timeout.tv_nsec += 500000;
  if (sem_timedwait(sem_wib_yld, &timeout) != 0){
    std::cout << "Failed to acquire " << WIB::SEMNAME_WIBYLD<< " semaphore, exiting...\n";
    sem_close(sem_wib_lck);
    sem_close(sem_wib_yld);
    return -1;
  }

  clock_gettime(CLOCK_REALTIME, &timeout);
  timeout.tv_sec += sem_timeout_ms / 1000;
  timeout.tv_nsec += (sem_timeout_ms % 1000) * 1000000;  
  if (sem_timedwait(sem_wib_lck, &timeout) != 0){
    std::cout << "Failed to acquire " << WIB::SEMNAME_WIBLCK << " semaphore, exiting...\n";
    sem_post(sem_wib_lck);
    sem_post(sem_wib_yld);
    sem_close(sem_wib_lck);
    sem_close(sem_wib_yld);
    return -1;
  }
  std::cout << "Acquired semaphores.\n";

  {//CLI Scope
  //Create CLI
  CLI cli;

  //Create Command launcher (early, so we can set things)
  Launcher launcher;

  
  try {

    TCLAP::CmdLine cmd("Tool for talking to AMC13 modules.",
		       ' ',
		       "AMC13Tool v2");
    
    //Script files
    TCLAP::ValueArg<std::string> scriptFile("X",              //one char flag
					    "script",         // full flag name
					    "script filename",//description
					    false,            //required
					    std::string(""),  //Default is empty
					    "string",         // type
					    cmd);

    //connections
    std::map<std::string,TCLAP::MultiArg<std::string>* >connections;
    std::vector<std::string> Devices = DevFac->GetDeviceNames();
    for(size_t iDevice = 0;
	iDevice < Devices.size();
	iDevice++){
      std::string  CLI_flag;      
      std::string  CLI_full_flag;
      std::string  CLI_description;

      if(DevFac->CLIArgs(Devices[iDevice],CLI_flag,CLI_full_flag,CLI_description)){
	  connections[Devices[iDevice]] = new TCLAP::MultiArg<std::string>(CLI_flag,       //one char flag
									   CLI_full_flag,  // full flag name
									   CLI_description,//description
									   false,          //required
									   "string",       // type
									   cmd);
      }
    }
    
    //Device libraries
    TCLAP::MultiArg<std::string> libraries("l",                    //one char flag
					   "add_library",          // full flag name
					   "Device library to add",//description
					   false,                  //required
					   "string",               // type
					   cmd);



    //Parse the command line arguments
    cmd.parse(argc,argv);

    //Load requested device libraries
    for(std::vector<std::string>::const_iterator it = libraries.getValue().begin(); 
	it != libraries.getValue().end();
	it++)
      {
	cli.ProcessString("add_lib " + *it);
      }


    //setup connections
    //Loop over all device types
    for(std::map<std::string,TCLAP::MultiArg<std::string>* >::iterator itDeviceType = connections.begin();
	itDeviceType != connections.end();
	itDeviceType++){
      //Loop over connections requests for each device
      for(std::vector<std::string>::const_iterator itDev = itDeviceType->second->getValue().begin(); 
	  itDev != itDeviceType->second->getValue().end();
	  itDev++)
	{
	  cli.ProcessString("add_device " + itDeviceType->first + " " +  *itDev);
	}
    }


    //Load scripts
    if(scriptFile.getValue().size()){
      cli.ProcessFile(scriptFile.getValue());
    }



  } catch (TCLAP::ArgException &e) {
    fprintf(stderr, "Error %s for arg %s\n",
	    e.error().c_str(), e.argId().c_str());
    return 0;
  }


  //============================================================================
  //Main loop
  //============================================================================

  while (running) 
    {
      try {
	//Get parsed command
	std::vector<std::string> command = cli.GetInput(&launcher);

	//Check if this is just the user hitting return (do nothing if it is)
	if(command.size() > 0){
	  //Launch command function
	  CommandReturn::status ret = launcher.EvaluateCommand(command);

	  //check return value
	  if(ret == CommandReturn::EXIT){
	    //Shutdown tool
	    running = false;       
	  }else if ( ret == CommandReturn::NOT_FOUND ){
	    if(cli.InScript()){ //Fail to command line if in script
	      std::cout << "Bad command in script. Exit!\n";
	      running = false;	  
	    }else{
	      std::cout << "Bad command\n";
	    }
	  }
	}
      }catch (WIBException::exBase & e){

	uint32_t verbose_level = launcher.GetVerbosity();

       	std::string errorstr(e.what());
       	std::cout << "\n\n\nCaught Tool exception: " << errorstr << std::endl;
	
	switch(verbose_level){
	case 1:
	  std::cout << LimitStringLines(e.Description(),5,5);
	  break;
	case 2:
	  std::cout << LimitStringLines(e.Description(),10,10);
	  break;
	case 3:
	  std::cout << LimitStringLines(e.Description(),20,20);
	  break;
	case 9:
	  std::cout << LimitStringLines(e.Description(),0,1000);
	  break;
	default:
	  std::cout << LimitStringLines(e.Description());
	  break;
	}
       	if(9 == verbose_level){
	  std::cout << e.StackTrace();
	}
	if(cli.InScript()){ //Fail to command line if in script
       	  running = false;
	}
       	cli.ClearInput(); //Clear any scripted commands
      }catch (std::exception  & e){
	std::string errorstr(e.what());
	errorstr.erase(std::remove(errorstr.begin(), 
				   errorstr.end(), 
				   '\n'), 
		       errorstr.end());
	
	std::cout << "\n\n\nCaught std::exception " << errorstr << ". Shutting down.\n";
	running = false;
	cli.ClearInput(); //Clear any scripted commands
      }
    }
  }//CLI Scope

  std::cout << "Releasing semaphores.\n";
  sem_post(sem_wib_yld);
  sem_post(sem_wib_lck);
  sem_close(sem_wib_lck);
  sem_close(sem_wib_yld);
  return 0;
}
