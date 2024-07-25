#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "LevelA.h"
#include "LevelB.h"

enum AppStatus { RUNNING, TERMINATED };

// ––––– CONSTANTS ––––– //
constexpr int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

constexpr float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL = 0;
constexpr GLint TEXTURE_BORDER = 0;

constexpr float PLATFORM_OFFSET = 5.0f;

// ––––– FILE PATH ––––– //

constexpr char  V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
                F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

//constexpr char  SPRITESHEET_FILEPATH[] = "assets/george_0.png",
//                PLATFORM_FILEPATH[] = "assets/platformPack_tile027.png",
//                ENEMY_FILEPATH[] = "assets/soph.png",
//                TILESET_FILEPATH[] = "assets/Tileset/DARK Edition Tileset with background.png", // 14 * 16 tileset
//                CHICKBOY_FILEPATH[] = "assets/Sprites/Chick-Boy/ChikBoyFullPack_10x14.png"; // 10 * 14 spritesheet
//
//constexpr char  BGM_FILEPATH[] = "assets/crypto.mp3",
//                SFX_FILEPATH[] = "assets/bounce.wav";

// ––––– VARIABLES ––––– //
Scene* g_current_scene;
LevelA* g_levelA;
LevelB* g_levelB;

Scene* g_levels[2];

SDL_Window* g_display_window;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

bool g_is_colliding_bottom = false;

AppStatus g_app_status = RUNNING;

void swtich_to_scene(Scene* scene);
void initialise();
void process_input();
void update();
void render();
void shutdown();

//// ––––– GENERAL FUNCTIONS ––––– //
//GLuint load_texture(const char* filepath)
//{
//    int width, height, number_of_components;
//    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
//
//    if (image == NULL)
//    {
//        LOG("Unable to load image. Make sure the path is correct.");
//        assert(false);
//    }
//
//    GLuint textureID;
//    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
//    glBindTexture(GL_TEXTURE_2D, textureID);
//    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
//
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//
//    stbi_image_free(image);
//
//    return textureID;
//}

// ––––– GENERAL FUNCTIONS ––––– //
void switch_to_scene(Scene* scene)
{
    g_current_scene = scene;
    g_current_scene->initialise(); // DON'T FORGET THIS STEP!
}

void initialise()
{
    // ––––– GENERAL STUFF ––––– //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Hello, AI!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    //if (context == nullptr)
    //{
    //    LOG("ERROR: Could not create OpenGL context.\n");
    //    shutdown();
    //}

#ifdef _WINDOWS
    glewInit();
#endif
    // ––––– VIDEO STUFF ––––– //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    //// ––––– PLATFORM ––––– //
    //GLuint platform_texture_id = load_texture(PLATFORM_FILEPATH);

    //g_game_state.platforms = new Entity[PLATFORM_COUNT];

    //for (int i = 0; i < PLATFORM_COUNT; i++)
    //{
    //    g_game_state.platforms[i] = Entity(platform_texture_id, 0.0f, 0.4f, 1.0f, PLATFORM);
    //    g_game_state.platforms[i].set_position(glm::vec3(i - PLATFORM_OFFSET, -3.0f, 0.0f));
    //    g_game_state.platforms[i].update(0.0f, NULL, NULL, 0);
    //}

    ////ChickBoy//
    //GLuint player_texture_id = load_texture(CHICKBOY_FILEPATH);

    //std::vector<std::vector<int>> chickboy_animation =
    //{
    //    { 0, 1, 2, 3, 4, 5 },                                   // idle
    //    { 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 },             // idle extra
    //    { 17, 18, 19, 20, 21, 22, 23, 24, 25, 26 },             // run
    //    { 27, 28 },                                             // jump start
    //    { 29 },                                                 // jumping
    //    { 30, 31, 32, 33 },                                     // jump end
    //    //{ 27, 28, 29, 30, 31, 32, 33 },                         // jump
    //    { 34 },                                                 // fall start
    //    { 35 },                                                 // falling
    //    { 36, 37, 38 },                                         // landing
    //    //{ 34, 35, 36, 37, 38 },                                 // fall
    //    { 42, 43 },												// double jump start
    //    { 44, 45 },												// double jump roll
    //    { 46, 47 },												// double jump top
    //    { 48 },													// double jump end
    //    //{ 42, 43, 44, 45, 46, 47, 48 },                         // double jump
    //    { 52, 53, 54, 55, 56, 57 },                             // wall grab
    //    { 58, 59, 60, 61 },                                     // wall slide
    //    { 62, 63, 64, 65 },                                     // floor slide
    //    { 66, 67, 68, 69, 70, 71, 72 },                         // dash
    //    { 4, 73, 74, 75 },                                      // take damage
    //    { 76, 77, 78, 79, 80 },                                 // attack 1
    //    { 81, 82, 83, 84, 85, 86 },                             // attack 2
    //    { 90, 91, 92, 93 },                                     // attack 3
    //    //{ 87, 88, 89, 90, 91, 92, 93 },                         // attack 3
    //    { 94, 95, 96, 97 },                                     // ducking
    //    { 97, 98, 99, 100 },                                    // crouch idle
    //    { 101, 102, 103, 104, 105, 106, 107, 108, 109, 110 }    // crouch walk
    //    //{ 0, 1, 111, 112, 113, 114, 115, 116, 116 },            // death 1
    //    //{ 117, 118, 119, 120, 121, 122, 123, 124, 124 }         // death 2
    //};

    //glm::vec3 gravity = glm::vec3(0.0f, -30.0f, 0.0f);

    //g_game_state.player = new Character (
    //    player_texture_id,         // texture id
    //    5.0f,                      // speed
    //    gravity,                   // acceleration
    //    9.0f,                      // jumping power
    //    chickboy_animation,        // animation index sets
    //    0.0f,                      // animation time
    //    0,                         // current animation index
    //    10,                        // animation column amount
    //    14,                        // animation row amount
    //    1.0f,                      // width
    //    1.0f                       // height
    //);

    //// ––––– SOPHIE ––––– //
    //GLuint enemy_texture_id = load_texture(ENEMY_FILEPATH);

    //g_game_state.enemies = new Entity[ENEMY_COUNT];

    //for (int i = 0; i < ENEMY_COUNT; i++)
    //{
    //    g_game_state.enemies[i] = Entity(enemy_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, GUARD, AI_IDLE);
    //}


    //g_game_state.enemies[0].set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    //g_game_state.enemies[0].set_movement(glm::vec3(0.0f));
    //g_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));

    //// ––––– AUDIO STUFF ––––– //
    //Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    //g_game_state.bgm = Mix_LoadMUS(BGM_FILEPATH);
    //Mix_PlayMusic(g_game_state.bgm, -1);
    //Mix_VolumeMusic(MIX_MAX_VOLUME / 4.0f);

    //g_game_state.jump_sfx = Mix_LoadWAV(SFX_FILEPATH);

    // ––––– GENERAL STUFF ––––– //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ––––– LEVELS ––––– //

    g_levelA = new LevelA();
    g_levelB = new LevelB();

    g_levels[0] = g_levelA;
    g_levels[1] = g_levelB;

    // Start at level A
    switch_to_scene(g_levels[0]);
}

void process_input()
{
    g_current_scene->get_state().player->set_movement(glm::vec3(0.0f));

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
                // Jump
                if (g_current_scene->get_state().player->get_collided_bottom())
                {
                    g_current_scene->get_state().player->jump();
                    //Mix_PlayChannel(-1, g_game_state.jump_sfx, 0);
                }
                break;

            case SDLK_j:
                g_current_scene->get_state().player->attack();
                break;

            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    CharacterAction current_action = static_cast<Character*>(g_current_scene->get_state().player)->get_current_action();

    if (key_state[SDL_SCANCODE_A]) g_current_scene->get_state().player->move_left();
    else if (key_state[SDL_SCANCODE_D]) g_current_scene->get_state().player->move_right();

    if (glm::length(g_current_scene->get_state().player->get_movement()) > 1.0f)
        g_current_scene->get_state().player->normalise_movement();
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;

    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
        g_current_scene->update(FIXED_TIMESTEP);
        g_shader_program.set_view_matrix(glm::translate(g_view_matrix, -g_current_scene->get_state().player->get_position()));

        /*g_game_state.enemies[0].set_position(glm::vec3(0.0f, 0.0f, 0.0f));*/
        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    g_current_scene->render(&g_shader_program);

    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

    delete g_levelA;
    delete g_levelB;
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