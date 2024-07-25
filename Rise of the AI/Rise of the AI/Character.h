#pragma once
#include "Entity.h"

enum CharacterState {
	GROUNDED,
	INAIR,
	ONWALL
};

enum CharacterAction {
	IDLE,
	IDLE_EXTRA,
	RUNING,
	JUMP_START,
	JUMPING,
	JUMP_END,
	FALL_START,
	FALLING,
	LANDING,
	DOUBLE_JUMP_START,
	DOUBLE_JUMP_ROLL,
	DOUBLE_JUMP_TOP,
	DOUBLE_JUMP_END,
	WALL_GRAB,
	WALL_SLIDE,
	FLOOR_SLIDE,
	DASH,
	TAKE_DAMAGE,
	ATTACK_1,
	ATTACK_2,
	ATTACK_3,
	DUCKING,
	CROUCH_IDLE,
	CROUCH_WALK
};

class Character : public Entity {
public:
	Character(GLuint texture_id, float speed, glm::vec3 acceleration, float jump_power,
		std::vector<std::vector<int>> animation_sequences, float animation_time,
		int animation_index, int animation_cols, int animation_rows, float width, float height);

	void update(float delta_time, Entity* player, Entity* collidable_entities, int collidable_entity_count, Map* map) override;

	void set_current_action(CharacterAction action) { current_action = action; }
	CharacterAction get_current_action() const { return current_action; }

	//void const jump_hold() { current_action = JUMP_START; }
	void const jump() override { 
		current_action = JUMP_START;
		m_is_attacking = false;
	}

private:
//	bool is_alive;
//	float life;
//	float attack_damage;
	CharacterState character_movement_state;
	CharacterAction current_action;
};