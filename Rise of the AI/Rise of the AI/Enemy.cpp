#include "Enemy.h"
#include "Character.h"

Enemy::Enemy()
    : Entity(), m_ai_walk_direction(-1.0f)
{
}

Enemy::Enemy(GLuint texture_id, float speed, glm::vec3 acceleration, float jump_power,
    std::vector<std::vector<int>> animation_sequences, float animation_time,
    int animation_index, int animation_cols, int animation_rows, float width, float height,
    AIType AIType, AIState AIState, int texture_width_ratio)

    : Entity(texture_id, speed, acceleration, jump_power, animation_sequences, animation_time, 
        animation_index, animation_cols, animation_rows, width, height, texture_width_ratio),
    m_ai_type(AIType), m_ai_state(AIState), m_ai_walk_direction(-1.0f)
{
    m_animation_indices = m_animation_sequences[0];
}

void Enemy::ai_activate(Entity* player)
{
    CharacterAction player_action = static_cast<Character*>(player)->get_current_action();
    if (check_collision(player) && (player_action == ATTACK_1 || player_action == ATTACK_2 || player_action == ATTACK_3)) {
        deactivate();
        return;
    }

    switch (m_ai_type)
    {
    case WALKER:
        ai_walk();
        break;

    case GUARD:
        ai_guard(player);
        break;

    case JUMPER:
        ai_jump(player);
        break;

    default:
        break;
    }
}

void Enemy::ai_walk()
{
    m_movement = glm::vec3(m_ai_walk_direction, 0.0f, 0.0f);
}

void Enemy::ai_guard(Entity* player)
{
    switch (m_ai_state) {
    case AI_IDLE:
        if (glm::distance(m_position, player->get_position()) < 3.0f) m_ai_state = AI_WALKING;
        break;

    case AI_WALKING:
        if (m_position.x > player->get_position().x) {
            m_movement = glm::vec3(-1.0f, 0.0f, 0.0f);
        }
        else {
            m_movement = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        break;

    case AI_ATTACKING:
        break;

    default:
        break;
    }
}

void Enemy::ai_jump(Entity* player)
{
    switch (m_ai_state) {
    case AI_IDLE:
        if (glm::distance(m_position, player->get_position()) < 3.0f) {
            m_is_ai_jump_attack_begin = true;
            m_ai_state = AI_JUMP_ATTACKING;
        }
        break;

    case AI_JUMP_ATTACKING:
        if (m_is_ai_jump_attack_begin) {
            m_position = player->get_position();
            m_position.y += 3.0f;
            m_is_ai_jump_attack_begin = false;
        }
        if (m_position.y > -3.9f) m_position.y -= 0.1f;
        else { m_ai_state = AI_IDLE; }
        break;

    default:
        break;
    }
}

void Enemy::update(float delta_time, Entity* player, Entity* collidable_entities, int collidable_entity_count, Map* map)
{
    if (!m_is_active) return;
    ai_activate(player);

    m_collided_top = false;
    m_collided_bottom = false;
    m_collided_left = false;
    m_collided_right = false;

    // --------- UPDATE PHYSICS ---------- //

    m_velocity.y = 0.0f;
    m_velocity.x = m_movement.x * m_speed;
    //std::cout << "Before velocity update: " << m_velocity.y << std::endl;
    m_velocity += m_acceleration * delta_time;
    //std::cout << "After velocity update: " << m_velocity.y << std::endl;

    //std::cout << "enemy acc.y: " << m_acceleration.y << std::endl;
    //std::cout << "enemy vel.y: " << m_velocity.y << std::endl;
    //if (m_entity_type == ENEMY) std::cout << "enemy delta_time: " << delta_time << std::endl;
    //std::cout << "enemy pos.y: " << m_velocity.y * delta_time << std::endl;
    //std::cout << "enemy m_pos.x: " << m_position.x << std::endl;
    //std::cout << "enemy m_pos.y: " << m_position.y << std::endl;

    if (m_is_jumping)
    {
        m_is_jumping = false;
        m_velocity.y += m_jumping_power;
    }

    m_position.y += m_velocity.y * delta_time;
    //check_collision_y(collidable_entities, collidable_entity_count);
    check_collision_y(map);
    if (m_collided_bottom) std::cout << "enemy collide bottom\n"; // why this is not working?

    m_position.x += m_velocity.x * delta_time;
    //check_collision_x(collidable_entities, collidable_entity_count);
    check_collision_x(map);

    //if (m_collided_bottom && m_entity_type == ENEMY) std::cout << "enemy collide bottom\n";

    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    // --------- UPDATE AI_WALKER ---------- //

    if (m_collided_left) m_ai_walk_direction = 1.0f;
    if (m_collided_right) m_ai_walk_direction = -1.0f;
    // --------- UPDATE ANIMATION ---------- //

    if (m_velocity.x < 0) { m_direction = -1; }
    if (m_velocity.x > 0) { m_direction = 1; }
    if (m_direction < 0) m_model_matrix = glm::rotate(m_model_matrix, (float)3.1415926f, glm::vec3(0.0f, -1.0f, 0.0f));
    m_model_matrix = glm::scale(m_model_matrix, m_scale);
    
    loop_play(delta_time);
}