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
#define JUCE_DECLARE_VST3_COM_REF_METHODS                                                                                    \
    Steinberg::uint32 PLUGIN_API addRef() override                                                                           \
    {                                                                                                                        \
        return (Steinberg::uint32)++refCount;                                                                                \
    }                                                                                                                        \
    Steinberg::uint32 PLUGIN_API release() override                                                                          \
    {                                                                                                                        \
        const int r = --refCount;                                                                                            \
        if (r == 0) delete this;                                                                                             \
        return (Steinberg::uint32)r;                                                                                         \
    }

#define JUCE_DECLARE_VST3_COM_QUERY_METHODS                                                                                  \
    Steinberg::tresult PLUGIN_API queryInterface(const Steinberg::TUID, void** obj) override                                 \
    {                                                                                                                        \
        jassertfalse;                                                                                                        \
        *obj = nullptr;                                                                                                      \
        return Steinberg::kNotImplemented;                                                                                   \
    }

    static bool doUIDsMatch(const Steinberg::TUID a, const Steinberg::TUID b) noexcept
    {
        return std::memcmp(a, b, sizeof(Steinberg::TUID)) == 0;
    }

#define TEST_FOR_AND_RETURN_IF_VALID(iidToTest, ClassType)                                                                   \
    if (doUIDsMatch(iidToTest, ClassType::iid))                                                                              \
    {                                                                                                                        \
        addRef();                                                                                                            \
        *obj = dynamic_cast<ClassType*>(this);                                                                               \
        return Steinberg::kResultOk;                                                                                         \
    }

#define TEST_FOR_COMMON_BASE_AND_RETURN_IF_VALID(iidToTest, CommonClassType, SourceClassType)                                \
    if (doUIDsMatch(iidToTest, CommonClassType::iid))                                                                        \
    {                                                                                                                        \
        addRef();                                                                                                            \
        *obj = (CommonClassType*)static_cast<SourceClassType*>(this);                                                        \
        return Steinberg::kResultOk;                                                                                         \
    }

//==============================================================================
#if VST_VERSION < 0x030608
#define kAmbi1stOrderACN kBFormat
#endif

    //==============================================================================
    inline juce::String toString(const Steinberg::char8* string) noexcept
    {
        return juce::String(string);
    }
    inline juce::String toString(const Steinberg::char16* string) noexcept
    {
        return juce::String(juce::CharPointer_UTF16((juce::CharPointer_UTF16::CharType*)string));
    }

    // NB: The casts are handled by a Steinberg::UString operator
    inline juce::String toString(const Steinberg::UString128& string) noexcept
    {
        return toString(static_cast<const Steinberg::char16*>(string));
    }
    inline juce::String toString(const Steinberg::UString256& string) noexcept
    {
        return toString(static_cast<const Steinberg::char16*>(string));
    }

    inline void toString128(Steinberg::Vst::String128 result, const char* source)
    {
        Steinberg::UString(result, 128).fromAscii(source);
    }

    inline void toString128(Steinberg::Vst::String128 result, const juce::String& source)
    {
        Steinberg::UString(result, 128).fromAscii(source.toUTF8());
    }

    inline Steinberg::Vst::TChar* toString(const juce::String& source) noexcept
    {
        return reinterpret_cast<Steinberg::Vst::TChar*>(source.toUTF16().getAddress());
    }

#if JUCE_WINDOWS
    static const Steinberg::FIDString defaultVST3WindowType = Steinberg::kPlatformTypeHWND;
#else
    static const Steinberg::FIDString defaultVST3WindowType = Steinberg::kPlatformTypeNSView;
#endif

    //==============================================================================
    static inline Steinberg::Vst::SpeakerArrangement getArrangementForBus(Steinberg::Vst::IAudioProcessor* processor,
                                                                          bool isInput, int busIndex)
    {
        Steinberg::Vst::SpeakerArrangement arrangement = Steinberg::Vst::SpeakerArr::kEmpty;

        if (processor != nullptr)
            processor->getBusArrangement(isInput ? Steinberg::Vst::kInput : Steinberg::Vst::kOutput,
                                         (Steinberg::int32)busIndex, arrangement);

        return arrangement;
    }

    /** For the sake of simplicity, there can only be 1 arrangement type per channel count.
        i.e.: 4 channels == k31Cine OR k40Cine
    */
    static inline Steinberg::Vst::SpeakerArrangement getArrangementForNumChannels(int numChannels) noexcept
    {
        using namespace Steinberg::Vst::SpeakerArr;

        switch (numChannels)
        {
        case 0:
            return kEmpty;
        case 1:
            return kMono;
        case 2:
            return kStereo;
        case 3:
            return k30Cine;
        case 4:
            return k31Cine;
        case 5:
            return k50;
        case 6:
            return k51;
        case 7:
            return k61Cine;
        case 8:
            return k71CineFullFront;
        case 9:
            return k90;
        case 10:
            return k91;
        case 11:
            return k101;
        case 12:
            return k111;
        case 13:
            return k130;
        case 14:
            return k131;
#if VST_VERSION >= 0x030608
        case 16:
            return kAmbi3rdOrderACN;
#endif
        case 24:
            return (Steinberg::Vst::SpeakerArrangement)1929904127;  // k222
        default:
            break;
        }

        jassert(numChannels >= 0);

        juce::BigInteger bi;
        bi.setRange(0, jmin(numChannels, (int)(sizeof(Steinberg::Vst::SpeakerArrangement) * 8)), true);
        return (Steinberg::Vst::SpeakerArrangement)bi.toInt64();
    }

    static inline Steinberg::Vst::Speaker getSpeakerType(const AudioChannelSet&       set,
                                                         AudioChannelSet::ChannelType type) noexcept
    {
        using namespace Steinberg::Vst;

        switch (type)
        {
        case AudioChannelSet::left:
            return kSpeakerL;
        case AudioChannelSet::right:
            return kSpeakerR;
        case AudioChannelSet::centre:
            return (set == AudioChannelSet::mono() ? kSpeakerM : kSpeakerC);

        case AudioChannelSet::LFE:
            return kSpeakerLfe;
        case AudioChannelSet::leftSurround:
            return kSpeakerLs;
        case AudioChannelSet::rightSurround:
            return kSpeakerRs;
        case AudioChannelSet::leftCentre:
            return kSpeakerLc;
        case AudioChannelSet::rightCentre:
            return kSpeakerRc;
        case AudioChannelSet::centreSurround:
            return kSpeakerCs;
        case AudioChannelSet::leftSurroundSide:
            return (1ull << 26); /* kSpeakerLcs */
        case AudioChannelSet::rightSurroundSide:
            return (1ull << 27); /* kSpeakerRcs */
        case AudioChannelSet::topMiddle:
            return (1ull << 11); /* kSpeakerTm */
        case AudioChannelSet::topFrontLeft:
            return kSpeakerTfl;
        case AudioChannelSet::topFrontCentre:
            return kSpeakerTfc;
        case AudioChannelSet::topFrontRight:
            return kSpeakerTfr;
        case AudioChannelSet::topRearLeft:
            return kSpeakerTrl;
        case AudioChannelSet::topRearCentre:
            return kSpeakerTrc;
        case AudioChannelSet::topRearRight:
            return kSpeakerTrr;
        case AudioChannelSet::LFE2:
            return kSpeakerLfe2;
        case AudioChannelSet::leftSurroundRear:
            return kSpeakerSl;
        case AudioChannelSet::rightSurroundRear:
            return kSpeakerSr;
        case AudioChannelSet::wideLeft:
            return kSpeakerPl;
        case AudioChannelSet::wideRight:
            return kSpeakerPr;
        case AudioChannelSet::ambisonicACN0:
            return (1ull << 20); /* kSpeakerACN0 */
        case AudioChannelSet::ambisonicACN1:
            return (1ull << 21); /* kSpeakerACN1 */
        case AudioChannelSet::ambisonicACN2:
            return (1ull << 22); /* kSpeakerACN2 */
        case AudioChannelSet::ambisonicACN3:
            return (1ull << 23); /* kSpeakerACN3 */
        case AudioChannelSet::ambisonicACN4:
            return (1ull << 38); /* kSpeakerACN4 */
        case AudioChannelSet::ambisonicACN5:
            return (1ull << 39); /* kSpeakerACN5 */
        case AudioChannelSet::ambisonicACN6:
            return (1ull << 40); /* kSpeakerACN6 */
        case AudioChannelSet::ambisonicACN7:
            return (1ull << 41); /* kSpeakerACN7 */
        case AudioChannelSet::ambisonicACN8:
            return (1ull << 42); /* kSpeakerACN8 */
        case AudioChannelSet::ambisonicACN9:
            return (1ull << 43); /* kSpeakerACN9 */
        case AudioChannelSet::ambisonicACN10:
            return (1ull << 44); /* kSpeakerACN10 */
        case AudioChannelSet::ambisonicACN11:
            return (1ull << 45); /* kSpeakerACN11 */
        case AudioChannelSet::ambisonicACN12:
            return (1ull << 46); /* kSpeakerACN12 */
        case AudioChannelSet::ambisonicACN13:
            return (1ull << 47); /* kSpeakerACN13 */
        case AudioChannelSet::ambisonicACN14:
            return (1ull << 48); /* kSpeakerACN14 */
        case AudioChannelSet::ambisonicACN15:
            return (1ull << 49); /* kSpeakerACN15 */
        case AudioChannelSet::topSideLeft:
            return (1ull << 24); /* kSpeakerTsl */
        case AudioChannelSet::topSideRight:
            return (1ull << 25); /* kSpeakerTsr */

        case AudioChannelSet::discreteChannel0:
            return kSpeakerM;
        default:
            break;
        }

        switch (static_cast<int>(type))
        {
        case (int)AudioChannelSet::discreteChannel0 + 3:
            return (1ull << 28); /* kSpeakerBfl */
        case (int)AudioChannelSet::discreteChannel0 + 4:
            return (1ull << 29); /* kSpeakerBfc */
        case (int)AudioChannelSet::discreteChannel0 + 5:
            return (1ull << 30); /* kSpeakerBfr */
        default:
            break;
        }

        auto channelIndex = static_cast<Steinberg::Vst::Speaker>(type)
                            - (static_cast<Steinberg::Vst::Speaker>(AudioChannelSet::discreteChannel0) + 6ull);
        return (1ull << (channelIndex + 33ull /* last speaker in vst layout + 1 */));
    }

    static inline AudioChannelSet::ChannelType getChannelType(Steinberg::Vst::SpeakerArrangement arr,
                                                              Steinberg::Vst::Speaker            type) noexcept
    {
        using namespace Steinberg::Vst;

        switch (type)
        {
        case kSpeakerL:
            return AudioChannelSet::left;
        case kSpeakerR:
            return AudioChannelSet::right;
        case kSpeakerC:
            return AudioChannelSet::centre;
        case kSpeakerLfe:
            return AudioChannelSet::LFE;
        case kSpeakerLs:
            return AudioChannelSet::leftSurround;
        case kSpeakerRs:
            return AudioChannelSet::rightSurround;
        case kSpeakerLc:
            return AudioChannelSet::leftCentre;
        case kSpeakerRc:
            return AudioChannelSet::rightCentre;
        case kSpeakerCs:
            return AudioChannelSet::centreSurround;
        case kSpeakerSl:
            return AudioChannelSet::leftSurroundRear;
        case kSpeakerSr:
            return AudioChannelSet::rightSurroundRear;
        case (1ull << 11):
            return AudioChannelSet::topMiddle; /* kSpeakerTm */
        case kSpeakerTfl:
            return AudioChannelSet::topFrontLeft;
        case kSpeakerTfc:
            return AudioChannelSet::topFrontCentre;
        case kSpeakerTfr:
            return AudioChannelSet::topFrontRight;
        case kSpeakerTrl:
            return AudioChannelSet::topRearLeft;
        case kSpeakerTrc:
            return AudioChannelSet::topRearCentre;
        case kSpeakerTrr:
            return AudioChannelSet::topRearRight;
        case kSpeakerLfe2:
            return AudioChannelSet::LFE2;
        case (1ull << 19):
            return ((arr & kSpeakerC) != 0 ? AudioChannelSet::discreteChannel0 : AudioChannelSet::centre);
        case (1ull << 20):
            return AudioChannelSet::ambisonicACN0; /* kSpeakerACN0 */
        case (1ull << 21):
            return AudioChannelSet::ambisonicACN1; /* kSpeakerACN1 */
        case (1ull << 22):
            return AudioChannelSet::ambisonicACN2; /* kSpeakerACN2 */
        case (1ull << 23):
            return AudioChannelSet::ambisonicACN3; /* kSpeakerACN3 */
        case (1ull << 38):
            return AudioChannelSet::ambisonicACN4; /* kSpeakerACN4 */
        case (1ull << 39):
            return AudioChannelSet::ambisonicACN5; /* kSpeakerACN5 */
        case (1ull << 40):
            return AudioChannelSet::ambisonicACN6; /* kSpeakerACN6 */
        case (1ull << 41):
            return AudioChannelSet::ambisonicACN7; /* kSpeakerACN7 */
        case (1ull << 42):
            return AudioChannelSet::ambisonicACN8; /* kSpeakerACN8 */
        case (1ull << 43):
            return AudioChannelSet::ambisonicACN9; /* kSpeakerACN9 */
        case (1ull << 44):
            return AudioChannelSet::ambisonicACN10; /* kSpeakerACN10 */
        case (1ull << 45):
            return AudioChannelSet::ambisonicACN11; /* kSpeakerACN11 */
        case (1ull << 46):
            return AudioChannelSet::ambisonicACN12; /* kSpeakerACN12 */
        case (1ull << 47):
            return AudioChannelSet::ambisonicACN13; /* kSpeakerACN13 */
        case (1ull << 48):
            return AudioChannelSet::ambisonicACN14; /* kSpeakerACN14 */
        case (1ull << 49):
            return AudioChannelSet::ambisonicACN15; /* kSpeakerACN15 */
        case (1ull << 24):
            return AudioChannelSet::topSideLeft; /* kSpeakerTsl */
        case (1ull << 25):
            return AudioChannelSet::topSideRight; /* kSpeakerTsr */
        case (1ull << 26):
            return AudioChannelSet::leftSurroundSide; /* kSpeakerLcs */
        case (1ull << 27):
            return AudioChannelSet::rightSurroundSide; /* kSpeakerRcs */
        case (1ull << 28):
            return static_cast<AudioChannelSet::ChannelType>((int)AudioChannelSet::discreteChannel0 + 3); /* kSpeakerBfl */
        case (1ull << 29):
            return static_cast<AudioChannelSet::ChannelType>((int)AudioChannelSet::discreteChannel0 + 4); /* kSpeakerBfc */
        case (1ull << 30):
            return static_cast<AudioChannelSet::ChannelType>((int)AudioChannelSet::discreteChannel0 + 5); /* kSpeakerBfr */
        case kSpeakerPl:
            return AudioChannelSet::wideLeft;
        case kSpeakerPr:
            return AudioChannelSet::wideRight;
        default:
            break;
        }

        auto channelType = BigInteger(static_cast<int64>(type)).findNextSetBit(0);

        // VST3 <-> JUCE layout conversion error: report this bug to the JUCE forum
        jassert(channelType >= 33);

        return static_cast<AudioChannelSet::ChannelType>(static_cast<int>(AudioChannelSet::discreteChannel0) + 6
                                                         + (channelType - 33));
    }

    static inline Steinberg::Vst::SpeakerArrangement getVst3SpeakerArrangement(const AudioChannelSet& channels) noexcept
    {
        using namespace Steinberg::Vst::SpeakerArr;

        if (channels == AudioChannelSet::disabled())
            return kEmpty;
        else if (channels == AudioChannelSet::mono())
            return kMono;
        else if (channels == AudioChannelSet::stereo())
            return kStereo;
        else if (channels == AudioChannelSet::createLCR())
            return k30Cine;
        else if (channels == AudioChannelSet::createLRS())
            return k30Music;
        else if (channels == AudioChannelSet::createLCRS())
            return k40Cine;
        else if (channels == AudioChannelSet::create5point0())
            return k50;
        else if (channels == AudioChannelSet::create5point1())
            return k51;
        else if (channels == AudioChannelSet::create6point0())
            return k60Cine;
        else if (channels == AudioChannelSet::create6point1())
            return k61Cine;
        else if (channels == AudioChannelSet::create6point0Music())
            return k60Music;
        else if (channels == AudioChannelSet::create6point1Music())
            return k61Music;
        else if (channels == AudioChannelSet::create7point0())
            return k70Music;
        else if (channels == AudioChannelSet::create7point0SDDS())
            return k70Cine;
        else if (channels == AudioChannelSet::create7point1())
            return k71CineSideFill;
        else if (channels == AudioChannelSet::create7point1SDDS())
            return k71Cine;
        else if (channels == AudioChannelSet::ambisonic())
            return kAmbi1stOrderACN;
        else if (channels == AudioChannelSet::quadraphonic())
            return k40Music;
        else if (channels == AudioChannelSet::create7point0point2())
            return k71_2 & ~(Steinberg::Vst::kSpeakerLfe);
        else if (channels == AudioChannelSet::create7point1point2())
            return k71_2;
        else if (channels == AudioChannelSet::ambisonic(0))
            return (1ull << 20);
        else if (channels == AudioChannelSet::ambisonic(1))
            return (1ull << 20) | (1ull << 21) | (1ull << 22) | (1ull << 23);
#if VST_VERSION >= 0x030608
        else if (channels == AudioChannelSet::ambisonic(2))
            return kAmbi2cdOrderACN;
        else if (channels == AudioChannelSet::ambisonic(3))
            return kAmbi3rdOrderACN;
#endif

        Steinberg::Vst::SpeakerArrangement result = 0;

        Array<AudioChannelSet::ChannelType> types(channels.getChannelTypes());

        for (int i = 0; i < types.size(); ++i) result |= getSpeakerType(channels, types.getReference(i));

        return result;
    }

    static inline AudioChannelSet getChannelSetForSpeakerArrangement(Steinberg::Vst::SpeakerArrangement arr) noexcept
    {
        using namespace Steinberg::Vst::SpeakerArr;

        if (arr == kEmpty)
            return AudioChannelSet::disabled();
        else if (arr == kMono)
            return AudioChannelSet::mono();
        else if (arr == kStereo)
            return AudioChannelSet::stereo();
        else if (arr == k30Cine)
            return AudioChannelSet::createLCR();
        else if (arr == k30Music)
            return AudioChannelSet::createLRS();
        else if (arr == k40Cine)
            return AudioChannelSet::createLCRS();
        else if (arr == k50)
            return AudioChannelSet::create5point0();
        else if (arr == k51)
            return AudioChannelSet::create5point1();
        else if (arr == k60Cine)
            return AudioChannelSet::create6point0();
        else if (arr == k61Cine)
            return AudioChannelSet::create6point1();
        else if (arr == k60Music)
            return AudioChannelSet::create6point0Music();
        else if (arr == k61Music)
            return AudioChannelSet::create6point1Music();
        else if (arr == k70Music)
            return AudioChannelSet::create7point0();
        else if (arr == k70Cine)
            return AudioChannelSet::create7point0SDDS();
        else if (arr == k71CineSideFill)
            return AudioChannelSet::create7point1();
        else if (arr == k71Cine)
            return AudioChannelSet::create7point1SDDS();
        else if (arr == kAmbi1stOrderACN)
            return AudioChannelSet::ambisonic();
        else if (arr == k40Music)
            return AudioChannelSet::quadraphonic();
        else if (arr == k71_2)
            return AudioChannelSet::create7point1point2();
        else if (arr == (k71_2 & ~(Steinberg::Vst::kSpeakerLfe)))
            return AudioChannelSet::create7point0point2();
        else if (arr == (1 << 20))
            return AudioChannelSet::ambisonic(0);
        else if (arr == ((1 << 20) | (1 << 21) | (1 << 22) | (1 << 23)))
            return AudioChannelSet::ambisonic(1);
#if VST_VERSION >= 0x030608
        else if (arr == kAmbi2cdOrderACN)
            return AudioChannelSet::ambisonic(2);
        else if (arr == kAmbi3rdOrderACN)
            return AudioChannelSet::ambisonic(3);
#endif

        AudioChannelSet result;

        BigInteger vstChannels(static_cast<int64>(arr));
        for (auto bit = vstChannels.findNextSetBit(0); bit != -1; bit = vstChannels.findNextSetBit(bit + 1))
        {
            AudioChannelSet::ChannelType channelType = getChannelType(arr, 1ull << static_cast<uint64>(bit));
            if (channelType != AudioChannelSet::unknown) result.addChannel(channelType);
        }

        // VST3 <-> JUCE layout conversion error: report this bug to the JUCE forum
        jassert(result.size() == vstChannels.countNumberOfSetBits());

        return result;
    }

    //==============================================================================
    template <class ObjectType>
    class ComSmartPtr
    {
    public:
        ComSmartPtr() noexcept
            : source(nullptr)
        {
        }
        ComSmartPtr(ObjectType* object, bool autoAddRef = true) noexcept
            : source(object)
        {
            if (source != nullptr && autoAddRef) source->addRef();
        }
        ComSmartPtr(const ComSmartPtr& other) noexcept
            : source(other.source)
        {
            if (source != nullptr) source->addRef();
        }
        ~ComSmartPtr()
        {
            if (source != nullptr) source->release();
        }

        operator ObjectType*() const noexcept
        {
            return source;
        }
        ObjectType* get() const noexcept
        {
            return source;
        }
        ObjectType& operator*() const noexcept
        {
            return *source;
        }
        ObjectType* operator->() const noexcept
        {
            return source;
        }

        ComSmartPtr& operator=(const ComSmartPtr& other)
        {
            return operator=(other.source);
        }

        ComSmartPtr& operator=(ObjectType* const newObjectToTakePossessionOf)
        {
            ComSmartPtr p(newObjectToTakePossessionOf);
            std::swap(p.source, source);
            return *this;
        }

        bool operator==(ObjectType* const other) noexcept
        {
            return source == other;
        }
        bool operator!=(ObjectType* const other) noexcept
        {
            return source != other;
        }

        bool loadFrom(Steinberg::FUnknown* o)
        {
            *this = nullptr;
            return o != nullptr && o->queryInterface(ObjectType::iid, (void**)&source) == Steinberg::kResultOk;
        }

        bool loadFrom(Steinberg::IPluginFactory* factory, const Steinberg::TUID& uuid)
        {
            jassert(factory != nullptr);
            *this = nullptr;
            return factory->createInstance(uuid, ObjectType::iid, (void**)&source) == Steinberg::kResultOk;
        }

    private:
        ObjectType* source;
    };

    //==============================================================================
    class MidiEventList : public Steinberg::Vst::IEventList
    {
    public:
        MidiEventList() {}
        virtual ~MidiEventList() {}

        JUCE_DECLARE_VST3_COM_REF_METHODS
        JUCE_DECLARE_VST3_COM_QUERY_METHODS

        //==============================================================================
        void clear()
        {
            events.clearQuick();
        }

        Steinberg::int32 PLUGIN_API getEventCount() override
        {
            return (Steinberg::int32)events.size();
        }

        // NB: This has to cope with out-of-range indexes from some plugins.
        Steinberg::tresult PLUGIN_API getEvent(Steinberg::int32 index, Steinberg::Vst::Event& e) override
        {
            if (isPositiveAndBelow((int)index, events.size()))
            {
                e = events.getReference((int)index);
                return Steinberg::kResultTrue;
            }

            return Steinberg::kResultFalse;
        }

        Steinberg::tresult PLUGIN_API addEvent(Steinberg::Vst::Event& e) override
        {
            events.add(e);
            return Steinberg::kResultTrue;
        }

        //==============================================================================
        static void toMidiBuffer(MidiBuffer& result, Steinberg::Vst::IEventList& eventList)
        {
            const int32 numEvents = eventList.getEventCount();

            for (Steinberg::int32 i = 0; i < numEvents; ++i)
            {
                Steinberg::Vst::Event e;

                if (eventList.getEvent(i, e) == Steinberg::kResultOk)
                {
                    switch (e.type)
                    {
                    case Steinberg::Vst::Event::kNoteOnEvent:
                        result.addEvent(
                            MidiMessage::noteOn(createSafeChannel(e.noteOn.channel), createSafeNote(e.noteOn.pitch),
                                                (Steinberg::uint8)denormaliseToMidiValue(e.noteOn.velocity)),
                            e.sampleOffset);
                        break;

                    case Steinberg::Vst::Event::kNoteOffEvent:
                        result.addEvent(
                            MidiMessage::noteOff(createSafeChannel(e.noteOff.channel), createSafeNote(e.noteOff.pitch),
                                                 (Steinberg::uint8)denormaliseToMidiValue(e.noteOff.velocity)),
                            e.sampleOffset);
                        break;

                    case Steinberg::Vst::Event::kPolyPressureEvent:
                        result.addEvent(MidiMessage::aftertouchChange(createSafeChannel(e.polyPressure.channel),
                                                                      createSafeNote(e.polyPressure.pitch),
                                                                      denormaliseToMidiValue(e.polyPressure.pressure)),
                                        e.sampleOffset);
                        break;

                    case Steinberg::Vst::Event::kDataEvent:
                        result.addEvent(MidiMessage::createSysExMessage(e.data.bytes, (int)e.data.size), e.sampleOffset);
                        break;

                    default:
                        break;
                    }
                }
            }
        }

        static void toEventList(Steinberg::Vst::IEventList& result, MidiBuffer& midiBuffer)
        {
            MidiBuffer::Iterator iterator(midiBuffer);
            MidiMessage          msg;
            int                  midiEventPosition = 0;

            enum
            {
                maxNumEvents = 2048
            };  // Steinberg's Host Checker states that no more than 2048 events are allowed at once
            int numEvents = 0;

            while (iterator.getNextEvent(msg, midiEventPosition))
            {
                if (++numEvents > maxNumEvents) break;

                Steinberg::Vst::Event e = {0};

                if (msg.isNoteOn())
                {
                    e.type            = Steinberg::Vst::Event::kNoteOnEvent;
                    e.noteOn.channel  = createSafeChannel(msg.getChannel());
                    e.noteOn.pitch    = createSafeNote(msg.getNoteNumber());
                    e.noteOn.velocity = normaliseMidiValue(msg.getVelocity());
                    e.noteOn.length   = 0;
                    e.noteOn.tuning   = 0.0f;
                    e.noteOn.noteId   = -1;
                }
                else if (msg.isNoteOff())
                {
                    e.type             = Steinberg::Vst::Event::kNoteOffEvent;
                    e.noteOff.channel  = createSafeChannel(msg.getChannel());
                    e.noteOff.pitch    = createSafeNote(msg.getNoteNumber());
                    e.noteOff.velocity = normaliseMidiValue(msg.getVelocity());
                    e.noteOff.tuning   = 0.0f;
                    e.noteOff.noteId   = -1;
                }
                else if (msg.isSysEx())
                {
                    e.type       = Steinberg::Vst::Event::kDataEvent;
                    e.data.bytes = msg.getSysExData();
                    e.data.size  = (uint32)msg.getSysExDataSize();
                    e.data.type  = Steinberg::Vst::DataEvent::kMidiSysEx;
                }
                else if (msg.isAftertouch())
                {
                    e.type                  = Steinberg::Vst::Event::kPolyPressureEvent;
                    e.polyPressure.channel  = createSafeChannel(msg.getChannel());
                    e.polyPressure.pitch    = createSafeNote(msg.getNoteNumber());
                    e.polyPressure.pressure = normaliseMidiValue(msg.getAfterTouchValue());
                }
                else
                {
                    continue;
                }

                e.busIndex     = 0;
                e.sampleOffset = midiEventPosition;

                result.addEvent(e);
            }
        }

    private:
        Array<Steinberg::Vst::Event, CriticalSection> events;
        Atomic<int>                                   refCount;

        static Steinberg::int16 createSafeChannel(int channel) noexcept
        {
            return (Steinberg::int16)jlimit(0, 15, channel - 1);
        }
        static int createSafeChannel(Steinberg::int16 channel) noexcept
        {
            return (int)jlimit(1, 16, channel + 1);
        }

        static Steinberg::int16 createSafeNote(int note) noexcept
        {
            return (Steinberg::int16)jlimit(0, 127, note);
        }
        static int createSafeNote(Steinberg::int16 note) noexcept
        {
            return jlimit(0, 127, (int)note);
        }

        static float normaliseMidiValue(int value) noexcept
        {
            return jlimit(0.0f, 1.0f, (float)value / 127.0f);
        }
        static int denormaliseToMidiValue(float value) noexcept
        {
            return roundToInt(jlimit(0.0f, 127.0f, value * 127.0f));
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiEventList)
    };

    //==============================================================================
    template <typename FloatType>
    struct VST3BufferExchange
    {
        typedef Array<FloatType*> Bus;
        typedef Array<Bus>        BusMap;

        static inline void assignRawPointer(Steinberg::Vst::AudioBusBuffers& vstBuffers, float** raw)
        {
            vstBuffers.channelBuffers32 = raw;
        }
        static inline void assignRawPointer(Steinberg::Vst::AudioBusBuffers& vstBuffers, double** raw)
        {
            vstBuffers.channelBuffers64 = raw;
        }

        /** Assigns a series of AudioBuffer's channels to an AudioBusBuffers'
            @warning For speed, does not check the channel count and offsets according to the AudioBuffer
        */
        static void associateBufferTo(Steinberg::Vst::AudioBusBuffers& vstBuffers, Bus& bus, AudioBuffer<FloatType>& buffer,
                                      int numChannels, int channelStartOffset, int sampleOffset = 0)
        {
            const int channelEnd = numChannels + channelStartOffset;
            jassert(channelEnd >= 0 && channelEnd <= buffer.getNumChannels());

            bus.clearQuick();

            for (int i = channelStartOffset; i < channelEnd; ++i) bus.add(buffer.getWritePointer(i, sampleOffset));

            assignRawPointer(vstBuffers, (numChannels > 0 ? bus.getRawDataPointer() : nullptr));
            vstBuffers.numChannels  = numChannels;
            vstBuffers.silenceFlags = 0;
        }

        static void mapArrangementToBuses(int& channelIndexOffset, int index, Array<Steinberg::Vst::AudioBusBuffers>& result,
                                          BusMap& busMapToUse, const AudioChannelSet& arrangement,
                                          AudioBuffer<FloatType>& source)
        {
            const int numChansForBus = arrangement.size();

            if (index >= result.size()) result.add(Steinberg::Vst::AudioBusBuffers());

            if (index >= busMapToUse.size()) busMapToUse.add(Bus());

            associateBufferTo(result.getReference(index), busMapToUse.getReference(index), source, numChansForBus,
                              channelIndexOffset);

            channelIndexOffset += numChansForBus;
        }

        static inline void mapBufferToBuses(Array<Steinberg::Vst::AudioBusBuffers>& result, BusMap& busMapToUse,
                                            const Array<AudioChannelSet>& arrangements, AudioBuffer<FloatType>& source)
        {
            int channelIndexOffset = 0;

            for (int i = 0; i < arrangements.size(); ++i)
                mapArrangementToBuses(channelIndexOffset, i, result, busMapToUse, arrangements.getUnchecked(i), source);
        }

        static inline void mapBufferToBuses(Array<Steinberg::Vst::AudioBusBuffers>& result,
                                            Steinberg::Vst::IAudioProcessor& processor, BusMap& busMapToUse, bool isInput,
                                            int numBuses, AudioBuffer<FloatType>& source)
        {
            int channelIndexOffset = 0;

            for (int i = 0; i < numBuses; ++i)
                mapArrangementToBuses(channelIndexOffset, i, result, busMapToUse,
                                      getArrangementForBus(&processor, isInput, i), source);
        }
    };

    template <typename FloatType>
    struct VST3FloatAndDoubleBusMapCompositeHelper
    {
    };

    struct VST3FloatAndDoubleBusMapComposite
    {
        VST3BufferExchange<float>::BusMap  floatVersion;
        VST3BufferExchange<double>::BusMap doubleVersion;

        template <typename FloatType>
        inline typename VST3BufferExchange<FloatType>::BusMap& get()
        {
            return VST3FloatAndDoubleBusMapCompositeHelper<FloatType>::get(*this);
        }
    };

    template <>
    struct VST3FloatAndDoubleBusMapCompositeHelper<float>
    {
        static inline VST3BufferExchange<float>::BusMap& get(VST3FloatAndDoubleBusMapComposite& impl)
        {
            return impl.floatVersion;
        }
    };

    template <>
    struct VST3FloatAndDoubleBusMapCompositeHelper<double>
    {
        static inline VST3BufferExchange<double>::BusMap& get(VST3FloatAndDoubleBusMapComposite& impl)
        {
            return impl.doubleVersion;
        }
    };

}  // namespace juce
