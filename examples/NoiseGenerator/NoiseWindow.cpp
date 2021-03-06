//
//  NoiseWindow.cpp
//  gtf
//
//  Created by David Gallardo on 26/05/16.
//  Copyright (c) 2016 GTF Development Group. All rights reserved.
//

#include "NoiseWindow.h"
#include "NoiseApp.h"

#include <gtf/RHI.h>

#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>


class GradientEditorWindow : public gtf::Window
{
public:
    GradientEditorWindow(class ImGradient* gradient)
    : Window("Color Gradient Editor", 400, 400)
    , m_gradientRef(gradient)
    {
        gtf::GRHI->setClearColor(0.3f, 0.3f, 0.28f, 1.0f);
    }
    virtual void frame(double deltaTime) override
    {
        ImGuiWindowFlags flags = 0;
        flags |= ImGuiWindowFlags_NoCollapse;
        flags |= ImGuiWindowFlags_NoMove;
        flags |= ImGuiWindowFlags_NoResize;
        flags |= ImGuiWindowFlags_NoTitleBar;
        flags |= ImGuiWindowFlags_HorizontalScrollbar;
        
        ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiSetCond_Always);
        ImGui::SetNextWindowSize(ImVec2(m_windowWidth - 40.0f, m_windowHeight - 40.0f), ImGuiSetCond_Always);
        
        ImGui::Begin("Color Gradient Editor", nullptr, flags	);
        //m_updated = GTFGUIGradientPicker::displayEditor(m_gradientRef, m_draggingMark, m_selectedMark);
        m_updated = ImGui::GradientEditor(m_gradientRef, m_draggingMark, m_selectedMark);
        ImGui::End();
        
        gtf::GRHI->viewport(0, 0, m_windowWidth, m_windowHeight);
		gtf::GRHI->clearColorAndDepthBuffers();
    }
    
    virtual bool wantToClose()
    {
		bool wants = gtf::Window::wantToClose();
        if(wants)
        {
			NoiseApp::instance->noiseWindow->nullifyGradientWindow();
            //this->stopClosing();
            //this->setVisible(false);
        }
        
        return wants;
    }
    
    bool updated() const { return m_updated; }
    
private:
    ImGradient* m_gradientRef { nullptr };
    ImGradientMark* m_draggingMark { nullptr };
    ImGradientMark* m_selectedMark { nullptr };
    bool m_updated;
};


NoiseWindow::NoiseWindow(const char* title) : Window(title, 1040, 720)
{
    gtf::GRHI->setClearColor(0.3f, 0.3f, 0.28f, 1.0f);
    m_worker = new GeneratorWorker();
    
    m_texture.reset(gtf::GRHI->createTexture());
    m_texture->setup(gtf::ERHITexInternalFormat::RHI_RGB8, 512, 512, gtf::ERHITexFormat::RHI_RGB, gtf::ERHIValueType::RHI_UNSIGNED_BYTE);
    
    m_info.dirty = true;
    m_worker->update(m_info);
}

NoiseWindow::~NoiseWindow()
{

}

void NoiseWindow::postSetMainInit()
{
    
    
    setVisible(true);
}

void NoiseWindow::frame(double deltaTime)
{

    // Start the frame
    /*
    static bool show_another_window = false;
    static ImVec4 clear_col = ImColor(114, 144, 154);
    
    // 1. Show a simple window
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
    {
        static float f;
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::ColorEdit3("clear color", (float*)&clear_col);
        if (ImGui::Button("Test Window")) show_test_window ^= 1;
        if (ImGui::Button("Another Window")) show_another_window ^= 1;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("Mouse X=%.1f, Y=%.1f", m_mouseCoords[0], m_mouseCoords[1]);
    }
    
    // 2. Show another simple window, this time using an explicit Begin/End pair
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window, ImVec2(200,100));
        ImGui::Text("Hello");
        ImGui::End();
    }
    
     static bool graphOpened = true;
     ShowExampleAppCustomNodeGraph(&graphOpened);*/
     
    // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
    /*static bool show_test_window = true;
    if (show_test_window)
    {
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
        ImGui::ShowTestWindow(&show_test_window);
    }*/
     
    
    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoCollapse;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoResize;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_HorizontalScrollbar;
    
    ImGui::SetNextWindowPos(ImVec2(m_windowWidth - 420.0f, 20.0f), ImGuiSetCond_Always);
    ImGui::SetNextWindowSize(ImVec2(400.0f, m_windowHeight - 40.0f), ImGuiSetCond_Always);
    
    ImGui::Begin("Options", nullptr, flags	);
    {
    
        ImGui::Text("Options");
        ImGui::Separator();
        int seed = m_info.seed;
        static unsigned int resList[] = {128, 256, 512, 1024, 2048};
        static const char* resListStr[] = {"128", "256", "512", "1024", "2048"};
        

        m_info.dirty |= ImGui::Combo("Resolution", &m_currentRes, resListStr, 5);
        m_info.resX = m_info.resY = resList[m_currentRes];
        
        m_info.dirty |= ImGui::InputInt("Seed", &seed);
        m_info.seed = seed;
        m_info.dirty |= ImGui::SliderFloat("PanX", &m_info.panX, -1.0f, 20.0f, "%.2f");
        ImGui::SameLine();
        m_info.dirty |= ImGui::Checkbox("X = Y", &m_info.reestrictPan);
        if(m_info.reestrictPan)
        {
            m_info.panY = m_info.panX;
        }
        else
        {
            m_info.dirty |= ImGui::SliderFloat("PanY", &m_info.panY, -1.0f, 20.0f, "%.2f");
        }
        m_info.dirty |= ImGui::SliderFloat("PanZ", &m_info.panZ, -1.0f, 20.0f, "%.2f");
        m_info.dirty |= ImGui::SliderFloat("Density", &m_info.density, 0.0f, 200.0f, "%.2f");
        
        m_info.dirty |= ImGui::Checkbox("Is Wood", &m_info.wood);
        
        //static bool showPopup = false;
        //static GTFGradientMark* draggingMark = nullptr;
        //static GTFGradientMark* selectedMark = nullptr;
        
      
        /*if(GTFGUIGradientPicker::displayWidget(&m_worker->m_gradient) && m_gradientWindow)
        {
            m_gradientWindow->setVisible(true);
        }*/
        
        if(ImGui::GradientButton(&m_worker->m_gradient))
        {
			m_createGradientEditorWindow = true;
        }
        
        if(m_gradientWindow)
            m_info.dirty |= m_gradientWindow->updated();
        
        
        //m_info.dirty |=
    }
    ImGui::End();
    
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiSetCond_Always);
    ImGui::SetNextWindowSize(ImVec2(m_windowWidth - 460.0f, m_windowHeight - 40.0f), ImGuiSetCond_Always);
    
    if(ImGui::Begin("Texture", nullptr, flags))
    {
        ImGui::Image(reinterpret_cast<ImTextureID>(m_texture->getName()), ImVec2(float(m_info.resX), float(m_info.resY)));
        if(ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(0, 0.0f))
        {
            ImVec2 scrolling = ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
            scrolling = scrolling - ImGui::GetIO().MouseDelta;
            ImGui::SetScrollX(scrolling.x);
            ImGui::SetScrollY(scrolling.y);
        }
        
    }
    ImGui::End();
    
    
    if(m_worker->update(m_info))
    {
        //m_texture->copyFromBuffer(m_info.image);
        m_texture->setup(gtf::ERHITexInternalFormat::RHI_RGB8, m_info.resX, m_info.resY, gtf::ERHITexFormat::RHI_RGB, gtf::ERHIValueType::RHI_UNSIGNED_BYTE, m_info.image);
        delete [] m_info.image;
    }
    
    //ImGui::ShowTestWindow();
    
    // Rendering
    gtf::GRHI->viewport(0, 0, m_windowWidth, m_windowHeight);
    gtf::GRHI->clearColorAndDepthBuffers();
}

void NoiseWindow::postFrame(double deltaTime)
{
	gtf::Window::postFrame(deltaTime);

	if (m_createGradientEditorWindow && !m_gradientWindow)
	{
		m_gradientWindow = new GradientEditorWindow(&m_worker->m_gradient);
		NoiseApp::instance->registerWindow(m_gradientWindow);
		m_createGradientEditorWindow = false;
		m_gradientWindow->setVisible(true);
	}

	
}