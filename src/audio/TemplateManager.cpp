//
// C++ Implementation: Audio::TemplateManager
//

#include "TemplateManager.h"
#include "config.h"

#include <utility>
#include <assert.h>

#include "XMLDocument.h"
#include "VSFileXMLSerializer.h"

#include "utils.h"

using namespace std;

namespace Audio {

    namespace __impl {
    
        struct TemplateManagerData {
            struct TemplateList {
                /** A weak pointer to the template itself
                * @remarks check to see if it is still valid! 
                */
                WeakPtr<SourceTemplate> templatePtr;
            };
            
            struct DefinitionFileInfo {
                /** Pointer to the already-parsed document */
                SharedPtr<XMLDOM::XMLDocument> parsed;
                
                /** Last time the file was used - for aging and purging */
                Timestamp lastUsageTime;
                
                /** Time since last usage that must pass before the entry is purged.
                 * @note 0 means keep forever. -1 means auto (calculate based on definition count)
                 */
                Duration expirationTime;
                
                void computeExpirationTime() throw()
                {
                    assert(parsed.get());
                    expirationTime = 10 + min(600, 2 * parsed->numChildren());
                }
                
                void load(const string &path) throw(Exception)
                {
                    XMLDOM::VSFileXMLSerializer serializer;
                    serializer.initialise();
                    serializer.importXML(path);
                    parsed.reset(serializer.close());
                    touch();
                }
                
                void touch() throw()
                {
                    lastUsageTime = getRealTime();
                }
            };
        
            // The many required indices
            typedef map<string, TemplateInfo> TemplateMap;
            typedef map<string, DefinitionFileInfo> DefinitionMap;
            
            TemplateMap loadedTemplates;
            DefinitionMap loadedDefinitions;
            
            string defaultDefinitionFile;
            
            SharedPtr<XMLDOM::XMLDocument> getDefinitionFile(const std::string &path)
                throw(Exception)
            {
                DefinitionMap::iterator it = loadedDefinitions.find(path);
                if (it != loadedDefinitions.end()) {
                    if (it->second.parsed.get() == 0)
                        it->second.load(path); else
                        it->second.touch();
                    return it->second.parsed;
                } else {
                    throw NotFoundException();
                }
            }
            
            SharedPtr<XMLDOM::XMLDocument> getDefinitionFile(const std::string &path) const
                throw(ResourceNotLoadedException)
            {
                DefinitionMap::iterator it = loadedDefinitions.find(path);
                if (it != loadedDefinitions.end()) {
                    if (it->second.parsed.get() == 0)
                        throw ResourceNotLoadedException(); else
                        it->second.touch();
                    return it->second.parsed;
                } else {
                    throw NotFoundException();
                }
            }
        };
    
    };
    
    using namespace __impl;

    TemplateManager::TemplateManager() throw() :
        data(new TemplateManagerData)
    {
    }
    
    TemplateManager::~TemplateManager()
    {
    }

    void TemplateManager::addDefinitionFile(const string &path, bool persistent) throw(Exception)
    {
        // Add an unparsed definition, for lazy loading.
        if (data->loadedDefinitions.count(path) == 0) {
            TemplateManagerData::DefinitionFileInfo &info = data->loadedDefinitions[path];
            info.expirationTime = persistent?-1:0;
        }
    }
    
    void TemplateManager::addDefinitionFile(const string &path, SharedPtr<XMLDOM::XMLDocument> definition) throw(Exception)
    {
        if (data->loadedDefinitions.count(path) == 0) {
            TemplateManagerData::DefinitionFileInfo &info = data->loadedDefinitions[path];
            info.expirationTime = 0;
            info.lastUsageTime = 0;
            info.parsed = definition;
        }
    }

    SharedPtr<XMLDOM::XMLDocument> TemplateManager::getDefinitionFile(const std::string &path) const 
        throw(ResourceNotLoadedException)
    {
        return ((const TemplateManagerData)*data)->getDefinitionFile();
    }
    
    SharedPtr<XMLDOM::XMLDocument> TemplateManager::getDefinitionFile(const std::string &path) 
        throw(Exception)
    {
        try {
            return data->getDefinitionFile();
        } catch(ResourceNotLoadedException e) {
            addDefinitionFile(path, false);
            return data->getDefinitionFile();
        }
    }

    void TemplateManager::setDefaultDefinitionFile(const std::string &x) 
        throw()
    {
        data->defaultDefinitionFile = x;
    }
    
    const std::string& TemplateManager::getDefaultDefinitionFile() const 
        throw()
    {
        return data->defaultDefinitionFile;
    }

};
