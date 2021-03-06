/*
MIT License
Copyright (c) 2017 Arlen Keshabyan (arlen.albert@gmail.com)
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <iostream>

#include "planar_movements_recognizer.hpp"
#include "platform.hpp"
#include "platform_utilities.hpp"
#include "strings.hpp"
#include "utilities.hpp"
#include "cmdline_options.hpp"
#include "process.hpp"

int main(int argc, char **argv)
{
    using namespace nstd::str;
    using namespace nstd::platform;
    using namespace nstd::platform::utilities;
    using namespace nstd::utilities;
    using namespace nstd::process;

    scoped_console_utf8 set_console_utf8;
    const std::u8string_view str { u8"Консоль поддерживает UTF-8..." };

    std::cout << std::string(std::begin(str), std::end(str))  << std::endl;

    Process proc("echo \"Redirected output\"", "", [](auto a, auto b){ std::cout << std::endl << ">> " << std::string(a, b) << std::endl; }, [](auto a, auto b){});

    (void)proc.get_exit_status();

    nstd::po::parser p;
    std::string shell_cmd;
    auto help_callback { [&p]{ std::cout << p << std::endl; } };

    p["execute"].abbreviation('E').type(nstd::po::string).description("Executes the provided shell command (not actually)").bind(shell_cmd);
    p["help"].abbreviation('?').callback(help_callback).description("Prints the help screen");

    auto pres { p(argc, argv) };

    auto &&exe = p["execute"];

    if (argc == 1 || !pres) help_callback();
    if (exe.was_set() && !is_empty_or_ws(shell_cmd)) { std::cout << "command to execute: " << shell_cmd << std::endl; return 0; }

    const std::string app_name { "pmr_example" };
    at_scope_exit execute_at_scope_exit { [] { const std::u8string_view str { u8"Всё ещё UTF8..." }; std::cout << std::string(std::begin(str), std::end(str))  << std::endl; } };
    std::vector<at_scope_exit> exit_chain;

    exit_chain.emplace_back([&app_name] { std::cout << std::endl << "#1. exitting " << app_name << "..." << std::endl; });
    exit_chain.emplace_back([] { std::cout << "#2. stopped" << std::endl; });

    constexpr const bool if_windows { current_os_family == os_family::Windows };

    auto cmd { compose_string(if_windows ? "where" : "which", " gcc 2>&1") };
    auto res { shell_execute(cmd) };

    std::cout << "shell execution result: \"" << trim(res) << "\"" << std::endl << std::endl;

    std::cout << "Is Little Endian: "   << boolalpha[is_little_endian]  << std::endl;
    std::cout << "Is 64 bit: "          << boolalpha[is_64bit]          << std::endl;
    std::cout << "      OS: "           << get_current_os_type_name()   << std::endl;
    std::cout << "Platform: "           << get_current_os_family_name() << std::endl;
    std::cout << "Compiler: "           << get_current_compiler_name()  << std::endl << std::endl;

    exit_chain.emplace_back([]{ std::cout << "#3. ..." << std::endl; });

    using namespace std::string_literals;
    using namespace nstd::pmr;

    enum command : uint8_t
    {
        open_file = 100, close_file,
        go_back, go_forward,
        reload
    };

    std::map<uint8_t, std::string> command_map
    {
        {0, "Unknown"s},
        {command::open_file, "Open file"s},
        {command::close_file, "Close file"s},
        {command::go_back, "Go back"s},
        {command::go_forward, "Go forward"s},
        {command::reload, "Reload"s}
    };

    using event = planar_movements_event_provider::event;

    planar_movements_event_provider pmep;
    command_recognizer<event, command> cr;
    remove_noise_filter rnf;

    cr. add_command(command::open_file,  { event::up }).
        add_command(command::close_file, { event::down }).
        add_command(command::go_back,    { event::left }).
        add_command(command::go_forward, { event::right }).
        add_command(command::reload,     { event::down, event::up })
    ;

    {
        std::vector<event> coords { pmep(100., 100.), pmep(150., 105.), pmep(200., 103.), pmep(250., 102.), pmep(300., 95.) }; // moving right

        std::cout << command_map[cr(rnf(std::move(coords)))] << std::endl;
    }

    {
        event_filter<event> ef(true);
        ef.set(event::right, event::left);

         // moving right, but mapping the right event to the left one using event_filter
        std::vector<event> coords { ef(pmep(100., 100.)), ef(pmep(150., 105.)), ef(pmep(200., 103.)), ef(pmep(250., 102.)), ef(pmep(300., 95.)) }; // moving right

        std::cout << command_map[cr(rnf(std::move(coords)))] << std::endl;
    }

    {
        std::vector<event> coords { pmep(295., 239.), pmep(310., 202.), pmep(300., 150.), pmep(300., 120.), pmep(300., 95.) }; // moving up

        std::cout << command_map[cr(rnf(std::move(coords)))] << std::endl;
    }

    {
        std::vector<event> coords { pmep(300., 95.), pmep(300., 120.), pmep(300., 150.), pmep(310., 202.), pmep(295., 239.) }; // moving down

        std::cout << command_map[cr(rnf(std::move(coords)))] << std::endl;
    }

    {
        std::vector<event> coords { pmep(300., 95.), pmep(300., 120.), pmep(300., 150.), pmep(310., 202.), pmep(295., 239.),
                                                                     pmep(295., 239.), pmep(310., 202.), pmep(300., 150.), pmep(300., 120.), pmep(300., 95.) }; // moving down & up

        std::cout << command_map[cr(rnf(std::move(coords)))] << std::endl;
    }

    return 0;
}
