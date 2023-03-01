#pragma once
#pragma once

#include "common.hpp"
#include "render_manager.hpp"
// #include "game_object.hpp"

class Bullet : public Entity
{
	// Shared between all fish, no need to load one for each instance
	static Texture bullet_texture;

public:
	bool init(int index = 0);

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

	//bool collides_with(const Walls& wall);

	void set_shooting(bool shooting);

	void set_running(bool running);

	bool get_runnning() { return running; }
	void set_rotation(float radians);
	void set_finished_shotting(bool finish);

	float get_radians() { return motion.radians; }

private:
	bool shooting;
	bool running;
	bool finished_shotting;
	int counting_down;
};
