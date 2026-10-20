#include <stdlib.h>
#include <stdio.h>

#include "GLFW/glfw3.h"
#include "glad/gl.h"

#define VM_IMPL
#include "vmlib/vmlib.h"

static void handle_glfw_error(int code, const char *description)
{
	printf("GLFW error %d: %s\n", code, description);
}

static void framebuffer_resize_update(GLFWwindow *window, int new_width, int new_height)
{
	(void)window;
	glViewport(0, 0, new_width, new_height);
}

static void init_glfw(void)
{
	glfwSetErrorCallback(handle_glfw_error);
	if (glfwInit()) return;
	fprintf(stderr, "Failed to initialize GLFW.\n");
	exit(EXIT_FAILURE);
}

static GLFWwindow *create_window(int width, int height)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

	GLFWwindow *window = glfwCreateWindow(width, height, "OpenGL triangle", NULL, NULL);
	if (window) return window;

	fprintf(stderr, "Window creation failed.\n");
	exit(EXIT_FAILURE);
}

static GLuint create_shader(const char *filename, GLenum type)
{
	FILE *sfile = fopen(filename, "r");
	if (!sfile) {
		fprintf(stderr, "Failed to open file %s\n", filename);
		abort();
	}

	fseek(sfile, 0, SEEK_END);
	size_t sfile_size = ftell(sfile);
	fseek(sfile, 0, SEEK_SET);

	char *source = calloc(1, sfile_size + 1);
	if (!source) {
		fprintf(stderr, "Allocation failed\n");
		abort();
	}

	fread(source, sfile_size, 1, sfile);
	fclose(sfile);

	GLuint shader = glCreateShader(type);

	// This copies the string for the implementation to use, so we can free directly after.
	glShaderSource(shader, 1, (const char **)&source, NULL);
	free(source);

	glCompileShader(shader);

	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if (success) return shader;

	char buffer[1024];
	glGetShaderInfoLog(shader, sizeof buffer, NULL, buffer);
	fprintf(stderr, "%s shader error: %s\n", type == GL_VERTEX_SHADER ? "Vertex" : "Fragment", buffer);
	abort();
}

static GLuint create_program(GLuint vertex_shader, GLuint fragment_shader)
{
	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	GLint success = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &success);

	if (!success) {
		char buffer[1024];
		glGetProgramInfoLog(program, sizeof buffer, NULL, buffer);
		fprintf(stderr, "Program error: %s\n", buffer);
		abort();
	}

	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	return program;
}

static GLuint create_active_vao(void)
{
	GLuint id;
	glGenVertexArrays(1, &id);
	glBindVertexArray(id);
	return id;
}

static void bind_vao(GLuint vao)
{
	glBindVertexArray(vao);
}

static GLuint create_active_buffer(GLenum type, const void *data, size_t bytes)
{
	GLuint id;
	glGenBuffers(1, &id);
	glBindBuffer(type, id);
	glBufferData(type, bytes, data, GL_STATIC_DRAW);
	return id;
}

static void set_attribute_data(int index, GLenum type, int count, size_t stride, size_t offset)
{
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, count, type, GL_FALSE, stride, (const void *)offset);
}

int main(void)
{
	// Library, context and window initialization.
	init_glfw();
	GLFWwindow *window = create_window(1280, 720);
	glfwMakeContextCurrent(window);
	gladLoadGL((GLADloadfunc)glfwGetProcAddress);

	glfwSetFramebufferSizeCallback(window, framebuffer_resize_update);

	// OpenGL object creation.

	GLuint program = create_program(
		create_shader("vertex_shader.vs", GL_VERTEX_SHADER),
		create_shader("fragment_shader.fs", GL_FRAGMENT_SHADER)
	);

	const float vertex_data[] = {
		-0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
		 0.0f,  0.5f, 0.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, 0.0f,  0.0f, 0.0f
	};

	// Make sure VBO and attribute config is done while VAO is active/bound.
	GLuint vao = create_active_vao();
	GLuint vbo = create_active_buffer(GL_ARRAY_BUFFER, vertex_data, sizeof vertex_data);
	set_attribute_data(0, GL_FLOAT, 3, 5 * sizeof(float), 0);
	set_attribute_data(1, GL_FLOAT, 2, 5 * sizeof(float), 3 * sizeof(float));
	bind_vao(0);

	// Uniform location.
	GLint transform_location = glGetUniformLocation(program, "transform");

	// Main loop.

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);

		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program);
		bind_vao(vao);

		// Since the program is active, we can now set the uniform data.
		mat4 identity = MAT4I;
		glUniformMatrix4fv(transform_location, 1, GL_FALSE, identity.d);

		// Any draw operation requires a program and VAO to be active.
		// The VAO determines the data, the program determines the shaders.

		// We don't want to skip any, so we are drawing 3 vertices starting at vertex (gl_VertexID) 0.
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// Unbind. We aren't really doing anything else, so this has no effect,
		// but it is good practice in OpenGL.
		glUseProgram(0);
		bind_vao(0);

		glfwSwapBuffers(window);
	}

	// OpenGL object deletion.
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteProgram(program);

	// Destroy created window and terminate the GLFW library.
	// The termination automatically destroys any windows, but it
	// is best to be explicit about what happens to your objects.
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
