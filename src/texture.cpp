#include <SOIL/SOIL.h>
#include "vertex_data.hpp"
#include "exception.hpp"
#include "texture.hpp"


namespace gp {

int Texture::s_index = 0;
	
Texture::Texture(const std::string& texture_file_path)
	: m_index(s_index++)
{
	if (s_index >= kMaxTexIndexValue)
		s_index = 0;

	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_2D, m_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	unsigned char* const img = SOIL_load_image(texture_file_path.c_str(), &m_width, &m_height, nullptr, SOIL_LOAD_RGBA);

	if (img == nullptr)
		throw Exception("Couldn't load texture " + texture_file_path + ": " + SOIL_last_result());

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);
	glGenerateMipmap(GL_TEXTURE_2D);

	SOIL_free_image_data(img);
	glBindTexture(GL_TEXTURE_2D, 0);
}


} // namespace gp

