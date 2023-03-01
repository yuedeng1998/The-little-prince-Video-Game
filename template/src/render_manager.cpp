#include "render_manager.hpp"
#include "common.hpp"

bool RenderManager::load_texture(const char *texturePath, Texture *texture, Entity *entity)
{
	if (!texture->is_valid())
	{
		if (!texture->load_from_file(texturePath))
		{
			fprintf(stderr, "Failed to load texture at path %s", texturePath);
			return false;
		}
	}
	return true;
}

void RenderManager::draw_texture(const mat3 &projection, Texture *texture, Entity *entity, vec2 position, float rotation, vec2 scale)
{
	glEnable(GL_DEPTH_TEST);
	entity->transform.begin();
	entity->transform.translate(position);
	entity->transform.rotate(rotation);
	entity->transform.scale(scale);
	entity->transform.end();

	// Setting shaders
	glUseProgram(entity->effect.program);

	// Enabling alpha channel for textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// glDisable(GL_DEPTH_TEST);

	// Getting uniform locations for glUniform* calls
	GLint transform_uloc = glGetUniformLocation(entity->effect.program, "transform");
	GLint color_uloc = glGetUniformLocation(entity->effect.program, "fcolor");
	GLint projection_uloc = glGetUniformLocation(entity->effect.program, "projection");

	// Setting vertices and indices
	glBindVertexArray(entity->mesh.vao);
	glBindBuffer(GL_ARRAY_BUFFER, entity->mesh.vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entity->mesh.ibo);

	// Input data location as in the vertex buffer
	GLint in_position_loc = glGetAttribLocation(entity->effect.program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(entity->effect.program, "in_texcoord");
	glEnableVertexAttribArray(in_position_loc);
	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)0);
	glVertexAttribPointer(in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex), (void *)sizeof(vec3));

	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->id);

	// Setting uniform values to the currently bound program
	glUniformMatrix3fv(transform_uloc, 1, GL_FALSE, (float *)&entity->transform.out);
	float color[] = {1.f, 1.f, 1.f};
	glUniform3fv(color_uloc, 1, color);
	glUniformMatrix3fv(projection_uloc, 1, GL_FALSE, (float *)&projection);

	// Drawing!
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
	glDisable(GL_DEPTH_TEST);
}

bool RenderManager::set_buffers(Texture *texture, Entity *entity, float zvalue, vec2 offset)
{
	float wr = texture->width * 0.5f;
	float hr = texture->height * 0.5f;

	TexturedVertex vertices[4];
	vertices[0].position = {-wr, +hr, zvalue};
	vertices[0].texcoord = {0.f, 1.f / offset.y};
	vertices[1].position = {+wr, +hr, zvalue};
	vertices[1].texcoord = {
		1.f / offset.x,
		1.f / offset.y,
	};
	vertices[2].position = {+wr, -hr, zvalue};
	vertices[2].texcoord = {1.f / offset.x, 0.f};
	vertices[3].position = {-wr, -hr, zvalue};
	vertices[3].texcoord = {0.f, 0.f};

	// Counterclockwise as it's the default opengl front winding direction.
	uint16_t indices[] = {0, 3, 1, 1, 3, 2};

	// Clearing errors
	gl_flush_errors();

	// Vertex Buffer creation
	glGenBuffers(1, &entity->mesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, entity->mesh.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(TexturedVertex) * 4, vertices, GL_STATIC_DRAW);

	// Index Buffer creation
	glGenBuffers(1, &entity->mesh.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, entity->mesh.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, indices, GL_STATIC_DRAW);

	// Vertex Array (Container for Vertex + Index buffer)
	glGenVertexArrays(1, &entity->mesh.vao);
	if (gl_has_errors())
		return false;

	return true;
}
