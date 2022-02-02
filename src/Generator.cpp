#include "Generator.h"

// TODO: 
//  Check for checks 
//  Castling
//  Organize code:
//      bitwise operations goes in seperate utils 

// BMove:
// Bits 0 - 5: from 
// Bits 6 - 11: to
// Bits 12 - 15: ??  

//Ranks and files. 
constexpr Bitboard FileABB = 0x0101010101010101ULL;
constexpr Bitboard FileBBB = FileABB << 1;
constexpr Bitboard FileCBB = FileABB << 2;
constexpr Bitboard FileDBB = FileABB << 3;
constexpr Bitboard FileEBB = FileABB << 4;
constexpr Bitboard FileFBB = FileABB << 5;
constexpr Bitboard FileGBB = FileABB << 6;
constexpr Bitboard FileHBB = FileABB << 7;

constexpr Bitboard Rank1BB = 0xFF;
constexpr Bitboard Rank2BB = Rank1BB << (8 * 1);
constexpr Bitboard Rank3BB = Rank1BB << (8 * 2);
constexpr Bitboard Rank4BB = Rank1BB << (8 * 3);
constexpr Bitboard Rank5BB = Rank1BB << (8 * 4);
constexpr Bitboard Rank6BB = Rank1BB << (8 * 5);
constexpr Bitboard Rank7BB = Rank1BB << (8 * 6);
constexpr Bitboard Rank8BB = Rank1BB << (8 * 7);

// Sliding piece lookup tables 
static Bitboard piece_attacks[6][64];
static Bitboard behind[64][64];

const Direction directions[] = { N, S, E, W, NE, NW, SE, SW };

template<typename T>
constexpr Square operator+(Square a, T b) 
{ 
    return static_cast<Square>(int(a) + int(b)); 
}

Square& operator++(Square& s) 
{ 
    s = static_cast<Square>(int(s) + 1); 
    return s;
}

constexpr Bitboard get_bit(Bitboard bb, int square) 
{ 
    return (bb >> square & 1ULL);
}

constexpr Bitboard get_bit(Bitboard bb, int x, int y) 
{ 
    return (bb >> y*8 + x) & 1ULL;
}

constexpr Bitboard bit(int s)
{
    return 1ULL << s;
}

constexpr void set_bit(Bitboard &bb, int square) 
{ 
    bb |= 1ULL << square; 
}

constexpr void clear_bit(Bitboard &bb, int square)
{
    bb &= ~(1ULL << square);
}

constexpr Square orig_bm(BMove m)  
{
    return static_cast<Square>((m >> 6) & 0x3f);
}

constexpr Square dest_bm(BMove m) 
{
    return static_cast<Square>(m & 0x3f);
}

Bitboard get_behind(Square from, Square to)
{
    return behind[from][to];
}

inline Square pop_bit(Bitboard &bb)
{
    Square index = Square(__builtin_ctzll(bb));
    bb &= ~(1ULL << index);
    return index;
} 

// For debugging
void print(Bitboard bb) 
{
    for(int y=7; y >=0; --y){
        std::cout << '\n';
        for(int x=0; x < 8; ++x)
            std::cout << get_bit(bb, x,y) << " ";
    }
    std::cout << '\n';
}

Bitboard occ_squares(Piece* squares, Colour colour)
{
    Bitboard occ = 0x00;
    for(int s=0; s<64; ++s) {
        if(colour == White && squares[s] >= 6 && squares[s] <= 11
        || colour == Black && squares[s] >= 0 && squares[s] <= 5) {
                set_bit(occ, s);
        }
    } 
    return occ;
}

constexpr Bitboard king_mask(Square origin)
{
    return (bit(origin + N) & ~Rank1BB)
        | (bit(origin + NE) & ~(FileABB | Rank1BB))
        | (bit(origin + E) & ~FileABB)
        | (bit(origin + SE) & ~(FileABB | Rank8BB))
        | (bit(origin + S) & ~Rank8BB)
        | (bit(origin + SW) & ~(FileHBB | Rank8BB))
        | (bit(origin + W) & ~FileHBB)
        | (bit(origin + NW) & ~(FileHBB | Rank1BB));
}


constexpr Bitboard knight_mask(Square origin)
{
    return (bit(origin + NEE) & ~(FileABB | FileBBB | Rank1BB))
        | (bit(origin + NNE) & ~(FileABB | Rank1BB | Rank2BB))
        | (bit(origin + NNW) & ~(FileHBB | Rank1BB | Rank2BB))
        | (bit(origin + NWW) & ~(FileHBB | FileGBB | Rank1BB))
        | (bit(origin + SWW) & ~(FileHBB | FileGBB | Rank8BB))
        | (bit(origin + SSW) & ~(FileHBB | Rank8BB | Rank7BB))
        | (bit(origin + SSE) & ~(FileABB | Rank8BB | Rank7BB))
        | (bit(origin + SEE) & ~(FileABB | FileBBB | Rank8BB));
}

constexpr int distance(int origin, int dest)
{
    int r2 = dest >> 3;
    int r1 = origin >> 3;
    int f2 = dest % 8;
    int f1 = origin % 8; 
    int r1r2 = std::abs(r2 - r1); 
    int f1f2 = std::abs(f2 - f1);

    return std::max(f1f2, r1r2);
}

constexpr bool in_bounds(int s, Direction d)
{
     return s >= a1 && s <= h8 && distance(s, s-d) == 1;
}

Bitboard trace_ray(int origin, Direction d)
{
    Bitboard ray = 0ULL;
    for(int s=origin + d; in_bounds(s,d); s += d) {
        ray |= bit(s);  
    }
    return ray;
}

void init_behind()
{
    for(Square f=a1; f<=h8; ++f) {
        for(int di=0; di<8; ++di) { 
            Direction d = directions[di];
            for(int t=f+d; in_bounds(t, d); t+=d) {
                behind[f][t] = trace_ray(t,d);
            }
        }
    }
}

void init_piece_attacks()
{
    for(Square sq=a1; sq<=h8; ++sq) {
        Bitboard ne = trace_ray(sq, NE);
        Bitboard sw = trace_ray(sq, SW);
        Bitboard nw = trace_ray(sq, NW);
        Bitboard se = trace_ray(sq, SE);

        Bitboard n = trace_ray(sq, N);
        Bitboard s = trace_ray(sq, S);
        Bitboard e = trace_ray(sq, E);
        Bitboard w = trace_ray(sq, W);

        piece_attacks[Bishop][sq] = ne | sw | nw | se; 
        piece_attacks[Rook][sq] = n | w | e | s; 
        piece_attacks[Queen][sq] = piece_attacks[Bishop][sq] | piece_attacks[Rook][sq];

        piece_attacks[Knight][sq] = knight_mask(sq);
        piece_attacks[King][sq] = king_mask(sq);
    }
}

void init_generator() 
{
    init_behind();
    init_piece_attacks();
}

Bitboard blockers_and_beyond(int p, int from, Bitboard occ)
{
    Bitboard ts = piece_attacks[p][from];
    Bitboard bb = occ;
    while(bb) {
        Square to = pop_bit(bb);
        ts &= ~behind[from][to];
    }
    
    return ts;
}

Bitboard pawn_squares(
        Bitboard friend_occ, 
        Bitboard op_occ, 
        BoardState board_state, 
        int origin)
{
    Colour us = board_state.side_to_move;
    Direction push_dir = us == White ? N : S;  

    Bitboard ratt = bit(origin + push_dir + E);
    Bitboard latt = bit(origin + push_dir + W);
    Bitboard single_push = bit(origin + push_dir);
    Bitboard double_push = bit(origin + push_dir + push_dir);

    Bitboard mask = ratt | latt | single_push | double_push;
    Bitboard squares = 0; 

    // Attacks 
    if(op_occ & ratt) {
        squares |= ratt;
    }
    if(op_occ & latt) {
        squares |= latt;
    }

    //En Passant
    if(board_state.ep_file != -1) {
        int rank = origin >> 3;
        int file = origin % 8;
        if(us == White && rank == 4
        || (us == Black && rank == 3)) {
            squares |= ((board_state.ep_file < file) ? latt : ratt) ;
        } 
    }

    // Double push 
    if((us == Black && origin >= a7 && origin <= h7) 
    || (us == White && origin >= a2 && origin <= h2)) {
        if(!(double_push & op_occ) && !(single_push & friend_occ)) {
            squares |= double_push;
        }
    }

    if(!(single_push & op_occ)) {
        squares |= single_push;
    }

    return squares & mask & ~friend_occ;
}

Bitboard get_to_squares(int p, int from, BoardState board_state)
{
    Bitboard ts = 0; 
    Bitboard friend_occ = board_state.get_friend_occ();
    Bitboard op_occ = board_state.get_op_occ();
    if(p == Pawn) {
        ts = pawn_squares(friend_occ, op_occ, board_state, from);
    } else {
        ts = blockers_and_beyond(p, from, board_state.occ);
    }
    ts &= ~friend_occ;
    return ts;
}

BMove* generator(BoardState board_state) 
{   
    static BMove moves[128];
    int i = 0;
    Colour us = board_state.side_to_move;
    for(int p = Pawn; p <= King; ++p) {
        Bitboard occ = board_state.get_friend_piece_bb(p);
        while(occ) {
            Square origin = pop_bit(occ);
            BMove from = static_cast<BMove>((origin << 6) & 0xfc0);
            PieceType pt = static_cast<PieceType>(p);
            moves[i] = 0;
            Bitboard to_squares = get_to_squares(pt, from, board_state);

            while(to_squares) {
                Square dest = pop_bit(to_squares);
                BMove to = static_cast<BMove>(dest & 0x3f);
                moves[i] = from | to;
                ++i;
            }
        }
    }
    return moves;
}
