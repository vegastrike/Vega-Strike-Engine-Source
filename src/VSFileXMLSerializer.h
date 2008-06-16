#ifndef __VSFILEXMLSERIALIZER_H__INCLUDED__
#define __VSFILEXMLSERIALIZER_H__INCLUDED__

#include "XMLDocument.h"
#include "vsfilesystem.h"

namespace XMLDOM {
        
    class VSFileXMLSerializer : public XMLDOM::XMLSerializer
    {
    public:
        bool importXML(const std::string &path, VSFileSystem::VSFileType fileType);
        virtual bool importXML(const std::string &path);
        virtual bool exportXML(std::ostream &stream);
    };

};

#endif//__VSFILEXMLSERIALIZER_H__INCLUDED__
