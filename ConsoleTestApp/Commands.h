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

    typedef std::variant<
        SomeCommand,
        AnotherCommand
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

    private:
        ParsingValidation() {}
    };
}
