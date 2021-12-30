#include "texture_manager.h"

TextureManager::TextureManager()
{

}


TextureManager& TextureManager::GetInstance() {
    static TextureManager instance;     // Guaranteed to be destroyed.
    return instance;                    // Instantiated on first use.
}


Texture* TextureManager::GetTexture( std::string const &name, enum FILTER mipmap ) {
    // This is weird. We already store textures in a hashmap but can't rely on it.
    // TODO: figure out how to rely on it.
    Texture *texture = Texture::Exists( name );

    // Texture already exists
    if (!texture) {
        // Need to create texture
        texture = new Texture( name.c_str(), 0, mipmap, TEXTURE2D, TEXTURE_2D, GFXTRUE );
        textures.push_back(texture);
    }

    return texture;
}
