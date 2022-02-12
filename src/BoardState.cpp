#include "BoardState.h"

enum Section 
{ 
    Board, 
    SideToMove, 
    CastlingRights, 
    EPSquare, 
    HalfmoveClock, 
    FullmoveCounter 
};

bool is_piece_ch(char ch);
Piece fen_to_piece(char ch);

// TODO: check if fen is properly formatted 
//       add update function
//

void BoardState::init_squares(std::string fen)
{
    // Parse out pieces 
    int rank=7;
    int file=0;
    int i=0;
    int section=0;
    
    for(int i=a1; i<=h8; ++i) {
        squares[i] = None;
    }

    while(fen[i] != ' ') {
        int sq = (rank)*8 + file;

        if(fen[i] == '/') {
            rank--;
            file=0;
        }

        if(fen[i] >= '0' && fen[i] <= '8') {
            file += (fen[i] - 0x30);
        }

        if(is_piece_ch(fen[i])) {
            squares[sq] = fen_to_piece(fen[i]);
            file++;
        } 

        i++;
    }

    section++;

    // Parse out info
    std::string info = fen.substr(i + 1, fen.length());
    for(int i=0; i<info.length(); ++i) {
        if(info[i] == ' ') {
            section++;
            continue;
        }
        switch(section) {
            case SideToMove: 
                if(info[i] == 'w') {
                    side_to_move = White;
                } else {
                    side_to_move = Black;
                }
                break;
            case CastlingRights:
                switch(info[i]) {
                    case '-':
                        w_castle_ks = false; 
                        w_castle_qs = false; 
                        b_castle_ks = false; 
                        b_castle_qs = false; 
                        break;
                    case 'k':
                        b_castle_ks = true;
                        break;
                    case 'q':
                        b_castle_qs = true;
                        break;
                    case 'K': 
                        w_castle_ks = true;
                        break;
                    case 'Q':
                        w_castle_qs = true;
                        break;
                    default: break;
                }
                break;
            case EPSquare:
                if(info[i] == '-') {
                    ep_file = -1;
                } else if(info[i] >= 'a' && info[i] <= 'h') {
                    ep_file = static_cast<int>(info[i] - 0x61);
                } 
                break;
            case HalfmoveClock:
                halfmove_clock = static_cast<int>(info[i] - 0x30);
                break;
            case FullmoveCounter:
                ply_count = static_cast<int>(info[i] - 0x30);
                break;
            default: break;
        }
    }
}

void BoardState::init_bbs() 
{
    white_occ = 0ULL; 
    black_occ = 0ULL;
    for(int s = 0; s<64; ++s) {
        int p = (int)squares[s];
        if(p >= 0) { 
            int pt = conversions::piece_to_piecetype(p);
            piece_bbs[p] |= (1ULL << s);
            occ |= piece_bbs[p];
            if(p >= 6 && p <= 11) {
                white_piece_bbs[pt] |= (1ULL << s);
                white_occ |= (1ULL << s);
            } else {
                black_piece_bbs[pt] |= (1ULL << s);
                black_occ |= (1ULL << s);
            }
        }
    }
}

void BoardState::init(std::string fen)
{
    init_squares(fen);
    init_bbs(); 
}

void BoardState::do_castle(int rook_from, int rook_to, int king_to)
{
    Piece rook = side_to_move == White ? WR : BR;
    Piece king = side_to_move == White ? WK : BK;
    int king_from = side_to_move == White ? e1 : e8;

    // Squares
    squares[king_from] = None;
    squares[rook_from] = None;
    squares[king_to] = king;
    squares[rook_to] = rook;
    // Bitboards
    piece_bbs[king] &= ~(1ULL << king_from);
    piece_bbs[rook] &= ~(1ULL << rook_from);
    piece_bbs[king] |= (1ULL << king_to);
    piece_bbs[rook] |= (1ULL << rook_to);
}

void BoardState::castle_kingside()
{
    int rook_from = side_to_move == White ? h1 : h8;
    int rook_to = side_to_move == White ? f1 : f8;
    int king_to = side_to_move == White ? g1 : g8;
    
    do_castle(rook_from, rook_to, king_to);
}

void BoardState::castle_queenside()
{
    int rook_from = side_to_move == White ? a1 : a8;
    int rook_to = side_to_move == White ? c1 : c8;
    int king_to = side_to_move == White ? d1 : d8;

    do_castle(rook_from, rook_to, king_to);
}

constexpr Bitboard temp_get_bit(Bitboard bb, int x, int y) 
{ 
    return (bb >> y*8 + x) & 1ULL;
}

void temp_print(Bitboard bb) 
{
    for(int y=7; y >=0; --y){
        std::cout << '\n';
        for(int x=0; x < 8; ++x)
            std::cout << temp_get_bit(bb, x,y) << " ";
    }
    std::cout << '\n';
}

// Assume legal
void BoardState::make_move(BMove m) 
{
    int from = (m >> 6) & 0x3f;
    int to = m & 0x3f;
    int flag = m & 0xf;
    int ep_pawn = -1;
    int ep_pawn_square = -1;
    int ep_rank;
    Piece p = squares[from];
    int pt = conversions::piece_to_piecetype(p);

    // Castling
    if(m == OO) {
        castle_kingside();
    } else if(m == OOO) {
        castle_queenside();
    } else {
        if(flag == EN_PASSANT) {
            if(side_to_move == White) {
                ep_rank = to + S;
                ep_pawn = BP;
            } else {
                ep_rank = to + N;
                ep_pawn = WP;
            }
            ep_pawn_square = ep_rank*8 + ep_file;
            squares[ep_pawn_square] = None;
            piece_bbs[ep_pawn] &= ~(1ULL << ep_pawn_square);
            occ &= ~(1ULL << ep_pawn_square);
        }
        // Squares
        squares[from] = None;
        squares[to] = p;
        // Bitboards
        piece_bbs[p] &= ~(1ULL << from); 
        piece_bbs[p] |= (1ULL << to); 
        occ &= ~(1ULL << from); 
        occ |= (1ULL << to); 
       
        // Specific occupations
        if(side_to_move == White) {
            if(flag == EN_PASSANT) {
                black_piece_bbs[Pawn] &= ~(1ULL << ep_pawn_square);
                black_occ &= ~(1ULL << ep_pawn_square);
            }
            white_piece_bbs[pt] &= ~(1ULL << from); 
            white_piece_bbs[pt] |= (1ULL << to); 
            white_occ &= ~(1ULL << from); 
            white_occ |= (1ULL << to); 
        } else {
            if(flag == EN_PASSANT) {
                white_piece_bbs[Pawn] &= ~(1ULL << ep_pawn_square);
                white_occ &= ~(1ULL << ep_pawn_square);
            }
            black_piece_bbs[pt] &= ~(1ULL << from); 
            black_piece_bbs[pt] |= (1ULL << to); 
            black_occ &= ~(1ULL << from); 
            black_occ |= (1ULL << to); 
        }
    }

    side_to_move = static_cast<Colour>(!(bool)side_to_move); 
    movelist[movelist_idx] = m;
    movelist_idx++;
}

Bitboard BoardState::get_friend_occ()
{
    return side_to_move == White ? white_occ : black_occ;
}

Bitboard BoardState::get_op_occ()
{
    return side_to_move == Black ? white_occ : black_occ;
}

Bitboard BoardState::get_friend_piece_bb(int pt) 
{
    return side_to_move == White ? white_piece_bbs[pt] : black_piece_bbs[pt];
}

Bitboard BoardState::get_op_piece_bb(int pt) 
{
    return side_to_move == Black ? white_piece_bbs[pt] : black_piece_bbs[pt];
}

bool is_piece_ch(char ch) 
{
    return (
            ch == 'p'
         || ch == 'r'
         || ch == 'q'
         || ch == 'k'
         || ch == 'n'
         || ch == 'b'
         || ch == 'P'
         || ch == 'R'
         || ch == 'Q'
         || ch == 'K'
         || ch == 'N'
         || ch == 'B'
    );
}


Piece fen_to_piece(char ch) 
{
    switch(ch) {
        case 'p': 
            return BP;
        case 'r':
            return BR;
        case 'q':
            return BQ;
        case 'k':
            return BK;
        case 'n':
            return BN;
        case 'b':
            return BB;
        case 'P':
            return WP;
        case 'R':    
            return WR;
        case 'Q':    
            return WQ;
        case 'K':    
            return WK;
        case 'N':    
            return WN;
        case 'B':    
            return WB;
        default: break;
    }
    return None;
}
