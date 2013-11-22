#ifndef DEBUG_HPP
#define DEBUG_HPP

#include <iostream>

#define cerr_line std::cerr<<__FILE__<<"("<<__LINE__<< "): "
#define DBG_MUTE if (false) std::cerr

#ifdef DEBUG
#define dbg_out std::cerr
#define dbg_out_line cerr_line
#else
#define dbg_out DBG_MUTE
#define dbg_out_line dbg_out
#endif // DEBUG

// Pro umlčení určitého typu DEBUG hlášek nahraďte "dbg_out" za "DBG_MUTE".
#define dbg_w_create dbg_out
#define dbg_w_destroy dbg_out
#define dbg_w_pixmap dbg_out
#define dbg_w_conf dbg_out
#define dbg_e_keyPress dbg_out
#define dbg_e_buttonPress dbg_out
#define dbg_e_reparent dbg_out
#define dbg_e_motion dbg_out
#define dbg_e_damage dbg_out

#endif // DEBUG_HPP
