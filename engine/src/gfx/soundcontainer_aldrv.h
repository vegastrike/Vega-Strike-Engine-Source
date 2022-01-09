/**
* soundcontainer_aldrv.h
*
* Copyright (C) 2001-2002 Daniel Horn
* Copyright (C) 2002-2019 pyramid3d and other Vega Strike Contributors
* Copyright (C) 2019-2022 Stephen G. Tuggy and other Vega Strike Contributors
*
* https://github.com/vegastrike/Vega-Strike-Engine-Source
*
* This file is part of Vega Strike.
*
* Vega Strike is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Vega Strike is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _SOUNDCONTAINER_ALDRV_H_
#define _SOUNDCONTAINER_ALDRV_H_

#include <string>

#include "soundcontainer.h"

/**
 * A reference to a soundfile that can be lazily loaded.
 * 
 * This class implements the operations on sounds using aldrv
 */
class AldrvSoundContainer : public GameSoundContainer
{
private:
    int sound;
    
public:
    /**
     * Create a refernece to an optionally-looping sound file.
     */
    AldrvSoundContainer( const SoundContainer &other );
    
    virtual ~AldrvSoundContainer();
    
protected:
    virtual void loadImpl();
    virtual void unloadImpl();
    virtual void playImpl();
    virtual void stopImpl();
    virtual bool isPlayingImpl() const;
};

#endif

