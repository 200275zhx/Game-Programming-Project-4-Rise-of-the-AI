#include "LevelA.h"
#include "Utility.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

constexpr char SPRITESHEET_FILEPATH[] = "assets/george_0.png",
            HELLBOT_FILEPATH[]       = "assets/Sprites/Enemy/Hell Bot DARK/idle 92x36.png",
            BLINDING_SPIDER_FILEPATH[] = "assets/Sprites/Enemy/Blinding Spider/static idle.png",
            FLOWER_ENEMY_FILEPATH[] = "assets/Sprites/Enemy/Flower Enemy/death 32x32.png",
            CHICKBOY_FILEPATH[] = "assets/Sprites/Chick-Boy/ChikBoyFullPack_10x14.png", // 10 * 14 spritesheet
            TILESET_FILEPATH[] = "assets/Tileset/DARK Edition Tileset No background.png"; // 14 * 16 tileset


unsigned int LEVELA_DATA[] =
{
    143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    143, 0, 0, 0, 0, 0, 0, 0, 0, 128, 128, 128, 128, 128,
    142, 128, 128, 128, 128, 128, 128, 128, 128, 142, 142, 142, 142, 142,
    142, 142, 142, 142, 142, 142, 142, 142, 142, 142, 142, 142, 142, 142
};

LevelA::~LevelA()
{
    delete [] m_game_state.enemies;
    delete    m_game_state.player;
    delete    m_game_state.map;
    Mix_FreeChunk(m_game_state.jump_sfx);
    Mix_FreeMusic(m_game_state.bgm);
}

void LevelA::initialise()
{
    m_game_state.next_scene_id = -1;
    m_number_of_enemies = ENEMY_COUNT;
    
    GLuint map_texture_id = Utility::load_texture(TILESET_FILEPATH);
    m_game_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELA_DATA, map_texture_id, 1.0f, 14, 16);
    
    // Code from main.cpp's initialise()
    //ChickBoy//
    GLuint player_texture_id = Utility::load_texture(CHICKBOY_FILEPATH);

    std::vector<std::vector<int>> chickboy_animation =
    {
        { 0, 1, 2, 3, 4, 5 },                                   // idle
        { 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 },             // idle extra
        { 17, 18, 19, 20, 21, 22, 23, 24, 25, 26 },             // run
        { 27, 28 },                                             // jump start
        { 29 },                                                 // jumping
        { 30, 31, 32, 33 },                                     // jump end
        //{ 27, 28, 29, 30, 31, 32, 33 },                         // jump
        { 34 },                                                 // fall start
        { 35 },                                                 // falling
        { 36, 37, 38 },                                         // landing
        //{ 34, 35, 36, 37, 38 },                                 // fall
        { 42, 43 },												// double jump start
        { 44, 45 },												// double jump roll
        { 46, 47 },												// double jump top
        { 48 },													// double jump end
        //{ 42, 43, 44, 45, 46, 47, 48 },                         // double jump
        { 52, 53, 54, 55, 56, 57 },                             // wall grab
        { 58, 59, 60, 61 },                                     // wall slide
        { 62, 63, 64, 65 },                                     // floor slide
        { 66, 67, 68, 69, 70, 71, 72 },                         // dash
        { 4, 73, 74, 75 },                                      // take damage
        { 76, 77, 78, 79, 80 },                                 // attack 1
        { 81, 82, 83, 84, 85, 86 },                             // attack 2
        { 90, 91, 92, 93 },                                     // attack 3
        //{ 87, 88, 89, 90, 91, 92, 93 },                         // attack 3
        { 94, 95, 96, 97 },                                     // ducking
        { 97, 98, 99, 100 },                                    // crouch idle
        { 101, 102, 103, 104, 105, 106, 107, 108, 109, 110 }    // crouch walk
        //{ 0, 1, 111, 112, 113, 114, 115, 116, 116 },            // death 1
        //{ 117, 118, 119, 120, 121, 122, 123, 124, 124 }         // death 2
    };

    glm::vec3 gravity = glm::vec3(0.0f, -30.0f, 0.0f);

    m_game_state.player = new Character(
        player_texture_id,         // texture id
        5.0f,                      // speed
        gravity,                   // acceleration
        9.0f,                      // jumping power
        chickboy_animation,        // animation index sets
        0.0f,                      // animation time
        0,                         // current animation index
        10,                        // animation column amount
        14,                        // animation row amount
        1.0f,                      // width
        1.0f                       // height
    );
        
    m_game_state.player->set_position(glm::vec3(5.0f, 0.0f, 0.0f));
    
    /**
    Enemies' stuff */
    m_game_state.enemies = new Enemy[ENEMY_COUNT];

    GLuint hell_bot_texture_id = Utility::load_texture(HELLBOT_FILEPATH);
    std::vector<std::vector<int>> hell_bot_animation = {
        {0, 1, 2, 3, 4, 5}
    };

    m_game_state.enemies[0] = Enemy(
        hell_bot_texture_id,         // texture id
        2.0f,                      // speed
        gravity,                   // acceleration
        9.0f,                      // jumping power
        hell_bot_animation,        // animation index sets
        0.0f,                      // animation time
        0,                         // current animation index
        1,                        // animation column amount
        6,                        // animation row amount
        1.0f,                      // width
        1.0f,                      // height
        GUARD,
        AI_IDLE,
        2                          // texture size ratio width / height
    );

    GLuint blinding_spider_texture_id = Utility::load_texture(BLINDING_SPIDER_FILEPATH);
    std::vector<std::vector<int>> blinding_spider_animation = {
        {0}
    };

    m_game_state.enemies[1] = Enemy(
        blinding_spider_texture_id,         // texture id
        3.0f,                      // speed
        gravity,                   // acceleration
        9.0f,                      // jumping power
        blinding_spider_animation,        // animation index sets
        0.0f,                      // animation time
        0,                         // current animation index
        1,                        // animation column amount
        1,                        // animation row amount
        1.0f,                      // width
        1.0f,                      // height
        WALKER,
        AI_IDLE,
        1.0f                          // texture size ratio width / height
    );

    GLuint flower_enemy_texture_id = Utility::load_texture(FLOWER_ENEMY_FILEPATH);
    std::vector<std::vector<int>> flower_enemy_animation = {
        {3, 2, 1, 0, 1, 2}
    };

    m_game_state.enemies[2] = Enemy(
        flower_enemy_texture_id,         // texture id
        3.0f,                      // speed
        gravity,                   // acceleration
        9.0f,                      // jumping power
        flower_enemy_animation,        // animation index sets
        0.0f,                      // animation time
        0,                         // current animation index
        1,                        // animation column amount
        4,                        // animation row amount
        1.0f,                      // width
        1.0f,                      // height
        JUMPER,
        AI_IDLE,
        1.0f                          // texture size ratio width / height
    );
    //for (int i = 0; i < ENEMY_COUNT; i++)
    //{
    //m_game_state.enemies[i] =  Entity(enemy_texture_id, 1.0f, 1.0f, 1.0f, ENEMY, GUARD, AI_IDLE);
    //}


    m_game_state.enemies[0].set_position(glm::vec3(8.0f, -5.0f, 0.0f));
    m_game_state.enemies[0].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    m_game_state.enemies[1].set_position(glm::vec3(3.0f, -5.0f, 0.0f));
    m_game_state.enemies[1].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    m_game_state.enemies[2].set_position(glm::vec3(12.0f, -4.0f, 0.0f));
    m_game_state.enemies[2].set_acceleration(glm::vec3(0.0f, 0.0f, 0.0f));
    
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_game_state.bgm = Mix_LoadMUS("assets/dooblydoo.mp3");
    Mix_PlayMusic(m_game_state.bgm, -1);
    Mix_VolumeMusic(0.0f);
    
    m_game_state.jump_sfx = Mix_LoadWAV("assets/bounce.wav");
}

void LevelA::update(float delta_time)
{
    m_game_state.player->update(delta_time, m_game_state.player, m_game_state.enemies, ENEMY_COUNT, m_game_state.map);

    for (int i = 0; i < ENEMY_COUNT; i++) {
        m_game_state.enemies[i].update(delta_time,
            m_game_state.player,
            NULL,
            0,
            m_game_state.map);
        //m_game_state.enemies[0].set_position(glm::vec3(8.0f, 0.0f, 0.0f));
        //std::cout << m_game_state.enemies[i].get_position() << std::endl;
    }
    //std::cout << "collidable entity position x: " << m_game_state.enemies[1].get_position().x << std::endl;
    static_cast<Character*>(m_game_state.player)->on_hit(m_game_state.enemies, ENEMY_COUNT, 10.0f, delta_time, 0.1f, 0.5f);

    if (m_game_state.player->get_position().y < -10.0f) m_game_state.next_scene_id = 1; // need change
    //std::cout << "player position: (" << m_game_state.player->get_position().x << ", " << m_game_state.player->get_position().y << ")\n";
}

void LevelA::render(ShaderProgram *program)
{
    m_game_state.map->render(program);
    m_game_state.player->render(program);
    for (int i = 0; i < ENEMY_COUNT; i++) {
        //std::cout << m_game_state.enemies[i].get_scale() << std::endl;
        m_game_state.enemies[i].render(program);
    }
}
