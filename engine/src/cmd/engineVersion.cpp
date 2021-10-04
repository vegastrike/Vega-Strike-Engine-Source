//#include <cstdlib>
#include <boost/python.hpp>

#include "cmd/engineVersion.h"
#include "python/python_class.h"

#include "version.h"

using namespace VegaStrike;

EngineVersionData::

EngineVersionData::EngineVersionData()
{
    this->major = static_cast<uint8_t>(VEGASTRIKE_VERSION_MAJOR_UINT);
    this->minor = static_cast<uint8_t>(VEGASTRIKE_VERSION_MINOR_UINT);
    this->patch = static_cast<uint8_t>(VEGASTRIKE_VERSION_PATCH_UINT);

    this->assetApiVersion = static_cast<uint16_t>(VEGASTRIKE_ASSETS_API_VERSION_UINT);

    // Alternate method of converting the string value to wchar_t for easier python consumption
    //  const char* raw_version = VEGASTRIKE_VERSION_TWEAK;
    //  const size_t wcsSize = 1024;
    //  wchar_t convertedBytes[wcsSize];
    //  std::mbstowcs(convertedBytes, raw_version, wcsSize);

    // if we can though...just let the compiler do it since it's already a C Macro the
    // compiler knows about
    const wchar_t* convertedBytes = L"" VEGASTRIKE_VERSION_TWEAK;
    this->hash = convertedBytes;
}

EngineVersionData::~EngineVersionData()
{
}

boost::python::tuple EngineVersionData::GetVersion() const
{
    return VS_BOOST_MAKE_TUPLE_4(
        this->major,
        this->minor,
        this->patch,
        this->hash
    );
}

uint16_t EngineVersionData::GetAssetAPIVersion() const
{
    return this->assetApiVersion;
}
