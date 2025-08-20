#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define MAP_SIZE 8
#define FOV (M_PI / 3.0)

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

void render_vertical_line(int screen[], int x, int height) {
    //SCREEN_HEIGHT / 2 - height
    for (int h = height; h > 0; --h) {
        screen[(SCREEN_HEIGHT / 2 - h) * SCREEN_WIDTH + x] = 0xFFFFFFFF;
    }
}

int main() {
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool running = true;
    SDL_Event event;

    int screen[SCREEN_WIDTH * SCREEN_HEIGHT];
    memset(screen, 0xFF000000, sizeof(screen));

    const int map[MAP_SIZE][MAP_SIZE] = {
        {1,1,1,1,1,1,1,1},
        {1,0,0,0,0,2,0,1},
        {1,0,1,0,1,0,0,1},
        {1,0,1,0,1,0,0,1},
        {1,0,0,0,0,0,0,1},
        {1,0,1,0,1,0,0,1},
        {1,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1},
    };
    const int speed = 3;
    const float rotation_speed = 0.05;

    const int wall_color = 200;
    const float wall_height_percentage = 0.55;
    const int tile_size = 96;
    const float rendering_distance_percentage = 1.15; // 1 is 255 pixels (comes from rgb), 2 is 255*2, etc.
    const int enemy_width = 64;

    long previous_frame_time = get_current_time_in_ms();

    float player_angle = 0;
    float player_x = tile_size + tile_size / 2;
    float player_y = tile_size + tile_size / 2;

    create_window(&window, &renderer);

    while (running) {
        // calculating the delta time (time in ms between frames) to make the movement not be framerate dependent
        long current_frame_time = get_current_time_in_ms();
        int delta_time = current_frame_time - previous_frame_time;
        previous_frame_time = current_frame_time;
        int current_frame_speed = delta_time * speed;

        // quit the app if user closes the window
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        // cleaning the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // rendering
        bool enemy_rendered = false;
        for (int pixel = 0; pixel < SCREEN_WIDTH; ++pixel) {
            bool hit_wall = false;
            bool hit_enemy = false;
            int wall_distance = 0;
            float ray_angle = (player_angle - FOV/2.0) + ((float)pixel / SCREEN_WIDTH) * FOV;

            while (!hit_wall && wall_distance < 1000) {
                wall_distance++;
                int y = (player_y + sin(ray_angle) * wall_distance) / tile_size;
                int x = (player_x + cos(ray_angle) * wall_distance) / tile_size;

                int current_cell = map[y][x];
                if (current_cell == 1) {
                    hit_wall = true;
                }
                if (current_cell == 2) {
                    hit_wall = true;
                    hit_enemy = true;
                }
            }

            int height = wall_height_percentage * (SCREEN_HEIGHT * tile_size / wall_distance);// + 0.05 * (SCREEN_WIDTH - pixel);
            // int draw_start = SCREEN_HEIGHT / 2 - height;
            // int draw_end = SCREEN_HEIGHT / 2 + height;

            int color = wall_distance >= rendering_distance_percentage * wall_color ? 0 : wall_color - wall_distance / rendering_distance_percentage; // the farther the wall the darker it is
            // SDL_SetRenderDrawColor(renderer, color, color, color, 255);
            // SDL_RenderDrawLine(renderer, pixel, draw_start, pixel, draw_end);

            render_vertical_line(screen, pixel, height);

            // if (hit_enemy && !enemy_rendered) {
            //     SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            //     SDL_RenderDrawLine(renderer, pixel, draw_start, pixel, draw_end);
            //     enemy_rendered = true;
            // }
        }

        // moving the player
        const Uint8 *keys = SDL_GetKeyboardState(NULL);

        if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_S]) {
            int direction = keys[SDL_SCANCODE_W] ? 1 : -1;
            int updated_x = player_x + direction * cos(player_angle) * speed;
            int updated_y = player_y + direction * sin(player_angle) * speed;

            if (map[updated_y / tile_size][updated_x / tile_size] != 1) {
                player_x = updated_x;
                player_y = updated_y;
            }
        }
        if (keys[SDL_SCANCODE_A]) player_angle -= rotation_speed;
        if (keys[SDL_SCANCODE_D]) player_angle += rotation_speed;

        // rendering minimap
        // for (int y = 0; y < MAP_SIZE; ++y) {
        //     for (int x = 0; x < MAP_SIZE; ++x) {

        //     }
        // }

        SDL_Texture *texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            SCREEN_WIDTH, SCREEN_HEIGHT
        );

        SDL_UpdateTexture(texture, NULL, screen, SCREEN_WIDTH * sizeof(int));

        SDL_RenderCopyEx(
            renderer,
            texture,
            NULL,
            NULL,
            0.0,
            NULL,
            SDL_FLIP_VERTICAL
        );

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }
}