
namespace ImBackends {
bool Init(const char* title, int width, int height);
void NewFrame();
void RenderVSync();
bool WaitNewFrame();
bool Cleanup();


};  // namespace ImBackends