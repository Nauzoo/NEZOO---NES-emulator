
#ifndef UTILS_H
#define UTILS_H

/* The following functions were "stolen" from YAKC. Shout out to @floooh at github.com */

#include <stdio.h>
#include "sizebits.h"
#include "imgui.h"

static const int editFlags = ImGuiInputTextFlags_CharsHexadecimal|
    ImGuiInputTextFlags_CharsUppercase|
    ImGuiInputTextFlags_EnterReturnsTrue;

char NibbleToStr(Byte n)
{
    return "0123456789ABCDEF"[n & 0xF];
}

void UByteToStr(Byte b, char *buf, int buf_size)
{
    buf[0] = NibbleToStr(b >> 4);
    buf[1] = NibbleToStr(b);
    buf[2] = 0;
}

void UWordToStr(Word w, char *buf, int bufSize)
{
    UByteToStr(w >> 8, buf, bufSize);
    buf += 2;
    bufSize -= 2;
    UByteToStr(w & 0xFF, buf, bufSize);
}

Byte ParseUByte(const char *str, Byte oldVal)
{
    int res = 0;
    if (sscanf(str, "%X", &res) == 1)
    {
        return (Byte)res;
    }
    else
    {
        return oldVal;
    }
}
Word ParseUWord(const char *str, Word oldVal)
{
    int res = 0;
    if (sscanf(str, "%X", &res) == 1)
    {
        return (Word)res;
    }
    else
    {
        return oldVal;
    }
}

Byte InputHex8(const char *label, Byte val)
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

Word InputHex16(const char *label, Word val)
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

#endif
