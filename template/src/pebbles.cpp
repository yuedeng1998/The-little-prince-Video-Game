#define _USE_MATH_DEFINES

// Header
#include "pebbles.hpp"

#include <cmath>
#include <iostream>

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// DON'T WORRY ABOUT THIS CLASS UNTIL ASSIGNMENT 3
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

constexpr int NUM_SEGMENTS = 12;

bool Pebbles::init()
{
	std::vector<GLfloat> screen_vertex_buffer_data;
	constexpr float z = -0.1;

	for (int i = 0; i < NUM_SEGMENTS; i++)
	{
		screen_vertex_buffer_data.push_back(std::cos(M_PI * 2.0 * float(i) / (float)NUM_SEGMENTS));
		screen_vertex_buffer_data.push_back(std::sin(M_PI * 2.0 * float(i) / (float)NUM_SEGMENTS));
		screen_vertex_buffer_data.push_back(z);

		screen_vertex_buffer_data.push_back(std::cos(M_PI * 2.0 * float(i + 1) / (float)NUM_SEGMENTS));
		screen_vertex_buffer_data.push_back(std::sin(M_PI * 2.0 * float(i + 1) / (float)NUM_SEGMENTS));
		screen_vertex_buffer_data.push_back(z);

		screen_vertex_buffer_data.push_back(0);
		screen_vertex_buffer_data.push_back(0);
		screen_vertex_buffer_data.push_back(z);
	}

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, screen_vertex_buffer_data.size() * sizeof(GLfloat), screen_vertex_buffer_data.data(),
				 GL_STATIC_DRAW);

	glGenBuffers(1, &m_instance_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);

	if (gl_has_errors())
		return false;

	// Loading shaders
	if (!effect.load_from_file(shader_path("pebble.vs.glsl"), shader_path("pebble.fs.glsl")))
		return false;

	return true;
}

// Releases all graphics resources
void Pebbles::destroy()
{
	glDeleteBuffers(1, &mesh.vbo);
	glDeleteBuffers(1, &m_instance_vbo);

	glDeleteShader(effect.vertex);
	glDeleteShader(effect.fragment);
	glDeleteShader(effect.program);

	m_pebbles.clear();
}

void Pebbles::update(float ms)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE PEBBLE UPDATES HERE
	// You will need to handle both the motion of pebbles
	// and the removal of dead pebbles.
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	auto pebble_it = m_pebbles.begin();
	while (pebble_it != m_pebbles.end())
	{
		if (pebble_it->life > 0.f)
		{
			if (pebble_it->direction == 0 || pebble_it->direction == 180)
			{
				// the game is top down so only when moving left or right will give gravity
				vec2 vel_change = {0.f, 9.81f * ms / 1000};
				pebble_it->velocity = add(pebble_it->velocity, vel_change);
			}
			pebble_it->position = add(pebble_it->position, mul(pebble_it->velocity, (ms / 1000)));
			pebble_it->life -= 0.01f;
			pebble_it->radius += 0.07f;
			++pebble_it;
		}
		else
		{
			pebble_it = m_pebbles.erase(pebble_it);
		}
	}

	pebble_it = m_pebbles.begin();
	while (pebble_it != m_pebbles.end())
	{
		auto pebble_a = m_pebbles.begin();
		while (pebble_a != m_pebbles.end())
		{
			if (pebble_a != pebble_it && (pebble_a->life < 9.f || pebble_it->life < 9.f))
			{
				vec2 vel_change = {0.f, 9.81f * ms / 1000};
				vec2 nvel = add(pebble_it->velocity, vel_change);
				vec2 nitpos = add(pebble_it->position, mul(nvel, (ms / 1000)));

				nvel = add(pebble_a->velocity, vel_change);
				vec2 napos = add(pebble_a->position, mul(nvel, ms / 1000));
				if (len(sub(nitpos, napos)) < (pebble_it->radius + pebble_a->radius))
				{
					vec2 v1 = pebble_it->velocity;
					vec2 v2 = pebble_a->velocity;
					vec2 p1 = pebble_it->position;
					vec2 p2 = pebble_a->position;
					vec2 a = sub(v1, v2);
					vec2 b = sub(p1, p2);
					vec2 c = mul(b, dot(a, b) / sq_len(b));
					pebble_it->velocity = sub(v1, c);
					a = mul(a, -1.f);
					b = mul(b, -1.f);
					c = mul(b, dot(a, b) / sq_len(b));
					pebble_a->velocity = sub(v2, c);
				}
			}
			pebble_a++;
		}
		pebble_it++;
	}
}

void Pebbles::spawn_pebble(vec2 position, int degree)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// HANDLE PEBBLE SPAWNING HERE
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	float radians = (float)degree * M_PI / 180.f;
	vec2 pos = add(position, {0.f, 35.f});
	int r = 1;
	for (int i = 0; i < r; i++)
	{
		Pebble pebble;
		pebble.position = pos;
		pebble.radius = rand() % 3 + 1; // range between 1 to 3
		pebble.direction = degree;
		float range = rand() % 45;
		range = range * M_PI / 180;
		int speed = 20;
		float x = cos(-1.f * radians + range) * speed;
		float y = sin(-1.f * radians + range) * speed;
		pebble.velocity = {x, y};
		m_pebbles.emplace_back(pebble);
	}
}

// Draw pebbles using instancing
void Pebbles::draw(const mat3 &projection)
{
	// Setting shaders
	glUseProgram(effect.program);

	// Enabling alpha channel for textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	// Getting uniform locations
	GLint projection_uloc = glGetUniformLocation(effect.program, "projection");
	GLint color_uloc = glGetUniformLocation(effect.program, "color");

	// Pebble color
	float color[] = {0.6f, 0.6f, 0.6f};
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float *)&projection);

	// Draw the screen texture on the geometry
	// Setting vertices
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

	// Mesh vertex positions
	// Bind to attribute 0 (in_position) as in the vertex shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glVertexAttribDivisor(0, 0);

	// Load up pebbles into buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_instance_vbo);
	glBufferData(GL_ARRAY_BUFFER, m_pebbles.size() * sizeof(Pebble), m_pebbles.data(), GL_DYNAMIC_DRAW);

	// Pebble translations
	// Bind to attribute 1 (in_translate) as in vertex shader
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Pebble), (GLvoid *)offsetof(Pebble, position));
	glVertexAttribDivisor(1, 1);

	// Pebble radii
	// Bind to attribute 2 (in_scale) as in vertex shader
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Pebble), (GLvoid *)offsetof(Pebble, radius));
	glVertexAttribDivisor(2, 1);

	// Draw using instancing
	// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDrawArraysInstanced.xhtml
	glDrawArraysInstanced(GL_TRIANGLES, 0, NUM_SEGMENTS * 3, m_pebbles.size());

	// Reset divisor
	glVertexAttribDivisor(1, 0);
	glVertexAttribDivisor(2, 0);
}