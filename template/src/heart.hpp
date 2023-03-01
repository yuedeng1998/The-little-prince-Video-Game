#pragma once

#include "common.hpp"
#include "render_manager.hpp"
//#include "game_object.hpp"

// Salmon food
class Heart : public Entity
{
	// Shared between all fish, no need to load one for each instance
	static Texture heart_texture;

public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all the associated resources
	void destroy();

	// Update fish
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Renders the fish
	// projection is the 2D orthographic projection matrix
	void draw(const mat3 &projection) override;

	// Returns the current fish position
	vec2 get_position() const;

	// Sets the new fish position
	void set_position(vec2 position);

	// Returns the fish' bounding box for collision detection, called by collides_with()
	vec2 get_bounding_box() const;

	void set_scale(vec2 scale);

	void set_eaten(int e);

// <<<<<<< HEAD
// 	int eaten;
	bool is_collected;
// =======
	float eaten;
// >>>>>>> master

};
