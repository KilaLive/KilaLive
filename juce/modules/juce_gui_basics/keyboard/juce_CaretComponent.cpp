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
    CaretComponent::CaretComponent(Component* const keyFocusOwner)
        : owner(keyFocusOwner)
    {
        setPaintingIsUnclipped(true);
        setInterceptsMouseClicks(false, false);
    }

    CaretComponent::~CaretComponent() {}

    void CaretComponent::paint(Graphics& g)
    {
        g.setColour(findColour(caretColourId, true));
        g.fillRect(getLocalBounds());
    }

    void CaretComponent::timerCallback()
    {
        setVisible(shouldBeShown() && !isVisible());
    }

    void CaretComponent::setCaretPosition(const Rectangle<int>& characterArea)
    {
        startTimer(380);
        setVisible(shouldBeShown());
        setBounds(characterArea.withWidth(2));
    }

    bool CaretComponent::shouldBeShown() const
    {
        return owner == nullptr || (owner->hasKeyboardFocus(false) && !owner->isCurrentlyBlockedByAnotherModalComponent());
    }

}  // namespace juce
