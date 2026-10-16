#include "UI.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include "Camera.h"
#include "CameraSettings.h"

UI::~UI()
{
    // 含初始化中途失败的路径；App 保证这里仍有当前 OpenGL 上下文。
    if (openglBackendInitialized) { ImGui_ImplOpenGL3_Shutdown(); }
    if (glfwBackendInitialized) { ImGui_ImplGlfw_Shutdown(); }
    if (contextCreated) { ImGui::DestroyContext(); }
}

bool UI::initialize(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    contextCreated = true;
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    // 官方后端接收所有事件，并串接 App 的失焦回调。
    glfwBackendInitialized = ImGui_ImplGlfw_InitForOpenGL(window, true);
    if (!glfwBackendInitialized)
    {
        std::cout << "ImGui GLFW initialization failed.\n";
        return false;
    }
    openglBackendInitialized = ImGui_ImplOpenGL3_Init("#version 330 core");
    if (!openglBackendInitialized || !ImGui_ImplOpenGL3_CreateDeviceObjects())
    {
        std::cout << "ImGui OpenGL initialization failed.\n";
        return false;
    }
    return true;
}

void UI::beginFrame(bool mouseLookActive)
{
    // 观察时 GLFW 坐标是虚拟累计位置，不能让它命中面板并意外中止观察。
    ImGuiIO& io = ImGui::GetIO();
    if (mouseLookActive) { io.ConfigFlags |= ImGuiConfigFlags_NoMouse; }
    else { io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse; }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

UIRequests UI::draw(CameraSettings& settings, const Camera& camera)
{
    UIRequests requests;
    const ImVec2 display = ImGui::GetIO().DisplaySize;
    const float panelWidth = std::min(300.0f, display.x * 0.4f);

    // 面板独占左侧；在其余区域居中放入完整 3:2 画面。
    viewport.width = std::max(0.0f, display.x - panelWidth);
    viewport.height = viewport.width / settings.aspectRatio();
    if (viewport.height > display.y)
    {
        viewport.height = display.y;
        viewport.width = viewport.height * settings.aspectRatio();
    }
    viewport.x = panelWidth + (display.x - panelWidth - viewport.width) * 0.5f;
    viewport.y = (display.y - viewport.height) * 0.5f;

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, display.y));
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
    if (ImGui::Begin("Focal length", nullptr, flags))
    {
        ImGui::TextUnformatted("Full frame / 36 x 24 mm / 3:2");
        ImGui::Spacing();
        ImGui::TextUnformatted("Focal length (mm)");
        float focalLength = settings.focalLengthMm;
        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::SliderFloat("##focal-length", &focalLength,
            CameraSettings::minFocalLengthMm, CameraSettings::maxFocalLengthMm,
            "%.1f mm", ImGuiSliderFlags_AlwaysClamp))
        {
            settings.setFocalLengthMm(focalLength);
        }

        // 窄窗口时改为两列；选中状态直接由参数值推导。
        const float presets[] = {24.0f, 35.0f, 50.0f, 85.0f};
        const char* labels[] = {"24 mm", "35 mm", "50 mm", "85 mm"};
        const int columns = ImGui::GetContentRegionAvail().x >= 250.0f ? 4 : 2;
        const float buttonWidth = (ImGui::GetContentRegionAvail().x
            - ImGui::GetStyle().ItemSpacing.x * (columns - 1)) / columns;
        for (int i = 0; i < 4; ++i)
        {
            if (i % columns != 0) { ImGui::SameLine(); }
            const bool selected = std::abs(settings.focalLengthMm - presets[i]) < 0.01f;
            if (selected)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            }
            if (ImGui::Button(labels[i], ImVec2(buttonWidth, 0.0f)))
            {
                settings.setFocalLengthMm(presets[i]);
            }
            if (selected) { ImGui::PopStyleColor(); }
        }
        ImGui::Text("Vertical FOV: %.2f deg", glm::degrees(settings.verticalFovRadians()));
        ImGui::TextWrapped("Longer focal length narrows the view. Zoom keeps the camera in place.");

        ImGui::Separator();
        ImGui::TextUnformatted("Camera position (m)");
        const glm::vec3& position = camera.getPosition();
        ImGui::Text("X %.2f   Y %.2f   Z %.2f", position.x, position.y, position.z);
        requests.resetPose = ImGui::Button("Reset pose (R)", ImVec2(-1.0f, 0.0f));
        requests.resetFocalLength = ImGui::Button("Reset focal length (50 mm)", ImVec2(-1.0f, 0.0f));

        ImGui::Separator();
        ImGui::TextUnformatted("Experiment A: zoom and movement");
        requests.resetExperiment = ImGui::Button("Reset experiment A (24 mm)", ImVec2(-1.0f, 0.0f));
        ImGui::TextWrapped("Reset, compare 24 and 85 mm at the same position, then move to compare perspective.");

        ImGui::Separator();
        ImGui::TextWrapped("Viewfinder: WASD move, Q/E height, hold RMB to look, wheel to zoom, Esc to exit.");
        ImGui::TextWrapped("Ctrl+click the slider to enter an exact focal length.");
    }
    ImGui::End();
    return requests;
}

void UI::render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool UI::capturesMouse() const
{
    return ImGui::GetIO().WantCaptureMouse || ImGui::IsAnyItemActive();
}

bool UI::capturesKeyboard() const
{
    const ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureKeyboard || io.WantTextInput || ImGui::IsAnyItemActive();
}

bool UI::rightMousePressed() const
{
    return ImGui::IsMouseClicked(ImGuiMouseButton_Right);
}

bool UI::resetPosePressed() const
{
    return ImGui::IsKeyPressed(ImGuiKey_R, false);
}

float UI::mouseWheel() const
{
    return ImGui::GetIO().MouseWheel;
}
