#include <Renderer/Stats.hpp>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES
#else
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>

namespace VaultRenderer {
    void Statistics::SetStats() {
        vendor = (char *)glGetString(GL_VENDOR);
        vendor = (char *)glGetString(GL_RENDERER);
        version = (char *)glGetString(GL_VERSION);
        shading_language = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
        draw_calls = 0;
    }

    void Statistics::ResetDrawCalls() {
        draw_calls = 0;
    }

    void Statistics::DrawCall() {
        draw_calls++;
    }

    int Statistics::GetDrawCalls() {
        return draw_calls;
    }
} // namespace VaultRenderer