#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define NUM_FRAMES 6  // 6 frames in the sprite sheet
#define FRAME_TIME 60 // Milliseconds per frame (~12 FPS)

typedef struct mouse_motion
{
    float x, y;
} MouseMotion;

int main(int argc, char *argv[])
{
    // Initialize SDL3 and SDL3_image
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    // Create window and renderer
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (!SDL_CreateWindowAndRenderer("Kitties", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_FULLSCREEN, &window, &renderer))
    {
        SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_SetWindowTitle(window, "SDL3 Sprite Sheet Animation (6 Frames)");

    // Load sprite sheet
    SDL_Surface *surface = IMG_Load("kitties_spritesheet.png");
    if (!surface)
    {
        SDL_Log("IMG_Load failed: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    if (!texture)
    {
        SDL_Log("SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Query texture dimensions and calculate frame size
    float tex_width, tex_height;
    SDL_GetTextureSize(texture, &tex_width, &tex_height);
    if ((int)tex_width % 3 != 0)
    {
        SDL_Log("Warning: Texture width (%.2f) not divisible by %d frames!", tex_width, NUM_FRAMES);
    }
    int frame_width = tex_width / 3;
    int frame_height = tex_height / 2;

    printf("Sprite sheet loaded: %.2fx%.2f | Frame size: %dx%d\n", tex_width, tex_height, frame_width, frame_height);

    // Animation state
    int current_frame = 0;
    Uint64 last_frame_time = SDL_GetTicks();
    SDL_FRect dst_rect = {0, 0, frame_width, frame_height}; // Will be updated to mouse pos

    // // Hide cursor
    // SDL_HideCursor();

    // Main loop
    SDL_Event event;
    bool running = true;
    bool dragging = false;

    float drag_offset_x = 0.0f, drag_offset_y = 0.0f;
    while (running)
    {
        // Poll events
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (event.key.key == SDLK_ESCAPE)
                    running = false;
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    float mouse_x = event.button.x;
                    float mouse_y = event.button.y;
                    // Check if mouse click collides with the texture's dst_rect
                    if (mouse_x >= dst_rect.x && mouse_x < dst_rect.x + dst_rect.w &&
                        mouse_y >= dst_rect.y && mouse_y < dst_rect.y + dst_rect.h)
                    {
                        dragging = true;
                        // Calculate offset to keep the drag relative to click position
                        drag_offset_x = mouse_x - dst_rect.x;
                        drag_offset_y = mouse_y - dst_rect.y;
                    }
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                if (dragging)
                {
                    // Update dst_rect position based on mouse motion
                    dst_rect.x = event.motion.x - drag_offset_x;
                    dst_rect.y = event.motion.y - drag_offset_y;
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    dragging = false; // Release: dst_rect is now fixed in new position
                }
                break;
            }
        }

        // Update animation
        Uint64 current_time = SDL_GetTicks();
        if (current_time - last_frame_time >= FRAME_TIME)
        {
            current_frame = (current_frame + 1) % NUM_FRAMES; // Loop 0->5->0
            last_frame_time = current_time;
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 216, 233, 243, 255); // Dark background
        SDL_RenderClear(renderer);

        // Source rect: Current frame from sprite sheet
        SDL_FRect src_rect = {
            (current_frame % 3) * frame_width,
            (current_frame / 3) * frame_height,
            frame_width * 1.0f,
            frame_height * 1.0f};

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
