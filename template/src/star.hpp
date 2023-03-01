#pragma once

#include "common.hpp"
#include "render_manager.hpp"
// #include "game_object.hpp"

class Star : public Entity
{
	// Shared between all fish, no need to load one for each instance
	static Texture star_texture;

public:
	int fly_on;
	// Creates all the associated render resources and default transform
	bool init();

	void destroy();

	// ms represents the number of milliseconds elapsed from the previous update() call
	void update(float ms);

	// Renders the fish
	// projection is the 2D orthographic projection matrix
	void draw(const mat3 &projection) override;

	vec2 get_position() const;

	void set_position(vec2 position);

	// Returns the fish' bounding box for collision detection, called by collides_with()
	vec2 get_bounding_box() const;

	// Moves the star's position by the specified offset
	void move(vec2 off);
	//DY PLAYABLE
	int get_star_state();

	void change_state(int s);

private:
	// set the sate of the star
	// 0 if not collisd with prince yet
	// 1 if colled and flying
	// 2 if finished flying and should be delete
	// 3 if the star is bar star
	int state;
};
