#pragma once
// -*- C++ -*-
// TRALICS, copyright (C)  INRIA/apics (Jose' Grimm) 2008

// This software is governed by the CeCILL license under French law and
// abiding by the rules of distribution of free software.  You can  use,
// modify and/ or redistribute the software under the terms of the CeCILL
// license as circulated by CEA, CNRS and INRIA at the following URL
// "http://www.cecill.info".
// (See the file COPYING in the main directory for details)

// This file holds code for Xkeyval

class XkvToken {
    TokenList initial; // of the form \gsavevalue{foo}=bar
    TokenList value;
    string    keyname;
    bool      is_global;
    bool      has_save;
    bool      has_val;
    TokenList action;

public:
    [[nodiscard]] auto key_empty() const -> bool { return keyname.empty(); }
    [[nodiscard]] auto val_empty() const -> bool { return value.empty(); }
    void               set_initial(TokenList L) { initial = L; }
    auto               get_all() -> TokenList { return initial; }
    auto               get_code() -> TokenList { return value; }
    auto               get_action() -> TokenList & { return action; }
    void               extract();
    auto               ignore_this(vector<string> &igna) -> bool;
    auto               get_name() -> string & { return keyname; }
    void               prepare(const string &fam);
    auto               is_defined(const string &fam) -> bool;
    auto               get_save() -> bool { return has_save; }
    auto               check_save() -> bool;
};

// Used by \setkeys in xkv
class XkvSetkeys {
    Parser *       P;
    Token          comma_token;
    Token          equals_token;
    Token          na_token;
    Token          rm_token;
    Token          fams_token;
    Token          savevalue_token;
    Token          usevalue_token;
    vector<string> Fams;    // the list of all families
    vector<string> Na;      // the list of keys not to set
    vector<string> Keys;    // the list of keys
    TokenList      fams;    // the list of families
    TokenList      na;      // the list of keys that should not be set
    TokenList      keyvals; // the keylist to set
    TokenList      delayed; // unknown ketyvalue pairs
    TokenList      action;  // expansion of \setkeys
    bool           no_err;  // Error when undefined ?
    bool           set_all; // set key in all families ?
    bool           in_pox;  // are in in \ProcessOptionsX ?
public:
    XkvSetkeys(Parser *P);
    void run(bool);
    void run2(bool);
    void check_preset(String);
    void extract_keys(TokenList &L, vector<string> &R);
    void fetch_fams();
    void special_fams();
    void fetch_na();
    void fetch_keys(bool);
    void check_action(XkvToken &);
    void run_key(Token, XkvToken &, const string &);
    void save_key(const string &Key, TokenList &L);
    void run_default(const string &, Token mac, bool);
    void replace_pointers(TokenList &);
    void new_unknown(TokenList &L) {
        delayed.splice(delayed.end(), L);
        delayed.push_back(comma_token);
    }
    void more_action(TokenList &L) { action.splice(action.end(), L); }
    void finish();
    void set_aux(TokenList &W, int idx);
    void set_aux() { set_aux(keyvals, -1); }
    void find_pointer(const string &);
    void set_inpox() { in_pox = true; }
    void dump_keys();
};
