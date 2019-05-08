#include <helpers/parseHelpers.hh>

#include <boost/tokenizer.hpp>

#include <vector>
#include <string>
#include <utility> //std::pair


std::vector<std::string> splitString(std::string const & line,
				     std::string const & separator){
  //Create a boost tokenizer that tokenizes on separator
  boost::char_separator<char> sep(separator.c_str());
  boost::tokenizer<boost::char_separator<char> > tokenizedFormat(line,sep);

  //Go through each token and add it to a vector
  std::vector<std::string> ret;
  for(boost::tokenizer<boost::char_separator<char> >::iterator itTok = tokenizedFormat.begin();
      itTok != tokenizedFormat.end();
      itTok++){
    ret.push_back(*itTok);
  }
  return ret;
}

std::vector<std::pair<size_t,size_t> > parseRange(std::string const & line){
  std::vector<std::pair<size_t,size_t> > ret;
  //Check if there is exactly one occurance of '-'
  if(1 == std::count(line.begin(),line.end(),'-')){
    //Split the string around '-'
    std::vector<std::string> bounds = splitString(line,"-");
    //Check if there are two non-zero parts
    if((2 == bounds.size()) &&
       (bounds[0].size() > 0) &&
       (bounds[1].size() > 0)){
      std::pair<size_t,size_t> pair;
     
      //Convert the first numeric value
      char const * strPtr = bounds[0].c_str();
      char * endPtr;
      pair.first  = strtoul(strPtr,&endPtr,0);
      if(size_t(endPtr-strPtr) == bounds[0].size()){      
	//Good numerical value
	strPtr = bounds[1].c_str();      
	pair.second = strtoul(strPtr,&endPtr,0);
	if(size_t(endPtr-strPtr) == bounds[1].size()){
	  //good numerical value
	  //Everything parsed correctly, add this pair
	  ret.push_back(pair);
	}
      }
    }
  }    
  return ret;
}

std::vector<size_t> parseList(std::string const & line){
  std::vector<size_t> ret;
  
  std::vector<std::string> commaSplit = splitString(line,",");
  for(size_t iSplit = 0; iSplit < commaSplit.size();iSplit++){
    //Check if this token is a range
    std::vector<std::pair<size_t,size_t> > range = parseRange(commaSplit[iSplit]);
    if(range.size() > 0){
      //we have a range
      for(size_t iRange = range[0].first; iRange <= range[0].second;iRange++){
	ret.push_back(iRange);
      }
    }else{
      //We don't have a range, so try to parse it as a number
      char const * strPtr =commaSplit[iSplit].c_str();
      char * endPtr;
      size_t val = strtoul(strPtr,&endPtr,0);
      if(size_t(endPtr-strPtr) == commaSplit[iSplit].size()){	
	//good numerical value
	ret.push_back(val);      
      }
    }
  }
  return ret;
}
