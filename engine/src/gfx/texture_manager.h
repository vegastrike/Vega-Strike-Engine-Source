#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include <vector>

#include "gfx/aux_texture.h"

class TextureManager
{
    std::vector<Texture*> textures;
public:
    TextureManager();

    static TextureManager& GetInstance();
    Texture* GetTexture( std::string const &name, enum FILTER mipmap );
};

#endif // TEXTUREMANAGER_H
