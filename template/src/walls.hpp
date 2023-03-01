#pragma once

#include "common.hpp"
#include "render_manager.hpp"
#include "ps.hpp"
#include "window.hpp"

// Salmon food
class Walls : public Entity
{
	// Shared between all fish, no need to load one for each instance
	static Texture walls_texture;
	static Texture trees_texture;
	static Texture water_texture;

public:
	static bool change_textures;

	// Creates all the associated render resources and default transform
	bool init(int n, int current_level);

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
	vec2 get_bounding_box() const;

	static void set_texture_change();
	static void unset_texture_change();

	bool is_brarrel();

	bool exploding();

	int get_points();

private:
	//choose which texture to use
	int t_t;

	Particles m_particles;
	Particles m_particles2;
	Window pop;

	bool checked;
};

//TODO HOW TO effectiently set the life time to replace the berral texture after explording
//    change the effect and color of the particles make it more likea destorying effect
