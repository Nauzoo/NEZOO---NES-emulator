
// INTERFACE ABSTRACTION FOR THE EMULATOR DEBUGGER AND GAME WINDOW

#include <stdio.h>
#include <cstdlib>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include "GLFW/glfw3.h" // Will drag system OpenGL headers

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif


static const int editFlags = ImGuiInputTextFlags_CharsHexadecimal|
    ImGuiInputTextFlags_CharsUppercase|
    ImGuiInputTextFlags_EnterReturnsTrue;
    
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

template<typename Derived>
class GUI 
{
    public:
    GUI()
    {
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            std::exit(1);

            // Decide GL+GLSL versions
        #if defined(IMGUI_IMPL_OPENGL_ES2)
            // GL ES 2.0 + GLSL 100
            const char* glsl_version = "#version 100";
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        #elif defined(__APPLE__)
            // GL 3.2 + GLSL 150
            const char* glsl_version = "#version 150";
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
        #else
            // GL 3.0 + GLSL 130
            const char* glsl_version = "#version 130";
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
            //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
            //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
        #endif

            // Create window with graphics context
            window = glfwCreateWindow(1280, 720, "NESoo - EMULATOR", nullptr, nullptr);
            if (window == nullptr)
                std::exit(1);
            glfwMakeContextCurrent(window);
            glfwSwapInterval(1); // Enable vsync

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls


            // Setup Platform/Renderer backends
            ImGui_ImplGlfw_InitForOpenGL(window, true);
        #ifdef __EMSCRIPTEN__
            ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
        #endif
            ImGui_ImplOpenGL3_Init(glsl_version);
    };
    ~GUI()
    {
        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    };

    void Run()
    {
        SetUp();

        glfwPollEvents();

        
        while(!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            
            Update();

            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }
    }
    void SetUp()
    {
        static_cast<Derived*>(this)->SetUp();
    }
    virtual void Update()
    {
        static_cast<Derived*>(this)->Update();
    }

    private:
    GLFWwindow* window;
    
    protected:
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    ImVec4 disabled_color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    ImVec4 enabled_color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);


    /* The following functions were stolen from YACK. Shout out to  */

    char NibbleToStr(uint8_t n) {
        return "0123456789ABCDEF"[n & 0xF];
    }

    void UByteToStr(uint8_t b, char *buf, int buf_size)
    {
        buf[0] = NibbleToStr(b >> 4);
        buf[1] = NibbleToStr(b);
        buf[2] = 0;
    }

    
    void UWordToStr(uint16_t w, char *buf, int bufSize)
    {
        UByteToStr(w >> 8, buf, bufSize);
        buf += 2;
        bufSize -= 2;
        UByteToStr(w & 0xFF, buf, bufSize);
    }

    uint8_t ParseUByte(const char *str, uint8_t oldVal)
    {
        int res = 0;
        if (sscanf(str, "%X", &res) == 1)
        {
            return (uint8_t)res;
        }
        else
        {
            return oldVal;
        }
    }
    uint16_t ParseUWord(const char *str, uint16_t oldVal)
    {
        int res = 0;
        if (sscanf(str, "%X", &res) == 1)
        {
            return (uint16_t)res;
        }
        else
        {
            return oldVal;
        }
    }
    
    uint8_t InputHex8(const char *label, uint8_t val)
    {
        static char buf[3];
        UByteToStr(val, buf, sizeof(buf));
        ImGui::PushItemWidth(22);
        if (ImGui::InputText(label, buf, sizeof(buf), editFlags))
        {
            val = ParseUByte(buf, val);
        }
        ImGui::PopItemWidth();
        return val;
    }

    InputHex16(const char *label, uint16_t val)
    {
        static char buf[5];
        UWordToStr(val, buf, sizeof(buf));
        ImGui::PushItemWidth(38);
        if (ImGui::InputText(label, buf, sizeof(buf), editFlags))
        {
            val = ParseUWord(buf, val);
        }
        ImGui::PopItemWidth();
        return val;
    }
};