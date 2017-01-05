#ifndef GPROJ_SHADERS_HPP_
#define GPROJ_SHADERS_HPP_
#include <string>
#include <vector>
#include <utility>
#include <GL/glew.h>
#include "vectors.hpp"


namespace gp {


extern bool initialize_shaders(const std::vector<std::pair<std::string, std::string>>& programs);
extern void terminate_shaders();
extern void set_shader_uniform(size_t program, const std::string& name, const Vec4f& value);


inline void bind_shader(const size_t program_index)
{
	extern std::vector<GLuint> programs_ids;
	glUseProgram(programs_ids[program_index]);
}


inline void unbind_shader()
{
	glUseProgram(0);
}


} // namespace gp
#endif