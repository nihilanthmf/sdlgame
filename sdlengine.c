#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 800

#define MAP_SIZE 20
#define FOV (M_PI / 3.0)
#define TILE_SIZE 48

const int map[MAP_SIZE][MAP_SIZE] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1},
    {1,0,1,1,1,0,0,1,0,1,1,1,0,1,0,0,1,1,0,1},
    {1,0,1,0,0,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1},
    {1,0,1,0,1,1,1,1,1,1,0,1,0,1,1,1,0,1,0,1},
    {1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1},
    {1,1,1,0,1,0,1,1,0,1,1,1,1,1,0,1,0,1,1,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1},
    {1,0,1,1,1,0,1,0,1,1,1,1,0,1,0,1,1,1,0,1},
    {1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,1},
    {1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1},
    {1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1},
    {1,1,1,1,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1},
    {1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
    {1,0,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,0,1,1},
    {1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

struct Enemy {float x; float y; int width; bool hit; float distance;};
typedef struct Enemy Enemy;

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

void create_vertical_line(int screen[], int x, int height, int r, int g, int b) {
    for (int h = 2 * height; h > 0; --h) {
        int screen_pixel_index = (SCREEN_HEIGHT / 2 - height + h) * SCREEN_WIDTH + x;
        if (screen_pixel_index >= 0 && screen_pixel_index < SCREEN_WIDTH * SCREEN_HEIGHT) {
            // converting from rgb to hex color representation
            int color = r * (0xFF / 255) * 16*16*16*16 + g * (0xFF / 255) * 16*16 + b * (0xFF / 255);
            screen[screen_pixel_index] = color;
        }
    }
}

void create_minimap(float player_x, float player_y, Enemy enemies[], int num_of_enemies, int screen[]) {
    const int minimap_cell_size = 5;
    const int amount_of_pixels_for_player = 3;
    int minimap_offset_x = SCREEN_WIDTH - MAP_SIZE * minimap_cell_size - 16;
    int minimap_offset_y = SCREEN_HEIGHT - MAP_SIZE * minimap_cell_size - 16;
    for (int y = 0; y < MAP_SIZE; ++y) {
        for (int x = 0; x < MAP_SIZE; ++x) {
            for (int h = 0; h < minimap_cell_size; ++h) {
                for (int w = 0; w < minimap_cell_size; ++w) {
                    int color;
                    // rendeing the walls and space
                    if (map[y][x] == 1) {
                        color = 0x000099;
                    } else {
                        color = 0x000000;
                    }

                    // rendering the player and enemies
                    if ((int)((player_x / TILE_SIZE) * minimap_cell_size)/amount_of_pixels_for_player == (x*minimap_cell_size+w)/amount_of_pixels_for_player && 
                        (int)((player_y / TILE_SIZE) * minimap_cell_size)/amount_of_pixels_for_player == (y*minimap_cell_size+h)/amount_of_pixels_for_player) {
                        color = 0x00FF00;
                    } else {
                        for (int e = 0; e < num_of_enemies; ++e) {
                            Enemy *enemy = &enemies[e];
                            if ((int)((enemy->x / TILE_SIZE) * minimap_cell_size)/amount_of_pixels_for_player == (x*minimap_cell_size+w)/amount_of_pixels_for_player && 
                                (int)((enemy->y / TILE_SIZE) * minimap_cell_size)/amount_of_pixels_for_player == (y*minimap_cell_size+h)/amount_of_pixels_for_player) {
                                color = 0xFF0000;
                            }
                        }
                    }

                    screen[(minimap_offset_y+y*minimap_cell_size+h)*SCREEN_WIDTH+minimap_offset_x+x*minimap_cell_size+w] = color;
                }
            }
        }
    }
}

int main() {
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool running = true;
    SDL_Event event;

    int screen[SCREEN_WIDTH * SCREEN_HEIGHT];

    const float speed = 0.15;
    const float rotation_speed = 0.0035;

    const int wall_color = 200;
    const float wall_height_percentage = 0.55;
    const float rendering_distance_percentage = 1.15; // 1 is 255 pixels (comes from rgb), 2 is 255*2, etc.

    long previous_frame_time = get_current_time_in_ms();

    float player_angle = 0;
    float player_x = TILE_SIZE + TILE_SIZE / 2;
    float player_y = TILE_SIZE + TILE_SIZE / 2;

    const int num_of_enemies = 2;
    Enemy enemies[num_of_enemies] = {
        {5 * TILE_SIZE, 1.5 * TILE_SIZE, 20, false, 0},
        {3 * TILE_SIZE, 1.5 * TILE_SIZE, 10, false, 0}
    };

    create_window(&window, &renderer);

    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH, SCREEN_HEIGHT
    );

    while (running) {
        // calculating the delta time (time in ms between frames) to make the movement not be framerate dependent
        long current_frame_time = get_current_time_in_ms();
        int delta_time = current_frame_time - previous_frame_time;
        previous_frame_time = current_frame_time;

        // quit the app if user closes the window
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }

        // cleaning the screen
        memset(screen, 0xFF000000, sizeof(screen));

        // rendering
        for (int pixel = 0; pixel < SCREEN_WIDTH; ++pixel) {
            bool hit_wall = false;
            int wall_distance = 0;
            float ray_angle = (player_angle - FOV/2.0) + ((float)pixel / SCREEN_WIDTH) * FOV;

            for (int e = 0; e < num_of_enemies; ++e) {
                enemies[e].hit = false;
                enemies[e].distance = 0;
            }

            while (!hit_wall && wall_distance < 1000) {
                wall_distance++;
                int y = player_y + sin(ray_angle) * wall_distance;
                int x = player_x + cos(ray_angle) * wall_distance;
                
                if (x / TILE_SIZE > MAP_SIZE || y / TILE_SIZE > MAP_SIZE) {
                    break;
                }

                int current_cell = map[y / TILE_SIZE][x / TILE_SIZE];
                if (current_cell == 1) {
                    hit_wall = true;
                }

                for (int e = 0; e < num_of_enemies; ++e) {
                    Enemy *enemy = &enemies[e];
                    if (x > enemy->x - enemy->width / 2 && x < enemy->x + enemy->width / 2 &&
                        y > enemy->y - enemy->width / 2 && y < enemy->y + enemy->width / 2 &&
                        !enemy->hit
                    ) {
                        enemy->hit = true;
                        enemy->distance = wall_distance;
                    }
                }
            }

            int height = wall_height_percentage * (SCREEN_HEIGHT * TILE_SIZE / wall_distance);
            // the farther the wall the darker it is
            int color = wall_distance >= rendering_distance_percentage * wall_color ? 0 : wall_color - wall_distance / rendering_distance_percentage;
            create_vertical_line(screen, pixel, height, color, color, color);
            
            for (int e = 0; e < num_of_enemies; ++e) {
                Enemy *enemy = &enemies[e];
                if (enemy->hit && enemy->distance) {
                    create_vertical_line(screen, pixel, 250*TILE_SIZE / enemy->distance, 255, 0, 0);
                }
            }
        }

        // moving the player
        const Uint8 *keys = SDL_GetKeyboardState(NULL);

        if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_S]) {
            int direction = keys[SDL_SCANCODE_W] ? 1 : -1;
            float updated_x = player_x + direction * cos(player_angle) * speed * delta_time;
            float updated_y = player_y + direction * sin(player_angle) * speed * delta_time;

            if (map[(int)(updated_y) / TILE_SIZE][(int)(updated_x) / TILE_SIZE] != 1) {
                player_x = updated_x;
                player_y = updated_y;
            }
        }
        if (keys[SDL_SCANCODE_A]) player_angle -= rotation_speed * delta_time;
        if (keys[SDL_SCANCODE_D]) player_angle += rotation_speed * delta_time;

        create_minimap(player_x, player_y, enemies, num_of_enemies, screen);

        // render the current frame
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
        // SDL_Delay(16); // ~60 FPS
    }
}