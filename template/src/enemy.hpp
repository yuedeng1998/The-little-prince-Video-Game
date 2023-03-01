#pragma once

#include "common.hpp"
#include "render_manager.hpp"
#include "bullet.hpp"

// Salmon enemy
class Walls;

class Enemy : public Entity
{
	// Shared between all turtles, no need to load one for each instance
	static Texture enemy_texture;
	static bool change_texture;
	int update_time;
	//the minimal distance between prince and enemy
	float safe_distance;
	float guard_distance;
	float attack_distance;
	bool find_prince;
	bool stop_and_attack;
	bool return_guard;
	vec2 prince_location;
	float actual_distance;
	float actual_guard_distance;
	vec2 guard_location;
	int key;
	int life;

	float weight = 200.f;

public:
	// Creates all the associated render resources and default transform
	bool init(int current_level);

	// Releases all the associated resources
	void destroy();

	// Update turtle due to current
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Renders the salmon
	// projection is the 2D orthographic projection matrix
	void draw(const mat3 &projection) override;

	// Returns the current turtle position
	vec2 get_position() const;

	// Sets the new turtle position
	void set_position(vec2 position);
	float get_weight();
	vec2 get_velocity();

	// set the guard location  of the enemy
	void set_guard_position(vec2 position);
	vec2 get_guard_position();

	// Returns the turtle' bounding box for collision detection, called by collides_with()
	vec2 get_bounding_box() const;

	//check whether prince is nearby
	void check_distance(vec2 prince_location);

	bool check_return_guard();

	static void set_texture_change();

	bool collides_with(const Walls &wall, float ms);
	void move(vec2 off);

	float get_safe_distance() const;

	void set_key(int key);

	int get_key() { return key; }

	bool get_stop_and_attack() { return stop_and_attack; }

	float get_angle();

	void minus_life();

	bool enemy_kill();

	//set origin location and when enemy is far away from prince then it will go back to it location;
};
