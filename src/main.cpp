
#include <stdbool.h>
#include <stdio.h>
#include "nz6502.h"
#include "nzBUS.h"

#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "UI/UI_template.hpp"
#include "UI/memory_editor.h"
#include "UI/registers_editor.hpp"

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
        
        registers_editor.DrawWindow("Registers Editor", cpuDebug);

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
    RegistersEditor registers_editor;

    
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
