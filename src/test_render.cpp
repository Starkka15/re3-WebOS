// Simple GLES 2.0 rendering test for webOS TouchPad
// This bypasses all of librw to test if basic rendering works

#ifdef WEBOS_TOUCHPAD

#include <GLES2/gl2.h>
#include <stdio.h>

// Simplest possible vertex shader
const char* test_vertex_shader =
    "#version 100\n"
    "attribute vec2 position;\n"
    "void main() {\n"
    "    gl_Position = vec4(position, 0.0, 1.0);\n"
    "}\n";

// Simplest possible fragment shader - solid red
const char* test_fragment_shader =
    "#version 100\n"
    "precision mediump float;\n"
    "void main() {\n"
    "    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";

GLuint compile_test_shader(GLenum type, const char* source) {
    FILE* log = NULL /* logging disabled */;

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (log) {
        fprintf(log, "TEST: Compiling %s shader... %s\n",
                type == GL_VERTEX_SHADER ? "vertex" : "fragment",
                success ? "SUCCESS" : "FAILED");

        if (!success) {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            fprintf(log, "TEST: Shader error: %s\n", infoLog);
        }
        fclose(log);
    }

    return success ? shader : 0;
}

GLuint create_test_program() {
    FILE* log = NULL /* logging disabled */;
    if (log) {
        fprintf(log, "TEST: Creating minimal test shader program...\n");
        fclose(log);
    }

    GLuint vs = compile_test_shader(GL_VERTEX_SHADER, test_vertex_shader);
    GLuint fs = compile_test_shader(GL_FRAGMENT_SHADER, test_fragment_shader);

    if (!vs || !fs) return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    log = NULL /* logging disabled */;
    if (log) {
        fprintf(log, "TEST: Linking shader program... %s\n", success ? "SUCCESS" : "FAILED");
        fclose(log);
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return success ? program : 0;
}

void test_simple_triangle() {
    FILE* log = NULL /* logging disabled */;
    if (log) {
        fprintf(log, "\n=== SIMPLE TRIANGLE TEST START ===\n");
        fclose(log);
    }

    // Create shader program
    GLuint program = create_test_program();
    if (!program) {
        log = NULL /* logging disabled */;
        if (log) {
            fprintf(log, "TEST: Failed to create shader program!\n");
            fclose(log);
        }
        return;
    }

    // Simple triangle vertices (NDC coordinates, no transformation needed)
    GLfloat vertices[] = {
         0.0f,  0.5f,  // Top
        -0.5f, -0.5f,  // Bottom left
         0.5f, -0.5f   // Bottom right
    };

    // Create VBO
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    log = NULL /* logging disabled */;
    if (log) {
        fprintf(log, "TEST: Created VBO %u with triangle data\n", vbo);
        fclose(log);
    }

    // Clear screen to blue so we can see if red triangle appears
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Use shader and draw
    glUseProgram(program);

    GLint posAttrib = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(posAttrib);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

    log = NULL /* logging disabled */;
    if (log) {
        fprintf(log, "TEST: Drawing triangle... position attrib = %d\n", posAttrib);
        fclose(log);
    }

    glDrawArrays(GL_TRIANGLES, 0, 3);

    GLenum err = glGetError();
    log = NULL /* logging disabled */;
    if (log) {
        if (err != GL_NO_ERROR) {
            fprintf(log, "TEST: GL ERROR after draw: 0x%x\n", err);
        } else {
            fprintf(log, "TEST: Draw completed with no errors\n");
            fprintf(log, "TEST: EXPECTED RESULT: Blue screen with red triangle in center\n");
        }
        fprintf(log, "=== SIMPLE TRIANGLE TEST END ===\n\n");
        fclose(log);
    }

    // Cleanup
    glDisableVertexAttribArray(posAttrib);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(program);
}

#endif // WEBOS_TOUCHPAD
