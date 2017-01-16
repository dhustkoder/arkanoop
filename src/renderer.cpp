#include <stdio.h>
#include <fstream>
#include <SOIL/SOIL.h>
#include "renderer.hpp"
#include "finally.hpp"

namespace gp {

static GLuint vao_id = 0;
static GLuint vbo_id = 0;
static GLuint ebo_id = 0;


GLuint textures_ids[kMaxTextures] { 0 };
GLuint programs_ids[kMaxShaders] { 0 };
static GLuint shaders_ids[kMaxShaders][2] { 0 };
static GLchar error_msg_buffer[kErrorMsgBufferSize] { 0 };


static bool create_glbuffers();
static bool create_textures(const std::vector<std::string>& textures_files);
static bool create_shaders(const std::vector<std::pair<std::string, std::string>>& programs);
static void free_glbuffers();
static void free_textures();
static void free_shaders();


static void fill_vbo(const Vertex* const vertices, const int count);

static bool read_sources(const std::string& vertex_file, const std::string& fragment_file,
                         std::string* vertex_source, std::string* fragment_source);

static bool push_new_shader_program(const std::pair<std::string, std::string>& program, int index);
static bool validate_compilation(GLuint shader_id);
static bool validate_linkage(GLuint program_id);



bool initialize_renderer(const std::vector<std::string>& textures_files,
                         const std::vector<std::pair<std::string, std::string>>& shaders_programs)
{
	auto failure_guard = finally([] {
		terminate_renderer();
	});

	if (!create_glbuffers() ||
	    !create_textures(textures_files) ||
	    !create_shaders(shaders_programs))
		return false;

	failure_guard.Abort();
	return true;
}


void terminate_renderer()
{
	free_shaders();
	free_textures();
	free_glbuffers();
}


void draw(const GLenum mode, const Elements& elements)
{
	const Vertices& vertices = elements.vertices;
	const Indices& indices = elements.indices;
	const GLsizei ind_count = indices.count;

	glBindVertexArray(vao_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

	fill_vbo(vertices.data, vertices.count);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*indices.data) * ind_count, indices.data, GL_STREAM_DRAW);
	glDrawElements(mode, ind_count, GL_UNSIGNED_INT, static_cast<GLvoid*>(0));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}


void draw(const GLenum mode, const Vertices& vertices)
{
	const int count = vertices.count;
	glBindVertexArray(vao_id);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

	fill_vbo(vertices.data, count);

	glDrawArrays(mode, 0, count);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


void set_uniform(const int program, const Mat4& mat4, const char* name)
{
	GLint location = glGetUniformLocation(programs_ids[program], name);
	glUniformMatrix4fv(location, 1, GL_FALSE, &mat4.data[0][0]);
}


void fill_vbo(const Vertex* vertices, const int count)
{
	constexpr auto vertsize = sizeof(*vertices);
	const auto buffsize = vertsize * count;
	const auto pos_offset = (GLvoid*) offsetof(Vertex, pos);
	const auto tex_offset = (GLvoid*) offsetof(Vertex, tex);
	const auto col_offset = (GLvoid*) offsetof(Vertex, color);

	glBufferData(GL_ARRAY_BUFFER, buffsize, vertices, GL_STREAM_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, vertsize, pos_offset);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertsize, tex_offset);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, vertsize, col_offset);
	glEnableVertexAttribArray(2);
}


bool create_glbuffers()
{
	glGenVertexArrays(1, &vao_id);
	glBindVertexArray(vao_id);
	glGenBuffers(1, &vbo_id);
	glGenBuffers(1, &ebo_id);
	glBindVertexArray(0);

	if (!vao_id || !vbo_id || !ebo_id) {
		fprintf(stderr, "%s\n", glewGetErrorString(glGetError()));
		return false;
	}

	return true;
}


void free_glbuffers()
{
	glBindVertexArray(vao_id);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &vbo_id);
	glDeleteBuffers(1, &ebo_id);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vao_id);
	ebo_id = 0;
	vao_id = 0;
	vbo_id = 0;
}


bool create_textures(const std::vector<std::string>& textures_files)
{
	if (textures_files.size() > kMaxTextures) {
		fprintf(stderr, "Max textures: %i\n", kMaxTextures);
		return false;
	}

	const int num_textures = static_cast<int>(textures_files.size());
	glGenTextures(num_textures, textures_ids);

	for (int i = 0; i < num_textures; ++i) {
		const auto tex_id = textures_ids[i];
		const auto& tex_file = textures_files[i];

		glBindTexture(GL_TEXTURE_2D, tex_id);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int width, height;
		unsigned char* const image = SOIL_load_image(tex_file.c_str(),
                                                             &width, &height,
		                                             nullptr, SOIL_LOAD_RGB);

		if (image == nullptr) {
			fprintf(stderr, "Couldn't load texture \'%s\' %s\n", tex_file.c_str(), SOIL_last_result());
			return false;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
		SOIL_free_image_data(image);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}


void free_textures()
{
	unbind_textures();
	glDeleteTextures(kMaxTextures, textures_ids);
}


bool create_shaders(const std::vector<std::pair<std::string, std::string>>& programs)
{
	if (programs.size() > kMaxShaders) {
		fprintf(stderr, "Max shaders: %i\n", kMaxShaders);
		return false;
	}

	int index = 0;
	for (const auto& program : programs) {
		if (!push_new_shader_program(program, index++))
			return false;
	}

	return true;
}


void free_shaders()
{
	unbind_shaders();
	for (int i = 0; i < kMaxShaders; ++i) {
		const auto program_id = programs_ids[i];
		if (program_id == 0)
			break;
		
		const auto vertex_id = shaders_ids[i][0];
		const auto fragment_id = shaders_ids[i][1];

		glDetachShader(program_id, vertex_id);
		glDetachShader(program_id, fragment_id);
		glDeleteShader(vertex_id);
		glDeleteShader(fragment_id);
		glDeleteProgram(program_id);	
	}
}


bool push_new_shader_program(const std::pair<std::string, std::string>& program, const int index)
{
	std::string vertex_source, fragment_source;

	if (!read_sources(program.first, program.second,
	                  &vertex_source, &fragment_source)) {
		return false;
	}

	const char* sources[] {
		vertex_source.c_str(),
		fragment_source.c_str()
	};

	const auto program_id = glCreateProgram();
	const auto vertex_id = glCreateShader(GL_VERTEX_SHADER);
	const auto fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
	programs_ids[index] = program_id;
	shaders_ids[index][0] = vertex_id;
	shaders_ids[index][1] = fragment_id;

	glShaderSource(vertex_id, 1, &sources[0], nullptr);
	glCompileShader(vertex_id);

	if (!validate_compilation(vertex_id))
		return false;

	glShaderSource(fragment_id, 1, &sources[1], nullptr);
	glCompileShader(fragment_id);

	if (!validate_compilation(fragment_id))
		return false;

	glAttachShader(program_id, vertex_id);
	glAttachShader(program_id, fragment_id);
	glLinkProgram(program_id);

	if (!validate_linkage(program_id))
		return false;

	return true;
}


bool validate_compilation(const GLuint shader_id)
{
	GLint success;
	
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);

	if (success == GL_FALSE) {
		glGetShaderInfoLog(shader_id, kErrorMsgBufferSize,
		                   nullptr, error_msg_buffer);
		fprintf(stderr, "%s\n", error_msg_buffer);
		return false;
	}

	return true;
}


bool validate_linkage(const GLuint program)
{
	const auto has_error = [program] {
		GLint success;	
		
		glGetProgramiv(program, GL_LINK_STATUS, &success);

		if (success == GL_FALSE) {
			glGetProgramInfoLog(program, kErrorMsgBufferSize,
			                    nullptr, error_msg_buffer);
			fprintf(stderr, "%s\n", error_msg_buffer);
			return true;
		}

		return false;
	};

	if (has_error())
		return true;

	glValidateProgram(program);
	return has_error() == false;
}


bool read_sources(const std::string& vertex_filepath, const std::string& fragment_filepath,
                 std::string* const vertex_source, std::string* const fragment_source)
{
	std::ifstream vertex_file(vertex_filepath);
	std::ifstream fragment_file(fragment_filepath);
	
	if (!vertex_file.good()) {
		fprintf(stderr, "Couldn't read \'%s\' source file\n",
		        vertex_filepath.c_str());
		return false;
	} else if (!fragment_file.good()) {
		fprintf(stderr, "Couldn't read \'%s\' source file\n",
		        fragment_filepath.c_str());
		return false;
	}

	*vertex_source =
	  std::string{ std::istreambuf_iterator<GLchar>(vertex_file),
	               std::istreambuf_iterator<GLchar>() };

	*fragment_source =
	  std::string{ std::istreambuf_iterator<GLchar>(fragment_file),
	               std::istreambuf_iterator<GLchar>() };

	return true;
}


} // namespace gp

