#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
#include <sstream>
#include <iostream>
#include "repl.h"

static Environment g_env;
static bool g_initialized = false;

inline std::string evaluate(std::string input) {
    if (!g_initialized) {
        g_env.T = 50;
        g_env.env["q"] = Series::q(50);
        g_initialized = true;
    }

    std::ostringstream oss;
    auto* old_cout = std::cout.rdbuf(oss.rdbuf());
    auto* old_cerr = std::cerr.rdbuf(oss.rdbuf());

    try {
        std::string trimmed = trim(input);
        if (trimmed.empty()) {
            std::cout.rdbuf(old_cout);
            std::cerr.rdbuf(old_cerr);
            return "";
        }

        bool suppress = false;
        if (trimmed.back() == ':') {
            trimmed.pop_back();
            trimmed = trim(trimmed);
            suppress = true;
        }

        StmtPtr stmt = parse(trimmed);
        EvalResult res = evalStmt(stmt.get(), g_env);

        if (!suppress && !std::holds_alternative<std::monostate>(res)
                      && !std::holds_alternative<DisplayOnly>(res)) {
            display(res, g_env, g_env.T);
        }
    } catch (const std::exception& e) {
        oss << "error: " << e.what() << std::endl;
    } catch (...) {
        oss << "error: unknown exception" << std::endl;
    }

    std::cout.rdbuf(old_cout);
    std::cerr.rdbuf(old_cerr);

    std::string result = oss.str();
    while (!result.empty() && result.back() == '\n')
        result.pop_back();
    return result;
}

inline std::string get_banner() {
    return "qseries v2.0";
}

EMSCRIPTEN_BINDINGS(qseries) {
    emscripten::function("evaluate", &evaluate);
    emscripten::function("get_banner", &get_banner);
}
#endif
