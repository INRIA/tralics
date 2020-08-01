#pragma once
// -*- C++ -*-
// TRALICS, copyright (C) INRIA/apics/marelle (Jose' Grimm) 2006 2015

// This software is governed by the CeCILL license under French law and
// abiding by the rules of distribution of free software.  You can  use,
// modify and/ or redistribute the software under the terms of the CeCILL
// license as circulated by CEA, CNRS and INRIA at the following URL
// "http://www.cecill.info".
// (See the file COPYING in the main directory for details)

#include "tralics/LineList.h"
#include <filesystem>
#include <fstream>

void readline(char *buffer, size_t screen_size); ///< Read a line from standart input (readline.cpp)

// This allows us to temporarily read from elsewhere
struct InputStack {
    std::vector<char32_t> line;
    states                state;     // copy of scanner state
    LineList              L;         // the lines
    int                   line_no;   // the current line number
    TokenList             TL;        // saved token list
    std::string           name;      // name of the current file
    long                  at_val;    // catcode of @ to restore
    long                  file_pos;  // file position to restore
    size_t                line_pos;  // position in B
    bool                  every_eof; // True if every_eof_token can be inserted
    bool                  eof_outer; // True if eof is outer

    InputStack(std::string N, int l, states S, long cfp, bool eof, bool eof_o)
        : state(S), line_no(l), name(std::move(N)), at_val(-1), file_pos(cfp), line_pos(0), every_eof(eof), eof_outer(eof_o) {}

    void destroy();
    void set_line_ptr(LineList &X) {
        L.clear_and_copy(X);
        X.file_name = "";
        L.set_interactive(X.interactive);
    }
    void get_line_ptr(LineList &X) {
        X.clear_and_copy(L);
        X.set_interactive(L.interactive);
    }
};

// data structure associated to \input3=some_file.
struct FileForInput {
    bool     is_open{false}; ///< is this file active ?
    LineList lines;          ///< the lines that are not yet read by TeX
    Buffer   cur_line;       ///< the current line
    int      line_no{0};     ///< the current line number

    void open(const std::string &file, const std::filesystem::path &fn, bool action);
    void close();
};

// From TeX:
// The sixteen possible \write streams are represented by the |write_file|
// array. The |j|th file is open if and only if |write_open[j]=true|. The last
// two streams are special; |write_open[16]| represents a stream number
// greater than 15, while |write_open[17]| represents a negative stream number,
// and both of these variables are always |false|.
// Since \write18 is special, we added another slot in write_open
class TexOutStream {
    std::array<std::ofstream, nb_output_channels> write_file{}; // \todo array<ofstream>
    std::array<bool, nb_output_channels>          write_open{};

public:
    TexOutStream();
    void               close(size_t chan);
    void               open(size_t chan, const std::string &fn);
    [[nodiscard]] auto is_open(size_t i) const -> bool { return write_open[i]; }
    void               write(size_t chan, const std::string &s) { write_file[chan] << s; }
};
