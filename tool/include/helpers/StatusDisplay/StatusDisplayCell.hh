#ifndef __STATUS_DISPLAY_CELL_HH__
#define __STATUS_DISPLAY_CELL_HH__


#include <ostream>
#include <iostream> //for std::cout
#include <vector>
#include <string>
#include <map>
#include <boost/unordered_map.hpp>
#include <boost/tokenizer.hpp> //for tokenizer

namespace BUTool{

  /*! \brief One cell in a status display.
   *
   * One cell corresponds to one item from the AMC13 address table,
   * along with all the additional information required to display it.
   * see http://bucms.bu.edu/twiki/bin/view/BUCMSPublic/AMC13AddressTable.
   */
  class StatusDisplayCell{
  public:
    ///! Constructor
    StatusDisplayCell(){Clear();};
    ///! Fill in values for a cell
    void Setup(std::string const & _address,  /// address table name stripped of Hi/Lo
	       std::string const & _description, /// long description
	       std::string const & _row,	 /// display row
	       std::string const & _col,	 /// display column
	       std::string const & _format,	 /// display format
	       std::string const & _rule,	 /// nz, z, nzr etc
	       std::string const & _statusLevel, /// status level to display
	       bool enabled = true);             /// enabled status
    ///! store a value plus a shift for a multi-word value
    void Fill(uint32_t value,
	      size_t bitShift = 0);

    ///! Determine if this cell should be displayed based on rule, statusLevel
    bool Display(int level,bool force = false) const;
    ///! Determine if this cell's row should be suppressed based on rule="nzr"
    bool SuppressRow( bool force) const;
    int DisplayLevel() const;
    std::string Print(int width,bool html = false) const;
    std::string const & GetRow() const;
    std::string const & GetCol() const;
    std::string const & GetDesc() const;
    std::string const & GetAddress() const;

    void SetAddress(std::string const & _address);
    uint32_t const & GetMask() const;
    void SetMask(uint32_t const & _mask);

    void SetEnabled(bool d){enabled = d;}
    bool GetEnabled(){return enabled;}

  private:    
    void Clear();
    void CheckAndThrow(std::string const & name,
		       std::string & thing1,
		       std::string const & thing2) const;
    uint64_t ComputeValue() const;

    std::string address;
    std::string description;
    std::string row;
    std::string col;
    bool enabled;

    uint32_t mask;
    
    std::vector<uint32_t > word;
    std::vector<int> wordShift;
    std::string format;
    std::string displayRule;   
    int statusLevel;
  };

}
#endif
