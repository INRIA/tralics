// Tralics, a LaTeX to XML translator.
// Copright INRIA/apics/marelle (Jose' Grimm) 2008-2011

// This software is governed by the CeCILL license under French law and
// abiding by the rules of distribution of free software.  You can  use,
// modify and/ or redistribute the software under the terms of the CeCILL
// license as circulated by CEA, CNRS and INRIA at the following URL
// "http://www.cecill.info".
// (See the file COPYING in the main directory for details)

// The file contains code for configurating the Raweb

#include "tralics/Logger.h"
#include "tralics/Parser.h"
#include "tralics/globals.h"
#include "tralics/util.h"
#include "txparam.h"

ParamDataVector config_data;

namespace config_ns {
    // We add a final slash, or double slash, this makes parsing easier;
    // We also remove an initial slash (This is not done if the separator is a
    // space, case where s is empty).
    // An initial plus sign means: append the line to the vector, else reset.

    auto start_interpret(Buffer &B, String s) -> bool {
        bool ret_val = false;
        B.append(s);
        B.ptrs.b = 0;
        if (B.head() == '+') {
            B.advance();
            the_log << "+";
        } else
            ret_val = true;
        if ((s[0] != 0) && B.head() == '/') B.advance();
        return ret_val;
    }
} // namespace config_ns

// --------------------------------------------------

// Configuration file option lists.
// The configuration file may contains lines of the form
// FOO_vals="/A/B/C", or FOO_vals="/A/a/B/b/C/c".
// transformed into a vector of data, named FOO, with some execptions.

// This error is fatal
// We make sure these four items always exist
ParamDataVector::ParamDataVector() {
    data.push_back(new ParamDataList("ur"));
    data.push_back(new ParamDataList("sections"));
    data.push_back(new ParamDataList("profession"));
    data.push_back(new ParamDataList("affiliation"));
}

// We may add a special slot at the end
void ParamDataList::check_other() {
    if ((std::islower(name[0]) != 0) && !empty()) push_back({"Other", "Other"});
}

// --------------------------------------------------

// --------------------------------------------------

// Return the data associated to name, may create depend on creat
auto ParamDataVector::find_list(const std::string &name, bool creat) -> ParamDataList * {
    auto n = data.size();
    for (size_t i = 0; i < n; i++)
        if (data[i]->its_me(name)) return data[i];
    if (!creat) return nullptr;
    auto *res = new ParamDataList(name);
    data.push_back(res);
    return res;
}

void ParamDataList::keys_to_buffer(Buffer &B) const {
    for (const auto &i : data) B.format(" {}", i.key);
}

// -----------------------------------------------------------

// --------------------------------------------------

// If str is, say `Cog A', this puts ` cog ' in the buffer, returns `cog'.
auto Buffer::add_with_space(const std::string &s) -> std::string {
    size_t i = 0;
    while (s[i] == ' ') ++i;
    clear();
    push_back(' ');
    while ((s[i] != 0) && (s[i] != ' ')) push_back(s[i++]);
    lowercase();
    std::string res = substr(1);
    push_back(' ');
    return res;
}

// --------------------------------------------------

// This creates the file foo_.bbl and initiates the bibtex translation.

// In the case of "foo bar gee", puts foo, bar and gee in the vector.
// Initial + means append, otherwise replace.

void Buffer::interpret_aux(std::vector<std::string> &bib, std::vector<std::string> &bib2) {
    if (config_ns::start_interpret(*this, "")) {
        bib.resize(0);
        bib2.resize(0);
    }
    for (;;) {
        bool keep = true;
        skip_sp_tab();
        if (head() == 0) break;
        auto a = ptrs.b;
        if (head() == '-') {
            keep = false;
            advance();
            if (head() == 0) break; // final dash ignored
            a++;
            the_log << "--";
        }
        while ((head() != 0) && (std::isspace(head()) == 0)) advance();
        ptrs.a        = a;
        std::string k = substring();
        if (keep)
            bib.emplace_back(k);
        else
            bib2.emplace_back(k);
        the_log << k << " ";
    }
    the_log << "\n";
}
