#pragma once

#include "common.hpp"
#include "render_manager.hpp"
//#include "game_object.hpp"

// Salmon food
class Heart_Grey : public Entity
{
	// Shared between all fish, no need to load one for each instance
	static Texture heart_grey_texture;

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

	// Sets the new fish position
	void set_position(vec2 position);

	void set_scale(vec2 scale);

};
