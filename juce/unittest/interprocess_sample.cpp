/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "JuceHeader.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <string>
#include <crtdbg.h>

#include <Windows.h>
//==============================================================================
class ConsoleLogger : public Logger
{
    void logMessage(const String& message) override
    {
        std::cout << message << std::endl;

#if JUCE_WINDOWS
        Logger::outputDebugString(message);
#endif
    }
};

//==============================================================================
class ConsoleUnitTestRunner : public UnitTestRunner
{
    void logMessage(const String& message) override
    {
        Logger::writeToLog(message);
    }
};

struct MasterProcess : public ChildProcessMaster, private Thread
{
    MasterProcess()
        : Thread("master_thread")
    {
    }
    ~MasterProcess()
    {
        stopThread(10000);
    }
    void handleMessageFromSlave(const MemoryBlock&) override
    {
        std::cout << "MasterProcess::handleMessageFromSlave" << std::endl;
    };

    void handleConnectionLost() override{};

    void startWork()
    {
        startThread();
    };

    void run() override
    {
        while (!threadShouldExit())
        {
            wait(1000);
            MemoryBlock msg(1);
            bool        ret = sendMessageToSlave(msg);
            std::cout << "MasterProcess::thread sendMessageToSlave ret " << ret << std::endl;

            timeCount++;
            if (timeCount == 10)
            {
                break;
            }
        }

        killSlaveProcess();
        std::cout << "MasterProcess::killSlaveProcess " << std::endl;
        MessageManager::getInstance()->stopDispatchLoop();
        std::cout << "MasterProcess::thread break " << std::endl;
    };

    int timeCount = 0;
};

struct SlaveProcess : public ChildProcessSlave
{
    SlaveProcess() {}
    ~SlaveProcess() {}

    void handleMessageFromMaster(const MemoryBlock&) override
    {
        MemoryBlock msg(1);
        sendMessageToMaster(msg);
        std::cout << "SlaveProcess::handleMessageFromMaster echo back" << std::endl;
    };

    void handleConnectionMade() override
    {
        std::cout << "SlaveProcess::handleConnectionMade" << std::endl;
    };

    void handleConnectionLost() override
    {
        MessageManager::getInstance()->stopDispatchLoop();
    };
};

String getCommandLineParameters(int juce_argc, char* juce_argv[])
{
    String argString;
    for (int i = 1; i < juce_argc; ++i)
    {
        String arg(juce_argv[i]);

        if (arg.containsChar(' ') && !arg.isQuotedString()) arg = arg.quoted('"');

        argString << arg << ' ';
    }
    return argString.trim();
}

int main(int argc, char* argv[])
{
    int  count = 10;
    bool ret   = false;

    String commandLineString = getCommandLineParameters(argc, argv);
    std::cout << commandLineString << std::endl;

    ScopedJuceInitialiser_GUI jucemsg;
    // master
    if (1 == argc)
    {
        std::cout << "MasterProcess::Start " << std::endl;
        ScopedPointer<MasterProcess> master = new MasterProcess();

        File slave_exe = File::getSpecialLocation(File::currentExecutableFile);
        ret            = master->launchSlaveProcess(slave_exe, L"123");
        std::cout << "MasterProcess::launchSlaveProcess " << slave_exe.getFullPathName() << " ret " << ret << std::endl;
        master->startWork();

        std::cout << "MessageManager runDispatchLoop Start" << std::endl;
        MessageManager::getInstance()->runDispatchLoop();
        std::cout << "MessageManager runDispatchLoop End" << std::endl;
    }
    else
    {
        std::cout << "SlaveProcess::Start " << std::endl;
        ScopedPointer<SlaveProcess> slave = new SlaveProcess();
        bool                        ret   = slave->initialiseFromCommandLine(commandLineString, L"123");
        std::cout << "SlaveProcess::initialiseFromCommandLine " << ret << std::endl;

        std::cout << "MessageManager runDispatchLoop Start" << std::endl;
        MessageManager::getInstance()->runDispatchLoop();
        std::cout << "MessageManager runDispatchLoop End" << std::endl;
    }

    return 0;
}