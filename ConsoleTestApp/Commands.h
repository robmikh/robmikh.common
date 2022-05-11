#pragma once

namespace commands
{
    struct SomeCommand {};
    struct AnotherCommand
    {
        std::wstring SomeArgument;
        std::wstring AnotherArgument;
        std::wstring YetAnotherArgument;
    };
    // Womp womp, the FindWindow macro collides with us here
    struct FindWindowCommand
    {
        std::wstring TitleQuery;
    };
    struct EnumMFTsCommand {};

    typedef std::variant<
        SomeCommand,
        AnotherCommand,
        FindWindowCommand,
        EnumMFTsCommand
    > Command;

    class ParsingValidation
    {
    public:
        static commands::Command ValidateAnotherCommand(robmikh::common::wcli::Matches& matches)
        {
            commands::AnotherCommand result = {};
            result.SomeArgument = matches.ValueOf(L"--someargument");
            result.AnotherArgument = matches.ValueOf(L"--anotherargument");
            result.YetAnotherArgument = matches.ValueOf(L"--yetanotherargument");
            return commands::Command(result);
        }

        static commands::Command ValidateFindWindowCommand(robmikh::common::wcli::Matches& matches)
        {
            commands::FindWindowCommand result = {};
            result.TitleQuery = matches.ValueOf(L"--title");
            return commands::Command(result);
        }

    private:
        ParsingValidation() {}
    };
}
