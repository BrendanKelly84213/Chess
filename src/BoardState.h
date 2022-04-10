#ifndef BOARDSTATE_H
#define BOARDSTATE_H 

#include <string> 
#include <vector>
#include <iostream>
#include <cassert>
#include <algorithm>
#include "utils/types.h"
#include "utils/conversions.h"
#include "utils/bits.h"

// Sliding piece lookup tables 
static Bitboard piece_attacks[6][64];
static Bitboard behind[64][64];

struct State {
    Colour side_to_move = White; 
    bool w_castle_ks = false; 
    bool w_castle_qs = false; 
    bool b_castle_ks = false; 
    bool b_castle_qs = false; 
    int ep_square = None;
    int halfmove_clock = 0;
    int ply_count = 0;     
    Piece last_captured = None;

    bool operator==(State &b);
};

class BoardState {
private: 
    State state;
    std::vector<BMove> movelist;
    State prev_state;
    Bitboard piece_bbs[12];
    Piece squares[64]; // Square centric lookup 
    Bitboard occ = 0ULL;
    Bitboard white_occ = 0ULL;
    Bitboard black_occ = 0ULL;

    void init_squares(std::string fen);
    void init_bbs();
    void castle_kingside();
    void uncastle_kingside();
    void castle_queenside();
    void uncastle_queenside();
    void do_castle(int rook_from, int rook_to, int king_from, int king_to);
    void move_piece(int from, int to);
    void move_piece(int from, int to, Piece p);
    void remove_piece(int sq);
    void put_piece(int sq, Piece p);
    void update_extras();
    bool board_ok();
    void init_behind();
    void init_piece_attacks();
    void init_attacks();
    Bitboard blockers_and_beyond(int p, int from) const;
    Bitboard pawn_squares(int origin, Colour us) const;

public: 
    void init(std::string fen);
    void make_move(BMove m);
    void unmake_move(BMove m);
    Bitboard get_occ() const; 
    Bitboard get_friend_occ() const;
    Bitboard get_friend_occ(Colour us) const;
    Bitboard get_op_occ() const;
    Bitboard get_op_occ(Colour us) const ;
    Bitboard get_friend_piece_bb(int pt) const;
    Bitboard get_op_piece_bb(int pt) const;
    Bitboard get_side_piece_bb(int pt, Colour side) const ;
    Bitboard attacks_to(int sq, Colour attacker) const;
    Bitboard get_to_squares(int p, int from, Colour us) const;
    Colour get_piece_colour(Piece p) const;
    bool in_check(Colour us);
    bool attacked(int sq, Colour by);
    Piece get_piece(int s) const;
    bool can_castle(Colour us, Move type);
    int get_ep_square() const;
    Colour get_side_to_move() const { return state.side_to_move; }

    void print_previous_moves();
    void print_occupied();
    void print_context(BMove m, bool capture, Move flag);
    bool operator==(BoardState b);
};

#endif
