#include "background.hpp"

#include <iostream>
#include <string>

Texture Background::background_texture;
std::string Background::img_path = "";
bool Background::diff_background = false;

bool Background::init()
{

	// Since we are not going to apply transformation to this screen geometry
	// The coordinates are set to fill the standard openGL window [-1, -1 .. 1, 1]
	// Make the size slightly larger then the screen to crop the boundary.
	static const GLfloat screen_vertex_buffer_data[] = {
		-1.05f,
		-1.05f,
		0.0f,
		1.05f,
		-1.05f,
		0.0f,
		-1.05f,
		1.05f,
		0.0f,
		-1.05f,
		1.05f,
		0.0f,
		1.05f,
		-1.05f,
		0.0f,
		1.05f,
		1.05f,
		0.0f,
	};

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screen_vertex_buffer_data), screen_vertex_buffer_data, GL_STATIC_DRAW);

	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("background.vs.glsl"), shader_path("background.fs.glsl")))
		return false;

	return true;
}

// Releases all graphics resources
void Background::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);
}

void Background::draw(const mat3 &projection)
{

	std::string dir_path = data_path "/textures/";
	std::string file_path = dir_path + img_path;
	const char *c = file_path.c_str();

	// std::cout << file_path << "\n";

	// Load shared texture
	if (!background_texture.is_valid() || diff_background)
	{
		diff_background = false;
		if (!background_texture.load_from_file(c))
		{
			// fprintf(stderr, "Failed to load ground texture!");
		}
	}

	// Enabling alpha channel for textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	// Setting shaders
	glUseProgram(effect.program);

	// Set screen_texture sampling to texture unit 0
	// Set clock
	GLuint screen_text_uloc = glGetUniformLocation(effect.program, "screen_texture");
	glUniform1i(screen_text_uloc, 0);

	// Draw the screen texture on the quad geometry
	// Setting vertices
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, background_texture.id);

	// Bind to attribute 0 (in_position) as in the vertex shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

	// Draw
	glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
	glDisableVertexAttribArray(0);
}

// Create the image path for the background
void Background::create_img_path(int current_level, int current_row, int current_col)
{
	if (current_level == 0)
	{
		img_path = std::to_string(current_level) + "/" + "ground-" + std::to_string(current_row) + std::to_string(current_col) + ".png";
	}
	else
	{
		img_path = std::to_string(current_level) + "/" + "ground.png";
	}
}

// set image path for intro screen
void Background::set_intro_img_path()
{
	img_path = "screen_textures/intro.png";
}

// set image path for level loading screen depending on level
void Background::set_level_load_img_path(int current_level)
{
	img_path = std::to_string(current_level) + "/" + "level-" + std::to_string(current_level + 1) + ".png";
}


 // set image path for lost game screen
void Background::set_lost_game_img_path(int i)
{
	img_path = "screen_textures/dead_screen"+std::to_string(i)+".png";
}

// set image path for end screen
// void set_end_img_path();

void Background::set_quit_screen_img_path()
{
	img_path = "/quit.png";
}

void Background::set_help_screen_img_path()
{
	img_path = "/help.png";
}

void Background::change_background()
{
	diff_background = true;
}
