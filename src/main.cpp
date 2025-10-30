#include <cstdio>
#include <cstdlib>
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

        if(disassembledRegion != nullptr){

            bool open = true;
            ImGui::Begin("Disassembler", &open);
            
            for (int i = 0; i < disassembledRegion->size; i++)
            {
                InstructionMap map = disassembledRegion->list[i];
                
                ImGui::Text("$%s : %s %s %s",
                    map.addrString, map.mneumonic, 
                    map.data_argument, map.address_argument
                ); ImGui::SameLine(200);
                ImGui::Text("{%s}", map.addrMode);
    
            }
            ImGui::End();
        }

    }

    void connectCpu(CPU* cpu){
        cpuDebug = cpu;
    }

    void connectBus(BUS* bus){
        busDebug = bus;
    }
    void setupDisassembler(Word initAddr, Word finAddr){
        if (cpuDebug == nullptr){
            disassembledRegion = nullptr;
            return;
        }

        if (disassembledRegion != nullptr)
            free(disassembledRegion);
        disassembledRegion = cupDisassemble(initAddr, finAddr);
    }
    private:
    CPU* cpuDebug;
    BUS* busDebug;
    MemoryEditor mem_editor;
    RegistersEditor registers_editor;
    InstructionList* disassembledRegion = nullptr;

    
};
int main(int argc, char *argv[])
{
    CPU newCpu;
    BUS newBus;
    
    cpuCreate(&newCpu);
    cpuConBus(&newBus);
    cpuReset();
    
    int codeSize = 28;
    Byte code[] = { 0xA2, 0x0A, 0x8E, 0x00,
                    0x00, 0xA2, 0x03, 0x8E,
                    0x01, 0x00, 0xAC, 0x00,
                    0x00, 0xA9, 0x00, 0x18,
                    0x6D, 0x01, 0x00, 0x88,
                    0xD0, 0xFA, 0x8D, 0x02,
                    0x00, 0xEA, 0xEA, 0xEA};
    
Word baseAddr = 0x8000;

    for (int i = 0; i < codeSize; i++)
    {
        cpuWrite(baseAddr + i, code[i]);
    }

    MyGUI myGUI;
    myGUI.connectBus(&newBus);
    myGUI.connectCpu(&newCpu);
    myGUI.setupDisassembler(baseAddr, baseAddr + codeSize);
    myGUI.Run();

    return 0;
}
