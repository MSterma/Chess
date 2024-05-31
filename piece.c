typedef struct piece {
    char name;
    char color;
    int pos;
    int* moveset;
} piece;
int king_moveset[8][2] = {
    {1,-1},
    {1,0},
    {1,1},
    {0,1},
    {-1,1},
    {-1,0},
    {-1,-1},
    {0,-1}
};
int pawn_moveset[8][2] = {

    {0,1},
    {1,1},
    {-1,1}
};
