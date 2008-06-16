//
// C++ Interface: Audio::CodecRegistry
//
#ifndef __AUDIO_CODECREGISTRY_H__INCLUDED__
#define __AUDIO_CODECREGISTRY_H__INCLUDED__

#include <string>
#include <map>
#include <set>
#include "Exceptions.h"
#include "Singleton.h"

#include "vsfilesystem.h"

namespace Audio {

    // The registry needs not know what it's managing... right?
    // At least not its interface.
    class Codec;
    class Stream;

    /**
     * Codec registry class.
     *
     * @remarks Use it to manage codec implementations.
     *      @par It handles codec factories and manages resolving file-to-codec associations.
     * @see Codec
     *
     */
    class CodecRegistry : public Singleton<CodecRegistry>
    {
    private:
        // The many required indexes
        typedef std::set<Codec*> CodecSet;
        typedef std::map<Codec*,int> CodecPriority;
        typedef std::map<std::string, CodecSet> ExtensionCodecs;
        typedef std::map<std::string, Codec*> NameCodec;
        
        CodecPriority codecPriority;
        ExtensionCodecs extensionCodecs;
        CodecSet universalCodecs;
        NameCodec nameCodec;
    
    public:
        /** Construct an empty registry 
         * @remarks End-users of the class shouldn't be using this. Singletons need it*/
        CodecRegistry() throw();
        
        ~CodecRegistry();
        
        /**
         * Add a codec to the registry
         * @remarks You may add a codec multiple times if you wish. The class guarantees
         *      that any further call will be a no-op. Previos priority will still apply.
         * @param codec The codec to be added to the registry.
         */
        void add(Codec* codec, int priority = 0) throw();
        
        /**
         * Remove a codec from the registry
         * @remarks If the codec had already been removed, this is a no-op.
         * @param codec The codec to be removed from the registry.
         */
        void remove(Codec* codec) throw();
        
        /**
         * Find a codec by its name
         * @returns A codec instance named by the specified name, never null.
         * @remarks If there's more than one codec with the same name, any one of them is returned.
         *      Not even necessarily the same each time. So avoid adding conflicting codecs.
         *      @par Instead of returning null, if a codec of such characteristics cannot be found, 
         *      a CodecNotFound exception is risen.
         */
        Codec* findByName(const std::string& name) const throw(CodecNotFoundException);
        
        /**
         * Find a codec that can handle the file.
         * @returns A codec instance that can handle the specified file (ie: canHandle() returns true).
         *      It will never return null.
         * @remarks If there's more than one codec that can handle the file, the one with the higher
         *      priority will be returned. Notice that it can't be assured that the codec will be able
         *      to open the file, so be prepared to handle failures further down the road.
         *      @par Instead of returning null, if a codec of such characteristics cannot be found, 
         *      an CodecNotFound exception is risen.
         */
        Codec* findByFile(const std::string& path, VSFileSystem::VSFileType type = VSFileSystem::UnknownFile) const throw(CodecNotFoundException);
        
        /**
         * Open the specified file with a suitable codec.
         * @see findByFile
         */
        Stream* open(const std::string& path, VSFileSystem::VSFileType type = VSFileSystem::UnknownFile) const throw(Exception);
        
    };
    
    class CodecRegistration
    {
        Codec *codec;
    public:
        CodecRegistration(Codec* codec, int priority = 0) throw();
        ~CodecRegistration();
    };

};

#endif//__AUDIO_CODECREGISTRY_H__INCLUDED__
