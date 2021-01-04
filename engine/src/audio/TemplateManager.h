/**
 * TemplateManager.h
 *
 * Copyright (C) Daniel Horn
 * Copyright (C) 2020 pyramid3d, Stephen G. Tuggy, and other Vega Strike
 * contributors
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike.  If not, see <https://www.gnu.org/licenses/>.
 */

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
        AutoPtr<__impl::TemplateManagerData> data;

    public:
        /** Construct a new manager
         * @remarks End-users of the class shouldn't be using this. Singletons need it.
         */
        TemplateManager();

        ~TemplateManager();


        /** Add a definition file, persistent or not */
        void addDefinitionFile(const std::string &path, bool persistent);

        /** Add a definition document under a specified path, always persistent (as there is no way to reload) */
        void addDefinitionFile(const std::string &path, SharedPtr<XMLDOM::XMLDocument> definition);

        /** Get an already loaded definition file, fail if not found or not loaded */
        SharedPtr<XMLDOM::XMLDocument> getDefinitionFile(const std::string &path) const;

        /** Get an already loaded definition file, load if not loaded */
        SharedPtr<XMLDOM::XMLDocument> getDefinitionFile(const std::string &path);


        /** Set default definition file
         * @remarks when a template named without its source definition file is requested, it is assumed
         *      to come from this definition file.
         */
        void setDefaultDefinitionFile(const std::string &x);

        /** Get the default definition file
         * @see setDefaultDefinitionFile
         */
        const std::string& getDefaultDefinitionFile() const;

        /** Get a source template by its key */
        SharedPtr<SourceTemplate> getSourceTemplate(const std::string &name);

        /** Add a manually-created template
          * @param name the name portion of the template's key
          * @param tpl the template to be added
          * @param perm if true, a strong reference will be held and the template will become
          *       permanently loaded.
          * @remarks The key to the newly added template will always be :[name] (empty path),
          *       denoting dynamically-created templates.
          * @note Since the manager only holds weak references to templates if perm is not given,
          *       you must hold onto a reference at least, or the manager will "forget" you added
          *       this resource.
          * @throws ResourceAlreadyLoadedException, when the key already has an associated template.
          */
        void addSourceTemplate(const std::string &name, SharedPtr<SourceTemplate> tpl, bool perm = true);

        /** Add a manually-created template
          * @param path the path portion of the template's key
          * @param name the name portion of the template's key
          * @param tpl the template to be added
          * @param perm if true, a strong reference will be held and the template will become
          *       permanently loaded.
          * @remarks The key to the newly added template will always be [path]:[name].
          *       Using this method is discouraged if collission with filesystem-based templates
          *       would be possible (pick paths that don't map to file system paths)
          * @note Since the manager only holds weak references to templates if perm is not given,
          *       you must hold onto a reference at least, or the manager will "forget" you added
          *       this resource.
          * @throws ResourceAlreadyLoadedException, when the key already has an associated template.
          */
        void addSourceTemplate(const std::string &path, const std::string &name, SharedPtr<SourceTemplate> tpl, bool perm = true);

    protected:

        /** Get a source template by its key */
        SharedPtr<SourceTemplate> loadSourceTemplate(const std::string &name);


    };

};

#endif//__AUDIO_TEMPLATEMANAGER_H__INCLUDED__
