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
#include "Utility.h"

enum AppStatus { RUNNING, TERMINATED };

// ––––– CONSTANTS ––––– //
constexpr int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

constexpr float BG_RED = 0.2f,
BG_BLUE = 0.2f,
BG_GREEN = 0.2f,
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

constexpr char  FONT_SPRITE_FILEPATH[] = "assets/font1.png";
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
GLuint g_text_texture_id;

void swtich_to_scene(Scene* scene);
void initialise();
void process_input();
void update();
void render();
void shutdown();

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

    // ––––– GENERAL STUFF ––––– //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    g_text_texture_id = Utility::load_texture(FONT_SPRITE_FILEPATH);

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
    Character* player = (Character*) g_current_scene->get_state().player;
    player->set_movement(glm::vec3(0.0f));

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
                if (player->get_collided_bottom() && player->movable())
                {
                    player->jump();
                    //Mix_PlayChannel(-1, g_game_state.jump_sfx, 0);
                }
                break;

            case SDLK_j:
                player->attack();
                break;

            case SDLK_LSHIFT:
                if (player->movable() && player->get_is_dashable()) {
                    player->set_animation_index(0);
                    player->dash();
                }
                break;

            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    CharacterAction current_action = player->get_current_action();

    if (key_state[SDL_SCANCODE_A] && player->movable()) player->move_left();
    else if (key_state[SDL_SCANCODE_D] && player->movable()) player->move_right();

    if (glm::length(player->get_movement()) > 1.0f)
        player->normalise_movement();
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

    Character* player = static_cast<Character*>(g_current_scene->get_state().player);
    Enemy* enemies = static_cast<Enemy*>(g_current_scene->get_state().enemies);
    if (!player->get_life()) {
        Utility::draw_text(&g_shader_program, g_text_texture_id, std::string("YOU LOSE!"), 0.25f, 0.0f, player->get_position() + glm::vec3(0.0f, 2.0f, 0.0f));
    }
    int num_of_enem = g_current_scene->get_number_of_enemies();

    bool is_win = true;
    for (int ind{}; ind < num_of_enem; ++ind) {
        if (enemies[ind].get_is_active()) { 
            is_win = false; 
            break;
        }
    }
    if (is_win) Utility::draw_text(&g_shader_program, g_text_texture_id, std::string("YOU WIN!"), 0.25f, 0.01f, player->get_position() + glm::vec3(0.0f, 2.0f, 0.0f));

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