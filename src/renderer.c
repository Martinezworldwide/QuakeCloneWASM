// Renderer implementation - Software raycasting composited through WebGL
// QuakeCloneWASM - Rendering system

#include <emscripten.h>
#include <emscripten/html5.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "renderer.h"

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#else
#ifdef _WIN32
#include <GL/glew.h>
#else
#include <OpenGL/gl3.h>
#endif
#endif

// GPU resources
static EMSCRIPTEN_WEBGL_CONTEXT_HANDLE g_webgl_context = 0;
static GLuint g_shader_program = 0;
static GLuint g_quad_vao = 0;
static GLuint g_quad_vbo = 0;
static GLuint g_scene_texture = 0;

// Software framebuffer
static uint32_t* g_framebuffer = NULL;
static size_t g_framebuffer_capacity = 0;

// Renderer state
static int g_renderer_initialized = 0;
static int g_viewport_width = 800;
static int g_viewport_height = 600;
int g_gl_state_ready = 0; // Exposed for world rendering checks

// Internal helpers
static GLuint compile_shader(GLenum type, const char* source);
static GLuint create_shader_program(void);
static void create_fullscreen_quad(void);
static void ensure_framebuffer_capacity(int width, int height);
static uint32_t pack_color(uint8_t r, uint8_t g, uint8_t b);

// Initialize the renderer and GPU resources
int renderer_init(int width, int height) {
    if (g_renderer_initialized) {
        return 1;
    }

    g_viewport_width = width;
    g_viewport_height = height;

    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.alpha = 0;
    attrs.depth = 0;
    attrs.stencil = 0;
    attrs.antialias = 1;
    attrs.premultipliedAlpha = 0;
    attrs.preserveDrawingBuffer = 0;
    attrs.powerPreference = EM_WEBGL_POWER_PREFERENCE_DEFAULT;
    attrs.failIfMajorPerformanceCaveat = 0;
    attrs.enableExtensionsByDefault = 1;
    attrs.explicitSwapControl = 0;
    attrs.renderViaOffscreenBackBuffer = 0;
    attrs.majorVersion = 2; // Target WebGL2 / GLES3

    g_webgl_context = emscripten_webgl_create_context("#canvas", &attrs);
    if (g_webgl_context < 0) {
        printf("ERROR: Failed to create WebGL context (%lu)\n", (unsigned long)g_webgl_context);
        return 0;
    }

    if (emscripten_webgl_make_context_current(g_webgl_context) != EMSCRIPTEN_RESULT_SUCCESS) {
        printf("ERROR: Failed to make WebGL context current\n");
        return 0;
    }

    g_shader_program = create_shader_program();
    if (!g_shader_program) {
        printf("ERROR: Failed to create shader program\n");
        return 0;
    }

    create_fullscreen_quad();
    ensure_framebuffer_capacity(g_viewport_width, g_viewport_height);

    glGenTextures(1, &g_scene_texture);
    glBindTexture(GL_TEXTURE_2D, g_scene_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, g_viewport_width, g_viewport_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glUseProgram(g_shader_program);
    GLint texture_location = glGetUniformLocation(g_shader_program, "uTexture");
    glUniform1i(texture_location, 0); // Texture unit 0

    glViewport(0, 0, g_viewport_width, g_viewport_height);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.12f, 0.16f, 1.0f);

    g_renderer_initialized = 1;
    g_gl_state_ready = 1;

    printf("Renderer initialized: %dx%d (WebGL2 compositing pipeline)\n", g_viewport_width, g_viewport_height);
    return 1;
}

// Clear the software framebuffer with a base color
void renderer_clear(void) {
    if (!g_renderer_initialized || !g_framebuffer) {
        return;
    }

    uint32_t clear_color = pack_color(20, 22, 28);
    size_t pixel_count = (size_t)g_viewport_width * (size_t)g_viewport_height;
    for (size_t i = 0; i < pixel_count; ++i) {
        g_framebuffer[i] = clear_color;
    }
}

// Upload the software framebuffer to the GPU texture and draw a fullscreen quad
void renderer_present(void) {
    if (!g_renderer_initialized || !g_framebuffer) {
        return;
    }

    emscripten_webgl_make_context_current(g_webgl_context);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_scene_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, g_viewport_width, g_viewport_height, GL_RGBA, GL_UNSIGNED_BYTE, g_framebuffer);

    glViewport(0, 0, g_viewport_width, g_viewport_height);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(g_shader_program);
    glBindVertexArray(g_quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

// Resize framebuffer and texture resources
void renderer_resize(int width, int height) {
    if (!g_renderer_initialized) {
        return;
    }

    g_viewport_width = width;
    g_viewport_height = height;

    ensure_framebuffer_capacity(width, height);

    emscripten_webgl_make_context_current(g_webgl_context);
    glBindTexture(GL_TEXTURE_2D, g_scene_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, g_viewport_width, g_viewport_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glViewport(0, 0, g_viewport_width, g_viewport_height);
}

// Expose viewport dimensions
void renderer_get_viewport(int* width, int* height) {
    if (width) *width = g_viewport_width;
    if (height) *height = g_viewport_height;
}

// Return the software framebuffer pointer
uint32_t* renderer_get_framebuffer(void) {
    return g_framebuffer;
}

// Renderer readiness flag
int renderer_gl_ready(void) {
    return g_gl_state_ready;
}

// Release GPU and CPU resources
void renderer_shutdown(void) {
    if (!g_renderer_initialized) {
        return;
    }

    emscripten_webgl_make_context_current(g_webgl_context);

    if (g_scene_texture) {
        glDeleteTextures(1, &g_scene_texture);
        g_scene_texture = 0;
    }
    if (g_quad_vbo) {
        glDeleteBuffers(1, &g_quad_vbo);
        g_quad_vbo = 0;
    }
    if (g_quad_vao) {
        glDeleteVertexArrays(1, &g_quad_vao);
        g_quad_vao = 0;
    }
    if (g_shader_program) {
        glDeleteProgram(g_shader_program);
        g_shader_program = 0;
    }

    if (g_framebuffer) {
        free(g_framebuffer);
        g_framebuffer = NULL;
        g_framebuffer_capacity = 0;
    }

    if (g_webgl_context) {
        emscripten_webgl_destroy_context(g_webgl_context);
        g_webgl_context = 0;
    }

    g_renderer_initialized = 0;
    g_gl_state_ready = 0;
}

// -------------------------------------------------------------------------
// Internal helpers
// -------------------------------------------------------------------------

static GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint log_length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        if (log_length > 0) {
            char* log = (char*)malloc((size_t)log_length);
            if (log) {
                glGetShaderInfoLog(shader, log_length, NULL, log);
                printf("Shader compile error: %s\n", log);
                free(log);
            }
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint create_shader_program(void) {
    const char* vertex_src =
        "#version 300 es\n"
        "layout (location = 0) in vec2 aPos;\n"
        "layout (location = 1) in vec2 aTex;\n"
        "out vec2 vTex;\n"
        "void main() {\n"
        "    vTex = aTex;\n"
        "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "}\n";

    const char* fragment_src =
        "#version 300 es\n"
        "precision mediump float;\n"
        "in vec2 vTex;\n"
        "layout (location = 0) out vec4 fragColor;\n"
        "uniform sampler2D uTexture;\n"
        "void main() {\n"
        "    fragColor = texture(uTexture, vTex);\n"
        "}\n";

    GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_src);
    if (!vs) {
        return 0;
    }
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_src);
    if (!fs) {
        glDeleteShader(vs);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint log_length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
        if (log_length > 0) {
            char* log = (char*)malloc((size_t)log_length);
            if (log) {
                glGetProgramInfoLog(program, log_length, NULL, log);
                printf("Program link error: %s\n", log);
                free(log);
            }
        }
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

static void create_fullscreen_quad(void) {
    const float quad_vertices[] = {
        // positions   // texcoords
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
    };

    glGenVertexArrays(1, &g_quad_vao);
    glGenBuffers(1, &g_quad_vbo);

    glBindVertexArray(g_quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static void ensure_framebuffer_capacity(int width, int height) {
    size_t required = (size_t)width * (size_t)height;
    if (required > g_framebuffer_capacity) {
        uint32_t* new_buffer = (uint32_t*)realloc(g_framebuffer, required * sizeof(uint32_t));
        if (!new_buffer) {
            printf("ERROR: Failed to allocate framebuffer (%zu pixels)\n", required);
            return;
        }
        g_framebuffer = new_buffer;
        g_framebuffer_capacity = required;
    }
}

static uint32_t pack_color(uint8_t r, uint8_t g, uint8_t b) {
    return 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

