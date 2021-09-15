#pragma once
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stdio.h>
#include <cstdlib>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#ifdef _DEBUG
#pragma comment (lib, "msvcrtd.lib")
#else
#endif

bool show_demo_window = false;
ImVec4 clear_color = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

const char* TITLE = "Kafka chat";

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

bool gQuit = false;
GLFWwindow* window;
void key_callback(GLFWwindow* win, int, int, int, int) {
    gQuit = glfwGetKey(window, GLFW_KEY_ESCAPE);
}

void cleanup()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
};

void setupWindow() {
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        exit(1);
};

const char* glsl_version = "#version 130";
void setGLVersions() {
    // GL 3.0 + GLSL 130
    glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
};

int win_W = 1280;
int win_H = 720;

void resize_callback(GLFWwindow* window, int width, int height)
{
    win_W = width;
    win_H = height;
}

void createWindowWithGraphicalContext() {
    // Create window with graphics context
    window = glfwCreateWindow(win_W, win_H, TITLE, NULL, NULL);
    if (window == NULL)
        exit(1);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, resize_callback);
};

void setupImgui() {

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
};

void setup() {
    setupWindow();
    setGLVersions();
    createWindowWithGraphicalContext();
    setupImgui();
};

void startAndPoll() {
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImVec2 pos = ImVec2(0.0f, 0.0f);
    ImGui::SetNextWindowPos(pos);

    ImVec2 size = ImVec2(static_cast<float>(win_W), static_cast<float>(win_H));
    ImGui::SetNextWindowSize(size);
};

void render() {// Rendering
    ImGui::Render();

    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
};

ImGuiWindowFlags getImguiFlags() {

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoMove;

    return window_flags;
}