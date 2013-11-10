#include <GLXW/glxw.h>
#include <GLWT/glwt.h>

#include <iostream>
#include <string>
#include <vector>

// helper to check and display for shader compiler errors
bool check_shader_compile_status(GLuint obj) {
    GLint status;
    glGetShaderiv(obj, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) {
        GLint length;
        glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetShaderInfoLog(obj, length, &length, &log[0]);
        std::cerr << &log[0];
        return false;
    }
    return true;
}

// helper to check and display for shader linker error
bool check_program_link_status(GLuint obj) {
    GLint status;
    glGetProgramiv(obj, GL_LINK_STATUS, &status);
    if(status == GL_FALSE) {
        GLint length;
        glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> log(length);
        glGetProgramInfoLog(obj, length, &length, &log[0]);
        std::cerr << &log[0];
        return false;
    }
    return true;
}

static void error_callback(const char *msg, void *userdata) {
    (void)userdata;
    std::cerr << msg << std::endl;
}

int main() {
    int width = 640;
    int height = 480;

    GLWTConfig config = {0, 0, 0, 0, 0, 0, 0, 0, GLWT_API_OPENGL | GLWT_PROFILE_CORE, 3, 3};

    if(glwtInit(&config, error_callback, 0)) {
        std::cerr << "failed to init GLWT" << std::endl;
        return 1;
    }

    // create a window
    GLWTWindow *window;
    if((window = glwtWindowCreate("glwt", width, height, 0, 0, 0)) == 0) {
        std::cerr << "failed to open window" << std::endl;
        glwtQuit();
        return 1;
    }
    glwtMakeCurrent(window);
    glwtWindowShow(window, 1);

    if(glxwInit()) {
        std::cerr << "failed to init GLXW" << std::endl;
        glwtWindowDestroy(window);
        glwtQuit();
        return 1;
    }

    // shader source code
    std::string vertex_source =
        "#version 330\n"
        "layout(location = 0) in vec4 vposition;\n"
        "layout(location = 1) in vec4 vcolor;\n"
        "out vec4 fcolor;\n"
        "void main() {\n"
        "   fcolor = vcolor;\n"
        "   gl_Position = vposition;\n"
        "}\n";

    std::string fragment_source =
        "#version 330\n"
        "in vec4 fcolor;\n"
        "layout(location = 0) out vec4 FragColor;\n"
        "void main() {\n"
        "   FragColor = fcolor;\n"
        "}\n";

    // program and shader handles
    GLuint shader_program, vertex_shader, fragment_shader;

    // we need these to properly pass the strings
    const char *source;
    int length;

    // create and compiler vertex shader
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    source = vertex_source.c_str();
    length = vertex_source.size();
    glShaderSource(vertex_shader, 1, &source, &length);
    glCompileShader(vertex_shader);
    if(!check_shader_compile_status(vertex_shader)) {
        glwtWindowDestroy(window);
        glwtQuit();
        return 1;
    }

    // create and compiler fragment shader
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    source = fragment_source.c_str();
    length = fragment_source.size();
    glShaderSource(fragment_shader, 1, &source, &length);
    glCompileShader(fragment_shader);
    if(!check_shader_compile_status(fragment_shader)) {
        glwtWindowDestroy(window);
        glwtQuit();
        return 1;
    }

    // create program
    shader_program = glCreateProgram();

    // attach shaders
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);

    // link the program and check for errors
    glLinkProgram(shader_program);
    if(!check_program_link_status(shader_program)) {
        glwtWindowDestroy(window);
        glwtQuit();
        return 1;
    }

    // vao and vbo handle
    GLuint vao, vbo;

    // generate and bind the vao
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // generate and bind the buffer object
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // data for a fullscreen quad
    GLfloat vertexData[] = {
    //  X     Y     Z           R     G     B
       1.0f, 1.0f, 0.0f,       1.0f, 0.0f, 0.0f, // vertex 0
      -1.0f, 1.0f, 0.0f,       0.0f, 1.0f, 0.0f, // vertex 1
       1.0f,-1.0f, 0.0f,       0.0f, 0.0f, 1.0f, // vertex 2
       1.0f,-1.0f, 0.0f,       0.0f, 0.0f, 1.0f, // vertex 3
      -1.0f, 1.0f, 0.0f,       0.0f, 1.0f, 0.0f, // vertex 4
      -1.0f,-1.0f, 0.0f,       1.0f, 0.0f, 0.0f, // vertex 5
    }; // 6 vertices with 6 components (floats) each

    // fill with data
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*6*6, vertexData, GL_STATIC_DRAW);

    // set up generic attrib pointers
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (char*)0 + 0*sizeof(GLfloat));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(GLfloat), (char*)0 + 3*sizeof(GLfloat));

    while(!glwtWindowClosed(window)) {
        glwtEventHandle(0);

        // clear first
        glClear(GL_COLOR_BUFFER_BIT);

        // use the shader program
        glUseProgram(shader_program);

        // bind the vao
        glBindVertexArray(vao);

        // draw
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // check for errors
        GLenum error = glGetError();
        if(error != GL_NO_ERROR) {
            std::cerr << error << std::endl;
            break;
        }

        // finally swap buffers
        glwtSwapBuffers(window);
    }

    // delete the created objects

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

    glDetachShader(shader_program, vertex_shader);
    glDetachShader(shader_program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glDeleteProgram(shader_program);

    glwtWindowDestroy(window);
    glwtQuit();
    return 0;
}

