// Input header - Keyboard and mouse input handling
// QuakeCloneWASM - Input system

#ifndef INPUT_H
#define INPUT_H

// Initialize input system
int input_init(void);

// Update input state (call each frame)
void input_update(void);

// Check if key is currently down
int input_is_key_down(int key_code);

// Check if key was just pressed this frame
int input_is_key_pressed(int key_code);

// Get mouse delta (movement since last frame)
void input_get_mouse_delta(float* dx, float* dy);

// Set mouse position (for centering)
void input_set_mouse_position(int x, int y);

// Shutdown input system
void input_shutdown(void);

#endif // INPUT_H

