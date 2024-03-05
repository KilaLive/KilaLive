
#include "windows.h"
#include "JuceConfig.h"
#include <juce_core/juce_core.h>

#if _MSC_VER
extern "C" BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH) juce::Process::setCurrentModuleInstanceHandle(instance);

    return true;
}
#endif
