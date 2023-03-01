#pragma once

// #include "portal.hpp"
// #include "enemy.hpp"
// #include "star.hpp"
// #include "heart.hpp"
// #include "key.hpp"
#include "level.hpp"
#include "common.hpp"
#include "prince.hpp"
#include "background.hpp"
#include "walls.hpp"
#include "key.hpp"
#include "points.hpp"
#include "arrow.hpp"
#include "pebbles.hpp"
#include "heart_grey.hpp"
#include "bullet.hpp"
#include "ps.hpp"

#include "render_manager.hpp"
#include "collision_manager.hpp"

// stlib
#include <vector>
#include <random>
#include <map>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <string.h>

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class World
{
public:
	World();
	~World();

	// Creates a window, sets up events and begins the game
	bool init(vec2 screen);

	// Releases all associated resources
	void destroy();

	// Steps the game ahead by ms milliseconds
	bool update(float ms);

	// Renders our scene
	void draw();

	// Should the game be over ?
	bool is_over() const;

	int current_level = 0;

private:
	enum game_state
	{
		game_start,
		game_play,
		game_level_screen,
		game_quit_screen,
		game_help_screen,
		game_dead,
		game_end

	} game_state;

	void check_collisions(float ms);
	// check if ready for next level (collect all the stars)
	bool is_ready_next_level();

	// updates the current level and parses 00 file of level folder
	void go_next_level();

	// Generates a portal
	void spawn_portal();

	// Generates a new star
	void spawn_star(int max_star, int view_id);

	// Generates a new key
	void spawn_key();

	// Generates a new heart
	void spawn_heart(int max_hearts, int view_id);

	// Generates a new wall
	bool spawn_walls(int n);

	//intial the enemy
	bool spawn_enemy(int max_enemies, int view_id);

	//intial the prince bullet
	bool spawn_prince_bullet(); //

	// !!! INPUT CALLBACK FUNCTIONS
	void on_key(GLFWwindow *, int key, int, int action, int mod);

	// Parse a file for the current world view
	void parse_sum_file(int current_level);

	// Parse a file for the current world view
	void parse_view_file(int row, int col);

	//Assign the background
	void assign_background();

	// Updates screen as the prince moves to the next level
	void update_screen(int pos);

	//Set Bars
	bool set_bars();

	void init_level(int level_num);

	// get current view id
	int get_curr_view();

	void init_matrix();
	void create_tutorial_level();
	void clear_camera_view();

	void load_save_file();
	void write_to_save_file();

private:
	// Window handle
	GLFWwindow *m_window;
	float m_screen_scale; // Screen to pixel coordinates scale factor

	// Screen texture
	// The draw loop first renders to this texture, then it is used for the water shader
	GLuint m_frame_buffer;
	Texture m_screen_tex;
	char copy;

	// Ground effect
	Background m_background;
	Pebbles m_pebbles_emitter;

	//the grid
	vec2 grid[12][8];

	//single view info
	std::string singleview[12][8];

	int level_info[6];

	//list of vec2 positions where there are no walls
	std::vector<vec2> matrix_positions;

	// Game entities
	Prince m_prince;
	Arrow arrow;
	Points point_0, point_1;
	Star star_bar;
	Level m_level;
	std::vector<Walls> m_walls;

	// Boolean for advanced mode
	std::vector<Heart> m_hearts_bar;
	std::vector<Heart_Grey> m_hearts_grey_bar;
	std::vector<Star> m_stars_bar;
	std::vector<Bullet> m_bullets_prince_;
	std::map<int, Bullet> m_bullets_enemy;
	float prev_prince_v;

	int enemy_count;
	bool is_next_level;
	bool draw_portal;
	bool enter_key_press;
	bool n_key_press;
	bool c_key_press;
	bool y_key_press;
	bool no_key_press;
	bool h_key_press;
	bool escape_key_press;

	//pauses the game at the dead screen before
	//loading save file
	float dead_pause;
	float m_current_speed;
	float m_next_portal_spawn;
	float m_next_star_spawn;
	float m_next_heart_spawn;
	float m_next_wall_spawn;

	int last_state;

	Mix_Music *m_background_music = nullptr;
	Mix_Chunk *m_prince_dead_sound = nullptr;
	Mix_Chunk *m_star_collect_sound = nullptr;
	Mix_Chunk *m_heart_collect_sound = nullptr;
	Mix_Chunk *m_lose_heart_sound = nullptr;
	Mix_Chunk *m_portal_through_sound = nullptr;

	// C++ rng
	std::default_random_engine m_rng;
	std::uniform_real_distribution<float> m_dist; // default 0..1
};
