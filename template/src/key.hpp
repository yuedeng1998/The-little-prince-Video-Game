#pragma once

#include "common.hpp"
// <<<<<<< HEAD
#include "render_manager.hpp"
// =======
// #include "level.hpp"
// >>>>>>> min-play-steph

class Key : public Entity
{
	// Shared between all fish, no need to load one for each instance
	static Texture key_texture;

public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all the associated resources
	void destroy();

	// Update fish
	// ms represents the number of milliseconds elapsed from the previous update() call
	void update();

	// Renders the fish
	// projection is the 2D orthographic projection matrix
	void draw(const mat3 &projection) override;

	// Returns the current fish position
	vec2 get_position() const;

	// Sets the new fish position
	void set_position(vec2 position);

	// Returns the fish' bounding box for collision detection, called by collides_with()
	vec2 get_bounding_box() const;

	void set_is_collected(bool b);
	bool get_is_collected();

private:
	bool is_collected;
};
