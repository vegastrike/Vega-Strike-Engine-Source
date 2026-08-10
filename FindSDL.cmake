# - Find SDL 1.2 (via sdl12-compat on modern systems)
# Finds the SDL 1.2 headers and library (libSDL.so from sdl12-compat).
# Sets SDL_FOUND, SDL_INCLUDE_DIR, SDL_LIBRARY.

find_path(SDL_INCLUDE_DIR SDL.h
  HINTS
    ENV SDLDIR
  PATH_SUFFIXES SDL
  PATHS
    /usr/include
    /usr/local/include
)

# sdl12-compat provides libSDL.so / libSDL-1.2.so
find_library(SDL_LIBRARY
  NAMES SDL SDL-1.2
  HINTS
    ENV SDLDIR
  PATHS
    /usr/lib
    /usr/local/lib
)

if(SDL_INCLUDE_DIR AND SDL_LIBRARY)
  set(SDL_FOUND TRUE)
  set(SDL_INCLUDE_DIRS ${SDL_INCLUDE_DIR})
  set(SDL_LIBRARIES ${SDL_LIBRARY})
  if(NOT TARGET SDL::SDL)
    add_library(SDL::SDL UNKNOWN IMPORTED)
    set_target_properties(SDL::SDL PROPERTIES
      IMPORTED_LOCATION ${SDL_LIBRARY}
      INTERFACE_INCLUDE_DIRECTORIES ${SDL_INCLUDE_DIR})
  endif()
  message(STATUS "Found SDL 1.2 (sdl12-compat): ${SDL_LIBRARY}")
endif()

mark_as_advanced(SDL_INCLUDE_DIR SDL_LIBRARY)
