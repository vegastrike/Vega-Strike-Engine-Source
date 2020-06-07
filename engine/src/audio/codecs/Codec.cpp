//
// C++ Implementation: Audio::Codec
//

#include "Codec.h"

namespace Audio {

    Codec::~Codec()
    {
    }
    
    Codec::Codec(const std::string &_name) : name(_name)
    {
    }
    
    const std::string& Codec::getName() const
    {
        return name;
    }
    
    const std::vector<std::string>* Codec::getExtensions() const
    {
        return 0;
    }

};
