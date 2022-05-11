#include "pch.h"
#include "Commands.h"
#include <Mftransform.h>

namespace winrt
{
    using namespace Windows::Foundation;
}

namespace util
{
    using namespace robmikh::common::wcli;
    using namespace robmikh::common::desktop;
    using namespace robmikh::common::uwp;
}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

int wmain(int argc, wchar_t* argv[])
{
    winrt::init_apartment();
    
    auto app = util::Application<commands::Command>(L"ConsoleTestApp")
        .Version(L"0.1.0")
        .Author(L"Robert Mikhayelyan (rob.mikh@outlook.com)")
        .About(L"A console test app.")
        .Command(util::Command(L"some-command", commands::Command(commands::SomeCommand())))
        .Command(util::Command(L"another-command", std::function(commands::ParsingValidation::ValidateAnotherCommand))
            .Argument(util::Argument(L"--someargument")
                .Required(true)
                .Alias(L"-sa")
                .TakesValue(true))
            .Argument(util::Argument(L"--anotherargument")
                .Required(true)
                .Alias(L"-aa")
                .TakesValue(true))
            .Argument(util::Argument(L"--yetanotherargument")
                .Required(true)
                .Alias(L"-yaa")
                .TakesValue(true)))
        .Command(util::Command(L"find-window", std::function(commands::ParsingValidation::ValidateFindWindowCommand))
            .Argument(util::Argument(L"--title")
                .Required(true)
                .Alias(L"-t")
                .TakesValue(true)))
        .Command(util::Command(L"enum-mfts", commands::Command(commands::EnumMFTsCommand())));

    commands::Command params;
    try
    {
        params = app.Parse(argc, argv);
    }
    catch (std::runtime_error const&)
    {
        app.PrintUsage();
        return 1;
    }

    std::visit(overloaded
    {
        [=](commands::SomeCommand const&) { wprintf(L"Some command!\n"); },
        [=](commands::AnotherCommand const& args) { wprintf(L"Another command! %s %s %s", args.SomeArgument.c_str(), args.AnotherArgument.c_str(), args.YetAnotherArgument.c_str()); },
        [=](commands::FindWindowCommand const& args)
        {
            auto windows = util::FindTopLevelWindowsByTitle(args.TitleQuery);
            wprintf(L"Found %I64i windows...\n", windows.size());
            for (auto&& window : windows)
            {
                wprintf(L"  %s\n", window.Title.c_str());
            }
        },
        [=](commands::EnumMFTsCommand const& args)
        {
            MFT_REGISTER_TYPE_INFO outputType = { MFMediaType_Audio, MFAudioFormat_MP3 };
            auto mfts = util::EnumerateMFTs(
                MFT_CATEGORY_AUDIO_ENCODER,
                MFT_ENUM_FLAG_SYNCMFT,
                nullptr,
                &outputType);

            wprintf(L"Audio Sync MFTs: \n");
            for (auto&& mft : mfts)
            {
                std::wstring friendlyName;
                if (auto name = util::GetStringAttribute(mft, MFT_FRIENDLY_NAME_Attribute))
                {
                    friendlyName = name.value();
                }
                else
                {
                    friendlyName = L"Unknown";
                }

                wprintf(L"  %s\n", friendlyName.c_str());
            }
        },
    }, params);

    return 0;
}
