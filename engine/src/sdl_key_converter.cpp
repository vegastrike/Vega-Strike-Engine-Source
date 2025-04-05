#include "src/sdl_key_converter.h"

#include <map>

const std::map<std::string, int> key_code_map = {
};

int SDLKeyConverter::Convert(const std::string key_string) {
    if(key_string.size() == 1) {
        const char& ch = key_string.at(0);

        // uppercase letters
        if(ch >= 65 && ch <= 90) {
            return ch + 32;
        }

        return ch;
    }

    return 0;
}
