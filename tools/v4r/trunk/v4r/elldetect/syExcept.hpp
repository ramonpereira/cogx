//
// (C) 2010, Aitor Aldoma Buchaca,
//           Johann Prankl,
//           Gerhard Obernberger <gerhard@obernberger.at>
//
//

#ifndef  _SYEXCEPT_HPP
#define  _SYEXCEPT_HPP

#include "multiplatform.hpp"

#include <stdexcept>

#define __HERE__   __FILE__, __FUNCTION__, __LINE__

using namespace std;

NAMESPACE_CLASS_BEGIN( RTE )

//begin class///////////////////////////////////////////////////////////////////
//
//    CLASS DESCRIPTION:
//       RTE Exception class
//
//    FUNCTION DESCRIPTION:
//       An informative exception class.
//       Example:
//            Except(__FILE__, __FUNCTION__, __LINE__, "There were %d %s in the tree.", 42, "elephants");
//       output:
//            "DumbFile.cc:StupidFunction:28: There were 42 elephants in the tree."
//       Note: You can use the __HERE__ macro to get shorter statements:
//            Except(__HERE__, "There were %d %s in the tree.", 42, "elephants");
//
////////////////////////////////////////////////////////////////////////////////
class CzExcept : public exception
{
   string _what;
public:
   CzExcept(const char *file, const char *function, int line,
      const char *format, ...) throw();
   virtual ~CzExcept() throw() {}
   virtual const char* what() const throw() {return _what.c_str();}
   void Set(const string &s) {_what = s;}
};
//end class/////////////////////////////////////////////////////////////////////

NAMESPACE_CLASS_END()

#endif
