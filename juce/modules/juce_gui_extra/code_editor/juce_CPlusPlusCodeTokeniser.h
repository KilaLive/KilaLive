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
    //==============================================================================
    /**
        A simple lexical analyser for syntax colouring of C++ code.

        @see CodeEditorComponent, CodeDocument
    */
    class JUCE_API CPlusPlusCodeTokeniser : public CodeTokeniser
    {
    public:
        //==============================================================================
        CPlusPlusCodeTokeniser();
        ~CPlusPlusCodeTokeniser();

        //==============================================================================
        int                               readNextToken(CodeDocument::Iterator&) override;
        CodeEditorComponent::ColourScheme getDefaultColourScheme() override;

        /** This is a handy method for checking whether a string is a c++ reserved keyword. */
        static bool isReservedKeyword(const String& token) noexcept;

        /** The token values returned by this tokeniser. */
        enum TokenType
        {
            tokenType_error = 0,
            tokenType_comment,
            tokenType_keyword,
            tokenType_operator,
            tokenType_identifier,
            tokenType_integer,
            tokenType_float,
            tokenType_string,
            tokenType_bracket,
            tokenType_punctuation,
            tokenType_preprocessor
        };

    private:
        //==============================================================================
        JUCE_LEAK_DETECTOR(CPlusPlusCodeTokeniser)
    };

}  // namespace juce
