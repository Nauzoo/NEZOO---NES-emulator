#ifndef DISASSEMBER_H
#define DISASSEMBER_H

#include "imgui.h"
#include "sizebits.h"
#include "nz6502.h"
#include <cstdlib>
#include "utils.hpp"
#include "colors.h"

class Disassembler {

    public:
    Word initAddr = 0x8000;
    Word finaAddr = 0x801b;
    
    void DrawWindow(const char* title) {
        
        bool open = true;
        ImGui::Begin("Disassembler", &open);

        if(disassembledRegion != nullptr)
        {


            ImVec2 contentSize = ImGui::GetContentRegionAvail();
            const float footerHeight = 40.0f;

            
            ImGui::BeginChild("DisassemblerList", ImVec2(0, contentSize.y - footerHeight), true, ImGuiWindowFlags_HorizontalScrollbar);
            
            
            for (int i = 0; i < disassembledRegion->size; i++)
            {
                InstructionMap map = disassembledRegion->list[i];
                bool isCurrent = (map.address == cpuDebug->PgCount);

                ImVec2 start = ImGui::GetCursorScreenPos(); // início da linha
                float lineHeight = ImGui::GetTextLineHeightWithSpacing();

                if (isCurrent)
                {
                    ImDrawList *drawList = ImGui::GetWindowDrawList();
                    drawList->AddRectFilled(
                        ImVec2(start.x, start.y),
                        ImVec2(start.x + ImGui::GetContentRegionAvail().x + ImGui::GetStyle().ItemSpacing.x * 2, start.y + lineHeight),
                        selection_color
                    );
                }

                ImGui::Text("$%s : %s %s %s",
                            map.addrString, map.mneumonic,
                            map.data_argument, map.address_argument);
                ImGui::SameLine(200);
                ImGui::Text("{%s}", map.addrMode);

                if (isCurrent)
                    ImGui::PopStyleColor();
            }

            ImGui::EndChild();
            ImGui::Separator();
            
            if (ImGui::Button("Disassemble"))
            {
                setupDisassembler(cpuDebug, initAddr, finaAddr);
            } ImGui::SameLine(100);

            initAddr = InputHex16("...", initAddr); ImGui::SameLine(170);
            finaAddr = InputHex16(".", finaAddr);
            ImGui::SameLine(230);
            if (ImGui::Button(">>"))
            {
                cpuClock();
                while (cpuDebug->cycles > 0)
                {
                    cpuClock();
                }
            }


        }
        else 
        {
            ImGui::Text("Please, setup this debuiging tool by using:");
            ImGui::Text("'setupDisassember(CPU* cpu, Word initAddr, Word finAddr)'");
        }
        ImGui::End();
    }
    void setupDisassembler(CPU* cpu, Word initAddr, Word finAddr){
        
        cpuDebug = cpu;

        if (cpuDebug == nullptr)
        {
            disassembledRegion = nullptr;
            return;
        }

        if (disassembledRegion != nullptr)
        {
            free(disassembledRegion);
        }
        disassembledRegion = cupDisassemble(initAddr, finAddr);
    }

    private:
    InstructionList* disassembledRegion = nullptr;
    CPU* cpuDebug;
};

#endif