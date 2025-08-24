#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 800

#define MAP_SIZE 20
#define FOV (M_PI / 3.0)
#define TILE_SIZE 24

const int map[MAP_SIZE][MAP_SIZE] = {
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2},
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
    {1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, 
    {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2}
};

struct Enemy {float x; float y; int width; bool hit_per_ray; bool shot_per_frame; float distance; int health;};
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

void create_vertical_line(int screen[], int x, int bottom_height, int top_height, int r, int g, int b) {
    for (int h = bottom_height + top_height; h > 0; --h) {
        // int screen_pixel_index = (SCREEN_HEIGHT / 2 - height + h) * SCREEN_WIDTH + x;
        int screen_pixel_index = (SCREEN_HEIGHT / 2 - bottom_height + h) * SCREEN_WIDTH + x;
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
                    if (map[y][x]) {
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
                                (int)((enemy->y / TILE_SIZE) * minimap_cell_size)/amount_of_pixels_for_player == (y*minimap_cell_size+h)/amount_of_pixels_for_player &&
                                enemy->health) {
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

void draw_weapon(int screen[], int tilt_x, int tilt_y, bool shot, int gun_offset_y) {
    SDL_Surface *gun_sprite = SDL_LoadBMP(shot ? "./art/gun_shot.bmp" : "./art/gun.bmp");
    int *pixels = (int*)gun_sprite->pixels;
    int w = gun_sprite->w;
    int h = gun_sprite->h;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int screen_x = (SCREEN_WIDTH/2)-(w/2) + x + tilt_x;
            int pixel = pixels[(h-y-1)*h+x];
            if (pixel) {
                screen[(y+tilt_y-gun_offset_y)*SCREEN_WIDTH + screen_x] = pixel;
            }
        }
    }
}

void handle_player_movement(const Uint8 *keys, float *player_x, float *player_y, float *player_angle, const float speed, const float rotation_speed, int *rotation_direction, int *direction) {
    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_S]) {
        // moving the player forward & backward
        *direction = keys[SDL_SCANCODE_W] ? 1 : -1;
        float updated_x = *player_x + *direction * cos(*player_angle) * speed;
        float updated_y = *player_y + *direction * sin(*player_angle) * speed;

        // checking wall collision
        if (!map[(int)(updated_y) / TILE_SIZE][(int)(updated_x) / TILE_SIZE]) {
            *player_x = updated_x;
            *player_y = updated_y;
        }
    }
    // rotating the player
    if (keys[SDL_SCANCODE_A]) *rotation_direction = -1;
    if (keys[SDL_SCANCODE_D]) *rotation_direction = 1;
    *player_angle += *rotation_direction * rotation_speed;
}

void handle_shooting(const Uint8 *keys, long current_frame_time, long *last_shot_timestep, int gun_firing_delta_ms, bool *shot) {
    if (keys[SDL_SCANCODE_SPACE] && current_frame_time - *last_shot_timestep > gun_firing_delta_ms) {
        *shot = true;
        *last_shot_timestep = current_frame_time;
    } else {
        *shot = false;
    }
}

void detect_enemies(int x, int y, int num_of_enemies, Enemy enemies[], int wall_distance, bool shot, float shooting_angle) {
    for (int e = 0; e < num_of_enemies; ++e) {
        Enemy *enemy = &enemies[e];
        if (x > enemy->x - enemy->width / 2 && x < enemy->x + enemy->width / 2 &&
            y > enemy->y - enemy->width / 2 && y < enemy->y + enemy->width / 2 &&
            !enemy->hit_per_ray &&
            enemy->health
        ) {
            enemy->hit_per_ray = true;
            enemy->distance = wall_distance;

            // checking the shot
            if (shot && shooting_angle < .1 && !(enemy->shot_per_frame)) {
                (enemy->health)--;
                enemy->shot_per_frame = true;
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

    const float speed = 0.1;
    const float rotation_speed = 0.0035;
    const float rendering_step = 1;

    const int wall_color = 200;
    const float wall_height_percentage = 0.55;
    const float rendering_distance_percentage = 2; // 1 is 255 pixels (comes from rgb), 2 is 255*2, etc.

    const int max_gun_tilt_y = 15;
    const int max_gun_tilt_x = 25;
    const int gun_shaking_time_ms = 200;
    const int gun_firing_delta_ms = 200;

    long previous_frame_time = get_current_time_in_ms();

    float player_angle = 0;
    float player_x = TILE_SIZE + TILE_SIZE / 2;
    float player_y = TILE_SIZE + TILE_SIZE / 2;

    long last_shot_timestep = 0;
    long gun_animation_last_timestep = 0;

    int gun_tilt_y = 0;

    const int num_of_enemies = 1;
    Enemy enemies[num_of_enemies] = {
        {10 * TILE_SIZE, 1.5 * TILE_SIZE, 10, false, false, 0, 10}
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
        int fps = 1000 / delta_time;
        // printf("FPS %d\n", fps);
        
        bool shot = false;
        
        int rotation_direction = 0;
        int direction = 0;

        // quit the app is user closes the window
        SDL_Event e;
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }

        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        handle_player_movement(keys, &player_x, &player_y, &player_angle, speed * delta_time, rotation_speed * delta_time, &rotation_direction, &direction);
        handle_shooting(keys, current_frame_time, &last_shot_timestep, gun_firing_delta_ms, &shot);

        // cleaning the screen & drawing the sky and the floor
        int screen_pixels = SCREEN_HEIGHT * SCREEN_WIDTH;
        for (int x = 0; x < screen_pixels; x++) {
            screen[x] = x < screen_pixels / 2 ? 0xFF000000 : 0x003399;
        }

        // rendering
        for (int e = 0; e < num_of_enemies; ++e) {
            enemies[e].shot_per_frame = false;
        }
        for (int pixel = 0; pixel < SCREEN_WIDTH; ++pixel) {
            bool hit_wall = false;
            int wall_distance = 0;
            float ray_angle = (player_angle - FOV/2.0) + ((float)pixel / SCREEN_WIDTH) * FOV;
            int wall_number;

            for (int e = 0; e < num_of_enemies; ++e) {
                enemies[e].hit_per_ray = false;
                enemies[e].distance = 0;
            }

            while (!hit_wall && wall_distance < 1000) {
                wall_distance += rendering_step;
                int y = player_y + sin(ray_angle) * wall_distance;
                int x = player_x + cos(ray_angle) * wall_distance;
                
                if (x / TILE_SIZE > MAP_SIZE || y / TILE_SIZE > MAP_SIZE) {
                    break;
                }

                int current_cell = map[y / TILE_SIZE][x / TILE_SIZE];
                if (current_cell) {
                    hit_wall = true;
                    wall_number = current_cell;
                }

                detect_enemies(x, y, num_of_enemies, enemies, wall_distance, shot, fabsf(player_angle - ray_angle));
            }

            int height = wall_height_percentage * (SCREEN_HEIGHT * TILE_SIZE / wall_distance);
            // the farther the wall the darker it is
            int color = wall_distance >= rendering_distance_percentage * wall_color ? 0 : wall_color - wall_distance / rendering_distance_percentage;
            create_vertical_line(screen, pixel, height, wall_number * height, color, color, color);
            
            for (int e = 0; e < num_of_enemies; ++e) {
                Enemy *enemy = &enemies[e];
                if (enemy->hit_per_ray && enemy->distance) {
                    int enemy_color = enemy->distance >= rendering_distance_percentage * wall_color ? 0 : wall_color - enemy->distance / rendering_distance_percentage;
                    create_vertical_line(screen, pixel, 250*TILE_SIZE / enemy->distance, 250*TILE_SIZE / enemy->distance, enemy_color * (enemy->health/10.0f), 0, 0);
                }
            }
        }

        // shaking the gun a little bit while walking
        if (current_frame_time - gun_animation_last_timestep > gun_shaking_time_ms) {
            gun_tilt_y = abs(direction) * (gun_tilt_y == max_gun_tilt_y ? 0 : max_gun_tilt_y);
            gun_animation_last_timestep = current_frame_time;
        }
        draw_weapon(screen, rotation_direction * -max_gun_tilt_x, gun_tilt_y, shot, max_gun_tilt_y);

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

        SDL_Delay(10);
    }
}