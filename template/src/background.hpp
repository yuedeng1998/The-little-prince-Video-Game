#pragma once

#include "common.hpp"


class Background : public Entity
{
    // This is the screen texture for the background
    static Texture background_texture;
    static std::string img_path;
    static bool diff_background;

public:
	// Creates all the associated render resources and default transform
	bool init();

	// Releases all associated resources
	void destroy();

	// Renders the background
	void draw(const mat3& projection)override;

  // set the image path for the background
  void create_img_path(int current_level, int current_row, int current_col);

  // set image path for intro screen
  void set_intro_img_path();

  // set image path for end screen
  void set_end_img_path();

  // set image path for lost game screen
  void set_lost_game_img_path(int i);

  // set image path for level loading screen depending on level
  void set_level_load_img_path(int current_level);

  void set_quit_screen_img_path();
  void set_help_screen_img_path();

  void change_background();
};
