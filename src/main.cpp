#include <uxs/cli/parser.h>
#include <uxs/db/json.h>
#include <uxs/format.h>
#include <uxs/io/ibuf.h>
#include <uxs/io/iobuf.h>

#include <exception>

#define XSTR(s) STR(s)
#define STR(s)  #s

int main(int argc, char* argv[]) {
    try {
        bool show_help = false;
        bool show_version = false;
        bool write_condensed = false;
        uxs::db::json::json_fmt_opts opts;

        opts.indent_size = 4;
        opts.indent_char = opts.object_ws_char = opts.array_ws_char = ' ';

        auto cli = uxs::cli::command(argv[0])
                   << uxs::cli::overview("JSON formatter")
                   << uxs::cli::option({"-c"}).set(write_condensed) % "Write condensed."
                   << (uxs::cli::option({"-i"}) & uxs::cli::value("<size>", opts.indent_size)) % "Indentation size."
                   << uxs::cli::option({"-t"}).call([&opts]() { opts.indent_char = '\t'; }) % "Use tabs."
                   << uxs::cli::option({"-a"}).call([&opts]() { opts.object_ws_char = '\n'; }) %
                          "New lines for object elements."
                   << uxs::cli::option({"-A"}).call([&opts]() { opts.object_ws_char = opts.array_ws_char = '\n'; }) %
                          "New lines for object and array elements."
                   << uxs::cli::option({"-h", "--help"}).set(show_help) % "Display this information."
                   << uxs::cli::option({"-V", "--version"}).set(show_version) % "Display version.";

        auto parse_result = cli->parse(argc, argv);
        if (show_help) {
            uxs::stdbuf::out().write(parse_result.node->get_command()->make_man_page(uxs::cli::text_coloring::colored));
            return 0;
        } else if (show_version) {
            uxs::println(uxs::stdbuf::out(), "{}", XSTR(VERSION));
            return 0;
        } else if (parse_result.status != uxs::cli::parsing_status::ok) {
            switch (parse_result.status) {
                case uxs::cli::parsing_status::unknown_option: {
                    uxs::println(uxs::stdbuf::err(), "\033[0;31merror:\033[0m unknown command line option `{}`",
                                 argv[parse_result.argc_parsed]);
                } break;
                case uxs::cli::parsing_status::invalid_value: {
                    if (parse_result.argc_parsed < argc) {
                        uxs::println(uxs::stdbuf::err(), "\033[0;31merror:\033[0m invalid command line argument `{}`",
                                     argv[parse_result.argc_parsed]);
                    } else {
                        uxs::println(uxs::stdbuf::err(),
                                     "\033[0;31merror:\033[0m expected command line argument after `{}`",
                                     argv[parse_result.argc_parsed - 1]);
                    }
                } break;
                default: break;
            }
            return -1;
        }

        const auto v = uxs::db::json::parse(uxs::stdbuf::in());
        if (write_condensed) {
            uxs::db::json::write(uxs::stdbuf::out(), v);
        } else {
            uxs::db::json::write_formatted(uxs::stdbuf::out(), v, opts);
        }
        return 0;

    } catch (const std::exception& e) {
        uxs::print("exception caught: {}", e.what());
        return -1;
    }
}
