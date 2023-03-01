// Header
#include "enemy.hpp"
#include "walls.hpp"

#include <cmath>
#include <string>

Texture Enemy::enemy_texture;
bool Enemy::change_texture;

bool Enemy::init(int current_level)
{
	std::string dir_path = data_path "/textures/";
	std::string enemy_path = dir_path + std::to_string(current_level) + "/" + "enemy.png";
	const char *texturePath = enemy_path.c_str();

	if (!enemy_texture.is_valid() || change_texture)
	{
		if (!enemy_texture.load_from_file(texturePath))
		{
			fprintf(stderr, "Failed to load turtle texture!");
			return false;
		}
	}

	if (!RenderManager::set_buffers(&enemy_texture, this, -0.03f, vec2{1, 1}))
	{
		return false;
	}

	change_texture = false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("textured.vs.glsl"), shader_path("textured.fs.glsl")))
		return false;

	motion.radians = 0.f;
	motion.speed = 200.f;

	// Setting initial values, scale is negative to make it face the opposite way
	// 1.0 would be as big as the original texture.
	physics.scale = {-0.8f, 0.8f};

	update_time = 0;
	safe_distance = 400.f;
	attack_distance = 300.f;
	find_prince = false;
	stop_and_attack = false;

	guard_distance = 120.f;
	life = 1;
	return true;
}

// Releases all graphics resources
void Enemy::destroy()
{

	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &mesh.ibo);
	glDeleteVertexArrays(1, &mesh.vao);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Enemy::update(float ms)
{
	// Move fish along -X based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	float step = -0.5 * motion.speed * (ms / 1000);
	double PI = 3.1415926535897932384626;
	if (stop_and_attack)
	{
		return;
	}
	else if (find_prince)
	{
		motion.position.x += step * ((motion.position.x - prince_location.x) / actual_distance);
		motion.position.y += step * ((motion.position.y - prince_location.y) / actual_distance);
	}
	else
	{
		if (!check_return_guard())
		{
			motion.position.x += step * 3 * ((motion.position.x - guard_location.x) / actual_guard_distance);
			motion.position.y += step * 3 * ((motion.position.y - guard_location.y) / actual_guard_distance);
		}
		else
		{
			// motion.position.x += step * 2 * sin(PI * update_time * 2 / 180.0);
			// motion.position.y += step * 2 * cos(PI * update_time * 2 / 180.0);
		}
	}
	update_time++;
}

void Enemy::draw(const mat3 &projection)
{
	RenderManager::draw_texture(projection, &enemy_texture, this, motion.position, motion.radians, physics.scale);
}

vec2 Enemy::get_position() const
{
	return motion.position;
}

void Enemy::set_position(vec2 position)
{
	motion.position = position;
}
float Enemy::get_weight()
{
	return weight;
}
vec2 Enemy::get_velocity()
{
	//    float step = -0.5 * motion.speed * (ms / 1000);
	//    return {step * ((motion.position.x - prince_location.x) / actual_distance),
	//           step * ((motion.position.y - prince_location.y) / actual_distance)};
	return {};
}
void Enemy::set_guard_position(vec2 position)
{
	guard_location = position;
}
vec2 Enemy::get_guard_position()
{
	return guard_location;
}
vec2 Enemy::get_bounding_box() const
{
	// Returns the local bounding coordinates scaled by the current size of the turtle
	// fabs is to avoid negative scale due to the facing direction.
	return {std::fabs(physics.scale.x) * enemy_texture.width, std::fabs(physics.scale.y) * enemy_texture.height};
}

void Enemy::check_distance(vec2 prince_location)
{
	actual_distance = len(sub(prince_location, motion.position));
	find_prince = actual_distance < safe_distance;
	stop_and_attack = actual_distance < attack_distance;
	if (find_prince)
	{
		// printf("actual distance: %f, prince location: %f,%f; enemy location: %f, %f\n",
		//         actual_distance, prince_location.x, prince_location.y,motion.position.x, motion.position.y);
		this->prince_location = prince_location;
	}
}

float Enemy::get_safe_distance() const
{
	return safe_distance;
}

bool Enemy::check_return_guard()
{
	actual_guard_distance = sqrt(pow(guard_location.x - motion.position.x, 2) + pow(guard_location.y - motion.position.y, 2));
	return_guard = actual_guard_distance <= guard_distance;
	return return_guard;
}

void Enemy::set_texture_change()
{
	change_texture = true;
}

bool Enemy::collides_with(const Walls &wall, float ms)
{
	vec2 enemy_bounding_box = get_bounding_box();
	vec2 wall_bounding_box = wall.get_bounding_box();
	vec2 wall_pos = wall.get_position();

	float step = -0.5 * motion.speed * ms / 1000;
	vec2 enemy_next_pos = motion.position;
	if (find_prince)
	{
		enemy_next_pos = add(motion.position, mul(sub(motion.position, prince_location), step / actual_distance));
	}
	else if (!check_return_guard())
	{
		enemy_next_pos = add(motion.position, mul(sub(motion.position, guard_location), step * 3 / actual_guard_distance));
	}

	float enemy_off_set_x = enemy_bounding_box.x / 2;
	float enemy_off_set_y = enemy_bounding_box.y / 2;

	float enemy_right = enemy_next_pos.x + enemy_off_set_x;
	float enemy_left = enemy_next_pos.x - enemy_off_set_x;
	float enemy_top = enemy_next_pos.y - enemy_off_set_y;
	float enemy_bottom = enemy_next_pos.y + enemy_off_set_y;

	float wall_off_set_x = wall_bounding_box.x / 2;
	float wall_off_set_y = wall_bounding_box.y / 2;

	float wall_right = wall_pos.x + wall_off_set_x;
	float wall_left = wall_pos.x - wall_off_set_x;
	float wall_top = wall_pos.y - wall_off_set_y;
	float wall_bottom = wall_pos.y + wall_off_set_y;

	if (enemy_top <= wall_bottom && enemy_bottom >= wall_top && enemy_left <= wall_right && enemy_right >= wall_left)
		return true;
	else
		return false;
}

void Enemy::move(vec2 off)
{
	motion.position.x += off.x;
	motion.position.y += off.y;
}

void Enemy::set_key(int key)
{
	this->key = key;
}

float Enemy::get_angle()
{
	//float M_PI_2 = 1.57079632679489661923;
	// float M_PI = 3.14159265359;
	float angle = atan2(prince_location.y - motion.position.y, prince_location.x - motion.position.x) + M_PI / 2;
	return angle;
}

void Enemy::minus_life()
{
	life--;
}

bool Enemy::enemy_kill()
{
	return life <= 0;
}
