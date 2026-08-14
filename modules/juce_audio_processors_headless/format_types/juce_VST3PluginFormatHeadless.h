/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace Steinberg
{
namespace Vst
{
class IEditController;
}
}

namespace juce
{

#if JUCE_INTERNAL_HAS_VST3 || DOXYGEN

/** Allows a VST3 host application to expose additional host-side interfaces to
    a hosted plug-in through the VST3 component handler.

    Implementations should set obj to the queried interface pointer and return
    true if the interface is supported. If the interface is not supported, set
    obj to nullptr and return false.

    This is the host-side counterpart to VST3ClientExtensions: it lets a JUCE
    host provide vendor or application-specific VST3 host extensions without
    replacing JUCE's built-in VST3 hosting implementation.

    @tags{Audio}
*/
struct JUCE_API VST3HostContextExtensions
{
    virtual ~VST3HostContextExtensions() = default;

    /** Called from the VST3 component handler's queryInterface() implementation
        after JUCE's built-in host interfaces have been checked.

        Implementations that return an interface pointer must follow normal VST3
        COM lifetime rules and increment the returned object's reference count
        before storing it in obj.
    */
    virtual bool queryIComponentHandler (const Steinberg::TUID iid, void** obj)
    {
        *obj = nullptr;
        ignoreUnused (iid);
        return false;
    }

    /** Called after JUCE has resolved the plug-in edit controller, and again
        with nullptr before the controller is detached.

        This allows host extensions to discover optional vendor-specific
        interfaces implemented by the edit controller without exposing JUCE's
        internal VST3PluginInstance implementation.
    */
    virtual void setIEditController (Steinberg::Vst::IEditController*)
    {
    }
};

/**
    Implements a plugin format for VST3s.

    @tags{Audio}
*/
class JUCE_API VST3PluginFormatHeadless   : public AudioPluginFormat
{
public:
    /** Constructor */
    VST3PluginFormatHeadless() = default;

    /** A factory that can provide additional host-side VST3 interfaces for a
        newly created plug-in instance.

        Returning nullptr keeps JUCE's default VST3 host behaviour unchanged.
        The returned object is owned by the VST3 host context for that plug-in
        instance and will be destroyed with it.
    */
    using HostContextExtensionFactory = std::function<std::unique_ptr<VST3HostContextExtensions> (const PluginDescription&)>;

    /** Sets a factory that will be consulted for each VST3 instance created by
        this format.

        The factory is called during plug-in instantiation, before the hosted
        plug-in receives the JUCE VST3 component handler.
    */
    void setHostContextExtensionFactory (HostContextExtensionFactory);

    //==============================================================================
    static String getFormatName()                   { return "VST3"; }
    String getName() const override                 { return getFormatName(); }
    bool canScanForPlugins() const override         { return true; }
    bool isTrivialToScan() const override           { return false; }

    void findAllTypesForFile (OwnedArray<PluginDescription>&, const String& fileOrIdentifier) override;
    bool fileMightContainThisPluginType (const String& fileOrIdentifier) override;
    String getNameOfPluginFromIdentifier (const String& fileOrIdentifier) override;
    bool pluginNeedsRescanning (const PluginDescription&) override;
    StringArray searchPathsForPlugins (const FileSearchPath&, bool recursive, bool) override;
    bool doesPluginStillExist (const PluginDescription&) override;
    FileSearchPath getDefaultLocationsToSearch() override;
    void createARAFactoryAsync (const PluginDescription&, ARAFactoryCreationCallback callback) override;

protected:
    //==============================================================================
    std::unique_ptr<VST3HostContextExtensions> createHostContextExtensions (const PluginDescription&) const;

private:
    //==============================================================================
    void createPluginInstance (const PluginDescription&, double initialSampleRate,
                               int initialBufferSize, PluginCreationCallback) override;
    bool requiresUnblockedMessageThreadDuringCreation (const PluginDescription&) const override;

    HostContextExtensionFactory hostContextExtensionFactory;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VST3PluginFormatHeadless)
};

#endif

} // namespace juce
