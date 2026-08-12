#include <SDL3/SDL.h>
#include <stdio.h>
#include <chrono>
#include "imgui/backends/imgui_impl_opengl3.h"
#include "imgui/backends/imgui_impl_sdl3.h"
#include "imgui/imgui.h"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

namespace {
SDL_Window* window = nullptr;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
SDL_GLContext gl_context = nullptr;
}  // namespace

namespace ImBackends {
bool Init(const char* title, int width, int height) {
    // Setup SDL
    // [If using SDL_MAIN_USE_CALLBACKS: all code below until the main loop starts
    // would likely be your SDL_AppInit() function]
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return 1;
    }

    // Select GL version + let the backend select a GLSL version
    const char* glsl_version = nullptr;
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + generally GLSL 150
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                        SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);  // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + generally GLSL 130
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags =
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    window = SDL_CreateWindow("Dear ImGui SDL3+OpenGL3 example", (int)(1280 * main_scale), (int)(800 * main_scale),
                              window_flags);
    if (window == nullptr) {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return 1;
    }
    gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr) {
        printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);  // Enable vsync
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad
    // Controls io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         //
    // Enable Docking io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       //
    // Enable Multi-Viewport / Platform Windows io.ConfigViewportsNoAutoMerge =
    // true; io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);    // Bake a fixed style scale. (until we have a solution for
                                        // dynamic style scaling, changing this requires resetting
                                        // Style + calling this again)
    style.FontScaleDpi = main_scale;    // Set initial font scale. (in docking branch: using
                                        // io.ConfigDpiScaleFonts=true automatically overrides this
                                        // for every window depending on the current monitor)
    io.ConfigDpiScaleFonts = true;      // [Experimental] Automatically overwrite style.FontScaleDpi in
                                        // Begin() when Monitor DPI changes. This will scale fonts but _NOT_
                                        // scale sizes/padding for now.
    io.ConfigDpiScaleViewports = true;  // [Experimental] Scale Dear ImGui and Platform Windows when Monitor
                                        // DPI changes.

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform
    // windows can look identical to regular ones.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    return true;
}
void NewFrame() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}
void RenderVSync() {
    ImGuiIO& io = ImGui::GetIO();
    // Rendering
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w,
                 clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we
    // save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call SDL_GL_MakeCurrent(window,
    //  gl_context) directly)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }

    SDL_GL_SwapWindow(window);
}

bool WaitNewFrame() {

    if (_g_wantFrameDelay > 0) {
        SDL_WaitEventTimeout(0, _g_wantFrameDelay * 1000.0f);
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        if (event.type == SDL_EVENT_QUIT)
            return false;
        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
            return false;
    }

    // 3. BURST PHASE: Ensure a sequence of frames for UI consistency (inertia)
    {
        static int framesRemaining = 0;

        // Reset burst counter if we are starting a new interaction cycle
        if (framesRemaining <= 0) {
            framesRemaining = 3;
        }

        framesRemaining--;

        // If burst is active, force immediate next frame (0s delay).
        // Once finished, go into deep sleep (100s delay) until next event.
        _g_wantFrameDelay = (framesRemaining > 0) ? 0.0f : 200.0f;
    }

    return true;
}

// bool WaitNewFrame() {
//     auto process = []() {
//         SDL_Event event;
//         while (SDL_PollEvent(&event)) {
//             ImGui_ImplSDL3_ProcessEvent(&event);
//             if (event.type == SDL_EVENT_QUIT)
//                 return false;
//             if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
//                 return false;
//         }
//         return true;
//     };

//     using namespace std::chrono;

//     if (_g_wantFrameDelay <= 0) {
//         // simple check messages
//         if (!process())
//             return false;
//     } else {
//         float toWaitSeconds = _g_wantFrameDelay;
//         do {
//             auto start = steady_clock::now();

//             auto res = SDL_WaitEventTimeout(0, toWaitSeconds * 1000.0f);
//             if (!res) {
//                 break;  // Wake up: requested delay has passed
//             }
//             if (!process())
//                 return false;

//             // Adjust remaining wait time based on time spent processing messages
//             // If DispatchMessage set a shorter delay, respect it
//             duration<float> elapsed = steady_clock::now() - start;
//             toWaitSeconds -= elapsed.count();
//             if (toWaitSeconds > _g_wantFrameDelay) {
//                 // Try to reduce wait time, but not below the remaining minDelay
//                 // threshol
//                 toWaitSeconds = _g_wantFrameDelay;
//             }

//         } while (toWaitSeconds > 0.001f);
//     }

//     // 3. BURST PHASE: Ensure a sequence of frames for UI consistency (inertia)
//     {
//         static int framesRemaining = 0;

//         // Reset burst counter if we are starting a new interaction cycle
//         if (framesRemaining <= 0) {
//             framesRemaining = 3;
//         }

//         framesRemaining--;

//         // If burst is active, force immediate next frame (0s delay).
//         // Once finished, go into deep sleep (100s delay) until next event.
//         _g_wantFrameDelay = (framesRemaining > 0) ? 0.0f : 200.0f;
//     }

//     return true;
// }
void Cleanup() {
    // Cleanup
    // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your
    // SDL_AppQuit() function]
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
};  // namespace ImBackends

// void test() {
//     ImGuiIO& io = ImGui::GetIO();
//     // Main loop
//     bool done = false;
//     while (!done) {
//         auto res = SDL_WaitEventTimeout(0, 100000);
//         SDL_Event event;
//         while (SDL_PollEvent(&event)) {
//             ImGui_ImplSDL3_ProcessEvent(&event);
//             if (event.type == SDL_EVENT_QUIT)
//                 done = true;
//             if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
//                 done = true;
//         }

//         // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your
//         // SDL_AppIterate() function]
//         // if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
//         //     SDL_Delay(10);
//         //     continue;
//         // }

//         // Start the Dear ImGui frame
//         ImGui_ImplOpenGL3_NewFrame();
//         ImGui_ImplSDL3_NewFrame();
//         ImGui::NewFrame();

//         ImGui::Begin("Hello, world!");
//         ImGui::Button("test");
//         ImGui::End();

//         // Rendering
//         ImGui::Render();
//         glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
//         glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w,
//                      clear_color.w);
//         glClear(GL_COLOR_BUFFER_BIT);
//         ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

//         // Update and Render additional Platform Windows
//         // (Platform functions may change the current OpenGL context, so we
//         // save/restore it to make it easier to paste this code elsewhere.
//         //  For this specific demo app we could also call SDL_GL_MakeCurrent(window,
//         //  gl_context) directly)
//         if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
//             SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
//             SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
//             ImGui::UpdatePlatformWindows();
//             ImGui::RenderPlatformWindowsDefault();
//             SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
//         }

//         SDL_GL_SwapWindow(window);
//     }
// }
