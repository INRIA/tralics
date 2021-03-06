#pragma once
#include "MathElt.h"
#include "MathQ.h"

// Helper for finding start and end of <mrow>
struct MathF {
    bool               in_mrow{true};
    bool               t_big{false};
    int                next_change{-1};
    int                next_finish{-1};
    MathQList          aux;
    std::list<MathElt> res;
    Xml *              t{};

    void change_state();
    void handle_t();
    void push_in_t(Xml *x);
    void push_in_res(const MathElt &x) { res.push_back(x); }
    void push_in_res(Xml *x) { res.emplace_back(x, -1, mt_flag_small); }
    void reset_MathF() {
        t       = nullptr;
        in_mrow = true;
        t_big   = false;
    }
    void finish(std::list<MathElt> &value);
    void pop_last(Xml *xval);
};
