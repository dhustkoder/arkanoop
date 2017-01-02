#ifndef GPROJ_MESH_HPP_
#define GPROJ_MESH_HPP_
#include <GL/glew.h>
#include <glm/vec4.hpp>

namespace gp {

using Vertex = glm::vec4;

struct Mesh {
	long vertices_count;
	long buffers_count;
	GLenum mode;
	GLuint vao_id;
	GLuint buffers_ids[];
};


extern Mesh* create_mesh(GLenum mode,
                         const Vertex* const* vertices,
                         long vertices_arrays_size,
			 long count);

extern void destroy_mesh(Mesh* mesh);

inline void draw_mesh(const Mesh& mesh)
{
	glBindVertexArray(mesh.vao_id);
	glDrawArrays(mesh.mode, 0, mesh.vertices_count);
}


} // namespace gp
#endif

