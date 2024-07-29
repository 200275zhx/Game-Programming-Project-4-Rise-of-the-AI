#pragma once
#include "Entity.h"

enum AIType { WALKER, GUARD, JUMPER };
enum AIState { AI_WALKING, AI_IDLE, AI_ATTACKING, AI_JUMP_ATTACKING };

class Enemy : public Entity {
public:
    Enemy();
	Enemy(GLuint texture_id, float speed, glm::vec3 acceleration, float jump_power,
        std::vector<std::vector<int>> animation_sequences, float animation_time,
        int animation_index, int animation_cols, int animation_rows, float width, float height,
        AIType AIType, AIState AIState, int texture_width_ratio);

    AIType     const get_ai_type()        const { return m_ai_type; };
    AIState    const get_ai_state()       const { return m_ai_state; };

    void const set_ai_type(AIType new_ai_type) { m_ai_type = new_ai_type; };
    void const set_ai_state(AIState new_state) { m_ai_state = new_state; };

    void ai_activate(Entity* player);
    void ai_walk();
    void ai_guard(Entity* player);
    void ai_jump(Entity* player);

    void update(float delta_time, Entity* player, Entity* collidable_entities, int collidable_entity_count, Map* map) override;
protected:
    bool m_is_active = true;

    AIType     m_ai_type;
    AIState    m_ai_state;

    float      m_ai_walk_direction;
    bool       m_is_ai_jump_attack_begin;

    glm::vec3  m_ai_velocity = glm::vec3(0.0f);
};