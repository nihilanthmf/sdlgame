#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define BACKWARD -1
#define STAND 0 
#define FORWARD 1

#define FOV (M_PI / 3.0)
#define TILE_SIZE 64

struct Vector { int x; int y; };
typedef struct Vector Vector;

int create_window(SDL_Window **window, SDL_Renderer **renderer) {
    if (SDL_Init(0) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    *window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
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

    int map[8][8] = {
        {1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,0,1},
        {1,0,1,0,1,0,0,1},
        {1,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,0,1},
        {1,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1},
    };

    long previous_frame_time = get_current_time_in_ms();

    const int speed = 3;
    const float rotation_speed = 0.05;

    float player_angle = 0;
    float player_x = TILE_SIZE + TILE_SIZE / 2;
    float player_y = TILE_SIZE + TILE_SIZE / 2;

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

        // cleaning the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // rendering
        for (int pixel = 0; pixel < SCREEN_WIDTH; ++pixel) {
            bool hit_wall = false;
            int wall_distance = 0;
            float ray_angle = (player_angle - FOV/2.0) + ((float)pixel / SCREEN_WIDTH) * FOV;//(FOV / 2) - ((float)FOV / SCREEN_WIDTH) * pixel;

            while (!hit_wall && wall_distance < 1000) {
                wall_distance++;
                int y = (player_y + sin(ray_angle) * wall_distance) / TILE_SIZE;
                int x = (player_x + cos(ray_angle) * wall_distance) / TILE_SIZE;

                if (map[y][x] == 1) {
                    hit_wall = true;
                }
            }

            int height = SCREEN_HEIGHT * TILE_SIZE / wall_distance;
            int draw_start = SCREEN_HEIGHT / 2 - height;
            int draw_end = SCREEN_HEIGHT / 2 + height;
            int color = wall_distance >= 255 ? 0 : 255 - wall_distance; // the farther the wall the darker it is
            SDL_SetRenderDrawColor(renderer, color, color, color, 255);
            SDL_RenderDrawLine(renderer, pixel, draw_start, pixel, draw_end);
        }

        // moving the player
        const Uint8 *keys = SDL_GetKeyboardState(NULL);

        if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_S]) {
            int direction = keys[SDL_SCANCODE_W] ? 1 : -1;
            int updated_x = player_x + direction * cos(player_angle) * speed;
            int updated_y = player_y + direction * sin(player_angle) * speed;

            if (!map[updated_y / TILE_SIZE][updated_x / TILE_SIZE]) {
                player_x = updated_x;
                player_y = updated_y;
            }
        }
        if (keys[SDL_SCANCODE_A]) player_angle -= rotation_speed;//player_y -= 1 * speed;
        if (keys[SDL_SCANCODE_D]) player_angle += rotation_speed;//player_y += 1 * speed;

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }
}