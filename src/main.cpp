
#include <stdbool.h>
#include <stdio.h>
#include "nz6502.h"
#include "nzBUS.h"

#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "NESoo_GUI/NESoo_GUI.hpp"
#include "memory_editor.h"

class MyGUI : public GUI<MyGUI>
{   
    public:
    MyGUI() = default;
    ~MyGUI() = default;

    void SetUp()
    {
        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();
    }
    
    void Update() 
    {
        mem_editor.DrawWindow("Memory Editor",
                              (unsigned char *)busDebug->RAM,
                              sizeof(Byte) * 64 * 1024,
                              0x0000);

        ImGui::Begin("Registers");
        
        ImGui::Text("STATUS FLAGS :");
        ImGui::TextColored(cpuIsActiveFlag(cpuDebug->flag_C) 
                ? enabled_color : disabled_color, "C"); ImGui::SameLine(1 * 20 + 4);
        ImGui::TextColored(cpuIsActiveFlag(cpuDebug->flag_Z)
                ? enabled_color : disabled_color, "Z"); ImGui::SameLine(2 * 20);
        ImGui::TextColored(cpuIsActiveFlag(cpuDebug->flag_I) 
                ? enabled_color : disabled_color, "I"); ImGui::SameLine(3 * 20);
        ImGui::TextColored(cpuIsActiveFlag(cpuDebug->flag_D)
                ? enabled_color : disabled_color, "D"); ImGui::SameLine(4 * 20);
        ImGui::TextColored(cpuIsActiveFlag(cpuDebug->flag_B)
                ? enabled_color : disabled_color, "B"); ImGui::SameLine(5 * 20);
        ImGui::TextColored(cpuIsActiveFlag(cpuDebug->flag_U) 
                ? enabled_color : disabled_color, "~"); ImGui::SameLine(6 * 20);
        ImGui::TextColored(cpuIsActiveFlag(cpuDebug->flag_V)
                ? enabled_color : disabled_color, "V"); ImGui::SameLine(7 * 20);
        ImGui::TextColored(cpuIsActiveFlag(cpuDebug->flag_N)
                ? enabled_color : disabled_color, "N");

        ImGui::Separator();
        ImGui::Text("DATA REGISTERS :");

        cpuDebug->A =  InputHex8("A", cpuDebug->A); ImGui::SameLine(1 * 48 + 4);
        cpuDebug->X =  InputHex8("X", cpuDebug->X); ImGui::SameLine(2 * 48);
        cpuDebug->Y =  InputHex8("Y", cpuDebug->Y); //ImGui::SameLine(3 * 48);
        cpuDebug->PgCount = InputHex16("PC", cpuDebug->PgCount);
        cpuDebug->StkPtr = InputHex8("Stack P.", cpuDebug->StkPtr); //ImGui::SameLine(4 * 48 + 40);
        ImGui::End();

    }

    void connectCpu(CPU* cpu){
        cpuDebug = cpu;
    }

    void connectBus(BUS* bus){
        busDebug = bus;
    }
    private:
    CPU* cpuDebug;
    BUS* busDebug;
    MemoryEditor mem_editor;
    
};
int main(int argc, char *argv[])
{
    CPU newCpu;
    BUS newBus;
    
    cpuCreate(&newCpu);
    cpuConBus(&newBus);
    cpuReset();
    
    cpuWrite(0x00, 0x78);
    cpuSetStaFlag(newCpu.flag_B, true);

    MyGUI myGUI;
    myGUI.connectBus(&newBus);
    myGUI.connectCpu(&newCpu);
    myGUI.Run();

    return 0;
}
