#ifndef REGISTERS_EDITOR_H
#define REGISTERS_EDITOR_H

#include "imgui.h"
#include "nz6502.h"
#include "colors.h"
#include "utils.hpp"

class RegistersEditor {
    
    public:

    void DrawWindow(const char* title, CPU* cpu)
    {
        if (cpu == nullptr) return;
        
        bool Open = true;
        ImGui::Begin(title, &Open);
        
        ImGui::Text("STATUS FLAGS :");
        ImGui::TextColored(cpuIsActiveFlag(cpu->flag_C) 
                ? enabled_color : disabled_color, "C"); ImGui::SameLine(1 * 20 + 4);
        ImGui::TextColored(cpuIsActiveFlag(cpu->flag_Z)
                ? enabled_color : disabled_color, "Z"); ImGui::SameLine(2 * 20);
        ImGui::TextColored(cpuIsActiveFlag(cpu->flag_I) 
                ? enabled_color : disabled_color, "I"); ImGui::SameLine(3 * 20);
        ImGui::TextColored(cpuIsActiveFlag(cpu->flag_D)
                ? enabled_color : disabled_color, "D"); ImGui::SameLine(4 * 20);
        ImGui::TextColored(cpuIsActiveFlag(cpu->flag_B)
                ? enabled_color : disabled_color, "B"); ImGui::SameLine(5 * 20);
        ImGui::TextColored(cpuIsActiveFlag(cpu->flag_U) 
                ? enabled_color : disabled_color, "~"); ImGui::SameLine(6 * 20);
        ImGui::TextColored(cpuIsActiveFlag(cpu->flag_V)
                ? enabled_color : disabled_color, "V"); ImGui::SameLine(7 * 20);
        ImGui::TextColored(cpuIsActiveFlag(cpu->flag_N)
                ? enabled_color : disabled_color, "N");

        ImGui::Separator();
        ImGui::Text("DATA REGISTERS :");

        cpu->A =  InputHex8("A", cpu->A); ImGui::SameLine(1 * 48 + 4);
        cpu->X =  InputHex8("X", cpu->X); ImGui::SameLine(2 * 48);
        cpu->Y =  InputHex8("Y", cpu->Y); //ImGui::SameLine(3 * 48);
        cpu->PgCount = InputHex16("PC", cpu->PgCount);
        cpu->StkPtr = InputHex8("Stack P.", cpu->StkPtr); //ImGui::SameLine(4 * 48 + 40);
        ImGui::End();
        
    }
};

#endif