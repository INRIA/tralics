#pragma once
// -*- C++ -*-
// TRALICS, copyright (C) INRIA/apics (Jose' Grimm) 2002,2004,2006, 2007,2008

// This software is governed by the CeCILL license under French law and
// abiding by the rules of distribution of free software.  You can  use,
// modify and/ or redistribute the software under the terms of the CeCILL
// license as circulated by CEA, CNRS and INRIA at the following URL
// "http://www.cecill.info".
// (See the file COPYING in the main directory for details)

// Declarations for UTF8 characters

#include "txvars.h"

class Utf8Char {
private:
    unsigned int value{0};

public:
    Utf8Char(unsigned int x) : value(x) {}
    Utf8Char() {}

public:
    void               make_invalid() { value = 0xFFFF; } // Not a Unicode char
    [[nodiscard]] auto is_invalid() const -> bool { return value == 0xFFFF; }
    [[nodiscard]] auto get_value() const -> uint { return value; }
    [[nodiscard]] auto ascii_value() const -> uchar { return uchar(value); }
    [[nodiscard]] auto is_ascii() const -> bool { return value < 128; }
    [[nodiscard]] auto is_delete() const -> bool { return value == 127; }
    [[nodiscard]] auto is_control() const -> bool { return value < 32; }
    [[nodiscard]] auto is_null() const -> bool { return value == 0; }
    [[nodiscard]] auto is_small() const -> bool { return value < 256; }
    [[nodiscard]] auto is_big() const -> bool { return value > 65535; }
    [[nodiscard]] auto is_verybig() const -> bool { return value > 0x1FFFF; }
    [[nodiscard]] auto non_null() const -> bool { return value != 0; }
    [[nodiscard]] auto char_val() const -> uchar { return uchar(value); }
    [[nodiscard]] auto is_digit() const -> bool { return '0' <= value && value <= '9'; }
    [[nodiscard]] auto val_as_digit() const -> int { return value - '0'; }
    [[nodiscard]] auto is_hex() const -> bool { return 'a' <= value && value <= 'f'; }
    [[nodiscard]] auto val_as_hex() const -> int { return value - 'a' + 10; }
    [[nodiscard]] auto is_Hex() const -> bool { return 'A' <= value && value <= 'F'; }
    [[nodiscard]] auto val_as_Hex() const -> int { return value - 'A' + 10; }
    [[nodiscard]] auto hex_val() const -> int;
    [[nodiscard]] auto is_letter() const -> bool { return is_ascii() && ::is_letter(value); }
    [[nodiscard]] auto is_upper_case() const -> bool { return 'A' <= value && value <= 'Z'; }
    [[nodiscard]] auto is_lower_case() const -> bool { return 'a' <= value && value <= 'z'; }
    [[nodiscard]] auto is_space() const -> bool { return value == ' ' || value == '\t' || value == '\n'; }
    [[nodiscard]] auto to_lower() const -> Utf8Char {
        if ('A' <= value && value <= 'Z')
            return Utf8Char(value + ('a' - 'A'));
        else
            return *this;
    }
};

inline auto operator==(const Utf8Char &a, const Utf8Char &b) -> bool { return a.get_value() == b.get_value(); }

inline auto operator!=(const Utf8Char &a, const Utf8Char &b) -> bool { return a.get_value() != b.get_value(); }

inline auto operator==(const Utf8Char &a, const unsigned char &b) -> bool { return a.get_value() == uint(b); }

inline auto operator!=(const Utf8Char &a, const unsigned char &b) -> bool { return a.get_value() != uint(b); }
