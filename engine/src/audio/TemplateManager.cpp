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
#include "xml_support.h"

#include "SourceTemplate.h"

#include "vs_math.h"

using std::string;
using std::min;
using std::map;

template<> Audio::TemplateManager* Singleton<Audio::TemplateManager>::_singletonInstance = 0;

namespace Audio {

    namespace __impl {
    
        struct TemplateManagerData {
            struct DefinitionFileInfo {
                /** Pointer to the already-parsed document */
                SharedPtr<XMLDOM::XMLDocument> parsed;
                
                /** Last time the file was used - for aging and purging */
                mutable Timestamp lastUsageTime;
                
                /** Time since last usage that must pass before the entry is purged.
                 * @note 0 means keep forever. -1 means auto (calculate based on definition count)
                 */
                Duration expirationTime;
                
                void computeExpirationTime() throw()
                {
                    assert(parsed.get());
                    expirationTime = 10 + min(600UL, 2UL * parsed->root.numChildren());
                }
                
                void load(const string &path) throw(Exception)
                {
                    XMLDOM::VSFileXMLSerializer serializer;
                    serializer.initialise();
                    serializer.importXML(path, VSFileSystem::SoundFile);
                    parsed.reset(serializer.close());
                    touch();
                }
                
                void touch() const throw()
                {
                    lastUsageTime = getRealTime();
                }
            };
        
            // The many required indices
            typedef map<string, WeakPtr<SourceTemplate> > TemplateMap;
            typedef map<string, SharedPtr<SourceTemplate> > PermTemplateMap;
            typedef map<string, DefinitionFileInfo> DefinitionMap;
            
            TemplateMap loadedTemplates;
            PermTemplateMap permLoadedTemplates;
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
                    throw NotFoundException(path);
                }
            }
            
            SharedPtr<XMLDOM::XMLDocument> getDefinitionFile(const std::string &path) const
                throw(Exception)
            {
                DefinitionMap::const_iterator it = loadedDefinitions.find(path);
                if (it != loadedDefinitions.end()) {
                    if (it->second.parsed.get() == 0)
                        throw ResourceNotLoadedException(); else
                        it->second.touch();
                    return it->second.parsed;
                } else {
                    throw NotFoundException(path);
                }
            }
        };
    
        static VSFileSystem::VSFileType parseVSFileType(const string &s)
        {
            static map<string, VSFileSystem::VSFileType> enumMap;
            if (enumMap.empty()) {
            
                enumMap["universe"] = VSFileSystem::UniverseFile;
                enumMap["system"] = VSFileSystem::SystemFile;
                enumMap["cockpit"] = VSFileSystem::CockpitFile;
                enumMap["unit"] = VSFileSystem::UnitFile;
                enumMap["unitSave"] = VSFileSystem::UnitSaveFile;
                enumMap["texture"] = VSFileSystem::TextureFile;
                enumMap["sound"] = VSFileSystem::SoundFile;
                enumMap["python"] = VSFileSystem::PythonFile;
                enumMap["mesh"] = VSFileSystem::MeshFile;
                enumMap["comm"] = VSFileSystem::CommFile;
                enumMap["ai"] = VSFileSystem::AiFile;
                enumMap["save"] = VSFileSystem::SaveFile;
                enumMap["anim"] = VSFileSystem::AnimFile;
                enumMap["video"] = VSFileSystem::VideoFile;
                enumMap["sprite"] = VSFileSystem::VSSpriteFile;
                enumMap["mission"] = VSFileSystem::MissionFile;
                enumMap["music"] = VSFileSystem::MusicFile;
                enumMap["account"] = VSFileSystem::AccountFile;
                enumMap["zoneBuffer"] = VSFileSystem::ZoneBuffer;
                enumMap["jpegBuffer"] = VSFileSystem::JPEGBuffer;
                enumMap["unknown"] = VSFileSystem::UnknownFile;
            }
            return parseEnum(s, enumMap);
        }
        
        static bool parseBool(const std::string &s)
        {
            if (s.empty())
                throw InvalidParametersException("Missing required attribute");
            else
                return XMLSupport::parse_bool(s);
        }
        
        static float parseFloat(const std::string &s)
        {
            if (s.empty())
                throw InvalidParametersException("Invalid float attribute");
            else
                return XMLSupport::parse_floatf(s);
        }
        
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
        return ((const TemplateManagerData &)*data).getDefinitionFile(path);
    }
    
    SharedPtr<XMLDOM::XMLDocument> TemplateManager::getDefinitionFile(const std::string &path) 
        throw(Exception)
    {
        try {
            return data->getDefinitionFile(path);
        } catch(NotFoundException e) {
            addDefinitionFile(path, false);
            return data->getDefinitionFile(path);
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

    SharedPtr<SourceTemplate> TemplateManager::getSourceTemplate(const std::string &name) 
        throw(Exception)
    {
        SharedPtr<SourceTemplate> rv;
        
        TemplateManagerData::TemplateMap::const_iterator it = data->loadedTemplates.find(name);
        if (it != data->loadedTemplates.end())
            rv = it->second.lock();
        
        if (!rv.get()) {
            rv = loadSourceTemplate(name);
            data->loadedTemplates[name] = rv;
        }
        
        return rv;
    }
    
    SharedPtr<SourceTemplate> TemplateManager::loadSourceTemplate(const std::string &name) 
        throw(Exception)
    {
        string::size_type sep = name.find_first_of(':');
        SharedPtr<XMLDOM::XMLDocument> def;
        
        if (sep != string::npos) 
            def = getDefinitionFile(name.substr(0,sep)); else
            def = getDefinitionFile(getDefaultDefinitionFile());
        
        const XMLDOM::XMLElement *tdef = 0;
        if (sep != string::npos)
            tdef = def->getElementByName(name.substr(sep+1, string::npos)); else
            tdef = def->getElementByName(name);
        
        if (!tdef)
            throw NotFoundException(name);
        
        string src = 
            tdef->getAttributeValue("src","");
        
        if ( src.empty() )
            throw InvalidParametersException("Invalid source template: no sound specified");
        
        VSFileSystem::VSFileType type = 
            parseVSFileType( tdef->getAttributeValue("type", "unknown") );
        bool looping =
            parseBool( tdef->getAttributeValue("looping", "false") );
        
        SharedPtr<SourceTemplate> rv(
            new SourceTemplate(src, type, looping) );
        
        rv->setAngleRange( Range<Scalar>(
            parseFloat( tdef->getAttributeValue("minAngle", "180") ) * M_PI / 180.f,
            parseFloat( tdef->getAttributeValue("maxAngle", "180") ) * M_PI / 180.f
        ) );
        
        rv->setPerFrequencyRadiusRatios( PerFrequency<Scalar>(
            parseFloat( tdef->getAttributeValue("lfRadiusRatio", "1") ),
            parseFloat( tdef->getAttributeValue("hfRadiusRatio", "1") )
        ) );
        
        rv->setPerFrequencyRadiusRatios( PerFrequency<Scalar>(
            parseFloat( tdef->getAttributeValue("lfReferenceFreq", "250") ),
            parseFloat( tdef->getAttributeValue("hfReferenceFreq", "5000") )
        ) );
        
        rv->setGain( 
            parseFloat( tdef->getAttributeValue("gain", "1") )
        );
        
        rv->setStreaming( 
            parseBool( tdef->getAttributeValue("streaming", "false") )
        );
        
        return rv;
    }
    
    
    void TemplateManager::addSourceTemplate(const string &name, SharedPtr<SourceTemplate> tpl, bool perm) 
        throw(ResourceAlreadyLoadedException)
    {
        static string empty;
        addSourceTemplate(empty, name, tpl, perm);
    }

    void TemplateManager::addSourceTemplate(const string &path, const string &name, SharedPtr<SourceTemplate> tpl, bool perm) 
        throw(ResourceAlreadyLoadedException)
    {
        string key(path + ":" + name);
        SharedPtr<SourceTemplate> rv;
        
        TemplateManagerData::TemplateMap::const_iterator it = data->loadedTemplates.find(key);
        if (it != data->loadedTemplates.end())
            rv = it->second.lock();
        
        if (!rv.get()) {
            rv = tpl;
            data->loadedTemplates[key] = rv;
            if (perm)
                data->permLoadedTemplates[key] = rv;
        } else {
            throw ResourceAlreadyLoadedException(key);
        }
    }
    
};
