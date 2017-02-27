//
// C++ Implementation: Audio::Codec
//

#include "Codec.h"
#include "config.h"

namespace Audio {

    Codec::~Codec()
    {
    }
    
    Codec::Codec(const std::string &_name) throw() : name(_name)
    {
    }
    
    const std::string& Codec::getName() const throw()
    {
        return name;
    }
    
    const std::vector<std::string>* Codec::getExtensions() const throw()
    {
        return 0;
    }

};
