#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define NUM_FRAMES 6     // 6 frames in the sprite sheet
#define FRAME_TIME 60    // Milliseconds per frame (~12 FPS)

int main(int argc, char *argv[]) {
    // Initialize SDL3 and SDL3_image
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    // Create window and renderer
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (!SDL_CreateWindowAndRenderer("Kitties", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_FULLSCREEN, &window, &renderer)) {
        SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_SetWindowTitle(window, "SDL3 Sprite Sheet Animation (6 Frames)");

    // Load sprite sheet
    SDL_Surface *surface = IMG_Load("kitties_spritesheet.png");
    if (!surface) {
        SDL_Log("IMG_Load failed: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    if (!texture) {
        SDL_Log("SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Query texture dimensions and calculate frame size
    float tex_width, tex_height;
    SDL_GetTextureSize(texture, &tex_width, &tex_height);
    if ((int)tex_width % 3 != 0) {
        SDL_Log("Warning: Texture width (%.2f) not divisible by %d frames!", tex_width, NUM_FRAMES);
    }
    int frame_width = tex_width / 3;
    int frame_height = tex_height / 2;

    printf("Sprite sheet loaded: %.2fx%.2f | Frame size: %dx%d\n", tex_width, tex_height, frame_width, frame_height);

    // Animation state
    int current_frame = 0;
    Uint64 last_frame_time = SDL_GetTicks();
    SDL_FRect dst_rect = { 0, 0, frame_width, frame_height }; // Will be updated to mouse pos

    //Hide cursor
    SDL_HideCursor();

    // Main loop
    SDL_Event event;
    bool running = true;
    while (running) {
        // Poll events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    // Center sprite on mouse position
                    dst_rect.x = event.motion.x - frame_width / 2.0f;
                    dst_rect.y = event.motion.y - frame_height / 2.0f;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    if(event.key.key == SDLK_ESCAPE)
                        running = false;
                    break;
            }
        }

        // Update animation
        Uint64 current_time = SDL_GetTicks();
        if (current_time - last_frame_time >= FRAME_TIME) {
            current_frame = (current_frame + 1) % NUM_FRAMES; // Loop 0->5->0
            last_frame_time = current_time;
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255); // Dark background
        SDL_RenderClear(renderer);

        // Source rect: Current frame from sprite sheet
        SDL_FRect src_rect = {
            (current_frame % 3) * frame_width,
            (current_frame / 3) * frame_height,
            frame_width * 1.0f,
            frame_height * 1.0f
        };

        // Draw animated frame at mouse position
        SDL_RenderTexture(renderer, texture, &src_rect, &dst_rect);

        SDL_RenderPresent(renderer);

        // Cap ~60 FPS
        SDL_Delay(16);
    }

    // Cleanup
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
