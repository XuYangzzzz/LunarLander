/**
* Author: [Regan Zhu]
* Assignment: Lunar Lander
* Date due: 2024-07-13, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.001f
#define TEACUP_COUNT 4

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"

//// Including the audio library
//#include <SDL_mixer.h>
//
//constexpr int CD_QUAL_FREQ    = 44100,  // compact disk (CD) quality frequency
//          AUDIO_CHAN_AMT  = 2,
//          AUDIO_BUFF_SIZE = 4096;
//
////BGM (.wav files channel 1),
//constexpr char BGM_FILEPATH[] = "music.mp3";
//constexpr int    LOOP_FOREVER = -1;  // -1 means loop forever in Mix_PlayMusic; 0 means play once and loop zero times
//
//Mix_Music *g_music;
//constexpr int PLAY_ONCE   =  0,
//          NEXT_CHNL   = -1,  // next available channel
//          MUTE_VOL    =  0,
//          MILS_IN_SEC = 1000,
//          ALL_SFX_CHN = -1;
//
//// Declare your sfx globally...
//Mix_Chunk *g_bouncing_sfx;


// ––––– STRUCTS AND ENUMS ––––– //
enum AppStatus { RUNNING, TERMINATED };
enum AppMode {PLAYING, WIN, LOSE};
bool g_lose = false;
bool g_true = false;
struct GameState
{
    Entity* rocket;
    Entity* teacups;
    Entity* landpad;
    Entity* fuel;
    Entity* win;
    Entity* lose;
};

// ––––– CONSTANTS ––––– //
constexpr int WINDOW_WIDTH  = 640 * 2,
          WINDOW_HEIGHT = 480 * 2;

constexpr float BG_RED     = 0.796f,
                BG_GREEN   = 0.764f,
                BG_BLUE    = 0.89f,
                BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;
constexpr char rocket_FILEPATH[] = "sprites/rocket.png";
constexpr char TEACUP_FILEPATH[]    = "sprites/bomb.png";
constexpr char landpad_FILEPATH[]    = "sprites/landpad.png";
constexpr char fuel_FILEPATH[]    = "sprites/fuel.png";
constexpr char win_FILEPATH[]    = "sprites/win.png";
constexpr char lose_FILEPATH[]    = "sprites/lose.png";

float fuelleft = 500.0f;

constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL  = 0;
constexpr GLint TEXTURE_BORDER   = 0;


// ––––– GLOBAL VARIABLES ––––– //
GameState g_game_state;

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
AppMode g_app_mode = PLAYING;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix, g_fuel_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

// ———— GENERAL FUNCTIONS ———— //
GLuint load_texture(const char* filepath);

void initialise();
void process_input();
void update();
void render();
void shutdown();

// ––––– GENERAL FUNCTIONS ––––– //
GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    stbi_image_free(image);
    
    return textureID;
}

void initialise()
{
//    // Initialising both the video AND audio subsystems
//    // We did something similar when we talked about video game controllers
//    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
//
//    // Start Audio
//    Mix_OpenAudio(
//        CD_QUAL_FREQ,        // the frequency to playback audio at (in Hz)
//        MIX_DEFAULT_FORMAT,  // audio format
//        AUDIO_CHAN_AMT,      // number of channels (1 is mono, 2 is stereo, etc).
//        AUDIO_BUFF_SIZE      // audio buffer size in sample FRAMES (total samples divided by channel count)
//        );
//    // Music initialisation above...
//
//        // Similar to our custom function load_texture
//        g_music = Mix_LoadMUS(BGM_FILEPATH);
//
//        // This will schedule the music object to begin mixing for playback.
//        // The first parameter is the pointer to the mp3 we loaded
//        // and second parameter is the number of times to loop.
//        Mix_PlayMusic(g_music, LOOP_FOREVER);
//
//        // Set the music to half volume
//        // MIX_MAX_VOLUME is a pre-defined constant
//        Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
//
//        // Stop the music
//        // This probably wouldn't go in initialise
//        // Mix_HaltMusic();
//    
//    
//    g_bouncing_sfx = Mix_LoadWAV("bounce.wav");
//
//    Mix_PlayChannel(
//        NEXT_CHNL,       // using the first channel that is not currently in use...
//        g_bouncing_sfx,  // ...play this chunk of audio...
//        PLAY_ONCE        // ...once.
//        );
//
//    // Fade in (from 0 to full volume) over 1 second
//    Mix_FadeInChannel(
//        NEXT_CHNL,       // using the first channel that is not currently in use...
//        g_bouncing_sfx,  // ...fade in this chunk of audio from volume 0 to max volume...
//        PLAY_ONCE,       // ...once...
//        MILS_IN_SEC      // ...over 1000 miliseconds.
//        );
//
//    Mix_Volume(
//        ALL_SFX_CHN,        // Set all channels...
//        MIX_MAX_VOLUME / 2  // ...to half volume.
//        );
//
//    Mix_VolumeChunk(
//        g_bouncing_sfx,     // Set the volume of the bounce sound...
//        MIX_MAX_VOLUME / 4  // ... to 1/4th.
//        );
//
//    // Stop all sound effects
//    // Mix_HaltChannel(ALL_SFX_CHN);
    
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Spaceship!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (context == nullptr)
    {
        LOG("ERROR: Could not create OpenGL context.\n");
        shutdown();
    }

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // ––––– TEACUP ––––– //
    g_game_state.teacups = new Entity[TEACUP_COUNT];
    GLuint teacup_texture_id = load_texture(TEACUP_FILEPATH);
    for (int i = 0; i < TEACUP_COUNT; i++)
        g_game_state.teacups[i] = Entity(
            teacup_texture_id, // texture id
            0.0f,              // speed
            1.0f,              // width
            1.0f               // height
        );
    
    g_game_state.teacups[0].set_position(glm::vec3(2.5f, 2.5f, 0.0f));
    g_game_state.teacups[0].update(0.0f, NULL, 0);
    g_game_state.teacups[0].set_scale(glm::vec3(1.0f, 1.0f, 0.0f));
    
    g_game_state.teacups[1].set_position(glm::vec3(-2.5f, -2.5f, 0.0f));
    g_game_state.teacups[1].update(0.0f, NULL, 0);
    g_game_state.teacups[1].set_scale(glm::vec3(1.0f, 1.0f, 0.0f));
    
    g_game_state.teacups[2].set_position(glm::vec3(-2.5f, 2.5f, 0.0f));
    g_game_state.teacups[2].update(0.0f, NULL, 0);
    g_game_state.teacups[2].set_scale(glm::vec3(1.0f, 1.0f, 0.0f));
    
    g_game_state.teacups[3].set_position(glm::vec3(2.5f, -2.5f, 0.0f));
    g_game_state.teacups[3].update(0.0f, NULL, 0);
    g_game_state.teacups[3].set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

    
    GLuint landpad_texture_id = load_texture(landpad_FILEPATH);
    g_game_state.landpad = new Entity(
                                      landpad_texture_id, 0.0f, 2.5f, 1.8f       );
    g_game_state.landpad->set_position(glm::vec3(0.0f, -2.4f, 0.0f));
    g_game_state.landpad->update(0.0f, NULL, 0);
    g_game_state.landpad->set_scale(glm::vec3(2.5f, 1.8f, 0.0f));

    GLuint g_fuel_texture_id =load_texture(fuel_FILEPATH);
    g_game_state.fuel = new Entity(
                                      g_fuel_texture_id, 0.0f, 1.0f, 1.0f       );
    g_game_state.fuel->set_position(glm::vec3(0.0f, 3.5f, 0.0f));
    g_game_state.fuel->update(0.0f, NULL, 0);
    g_game_state.fuel->set_scale(glm::vec3(fuelleft*0.01, 0.2f, 0.0f));

    GLuint g_win_texture_id =load_texture(win_FILEPATH);
    g_game_state.win = new Entity(
                                  g_win_texture_id, 0.0f, 1.0f, 1.0f       );
    g_game_state.win->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    g_game_state.win->update(0.0f, NULL, 0);
    g_game_state.win->set_scale(glm::vec3(5.0f, 2.0f, 0.0f));

    GLuint g_lose_texture_id =load_texture(lose_FILEPATH);
    g_game_state.lose = new Entity(
                                  g_lose_texture_id, 0.0f, 1.0f, 1.0f       );
    g_game_state.lose->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    g_game_state.lose->update(0.0f, NULL, 0);
    g_game_state.lose->set_scale(glm::vec3(5.0f, 2.0f, 0.0f));
    // ––––– rocket ––––– //
    GLuint rocket_texture_id = load_texture(rocket_FILEPATH);

    g_game_state.rocket = new Entity(
        rocket_texture_id,         // texture id
        1.0f,                      // speed
        1.0f,                      // width
        1.0f                       // height
    );
    
    g_game_state.rocket->set_position(glm::vec3(0.0f, 2.5f, 0.0f));
    g_game_state.rocket->set_scale(glm::vec3(1.0f, 1.0f, 0.0f));

    
    // ––––– GENERAL ––––– //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_game_state.rocket->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_app_status = TERMINATED;
                        break;
                        
                    case SDLK_SPACE:
                        /** PART I: Let go of the rocket */
                        g_game_state.rocket->set_acceleration(glm::vec3(0.0f, -0.1f, 0.0f));
                        break;
                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    if (fuelleft > 0 ){
        if (key_state[SDL_SCANCODE_LEFT]){
            glm::vec3 acc =g_game_state.rocket->get_acceleration();
            acc.x -= 10.5f;
            g_game_state.rocket->set_acceleration(acc);
            g_game_state.rocket->rotate(12.0f);
            fuelleft -= 1.0f;
        }
        else if (key_state[SDL_SCANCODE_RIGHT]){
            glm::vec3 acc =g_game_state.rocket->get_acceleration();
            acc.x += 10.5f;
            g_game_state.rocket->set_acceleration(acc);
            g_game_state.rocket->rotate(-12.0f);
            fuelleft -= 1.0f;
        }
        else if (key_state[SDL_SCANCODE_UP]){
            glm::vec3 acc =g_game_state.rocket->get_acceleration();
            acc.y+= 0.8f;
            g_game_state.rocket->set_acceleration(acc);
            fuelleft -= 1.0f;
        }
        else if (key_state[SDL_SCANCODE_DOWN]){
            glm::vec3 acc =g_game_state.rocket->get_acceleration();
            acc.y-= 0.5f;
            g_game_state.rocket->set_acceleration(acc);
            fuelleft -= 1.0f;
        }
    }
    if (glm::length(g_game_state.rocket->get_movement()) > 1.0f)
            g_game_state.rocket->normalise_movement();
}

void update()
{
    if(g_app_mode==PLAYING){
        float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
        float delta_time = ticks - g_previous_ticks;
        g_previous_ticks = ticks;
        
        delta_time += g_accumulator;
        
        if (delta_time < FIXED_TIMESTEP)
        {
            g_accumulator = delta_time;
            return;
        }
        if(g_game_state.landpad ->check_collision(g_game_state.rocket)){
            g_app_mode = WIN;
        }
        for (int i = 0; i < TEACUP_COUNT; i++)
            if(g_game_state.teacups[i].check_collision(g_game_state.rocket)){
                g_app_mode = LOSE;
            }
        if (g_game_state.rocket->get_position().y > 3.5f || g_game_state.rocket->get_position().y < -3.5f){
                        g_app_mode = LOSE;
                    }
        if (g_game_state.rocket->get_position().x > 5.0f ){
                        g_app_mode = LOSE;
                    }
        if (g_game_state.rocket->get_position().x < -5.0f ){
                        g_app_mode = LOSE;
                    }
    //    if(g_true){
    //        glm::vec3 sl = g_game_state.rocket -> get_scale();
    //        sl = sl*0.9f;
    //        g_game_state.rocket-> set_scale(sl);
    //    }

        glm::vec3 acc = g_game_state.rocket->get_acceleration();
        if(acc.y>0.0f){
            acc.y-=0.3f;
            g_game_state.rocket->set_acceleration(acc);
        }
        if(acc.y<-0.8f){ //gravity
            acc.y+=0.3f;
            g_game_state.rocket->set_acceleration(acc);
        }
        if(acc.x<0.0f){
            acc.x+=0.3f;
            g_game_state.rocket->set_acceleration(acc);
        }
        if(acc.x>0.0f){
            acc.x-=0.3f;
            g_game_state.rocket->set_acceleration(acc);
        }
        
        //max speeds
        if(acc.y>1.0f){
            acc.y=1.0f;
            g_game_state.rocket->set_acceleration(acc);
        }
        g_game_state.fuel->set_scale(glm::vec3(fuelleft*0.01, 0.2f, 0.0f));

    ////    if(acc.y<-1.0f){ //gravity
    ////        acc.y=1.0f;
    ////        g_game_state.rocket->set_acceleration(acc);
    ////    }
    //    if(acc.x<-2.0f){
    //        acc.x=-2.0f;
    //        g_game_state.rocket->set_acceleration(acc);
    //    }
    //    if(acc.x>2.0f){ //gravity
    //        acc.x=2.0f;
    //        g_game_state.rocket->set_acceleration(acc);
    //    }
        g_game_state.rocket->rotate(0.0f);
        while (delta_time >= FIXED_TIMESTEP)
        {
            g_game_state.rocket->update(FIXED_TIMESTEP, g_game_state.teacups, TEACUP_COUNT);
            g_game_state.rocket->update(FIXED_TIMESTEP, g_game_state.landpad, 1);
            g_game_state.teacups[0].update(FIXED_TIMESTEP, g_game_state.rocket, 1);
            g_game_state.teacups[1].update(FIXED_TIMESTEP, g_game_state.rocket, 1);
            g_game_state.teacups[2].update(FIXED_TIMESTEP, g_game_state.rocket, 1);
            g_game_state.teacups[3].update(FIXED_TIMESTEP, g_game_state.rocket, 1);
            g_game_state.landpad->update(FIXED_TIMESTEP, g_game_state.rocket, 1);
            g_game_state.fuel->update(0.0f,NULL,0.0f);
            delta_time -= FIXED_TIMESTEP;
        }
        g_accumulator = delta_time;
    }
//    if(g_app_mode == WIN || g_app_mode==LOSE) {
//        return; 
//    }

}


void draw_object(glm::mat4 &object_g_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so use 6, not 3
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    // Vertices
            float vertices[] =
            {
                -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
                -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
            };

            // Textures
            float texture_coordinates[] =
            {
                0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
                0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
            };
            
            glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false,
                                  0, vertices);
            glEnableVertexAttribArray(g_shader_program.get_position_attribute());
            
            glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT,
                                  false, 0, texture_coordinates);
            glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
            
    if( g_app_mode == PLAYING){
        g_game_state.rocket->render(&g_shader_program);
        g_game_state.landpad->render(&g_shader_program);
        g_game_state.fuel->render(&g_shader_program);
        for (int i = 0; i < TEACUP_COUNT; i++)
            g_game_state.teacups[i].render(&g_shader_program);
    }
    else if(g_app_mode == WIN){
        g_game_state.win->render(&g_shader_program);
    }
    else if(g_app_mode == LOSE){
        g_game_state.lose->render(&g_shader_program);
    }
    
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
//    Mix_FreeChunk(g_bouncing_sfx);
//    Mix_FreeMusic(g_music);
    SDL_Quit();
    delete g_game_state.fuel;
    delete [] g_game_state.teacups;
    delete g_game_state.rocket;
    delete g_game_state.landpad;
    delete g_game_state.win;
    delete g_game_state.lose;

}

// ––––– GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
