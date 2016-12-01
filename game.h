#ifndef GAME_H
#define GAME_H

// This game code is a port of Darwin's node code. I can optimize this a lot but
//  probably don't have time to. Do not take any of this as best practice in C++

// Player ids
#define NO_PLAYER -1
#define BOTH_PLAYER 2
#define PLAYER1 0
#define PLAYER2 1

// Cardinal directions
#define NODIR -1 // no direction
#define WEST 0
#define NORTH 1
#define EAST 2
#define SOUTH 3

// Orientations
#define HORIZONTAL 0
#define VERTICAL 1
#define ORIGIN 2

// Things that can be in a SPace
#define SP_OUT 0
#define SP_HARDBLOCK 1
#define SP_SOFTBLOCK 2
#define SP_BOMB 3
#define SP_PLAYER1 4
#define SP_PLAYER2 5
#define SP_AIR 6

// Portal Colors
#define PC_ORANGE 0
#define PC_BLUE 1


#include <cmath>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include <stdexcept>

#include "json.h"
using json = nlohmann::json;

class Player;

class Coord {
  public:
    Coord();
    Coord(int x, int y);
    Coord(int x, int y, int dir);
    Coord(const Coord& other);

    std::string toString();

    int x, y; // position
    int dir; // direction
};

class Portal {
  public:
    Portal();
    Portal(Player* owner, int color, Coord& location);
    Portal(Portal* other, Player* newOwner);

    Coord location;
    Player* owner;
    int color;
};

class Trail {
  public:
    Trail(int owner, int tick, int type);
    Trail(Trail* other);

    void set(int owner, int tick, int type);

    int p1Tick, p1Type;
    bool p1Exist;

    int p2Tick, p2Type;
    bool p2Exist;
};

class Bomb {
  public:
    Bomb(Player* owner, Coord& location);
    Bomb(Bomb* other, Player* newOwner);

    Coord location;
    Player* owner;
    int tick;
};

class Player {
  public:
    Player(int playerNum);
    Player(Player* other);

    void kill();

    bool alive;
    int playerNum; // is this player1 or player2?
    int coins, bombCount, bombPierce, bombRange;
    Coord location;
    Portal *orangePortal, *bluePortal;

    std::string lastMove;
};

class Game {
  // The actual game. I just converted Darwin's node code to c++
  // because node is garbage

  public:
    Game();
    Game(Game* other);
    virtual ~Game();

    void loadFromJSON(const json& j);
    std::string hash();

    int whoWon();
    bool isOutOfBounds(const Coord& loc);

    Coord getNextSquare(int x, int y, int direction);
    Coord simulateMovement(int x, int y, int direction);

    int getBlockValue(int x, int y);
    int querySpace(int x, int y);

    void trailResolveSquare(int x, int y);
    void placeTrail(Player* owner, int x, int y, int type);
    void recursiveDetonate(int x, int y, int direction, int range, int pierce, bool pierceMode);
    void detonate(int x, int y);

    void deletePortal(int x, int y, int direction);
    void shootPortal(Player* owner, int direction, int portalColor);

    std::string submit(Player* currentPlayer, const std::string& move);

    std::vector<std::string> filterPointlessMoves();
    std::vector<Game*> getChildren();

    void render();

    static bool isAMoveMove(std::string move);
    static std::string getCounterMove(std::string move);
    static int getDirectionOf(std::string clm);

    static std::vector<std::string> allMoves;

    // Maybe should use getters but meh

    int boardSize;
    bool running, p1First; // Whether player1 goes first this turn
    int winner, mePlayer, lastPlayer; // 0 for player1, 1 for player2;
    std::string lastMove; // Here we have the move that was last made. Or something.
    Player *player1, *player2; // Player class also has person id

    Player *currentPlayer;
    std::map<int, Bomb*> bombMap;

    std::vector<int> hardBlockBoard, softBlockBoard;
    int currentTurn; // 0 for first in order, 1 for second

    std::map<int, Trail*> trailMap;
    std::map<int, std::map<int, Portal*> > portalMap;
};

#endif
