// Header
#include "bullet.hpp"
#include "walls.hpp"

#include <cmath>
#include <string>
Texture Bullet::bullet_texture;

bool Bullet::init(int index)
{
	std::string dir_path = data_path "/textures/";
	std::string bullet_path = dir_path + "bullet/bullet" + std::to_string(index) + ".png";
	const char *texturePath = bullet_path.c_str();
	/*if (!RenderManager::load_texture(texturePath, &bullet_texture, this))
	{
		return false;
	}*/
	if (!bullet_texture.is_valid())
	{
		if (!bullet_texture.load_from_file(texturePath))
		{
			fprintf(stderr, "Failed to load turtle texture!");
			return false;
		}
	}

	if (!RenderManager::set_buffers(&bullet_texture, this, -0.01, vec2{1, 1}))
	{
		return false;
	}

	// Loading shaders
	if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
		return false;

	motion.radians = 0.f;
	motion.speed = 400.f;
	physics.scale = {-1.9f, 1.9f};
	running = false;
	shooting = false;
	counting_down = 0;
	finished_shotting = false;
	return true;
}

void Bullet::update(float m)
{
	float step = 1.5f * motion.speed * (m / 1000);
	//dy playable

	//counting_down -= 1;
	if (running)
	{
		float x = step * sin(motion.radians);
		float y = -step * cos(motion.radians);
		move({x, y});
	}
	if (shooting)
	{
		set_running(true);
	}
	if (finished_shotting)
	{
		shooting = false;
		running = false;
		counting_down -= 1;
	}
	if (counting_down == 0)
	{
		finished_shotting = false;
	}
}

void Bullet::draw(const mat3 &projection)
{
	RenderManager::draw_texture(projection, &bullet_texture, this, motion.position, motion.radians, physics.scale);
}

void Bullet::destroy()
{
	//CollisionManager::get_instance().unregister_collision_game_object(this);
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteBuffers(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

vec2 Bullet::get_bounding_box() const
{
	// Returns the local bounding coordinates scaled by the current size of the star
	// fabs is to avoid negative scale due to the facing direction.
	return {std::fabs(physics.scale.x) * bullet_texture.width, std::fabs(physics.scale.y) * bullet_texture.height};
}

void Bullet::move(vec2 off)
{
	motion.position.x += off.x;
	motion.position.y += off.y;
}

void Bullet::set_shooting(bool shooting)
{
	this->shooting = shooting;
}

void Bullet::set_running(bool running)
{
	this->running = running;
}

void Bullet::set_rotation(float radians)
{
	motion.radians = radians;
}

vec2 Bullet::get_position() const
{
	return motion.position;
}

void Bullet::set_position(vec2 position)
{
	motion.position = position;
}

void Bullet::set_finished_shotting(bool finish)
{
	this->counting_down = 150;
	finished_shotting = finish;
}
