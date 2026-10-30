#include <bits/stdc++.h>
#include <random>

#include <SDL2/SDL.h>

#include "vec3.h"


double randomDouble() {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<std::mt19937::result_type> dist6(0, INT_MAX);
    return static_cast<double>(dist6(rng)) / INT_MAX * 2 - 1;
}

int isInCircle(vec3 point) {
    if (sqrt(point[0]*point[0] + point[1]*point[1] + point[2]*point[2]) <= 1) {
        return 1;
    }
    return 0;
}


// rotation around a point
vec3 rotateY(vec3 v, double angle) {
    double c = cos(angle);
    double s = sin(angle);
    return vec3(
        c * v.x() + s * v.z(),
        v.y(),
        -s * v.x() + c * v.z()
    );
}

vec3 rotateX(vec3 v, double angle) {
    double c = cos(angle);
    double s = sin(angle);
    return vec3(
        v.x(),
        c * v.y() - s * v.z(),
        s * v.y() + c * v.z()
    );
}

// camera movement
void input(const Uint8* keys, vec3& camera_center, double& angle_x, double& angle_y, double sensitivity, double speed){
    if(keys[SDL_SCANCODE_LEFT])
        angle_x -= sensitivity;
    if(keys[SDL_SCANCODE_RIGHT])
        angle_x += sensitivity;
    if(keys[SDL_SCANCODE_DOWN])
        angle_y -= sensitivity;
    if(keys[SDL_SCANCODE_UP])
        angle_y += sensitivity;

    double CS = cos(angle_x);
    double SN = sin(angle_x);
    
    if(keys[SDL_SCANCODE_W]){
        camera_center[2] += speed*CS;
        camera_center[0] += speed*SN;
    }
    if(keys[SDL_SCANCODE_S]){
        camera_center[2] -= speed*CS;
        camera_center[0] -= speed*SN;
    }
    if(keys[SDL_SCANCODE_A]){
        camera_center[2] += speed*SN;
        camera_center[0] -= speed*CS;
    }
    if(keys[SDL_SCANCODE_D]){
        camera_center[2] -= speed*SN;
        camera_center[0] += speed*CS;
    }
    if(keys[SDL_SCANCODE_SPACE]){
        camera_center[1] -= speed;
    }
    if(keys[SDL_SCANCODE_LSHIFT]){
        camera_center[1] += speed;
    }
    angle_y = fmod(angle_y, 2*M_PI);
    angle_x = fmod(angle_x, 2*M_PI);
}

int main() {
    int image_width = 1000;
    int image_height = 500;

    SDL_Renderer* renderer = NULL;
        
    SDL_Window* window = NULL;

    uint32_t* framebuffer;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return 1;
    }

    window = SDL_CreateWindow("3d", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, image_width, image_height, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        return 1;
    }

    
    SDL_Event event;

    uint64_t max_display_point_num = 10000;
    uint64_t point_num = 1000000;
    uint64_t hit = 0;
    std::deque<vec3> hit_points;
    std::deque<vec3> miss_points;

    vec3 camera_center = {0, 0, 0};
    double angle_x = 0;
    double angle_y = 0;
    double sensitivity = 0.002;
    double speed = 0.005;

    uint64_t total_samples = 0;

    int running = 1;
    while(running) {
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        input(keys, camera_center, angle_x, angle_y, sensitivity, speed);
        
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT){
                running = 0;
            }
        }
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);



        vec3 point = {
            randomDouble(),
            randomDouble(),
            randomDouble(),
        };

        total_samples++;

        if (isInCircle(point)){
            hit++;
            hit_points.push_back(point);
            if(hit_points.size() > max_display_point_num) {
                hit_points.pop_front();
            }
        } else {
            miss_points.push_back(point);
            if(miss_points.size() > max_display_point_num) {
                miss_points.pop_front();
            }
        }


        for(vec3 hit_point : hit_points) {
            vec3 v = hit_point - camera_center;
            
            v = rotateY(v, -angle_x);
            v = rotateX(v, -angle_y);

            if (v.z() <= 1e-6) continue;

            // Perspective projection
            double fov = 60.0;
            double fov_factor = tan(fov * M_PI / 360.0);
            double aspect_ratio = static_cast<double>(image_width) / image_height;
            
            int screen_x = static_cast<int>((v.x() / (v.z() * fov_factor * aspect_ratio) + 1.0) * image_width / 2.0);
            int screen_y = static_cast<int>((1.0 - v.y() / (v.z() * fov_factor)) * image_height / 2.0);

            if (screen_x < 0 || screen_x >= image_width || screen_y < 0 || screen_y >= image_height)
                continue;
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawPoint(renderer, screen_x, image_height - screen_y);
        
        }
        for(vec3 miss_point : miss_points) {
            vec3 v = miss_point - camera_center;
            
            v = rotateY(v, -angle_x);
            v = rotateX(v, -angle_y);

            if (v.z() <= 1e-6) continue;

            // Perspective projection
            double fov = 60.0;
            double fov_factor = tan(fov * M_PI / 360.0);
            double aspect_ratio = static_cast<double>(image_width) / image_height;
            
            int screen_x = static_cast<int>((v.x() / (v.z() * fov_factor * aspect_ratio) + 1.0) * image_width / 2.0);
            int screen_y = static_cast<int>((1.0 - v.y() / (v.z() * fov_factor)) * image_height / 2.0);

            if (screen_x < 0 || screen_x >= image_width || screen_y < 0 || screen_y >= image_height)
                continue;
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 100);
            SDL_RenderDrawPoint(renderer, screen_x, image_height - screen_y);
        
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderPresent(renderer);
        // SDL_Delay(16);
    }


    double pi = 6.0 * hit / total_samples;
    std::cout << pi << " " << total_samples << '\n';


    return 0;
}