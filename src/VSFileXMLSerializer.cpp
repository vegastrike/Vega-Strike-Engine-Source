#include "VSFileXMLSerializer.h"
#include "config.h"

#include "vsfilesystem.h"
#include "audio/Exceptions.h"

namespace XMLDOM {

    bool VSFileXMLSerializer::importXML(const std::string &path)
    {
        return importXML(path, VSFileSystem::UnknownFile);
    }

    bool VSFileXMLSerializer::importXML(const std::string &path, VSFileSystem::VSFileType fileType)
    {
        VSFileSystem::VSFile file;
        if (file.OpenReadOnly(path, fileType) > VSFileSystem::Ok)
            throw Audio::FileOpenException(string("Cannot open \"") + path + "\"");
        
        char buffer[4096];
        size_t rode;
        
        while (rode = file.Read(buffer, 4096)) 
            parse(buffer, rode);
        
        file.Close();
    }

    bool VSFileXMLSerializer::exportXML(std::ostream &stream)
    {
        throw Audio::Exception("Cannot export to VSFile");
    }

};

