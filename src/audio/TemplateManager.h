//
// C++ Interface: Audio::TemplateManager
//
#ifndef __AUDIO_TEMPLATEMANAGER_H__INCLUDED__
#define __AUDIO_TEMPLATEMANAGER_H__INCLUDED__

#include <string>
#include <map>
#include <set>
#include "Types.h"
#include "Exceptions.h"
#include "Singleton.h"

namespace XMLDOM {
    class XMLDocument;
};


namespace Audio {

    // The manager needs not know what it's managing... right?
    // At least not its interface.
    class SourceTemplate;
    
    namespace __impl {
        
        // Forward declaration of internal template manager data
        struct TemplateManagerData;
        
    };

    /**
     * Template manager class.
     *
     * @remarks Use it to create and manage source templates.
     *      @par Instead of storing source templates themselves, since they may be shared,
     *      you should store shared pointers to them. The manager will automatically
     *      detect and unload unreferenced templates.
     *      @par Templates are addressed by key - keys are composed in a very specific way.
     *      They are all of the form [path]:[name], where [path] is a source definition
     *      xml file and [name] is a source name within the xml file.
     *      @par Usually xml files are loaded on-demand, and unloaded shortly after. Some
     *      definition files, however, may contain heavily used sources and those are useful
     *      to keep laying around, preparsed. You can instruct the manager to do so with
     *      addDefinitionFile().
     * @see SourceTemplate
     *
     */
    class TemplateManager : public Singleton<TemplateManager>
    {
    private:
        AutoPtr<TemplateManagerData> data;
    
    public:
        /** Construct a new manager 
         * @remarks End-users of the class shouldn't be using this. Singletons need it.
         */
        TemplateManager() throw();
        
        ~TemplateManager();
        
        
        /** Add a definition file, persistent or not */
        void addDefinitionFile(const std::string &path, bool persistent) throw(Exception);
        
        /** Add a definition document under a specified path, always persistent (as there is no way to reload) */
        void addDefinitionFile(const std::string &path, SharedPtr<XMLDOM::XMLDocument> definition) throw(Exception);
        
        /** Get an already loaded definition file, fail if not found or not loaded */
        SharedPtr<XMLDOM::XMLDocument> getDefinitionFile(const std::string &path) const throw(ResourceNotLoadedException);
        
        /** Get an already loaded definition file, load if not loaded */
        SharedPtr<XMLDOM::XMLDocument> getDefinitionFile(const std::string &path) throw(Exception);
        
        
        /** Set default definition file
         * @remarks when a template named without its source definition file is requested, it is assumed
         *      to come from this definition file.
         */
        void setDefaultDefinitionFile(const std::string &x) throw();
        
        /** Get the default definition file
         * @see setDefaultDefinitionFile
         */
        const std::string& getDefaultDefinitionFile() const throw();
        
        /** Get a source template by its name */
        SharedPtr<SourceTemplate> getSourceTemplate(const std::string &name) throw(Exception);
    };
    
};

#endif//__AUDIO_TEMPLATEMANAGER_H__INCLUDED__
