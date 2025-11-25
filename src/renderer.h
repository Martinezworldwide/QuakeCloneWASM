// Renderer header - 3D rendering interface
// QuakeCloneWASM - Rendering system

#ifndef RENDERER_H
#define RENDERER_H

#include <stdint.h>

// Initialize renderer with given dimensions
int renderer_init(int width, int height);

// Clear the screen
void renderer_clear(void);

// Present the rendered frame
void renderer_present(void);

// Resize renderer
void renderer_resize(int width, int height);

// Get viewport dimensions
void renderer_get_viewport(int* width, int* height);

// Access the software framebuffer
uint32_t* renderer_get_framebuffer(void);

// Check if GL state is initialized and ready
int renderer_gl_ready(void);

// Shutdown renderer
void renderer_shutdown(void);

#endif // RENDERER_H

