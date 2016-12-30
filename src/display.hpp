#ifndef GPROJ_DISPLAY_HPP_
#define GPROJ_DISPLAY_HPP_
#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace gp {

	
extern bool init_display(const char* title, int w, int h);
extern void close_display();


inline bool update_display()
{
	extern GLFWwindow* window;
	glfwPollEvents();
	glfwSwapBuffers(window);
	return glfwWindowShouldClose(window) == 0;
}

inline void clear_display(const float r,const float g,
                           const float b, const float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
}


} // namespace gp
#endif
