#ifndef VS_ENGINE_VERSION_H__
#define VS_ENGINE_VERSION_H__

#include <string>
#include <unistd.h>

#include "boost/tuple/tuple.hpp"

namespace VegaStrike {

    //typedef ::boost::tuple<uint8_t, uint8_t, uint8_t, std::wstring> EngineVersionTuple;

    class EngineVersionData
    {
    public:
        EngineVersionData();
        ~EngineVersionData();

        // Game Engine Version <major>.<minor>.<patch>.<hash>
        // presented as a tuple
        boost::python::tuple GetVersion() const;

        // Asset API Version, simply numeric incrementer
        uint16_t GetAssetAPIVersion() const;
    protected:
        uint8_t major;
        uint8_t minor;
        uint8_t patch;
        std::wstring hash;

        uint16_t assetApiVersion;
    };
}

#endif //VS_ENGINE_VERSION_H__
