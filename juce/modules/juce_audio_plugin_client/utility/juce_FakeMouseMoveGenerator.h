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

namespace juce
{
#if JUCE_MAC

    //==============================================================================
    // Helper class to workaround windows not getting mouse-moves...
    class FakeMouseMoveGenerator : private Timer
    {
    public:
        FakeMouseMoveGenerator()
        {
            startTimer(1000 / 30);
        }

        void timerCallback() override
        {
            // Workaround for windows not getting mouse-moves...
            auto screenPos = Desktop::getInstance().getMainMouseSource().getScreenPosition();

            if (screenPos != lastScreenPos)
            {
                lastScreenPos = screenPos;
                auto mods     = ModifierKeys::getCurrentModifiers();

                if (!mods.isAnyMouseButtonDown())
                {
                    if (auto* comp = Desktop::getInstance().findComponentAt(screenPos.roundToInt()))
                    {
                        safeOldComponent = comp;

                        if (auto* peer = comp->getPeer())
                        {
                            if (!peer->isFocused())
                            {
                                peer->handleMouseEvent(MouseInputSource::InputSourceType::mouse,
                                                       peer->globalToLocal(screenPos), mods,
                                                       MouseInputSource::invalidPressure,
                                                       MouseInputSource::invalidOrientation, Time::currentTimeMillis());
                            }
                        }
                    }
                    else
                    {
                        if (safeOldComponent != nullptr)
                        {
                            if (auto* peer = safeOldComponent->getPeer())
                            {
                                peer->handleMouseEvent(MouseInputSource::InputSourceType::mouse, {-1.0f, -1.0f}, mods,
                                                       MouseInputSource::invalidPressure,
                                                       MouseInputSource::invalidOrientation, Time::currentTimeMillis());
                            }
                        }

                        safeOldComponent = nullptr;
                    }
                }
            }
        }

    private:
        Point<float>             lastScreenPos;
        WeakReference<Component> safeOldComponent;
    };

#else
    struct FakeMouseMoveGenerator
    {
    };
#endif

}  // namespace juce
