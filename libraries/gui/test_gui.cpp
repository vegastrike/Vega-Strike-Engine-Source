#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>

#include "main_menu.h"

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    constexpr SDL_WindowFlags window_flags = 0;
    SDL_Window* window = SDL_CreateWindow(
        "SDL3 Window",
        800, 600,
        window_flags
    );

    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    if (window && renderer) {
        std::cout << "Created window and renderer\n" << std::flush;
    } else {
        std::cout << "Error creating window and renderer\n" << std::flush;
    }

    showMenu(renderer, window);

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
