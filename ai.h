#ifndef AI_H
#define AI_H

/*
Built off the sample c++ AI by Mike Yang
  (only code that's the same is HTTP stuff)

From him:
The C++ standard library does not include HTTP or JSON functionality.
This file makes use of two libraries:
  libcurl (actually a C library)
  JSON for Modern C++
Make sure to include the two!
Remember to compile using -lcurl to link the curl library
Additionally, compile using at least C++11 for json.h

mu - move up
ml - move left
md - move down
mr - move right
tu - turn up
tl - turn left
td - turn down
tr - turn right
b - place bomb
op - shoot orange portal
bp - shoot blue portal
(empty string) - do nothing
buy_count - costs 1, increases number of bombs you can have out by 1 (bomb count)
buy_pierce - costs 1, increases range of your bombs after collision with solid objects
buy_range - costs 1, increases raw range of bombs
buy_block - costs <value of block>, creates a block in the square you’re facing assuming there’s not a solid object already there
Value formula (min 1, even if the formula returns 0):

*/

#include <algorithm>
#include <curl/curl.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <queue>
#include <sstream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/time.h>
#include <thread>
#include <time.h>
#include <unistd.h>
#include <vector>

#include "game.h"

#include "json.h"
using json = nlohmann::json;

class Creature {
  public:
    Creature();
    Creature(Creature* parent);
    Creature(std::vector<float>& weights, int parentNum);

    int age(); // wins + losses
    int avg(); // score / age;

    std::vector<float> weights;
    // current policy means losses dont matter, but keep track of them just in case
    int wins, losses, score;
    int playerNum;
};

#define DEV_KEY "581a9c3dda8dce85358c62b8"
#define USERNAME "milk"
#define URL "http://aicomp.io/api/games/submit/"


#define TIMEOUT_ABSOLUTE 16000 // 15 second timeout for making a move, set to 16 for leeway before program termination
#define TIMEOUT TIMEOUT_ABSOLUTE-1500 // milliseconds until termination of minimax
#define SEARCH_DEPTH 11 // minimax depth (11 guarantees you wont kill yourself)
#define MLCT 13 // number of weights
#define MAX_WEIGHT 30 // maximum weight for a variable
#define POP_SIZE 100 // population size for each generation
#define NEW_CHANCE 10 // percent chance that a gene will randomize
#define MAX_DELTA 1 // maximum change of a gene in one generation

#define Y1 1 // number of games to simulate in phase 2
#define Y2 5 // number of games to simulate in phase 9
#define GENERATIONS 1000 // number of generations to run

#define OPP_SEARCH_DEPTH 11 // want the opponent to be able to make a mistake so games don't go forever

// sanity cutoffs for simulations
#define HARDCUTOFF 150 // if game goes this long, end it
#define SOFTCUTOFF HARDCUTOFF-50  // make sure player has coins at this point

class AI {
  public:
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
    static void PostMove(json* j, std::string move);
    static void StartServerGame(json* j, bool practice);
    static void StartPracticeGame(json* j);
    static void StartMatchmadeGame(json* j);

    void hotseatGame();
    void onePlayerGame();
    void duckGame();
    void cpuVsCpuGame();
    void machineLearn();
    void serverGame(bool practice);
    void practiceServerGame();
    void matchmadeServerGame();
    void hostServer();

    // Minimax functions
    static bool lessComp(const Game* firstElem, const Game* secondElem);
    static bool greatComp(const Game* firstElem, const Game* secondElem);
    static void qsortGames(std::vector<Game*>& games, int l, int r);

    void loadWeights(Creature* creature);
    int heuristic(Game* node);
    std::string chooseMove();
    int minimax(Game* node, int depth, int alpha, int beta);
    void doMinimax(int* out, Game* node, int depth);

    void initGame(char* arg, bool continuous);

    // The real game (not a minimax simulation)
    Game* realGame;

    bool verbose;
    timespec start;
    // Why not throw in some ML?
    bool learning;
    Creature *p1Creature, *p2Creature, *currentCreature;

    float A, B, C, D, E, G, H, I, J, K, L, N, O; // There is no F
};

#endif
