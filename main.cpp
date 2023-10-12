#include <iostream>
#include <assert.h>
#include <string.h>
#include <unordered_map>

#define U64 unsigned long long
#define BOARD_SIZE 8
#define WHITE 0
#define BLACK 1
#define BOTH 2
#define get_bit(bitboard, square) ((bitboard & (1ULL << square))?1:0)
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define delete_bit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))
using namespace std;

enum sliding_pieces{rook, bishop};
enum squares{
a8, b8, c8, d8, e8, f8, g8, h8,
a7, b7, c7, d7, e7, f7, g7, h7,
a6, b6, c6, d6, e6, f6, g6, h6,
a5, b5, c5, d5, e5, f5, g5, h5,
a4, b4, c4, d4, e4, f4, g4, h4,
a3, b3, c3, d3, e3, f3, g3, h3,
a2, b2, c2, d2, e2, f2, g2, h2,
a1, b1, c1, d1, e1, f1, g1, h1, no_sq
};
enum castling{wk=1, wq=2, bk=4, bq=8};
enum pieces{P, R, N, B, Q, K, p, r, n, b, q, k};
char ascii_pieces[12] = {'P','R','N','B','Q','K','p','r','n','b','q','k'};
//char *unicode_pieces[12] = {"♟︎","♜","♞","♝","♛","♚","♙","♖","♘","♗","♕","♔"};

unordered_map<char, int> char_pieces = {
    {'P', P}, {'R', R}, {'N', N}, {'B', B}, {'Q', Q}, {'K', K},
    {'p', p}, {'r', r}, {'n', n}, {'b', b}, {'q', q}, {'k', k}
};
/*
        NOT A FILE
 8  0  1  1  1  1  1  1  1
 7  0  1  1  1  1  1  1  1
 6  0  1  1  1  1  1  1  1
 5  0  1  1  1  1  1  1  1
 4  0  1  1  1  1  1  1  1
 3  0  1  1  1  1  1  1  1
 2  0  1  1  1  1  1  1  1
 1  0  1  1  1  1  1  1  1
    a  b  c  d  e  f  g  h
*/
/*
        NOT H FILE
 8  1  1  1  1  1  1  1  0
 7  1  1  1  1  1  1  1  0
 6  1  1  1  1  1  1  1  0
 5  1  1  1  1  1  1  1  0
 4  1  1  1  1  1  1  1  0
 3  1  1  1  1  1  1  1  0
 2  1  1  1  1  1  1  1  0
 1  1  1  1  1  1  1  1  0
    a  b  c  d  e  f  g  h
*/
/*
        NOT GH FILE
 8  1  1  1  1  1  1  0  0
 7  1  1  1  1  1  1  0  0
 6  1  1  1  1  1  1  0  0
 5  1  1  1  1  1  1  0  0
 4  1  1  1  1  1  1  0  0
 3  1  1  1  1  1  1  0  0
 2  1  1  1  1  1  1  0  0
 1  1  1  1  1  1  1  0  0
    a  b  c  d  e  f  g  h
*/
/*
        NOT AB FILE
 8  0  0  1  1  1  1  1  1
 7  0  0  1  1  1  1  1  1
 6  0  0  1  1  1  1  1  1
 5  0  0  1  1  1  1  1  1
 4  0  0  1  1  1  1  1  1
 3  0  0  1  1  1  1  1  1
 2  0  0  1  1  1  1  1  1
 1  0  0  1  1  1  1  1  1
    a  b  c  d  e  f  g  h
*/
const char *square_to_coordinate[] = {
"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};
const U64 not_a_file = 18374403900871474942ULL;
const U64 not_h_file = 9187201950435737471ULL;
const U64 not_gh_file = 4557430888798830399ULL;
const U64 not_ab_file = 18229723555195321596ULL;
const int index64[64] = {
    0,  1, 48,  2, 57, 49, 28,  3,
   61, 58, 50, 42, 38, 29, 17,  4,
   62, 55, 59, 36, 53, 51, 43, 22,
   45, 39, 33, 30, 24, 18, 12,  5,
   63, 47, 56, 27, 60, 41, 37, 16,
   54, 35, 52, 21, 44, 32, 23, 11,
   46, 26, 40, 15, 34, 20, 31, 10,
   25, 14, 19,  9, 13,  8,  7,  6
};
const int bishop_relevant_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};
const int rook_relevant_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};
static inline int count_bits(U64 bitboard);
static inline int get_1st_bit_index(U64 bitboard);
static inline U64 get_bishop_attacks(int square, U64 occupancy);
static inline U64 get_rook_attacks(int square, U64 occupancy);
U64 rook_magic_numbers[64] = {
0x8a80104000800020ULL,
0x140002000100040ULL,
0x2801880a0017001ULL,
0x100081001000420ULL,
0x200020010080420ULL,
0x3001c0002010008ULL,
0x8480008002000100ULL,
0x2080088004402900ULL,
0x800098204000ULL,
0x2024401000200040ULL,
0x100802000801000ULL,
0x120800800801000ULL,
0x208808088000400ULL,
0x2802200800400ULL,
0x2200800100020080ULL,
0x801000060821100ULL,
0x80044006422000ULL,
0x100808020004000ULL,
0x12108a0010204200ULL,
0x140848010000802ULL,
0x481828014002800ULL,
0x8094004002004100ULL,
0x4010040010010802ULL,
0x20008806104ULL,
0x100400080208000ULL,
0x2040002120081000ULL,
0x21200680100081ULL,
0x20100080080080ULL,
0x2000a00200410ULL,
0x20080800400ULL,
0x80088400100102ULL,
0x80004600042881ULL,
0x4040008040800020ULL,
0x440003000200801ULL,
0x4200011004500ULL,
0x188020010100100ULL,
0x14800401802800ULL,
0x2080040080800200ULL,
0x124080204001001ULL,
0x200046502000484ULL,
0x480400080088020ULL,
0x1000422010034000ULL,
0x30200100110040ULL,
0x100021010009ULL,
0x2002080100110004ULL,
0x202008004008002ULL,
0x20020004010100ULL,
0x2048440040820001ULL,
0x101002200408200ULL,
0x40802000401080ULL,
0x4008142004410100ULL,
0x2060820c0120200ULL,
0x1001004080100ULL,
0x20c020080040080ULL,
0x2935610830022400ULL,
0x44440041009200ULL,
0x280001040802101ULL,
0x2100190040002085ULL,
0x80c0084100102001ULL,
0x4024081001000421ULL,
0x20030a0244872ULL,
0x12001008414402ULL,
0x2006104900a0804ULL,
0x1004081002402ULL
};
U64 bishop_magic_numbers[64] = {
0x40040844404084ULL,
0x2004208a004208ULL,
0x10190041080202ULL,
0x108060845042010ULL,
0x581104180800210ULL,
0x2112080446200010ULL,
0x1080820820060210ULL,
0x3c0808410220200ULL,
0x4050404440404ULL,
0x21001420088ULL,
0x24d0080801082102ULL,
0x1020a0a020400ULL,
0x40308200402ULL,
0x4011002100800ULL,
0x401484104104005ULL,
0x801010402020200ULL,
0x400210c3880100ULL,
0x404022024108200ULL,
0x810018200204102ULL,
0x4002801a02003ULL,
0x85040820080400ULL,
0x810102c808880400ULL,
0xe900410884800ULL,
0x8002020480840102ULL,
0x220200865090201ULL,
0x2010100a02021202ULL,
0x152048408022401ULL,
0x20080002081110ULL,
0x4001001021004000ULL,
0x800040400a011002ULL,
0xe4004081011002ULL,
0x1c004001012080ULL,
0x8004200962a00220ULL,
0x8422100208500202ULL,
0x2000402200300c08ULL,
0x8646020080080080ULL,
0x80020a0200100808ULL,
0x2010004880111000ULL,
0x623000a080011400ULL,
0x42008c0340209202ULL,
0x209188240001000ULL,
0x400408a884001800ULL,
0x110400a6080400ULL,
0x1840060a44020800ULL,
0x90080104000041ULL,
0x201011000808101ULL,
0x1a2208080504f080ULL,
0x8012020600211212ULL,
0x500861011240000ULL,
0x180806108200800ULL,
0x4000020e01040044ULL,
0x300000261044000aULL,
0x802241102020002ULL,
0x20906061210001ULL,
0x5a84841004010310ULL,
0x4010801011c04ULL,
0xa010109502200ULL,
0x4a02012000ULL,
0x500201010098b028ULL,
0x8040002811040900ULL,
0x28000010020204ULL,
0x6000020202d0240ULL,
0x8918844842082200ULL,
0x4010011029020020ULL
};
U64 pawn_attacks[2][64];
U64 knight_attacks[64];
U64 king_attacks[64];
U64 bishop_masks[64];
U64 rook_masks[64];
U64 bishop_attacks[64][512];
U64 rook_attacks[64][4096];
U64 piece_bitboards[12];
U64 occupancy_bitboards[3];
int side = -1;
int enpassant = no_sq;
int castle;
void print_bitboard(U64 bitboard);
void print_chess_board();
U64 mask_pawn_attacks(int side, int square);
U64 mask_knight_attacks(int square);
U64 mask_king_attacks(int square);
U64 mask_bishop_attacks(int square);
U64 mask_rook_attacks(int square);
U64 generate_bishop_attacks_with_block(int square, U64 block);
U64 generate_rook_attacks_with_block(int square, U64 block);
void init_all_leapers_attacks();
void init_all_sliders_attacks(int bishop);
void init_all_pawn_attacks();
void init_all_knight_attacks();
void init_all_king_attacks();
void init_magic_numbers();
void init_all();
int bitScanForward(U64 bitboard);
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask);
unsigned int state = 1804289383;
unsigned int get_random_number_32();
U64 get_random_number_64();
U64 generate_magic_number();
U64 find_magic_number(int square, int relevant_bits, U64 bishop);

int main()
{
    //init_all();
    set_bit(piece_bitboards[P], e1);
    set_bit(piece_bitboards[R], b3);
    set_bit(piece_bitboards[K], a4);
    set_bit(piece_bitboards[q], d5);
    set_bit(piece_bitboards[b], f6);
    set_bit(piece_bitboards[n], h8);
    //print_bitboard(piece_bitboards[P]);
    side = WHITE;
    print_chess_board();
    /*cout<<ascii_pieces[P]<<endl;
    cout<<ascii_pieces[char_pieces['K']];*/
    //print_bitboard(occupancy);
    //print_bitboard(get_rook_attacks(e4, occupancy));
    std::cin.get();
    return 0;
}

void print_bitboard(U64 bitboard){
    for(int i=0; i<BOARD_SIZE; i++)
    {
        for(int j=0; j<BOARD_SIZE; j++)
        {
            if(!j)
            {
                cout<<' '<<8-i<<' ';
            }
            int square = (i*8)+j;
            cout<<' '<<get_bit(bitboard, square)<<' ';
        }
        cout<<endl;
        if(i==7)
        {
            for(char k='a'; k<='h'; k++)
            {
                if(k=='a')
                {
                    cout<<"    "<<k;
                }
                else
                {
                    cout<<"  "<<k;
                }
            }
            cout<<endl;
        }
    }
    cout<<"Bitboard: "<<bitboard<<endl;
}
void print_chess_board(){
    for(int i=0; i<BOARD_SIZE; i++)
    {
        for(int j=0; j<BOARD_SIZE; j++)
        {
            int square = i*8 + j;
            if(!j)
            {
                cout<<' '<<8-i<<' ';
            }
            int piece = -1;
            for(int bb_count = P; bb_count<=k; bb_count++)
            {
                if(get_bit(piece_bitboards[bb_count], square)) piece = bb_count;
            }
            cout<<' '<<((piece == -1)?'.':ascii_pieces[piece])<<' ';
        }
        cout<<endl;
        if(i==7)
        {
            for(char k='a'; k<='h'; k++)
            {
                if(k=='a')
                {
                    cout<<"    "<<k;
                }
                else
                {
                    cout<<"  "<<k;
                }
            }
            cout<<endl;
        }
    }
    cout<<' '<<((!side)?"white":"black")<<' ';
}

U64 mask_pawn_attacks(int side, int square){
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    set_bit(bitboard, square);
    if(!side)
    {
        if((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
        if((bitboard >> 9)& not_h_file) attacks |= (bitboard >> 9);
    }
    else
    {
        if((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
        if((bitboard << 9)& not_a_file) attacks |= (bitboard << 9);
    }
    return attacks;
}
void init_all_pawn_attacks()
{
    for(int i=0; i<BOARD_SIZE*BOARD_SIZE; i++)
    {
        pawn_attacks[WHITE][i] = mask_pawn_attacks(WHITE, i);
        pawn_attacks[BLACK][i] = mask_pawn_attacks(BLACK, i);
    }
}
U64 mask_knight_attacks(int square){
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    set_bit(bitboard, square);
    if((bitboard >> 15) & not_a_file) attacks |= (bitboard >> 15);
    if((bitboard >> 17)& not_h_file) attacks |= (bitboard >> 17);
    if((bitboard >> 10)& not_gh_file) attacks |= (bitboard >> 10);
    if((bitboard >> 6)& not_ab_file) attacks |= (bitboard >> 6);
    if((bitboard << 15) & not_h_file) attacks |= (bitboard << 15);
    if((bitboard << 17)& not_a_file) attacks |= (bitboard << 17);
    if((bitboard << 10)& not_ab_file) attacks |= (bitboard << 10);
    if((bitboard << 6)& not_gh_file) attacks |= (bitboard << 6);
    return attacks;
}
void init_all_knight_attacks(){
    for(int i=0; i<BOARD_SIZE*BOARD_SIZE; i++)
    {
        knight_attacks[i] = mask_knight_attacks(i);
        knight_attacks[i] = mask_knight_attacks(i);
    }
}
U64 mask_king_attacks(int square){
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    set_bit(bitboard, square);
    if((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
    if((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
    if((bitboard >> 1) & not_h_file) attacks |= (bitboard >> 1);
    if((bitboard << 1) & not_a_file) attacks |= (bitboard << 1);
    if((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
    if((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);
    if((bitboard >> 8)) attacks |= (bitboard >> 8);
    if((bitboard << 8)) attacks |= (bitboard << 8);
    return attacks;
}
void init_all_king_attacks(){
    for(int i=0; i<BOARD_SIZE*BOARD_SIZE; i++)
    {
        king_attacks[i] = mask_king_attacks(i);
        king_attacks[i] = mask_king_attacks(i);
    }
}
U64 mask_bishop_attacks(int square){
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    set_bit(bitboard, square);
    int i, j, targetI, targetJ;
    targetI = square / 8;
    targetJ = square % 8;
    for(i= targetI+1, j=targetJ+1; i<7 && j<7; i++, j++)
    {
        attacks |= (1ULL<<(i*8+j));
    }
    for(i= targetI-1, j=targetJ+1; i>0 && j<7; i--, j++)
    {
        attacks |= (1ULL<<(i*8+j));
    }
    for(i= targetI+1, j=targetJ-1; i<7 && j>0; i++, j--)
    {
        attacks |= (1ULL<<(i*8+j));
    }
    for(i= targetI-1, j=targetJ-1; i>0 && j>0; i--, j--)
    {
        attacks |= (1ULL<<(i*8+j));
    }
    return attacks;
}
U64 mask_rook_attacks(int square){
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    set_bit(bitboard, square);
    int i,j, targetI, targetJ;
    targetI = square/8;
    targetJ = square%8;
    for(i= targetI+1; i<7; i++)
    {
        attacks |= (1ULL<<(i*8)+targetJ);
    }
    for(i= targetI-1; i>0; i--)
    {
        attacks |= (1ULL<<(i*8)+targetJ);
    }
    for(j= targetJ+1; j<7; j++)
    {
        attacks |= (1ULL<<(targetI*8)+j);
    }
    for(j= targetJ-1; j>0; j--)
    {
        attacks |= (1ULL<<(targetI*8)+j);
    }
    return attacks;
}
void init_all_leapers_attacks(){
    init_all_pawn_attacks();
    init_all_knight_attacks();
    init_all_king_attacks();
}
U64 generate_bishop_attacks_with_block(int square, U64 block){
    U64 attacks = 0ULL;
    int i, j, targetI, targetJ;
    targetI = square / 8;
    targetJ = square % 8;
    for(i= targetI+1, j=targetJ+1; i<8 && j<8; i++, j++)
    {
        attacks |= (1ULL<<(i*8+j));
        if((1ULL<<(i*8+j)) & block) break;
    }
    for(i= targetI-1, j=targetJ+1; i>=0 && j<8; i--, j++)
    {
        attacks |= (1ULL<<(i*8+j));
        if((1ULL<<(i*8+j)) & block) break;
        
    }
    for(i= targetI+1, j=targetJ-1; i<8 && j>=0; i++, j--)
    {
        attacks |= (1ULL<<(i*8+j));
        if((1ULL<<(i*8+j)) & block) break;
    }
    for(i= targetI-1, j=targetJ-1; i>=0 && j>=0; i--, j--)
    {
        attacks |= (1ULL<<(i*8+j));
        if((1ULL<<(i*8+j)) & block) break;
    }
    return attacks;
}
U64 generate_rook_attacks_with_block(int square, U64 block){
    U64 attacks = 0ULL;
    int i,j, targetI, targetJ;
    targetI = square/8;
    targetJ = square%8;
    for(i= targetI+1; i<8; i++)
    {
        attacks |= (1ULL<<(i*8)+targetJ);
        if((1ULL<<(i*8)+targetJ) & block) break;
    }
    for(i= targetI-1; i>=0; i--)
    {
        attacks |= (1ULL<<(i*8)+targetJ);
        if((1ULL<<(i*8)+targetJ) & block) break;
    }
    for(j= targetJ+1; j<8; j++)
    {
        attacks |= (1ULL<<(targetI*8)+j);
        if((1ULL<<(targetI*8)+j) & block) break;
    }
    for(j= targetJ-1; j>=0; j--)
    {
        attacks |= (1ULL<<(targetI*8)+j);
        if((1ULL<<(targetI*8)+j) & block) break;
    }
    return attacks;
}
static inline int count_bits(U64 bitboard){
    int count = 0;
    while(bitboard)
    {
        count++;
        bitboard &= bitboard -1;
    }
    return count;
}
static inline int get_1st_bit_index(U64 bitboard){
    if(bitboard)
    {
        return count_bits((bitboard & -bitboard)-1);
    }
    else return -1;
}
int bitScanForward(U64 bitboard) {
   const U64 debruijn64 = 0x03f79d71b4cb0a89;
   assert(bitboard != 0);
   return index64[((bitboard & - bitboard) * debruijn64) >> 58];
}
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask){
    U64 occupancy = 0ULL;
    for(int i=0; i<bits_in_mask; i++)
    {
        int square = get_1st_bit_index(attack_mask);
        delete_bit(attack_mask, square);
        if(index &  (1<<i))
        {
            occupancy |= (1ULL << square);
        }
    }
    return occupancy;
}
unsigned int get_random_number_32(){
    unsigned int number = state;
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;
    state = number;
    return number;
}
U64 get_random_number_64(){
    U64 r1, r2, r3, r4;
    r1 = (U64)(get_random_number_32() & 0xFFFF);
    r2 = (U64)(get_random_number_32() & 0xFFFF);
    r3 = (U64)(get_random_number_32() & 0xFFFF);
    r4 = (U64)(get_random_number_32() & 0xFFFF);
    return r1 | (r2<<16) | (r3<<32) | (r4<<48);
}
U64 generate_magic_number(){
    return (get_random_number_64() & get_random_number_64() & get_random_number_64());
}
U64 find_magic_number(int square, int relevant_bits, int bishop){
    U64 occupancies[4096];
    U64 attacks[4096];
    U64 used_attacks[4096];
    U64 attack_mask = bishop ? mask_bishop_attacks(square): mask_rook_attacks(square);
    int occupancy_indices = 1 << relevant_bits;
    for(int i=0; i<occupancy_indices; i++)
    {
        occupancies[i] = set_occupancy(i, relevant_bits, attack_mask);
        attacks[i] = bishop ? generate_bishop_attacks_with_block(square, occupancies[i]): generate_rook_attacks_with_block(square, occupancies[i]);
    } 
    for(int random=0; random<100000000; random++)
    {
        U64 magic_number = generate_magic_number();
        if (count_bits((attack_mask*magic_number) & 0xFF00000000000000)<6) continue; 
        memset(used_attacks, 0ULL, sizeof(used_attacks));
        int i, j;
        for(i=0, j=0; !j && i < occupancy_indices; i++)
        {
            int magic_index = (int)((occupancies[i] * magic_number) >> (64-relevant_bits));
            if(used_attacks[magic_index] == 0ULL)
            {
                used_attacks[magic_index] = attacks[i];
            }
            else if(used_attacks[magic_index] != attacks[i])
            {
                j=1;
            }
        }
        if(!j) return magic_number;
    }
    cout<<"failed";
    return 0ULL;
}
void init_magic_numbers(){
    for(int i=0; i<BOARD_SIZE*BOARD_SIZE; i++)
    {
        rook_magic_numbers[i] = find_magic_number(i, rook_relevant_bits[i], rook);
    }
    for(int i=0; i<BOARD_SIZE*BOARD_SIZE; i++)
    {
        bishop_magic_numbers[i] = find_magic_number(i, bishop_relevant_bits[i], bishop);
    }
}   
void init_all(){
    init_all_leapers_attacks();
    init_all_sliders_attacks(bishop);
    init_all_sliders_attacks(rook);
    //add anothers
}
void init_all_sliders_attacks(int bishop){
    for(int i=0; i<BOARD_SIZE*BOARD_SIZE; i++)
    {
        bishop_masks[i] = mask_bishop_attacks(i);
        rook_masks[i] = mask_rook_attacks(i);
        U64 attack_mask = bishop ? bishop_masks[i] : rook_masks[i];
        int relevant_bits_count = count_bits(attack_mask);
        int occupancy_indices = (1 << relevant_bits_count);
        for(int j=0; j<occupancy_indices; j++)
        {
            if(bishop)
            {
                U64 occupancy = set_occupancy(j, relevant_bits_count, attack_mask);
                int magic_index = (occupancy * bishop_magic_numbers[i]) >> (64-bishop_relevant_bits[i]);
                bishop_attacks[i][magic_index] = generate_bishop_attacks_with_block(i, occupancy);
            }
            else
            {
                U64 occupancy = set_occupancy(j, relevant_bits_count, attack_mask);
                int magic_index = (occupancy * rook_magic_numbers[i]) >> (64-rook_relevant_bits[i]);
                rook_attacks[i][magic_index] = generate_rook_attacks_with_block(i, occupancy);
            }
        }
    }
}
static inline U64 get_bishop_attacks(int square, U64 occupancy){
    occupancy &= bishop_masks[square];
    occupancy *= bishop_magic_numbers[square];
    occupancy >>= 64-bishop_relevant_bits[square];
    return bishop_attacks[square][occupancy];
}
static inline U64 get_rook_attacks(int square, U64 occupancy){
    occupancy &= rook_masks[square];
    occupancy *= rook_magic_numbers[square];
    occupancy >>= 64-rook_relevant_bits[square];
    return rook_attacks[square][occupancy];
}











