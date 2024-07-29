#include "Character.h"
#include <cmath>

Character::Character(GLuint texture_id, float speed, glm::vec3 acceleration, float jump_power,
    std::vector<std::vector<int>> animation_sequences, float animation_time,
    int animation_index, int animation_cols, int animation_rows, float width, float height)

    : Entity(texture_id, speed, acceleration, jump_power, animation_sequences, animation_time,
        animation_index, animation_cols, animation_rows, width, height
        //, PLAYER
    ), character_movement_state(INAIR), current_action(IDLE), hit_back_animation_timer(0.0f),
    is_on_hit(false), is_movable(true), life(3), is_dashable(true), in_air_dash_count(1), 
    is_take_damage(false)
{
}

void Character::on_hit(Entity* collidable_entities, int collidable_entity_count, 
    float hit_back_power, float delta_time, float hit_back_duration, float invulnerable_duration) {

    Entity* collidable_entity = nullptr;

    for (int i = 0; i < collidable_entity_count; i++)
    {
        if (collidable_entities[i].get_is_active()) {
            collidable_entity = &collidable_entities[i];
            //std::cout << "current enemy position: " << collidable_entity->get_position().x << std::endl;
            //std::cout << check_collision(collidable_entity) << std::endl;
            //std::cout << "is on hit: " << is_on_hit << std::endl;
            //std::cout << "i: " << i << std::endl << std::endl;
            //std::cout << "collidable entity position x: " << collidable_entities[i].get_position().x << std::endl; // why can't get position of 1 and 2???
            if (check_collision(collidable_entity) && !is_on_hit) {
                glm::vec3 hit_back_dir = glm::normalize(glm::vec3(m_position - collidable_entity->get_position()));
                hit_back = hit_back_dir * hit_back_power;
                is_on_hit = true;
                is_take_damage = true;
                is_movable = false;
                break;
            }
            //else {
            //    std::cout << "i" << i << std::endl;
            //    std::cout << "position x: " << m_position.x << "\nposition y: " << m_position.y << std::endl;
            //    std::cout << "enemy position x: " << collidable_entity->get_position().x << "\nenemy position y: " << collidable_entity->get_position().y << std::endl;
            //}
        }
    }

    if (is_on_hit && collidable_entities) {
        //std::cout << "hit_back_animation_timer: " << hit_back_animation_timer << std::endl;
        //std::cout << "hit_back: " << hit_back.x << std::endl;
        hit_back_animation_timer += delta_time;
        if (hit_back_animation_timer < hit_back_duration) {
            m_position += hit_back * delta_time / (1 + hit_back_duration - hit_back_animation_timer);
        }
        if (hit_back_animation_timer > hit_back_duration) {
            is_movable = true;
        }
        if (hit_back_animation_timer > hit_back_duration + invulnerable_duration) {
            is_on_hit = false;
            hit_back_animation_timer = 0;
        }
    }

    if (is_take_damage) {
        --life;
        m_animation_index = 0;
        current_action = TAKE_DAMAGE;
        is_take_damage = false;
    }
}

void Character::update(float delta_time, Entity* player, Entity* collidable_entities, int collidable_entity_count, Map* map)
{
    if (life == 0) deactivate();
    if (!m_is_active) return;

    m_collided_top = false;
    m_collided_bottom = false;
    m_collided_left = false;
    m_collided_right = false;

    CharacterState prev_state = character_movement_state;

    // --------- UPDATE PHYSICS ---------- //

    m_velocity.x = m_movement.x * m_speed;
    m_velocity += m_acceleration * delta_time;

    m_position.y += m_velocity.y * delta_time;
    //check_collision_y(collidable_entities, collidable_entity_count);
    check_collision_y(map);

    m_position.x += m_velocity.x * delta_time;
    //check_collision_x(collidable_entities, collidable_entity_count);
    check_collision_x(map);
    

    if (m_is_jumping)
    {
        m_is_jumping = false;
        m_velocity.y += m_jumping_power;
    }

    // --------- UPDATE MOVEMENT STATE ---------- //

    if (m_collided_bottom) character_movement_state = GROUNDED;
    else if (m_collided_left || m_collided_right) character_movement_state = ONWALL;
    else character_movement_state = INAIR;

    // --------- UPDATE ACTION ---------- //

    CharacterAction prev_action = current_action;

    if (current_action != JUMP_START && current_action != DASH && current_action != TAKE_DAMAGE)
    switch (character_movement_state)
    {
    case GROUNDED:
        if (prev_state == INAIR) {
            if (prev_action == ATTACK_3) m_is_attacking = false;
            else if (prev_action == FALLING) current_action = LANDING;
        }
        else if (is_current_animation_complete) {
            if (m_velocity.x) current_action = RUNING;
            else current_action = IDLE;
        }

        if (m_is_attacking) {
            if (prev_action == IDLE && is_current_animation_complete) {
                m_animation_index = 0;
                current_action = ATTACK_1;
            }
            else if (prev_action == RUNING && is_current_animation_complete) {
                m_animation_index = 0;
                current_action = ATTACK_2;
            }
        }
        break;
    case INAIR:
        if (is_current_animation_complete) {
            if (m_velocity.y >= 0) { current_action = m_velocity.y > 1.0 ? JUMPING : JUMP_END; }
            else current_action = m_velocity.y > -1.0 ? FALL_START : FALLING;
        }
        if (m_is_attacking) {
            if (prev_action != current_action && is_current_animation_complete) {
                m_animation_index = 0;
                current_action = ATTACK_3;
                m_velocity.y -= m_jumping_power;
            }
        }
        break;
    case ONWALL:
        break;
    default:
        break;
    }

    // --------- UPDATE ANIMATION ---------- //

    switch (current_action)
    {
    case IDLE:
        m_animation_indices = m_animation_sequences[IDLE];
        loop_play(delta_time);
        break;
    case IDLE_EXTRA:
        break;
    case RUNING:
        m_animation_indices = m_animation_sequences[RUNING];
        loop_play(delta_time);
        break;
    case JUMP_START:
        m_animation_indices = m_animation_sequences[JUMP_START];
        play_once(delta_time);
        if (is_current_animation_complete) {
            current_action = JUMPING;
            m_is_jumping = true;
        }
        break;
    case JUMPING:
        m_animation_indices = m_animation_sequences[JUMPING];
        loop_play(delta_time);
        break;
    case JUMP_END:
        m_animation_indices = m_animation_sequences[JUMP_END];
        play_once(delta_time);
        break;
    case FALL_START:
        m_animation_indices = m_animation_sequences[FALL_START];
        play_once(delta_time);
        break;
    case FALLING:
        m_animation_indices = m_animation_sequences[FALLING];
        loop_play(delta_time);
        break;
    case LANDING:
        is_dashable = true;
        m_animation_indices = m_animation_sequences[LANDING];
        play_once(delta_time);
        break;
    case DOUBLE_JUMP_START:
        break;
    case DOUBLE_JUMP_ROLL:
        break;
    case DOUBLE_JUMP_TOP:
        break;
    case DOUBLE_JUMP_END:
        break;
    case WALL_GRAB:
        break;
    case WALL_SLIDE:
        break;
    case FLOOR_SLIDE:
        break;
    case DASH:
        is_dashable = false;
        if (character_movement_state == INAIR) {
            m_movement.x = 0.0f;
            m_position.y -= m_velocity.y * delta_time;
            m_velocity.y = 0.0f;
        }
        m_animation_indices = m_animation_sequences[DASH];
        m_position += glm::vec3(m_direction, 0.0f, 0.0f) * delta_time * 10.0f;
        play_once(delta_time * 2);
        if (is_current_animation_complete) {
            if (character_movement_state == INAIR) is_dashable = false;
            else if (character_movement_state == GROUNDED) is_dashable = true;
            current_action = IDLE;
        }
        break;
    case TAKE_DAMAGE:
        std::cout << "taking damage\n";
        is_movable = false;
        m_animation_indices = m_animation_sequences[TAKE_DAMAGE];
        play_once(delta_time);
        if (is_current_animation_complete) {
            is_movable = true;
            current_action = IDLE;
        }
        break;
    case ATTACK_1:
        m_animation_indices = m_animation_sequences[ATTACK_1];
        play_once(delta_time);
        if (is_current_animation_complete) m_is_attacking = false;
        break;
    case ATTACK_2:
        m_animation_indices = m_animation_sequences[ATTACK_2];
        play_once(delta_time);
        if (is_current_animation_complete) m_is_attacking = false;
        break;
    case ATTACK_3:
        m_animation_indices = m_animation_sequences[ATTACK_3];
        play_once(delta_time);
        if (is_current_animation_complete) m_is_attacking = false;
        break;
    case DUCKING:
        break;
    case CROUCH_IDLE:
        break;
    case CROUCH_WALK:
        break;
    default:
        break;
    }

    // --------- UPDATE TRANSFORMATION ---------- //

    m_model_matrix = glm::mat4(1.0f);
    m_model_matrix = glm::translate(m_model_matrix, m_position);
    if (m_velocity.x < 0) { m_direction = -1; }
    if (m_velocity.x > 0) { m_direction = 1; }
    if (m_direction < 0) m_model_matrix = glm::rotate(m_model_matrix, (float)3.1415926f, glm::vec3(0.0f, -1.0f, 0.0f));
    m_model_matrix = glm::scale(m_model_matrix, m_scale);
}