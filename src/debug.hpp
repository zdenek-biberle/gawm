#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <iostream>

#define cerr_line std::cerr<<__FILE__<<"("<<__LINE__<< "): "

#ifdef DEBUG
#define dbg_out std::cerr
#define dbg_out_line cerr_line
#else
#define dbg_out if (false) std::cerr
#define dbg_out_line dbg_out
#endif // DEBUG

#define dbg_w_create dbg_out
#define dbg_w_destroy dbg_out
#define dbg_w_pixmap dbg_out
#define dbg_w_conf dbg_out
#define dbg_e_keyPress dbg_out
#define dbg_e_buttonPress dbg_out
#define dbg_e_reparent dbg_out
#define dbg_e_motion dbg_out

#endif // DEBUG_HPP
