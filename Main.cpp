//System includes
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <time.h>
#include <sys/stat.h>
#include <chrono>
#include <cista.h>

//My includes
#include "UtilityFunctions.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_opengl3.h"
#include "ImGui/imgui_impl_glfw.h"
#include "implot.h"
#include "SimGrid.h"
#include <glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Cell.h"
#include "Agent.h"
#include "Brain.h"
#include "Globals.h"
#include "Neurons.h"

namespace data = cista::offset;
using namespace std;
using namespace utility;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

//Handles mouse inputs for cell selection
void ProcessMouseInputs(GLFWwindow* window, int button, int action, int mods)
{
    SimGrid* SimulationGrid = static_cast<SimGrid*>(glfwGetWindowUserPointer(window));

    glfwGetCursorPos(window, &gCursorPosX, &gCursorPosY);
    int windSizeX;
    int windSizeY;
    float offsetX;
    float offsetY;
    SimulationGrid->GetOffset(offsetX, offsetY);
    float zoom = SimulationGrid->GetZoom();

    glfwGetWindowSize(window, &windSizeX, &windSizeY);

    //Mouse position in texture space
    float mousePosX = ((gCursorPosX / windSizeX) * 2) - 1.0f;
    float mousePosY = (2.0f - ((gCursorPosY / windSizeY) * 2)) - 1.0f;

    mousePosX /= zoom;
    mousePosY /= zoom;

    mousePosX -= offsetX;
    mousePosY -= offsetY;

    gSelectedCellX = (SimulationGrid->GetGridSize()/2) + (mousePosX * (SimulationGrid->GetGridSize() /2));
    gSelectedCellY = (SimulationGrid->GetGridSize() /2) + (mousePosY * (SimulationGrid->GetGridSize() /2));

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        SimulationGrid->mSelectedCell = SimulationGrid->GetCell(gSelectedCellX, gSelectedCellY);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        SimulationGrid->mSelectedAgent = SimulationGrid->GetAgentAtCell(gSelectedCellX, gSelectedCellY);
    }
}

void ProcessMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
    SimGrid* SimulationGrid = static_cast<SimGrid*>(glfwGetWindowUserPointer(window));

    if (SimulationGrid != nullptr)
    {
        SimulationGrid->ZoomIn(yoffset);
    }
}

//Handles keyboard inputs
void ProcessKeyboardInputs(GLFWwindow* window, SimGrid& SimulationGrid)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        SimulationGrid.ScrollDown(-1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        SimulationGrid.ScrollDown(1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        SimulationGrid.ScrollRight(-1.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        SimulationGrid.ScrollRight(1.0f);
    }
}

//Spawns an agent at a random location
void SpawnAgent(SimGrid& SimulationGrid)
{
    SimulationGrid.AddAgent(RandomNumber(0, SimulationGrid.GetGridSize() - 1), RandomNumber(0, SimulationGrid.GetGridSize() - 1));
}

void RenderWindow(
    GLFWwindow* window,
    GLFWwindow* UIwindow,
    ImGuiIO& io, 
    ImVec4 clearColour,
    SimGrid& SimulationGrid)
{
    //Rendering

    // Prepping ImGui stuff for render
    // Rendering
    glfwMakeContextCurrent(UIwindow);
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();   
    
    ImGui::Begin("ArtiEco Control Panel");                          

    if (ImGui::Button("Begin Simulation"))
    {
        glfwMakeContextCurrent(window);
        if (!SimulationGrid.Initialized)
        {
            SimulationGrid.InitGrid(SimulationGrid.GetGridSize(), SimulationGrid.GetGridSize(), -1.0f, -1.0f);
        }
        if(!SimulationGrid.ShaderLoaded)
        {
            SimulationGrid.LoadShader();
        }
        glfwSwapBuffers(window);
        glfwMakeContextCurrent(UIwindow);
    }
    if (ImGui::Button("Reset Simulation"))
    {
        SimulationGrid.ResetGrid();
    }
    SimulationGrid.PrintGridControls();
    if (ImGui::Button("Spawn Agent"))
    {
        SpawnAgent(SimulationGrid);
    }
    if (ImGui::Checkbox("Auto Select Oldest Agent", &SimulationGrid.mAutoSelectOldestAgent))
    {
        SimulationGrid.mAutoSelectHighestEnergyGain = false;
        SimulationGrid.mAutoSelectHighestEfficiency = false;
    }
    if (ImGui::Checkbox("Auto Select Highest Energy Agent", &SimulationGrid.mAutoSelectHighestEnergyGain))
    {
        SimulationGrid.mAutoSelectOldestAgent = false;
        SimulationGrid.mAutoSelectHighestEfficiency = false;
    }
    if (ImGui::Checkbox("Auto Select Highest Efficiency Agent", &SimulationGrid.mAutoSelectHighestEfficiency))
    {
        SimulationGrid.mAutoSelectHighestEnergyGain = false;
        SimulationGrid.mAutoSelectOldestAgent = false;
    }
    //General grid info
    if (ImGui::CollapsingHeader("Grid Data"))
    {
        ImGui::Text("Alive Agents:           %i", SimulationGrid.mAliveAgents);
        ImGui::Text("Unique Species:         %i", SimulationGrid.GetNumberOfSpecies());
        ImGui::Text("Longest Time Alive:     %.2f", SimulationGrid.mLongestTimeAlive);
        ImGui::Text("Highest Energy Gained:  %.2f", SimulationGrid.mHighestEnergyGained);
        ImGui::Text("Highest Efficiency:     %.2f", SimulationGrid.mHighestEfficiency);
        ImGui::Text("Highest Generation:     %i", SimulationGrid.mHighestGeneration);
        ImGui::Text("Average Learning Rate:  %.2f", SimulationGrid.mAverageLearningRate);
        ImGui::Text("Top Agents Efficiency:  %.2f", SimulationGrid.mAverageEfficiencyOfTopAgents);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::Text("Cursor Position: %i, %i", gSelectedCellX, gSelectedCellY);
    }
    if (ImGui::CollapsingHeader("Graphs"))
    {
        SimulationGrid.RenderGraphs();
    }
    //Cell data
    if (SimulationGrid.mSelectedCell != nullptr)
    {
        SimulationGrid.PrintCellData();
    }
    //Agent data
    if (SimulationGrid.mSelectedAgent != nullptr)
    {
        SimulationGrid.RenderNeuralNetwork();
        SimulationGrid.PrintAgentData();
    }

    ImGui::End();
    //ImPlot::ShowDemoWindow();
    
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(UIwindow, &display_w, &display_h);
    //glViewport(0, 0, display_w, display_h);
    glClearColor(clearColour.x * clearColour.w, clearColour.y * clearColour.w, clearColour.z * clearColour.w, clearColour.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(UIwindow);

    //Render the grid
    glfwMakeContextCurrent(window);
    glClearColor(clearColour.x * clearColour.w, clearColour.y * clearColour.w, clearColour.z * clearColour.w, clearColour.w);
    glClear(GL_COLOR_BUFFER_BIT);
    if (SimulationGrid.Initialized)
    {
        SimulationGrid.RenderGrid();
    }

    //Check and call events and swap the buffers
    glfwSwapBuffers(window);
}

int main(int argc, char* argv[])
{
    gRandomNumberGen.seed(gInitSeed);
    glfwInit();

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_FLOATING, 1);
    GLFWwindow* window = glfwCreateWindow(700, 700, "ArtiEco", NULL, NULL);
    glfwWindowHint(GLFW_FLOATING, 0);
    GLFWwindow* UIWindow = glfwCreateWindow(1920, 1080, "UI", NULL, NULL);

    glfwSetWindowAspectRatio(window, 1, 1);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetFramebufferSizeCallback(UIWindow, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, 700, 700);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(UIWindow, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    //Setting up grid and populating with a bunch of agents
    SimGrid SimulationGrid;

    std::chrono::steady_clock::time_point lastUpdate;
    float deltaTime;
    auto now = std::chrono::steady_clock::now();

    //Set up inputs
    glfwSetWindowUserPointer(window, &SimulationGrid);
    glfwSetMouseButtonCallback(window, ProcessMouseInputs);
    glfwSetScrollCallback(window, ProcessMouseScroll);

    gRandomNumberGen.seed(gInitSeed);
    

    while (!glfwWindowShouldClose(window) && !glfwWindowShouldClose(UIWindow))
    {
        deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - lastUpdate).count() / 1000000.0f;
        //Input
        ProcessKeyboardInputs(window, SimulationGrid);

        if (!SimulationGrid.mIsPaused && SimulationGrid.Initialized)
        {
            SimulationGrid.Update(deltaTime);
        }

        RenderWindow(
            window,
            UIWindow,
            io,
            ImVec4(0.45f, 0.55f, 0.60f, 1.00f),
            SimulationGrid);

        glfwPollEvents();

        lastUpdate = now;
        now = std::chrono::steady_clock::now();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwDestroyWindow(UIWindow);
    glfwTerminate();
    return 0;
}