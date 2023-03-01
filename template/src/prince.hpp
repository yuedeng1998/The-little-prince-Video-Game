#pragma once

#include "common.hpp"
#include "game_object.hpp"
#include "spritesheet.hpp"

#include <vector>

class Enemy;
class Star;
class Heart;
class Key;
class Portal;
class Walls;

class Prince : public GameObject
{
	static Texture prince_texture;

public:
	// Offset for the prince position
//	vec2 m_offset;
	unsigned int m_points;
    int m_lives; // prince number of lives
    float weight;
    // Renders the prince
    SpriteSheet spritesheet;
    vec2 m_velocity;

	// Creates all the associated render resources and default transform
	bool init() override; //dy playable

	// Update prince position based on direction
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	void animate();
	void draw(const mat3 &projection) override;

	void reset_position(int pos);

	// Moves the prince's position by the specified offset
	void move(vec2 off);

	void minus_live();
	int get_lives();
	void velocity_to_zero(float ms);


	// if prince has extra lives
	bool is_extra_lives();
	// True if the prince is alive
	bool is_alive() const;
	// add lives if collide with heart
	void add_life();
	// Kills the prince, changing its alive state and triggering on death events
	void kill();

	void set_lives(int num);

	vec2 get_bounding_box() const;

private:
	float m_light_up_countdown_ms; // Used to keep track for how long the prince should be lit up

	std::vector<Vertex> m_vertices;
	std::vector<uint16_t> m_indices;

	float m_animation_time = 0.f;
	int m_last_frame_index = 0;

	float m_sprite_width;
	float m_sprite_height;
};
