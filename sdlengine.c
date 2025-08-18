#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define BACKWARD -1
#define STAND 0 
#define FORWARD 1

struct Vector { int x; int y; };
typedef struct Vector Vector;

int create_window(SDL_Window **window, SDL_Renderer **renderer) {
    if (SDL_Init(0) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    *window = SDL_CreateWindow("Name", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!*window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (!*renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    return 0;
}

long get_current_time_in_ms() {
    struct timespec ts;

    timespec_get(&ts, TIME_UTC);
    long ms = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    return ms;
}

int main() {
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool running = true;
    SDL_Event event;

    SDL_Rect player = {100, 100, 50, 50};
    SDL_Rect square = {200, 200, 100, 100};

    long previous_frame_time = get_current_time_in_ms();

    bool collided = false;
    const int speed = 1;

    create_window(&window, &renderer);

    while (running) {
        // calculating the delta time (time in ms between frames) to make the movement not be framerate dependent
        long current_frame_time = get_current_time_in_ms();
        int delta_time = current_frame_time - previous_frame_time;
        previous_frame_time = current_frame_time;
        int current_frame_speed = delta_time * speed;

        // this vector is 
        Vector movement_vector = {STAND, STAND};

        // quit the app if user closes the window
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &player);
        SDL_RenderFillRect(renderer, &square);

        // input
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_ESCAPE]) running = false;
        if (keys[SDL_SCANCODE_D]) movement_vector.x = FORWARD;
        if (keys[SDL_SCANCODE_A]) movement_vector.x = BACKWARD;
        if (keys[SDL_SCANCODE_W]) movement_vector.y = BACKWARD;
        if (keys[SDL_SCANCODE_S]) movement_vector.y = FORWARD; 
        
        // collision
        if (player.x + player.w > square.x && player.x < square.x + square.w &&
            player.y + player.h > square.y && player.y < square.y + square.h) { // check collision with an enemy
            collided = true;
        } else {
            collided = false;
        }

        // check collision with the walls and recalculate the movement vector
        movement_vector.x = movement_vector.x == FORWARD && player.x + player.w > SCREEN_WIDTH ? STAND : movement_vector.x;
        movement_vector.x = movement_vector.x == BACKWARD && player.x < 0 ? STAND : movement_vector.x;
        movement_vector.y = movement_vector.y == FORWARD && player.y + player.h > SCREEN_HEIGHT ? STAND : movement_vector.y;
        movement_vector.y = movement_vector.y == BACKWARD && player.y < 0 ? STAND : movement_vector.y;

        // moving the player
        player.x += movement_vector.x * current_frame_speed;
        player.y += movement_vector.y * current_frame_speed;

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }
}