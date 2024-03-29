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
    namespace RelativePointHelpers
    {
        inline void skipComma(String::CharPointerType& s)
        {
            s = s.findEndOfWhitespace();

            if (*s == ',') ++s;
        }
    }  // namespace RelativePointHelpers

    //==============================================================================
    RelativePoint::RelativePoint() {}

    RelativePoint::RelativePoint(Point<float> absolutePoint)
        : x(absolutePoint.x)
        , y(absolutePoint.y)
    {
    }

    RelativePoint::RelativePoint(const float x_, const float y_)
        : x(x_)
        , y(y_)
    {
    }

    RelativePoint::RelativePoint(const RelativeCoordinate& x_, const RelativeCoordinate& y_)
        : x(x_)
        , y(y_)
    {
    }

    RelativePoint::RelativePoint(const String& s)
    {
        String                  error;
        String::CharPointerType text(s.getCharPointer());
        x = RelativeCoordinate(Expression::parse(text, error));
        RelativePointHelpers::skipComma(text);
        y = RelativeCoordinate(Expression::parse(text, error));
    }

    bool RelativePoint::operator==(const RelativePoint& other) const noexcept
    {
        return x == other.x && y == other.y;
    }

    bool RelativePoint::operator!=(const RelativePoint& other) const noexcept
    {
        return !operator==(other);
    }

    Point<float> RelativePoint::resolve(const Expression::Scope* scope) const
    {
        return Point<float>((float)x.resolve(scope), (float)y.resolve(scope));
    }

    void RelativePoint::moveToAbsolute(Point<float> newPos, const Expression::Scope* scope)
    {
        x.moveToAbsolute(newPos.x, scope);
        y.moveToAbsolute(newPos.y, scope);
    }

    String RelativePoint::toString() const
    {
        return x.toString() + ", " + y.toString();
    }

    bool RelativePoint::isDynamic() const
    {
        return x.isDynamic() || y.isDynamic();
    }

}  // namespace juce
