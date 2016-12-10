#include "ai.h"

void sleep(int milisec) {
  struct timespec req = {0};
  req.tv_sec = 0;
  req.tv_nsec = milisec * 1000000L;
  nanosleep(&req, (struct timespec *)NULL);
}

float randFloat(int mx) {
  float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
  return mx * r;
}

timespec timeElapsed(timespec start, timespec end) {
  timespec temp;
  if ((end.tv_nsec-start.tv_nsec) < 0) {
    temp.tv_sec = end.tv_sec-start.tv_sec-1;
    temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }
  return temp;
}

long tsMs(const timespec convert) {
  int mill = (int)(convert.tv_sec*1000 + convert.tv_nsec/1000000L);
  return mill;
}

////////////////////////////
// Creature definitions
////////////////////////////

Creature::Creature() {
  for (size_t i = 0; i < MLCT; i++) {
    this->weights.push_back(randFloat(MAX_WEIGHT));
  }

  this->wins = 0;
  this->losses = 0;
  this->score = 0;

  this->playerNum = rand() % 2;
}

Creature::Creature(Creature* parent) {
  // use MLCT instead of parent->weights.size() to help catch bugs
  for (size_t i = 0; i < MLCT; i++) {
    float w;
    if (rand() % 100 < NEW_CHANCE) {
      w = randFloat(MAX_WEIGHT);
    } else {
      w = parent->weights[i];
      w = std::max(0.01f, w - randFloat(2*MAX_DELTA) + MAX_DELTA);
    }

    this->weights.push_back(w);
  }

  this->wins = 0;
  this->losses = 0;
  this->score = 0;

  this->playerNum = parent->playerNum;
}

Creature::Creature(std::vector<float>& weights, int parentNum) {
  for (size_t i = 0; i < MLCT; i++) {
    this->weights.push_back(weights[i]);
  }

  this->wins = 0;
  this->losses = 0;
  this->score = 0;

  this->playerNum = parentNum;
}

int Creature::age() {
  return this->wins + this->losses;
}

int Creature::avg() {
  return (int)(this->score / (this->age()+1));
}

////////////////////////////
// AI definitions
////////////////////////////

size_t AI::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void AI::StartServerGame(json* j, bool practice) {
  CURL* curl;
  CURLcode res;
  std::string read_buffer;
  std::string post_fields = "devkey=" DEV_KEY "&username=" USERNAME;

  curl = curl_easy_init();
  if (curl) {
    if (practice) {
      curl_easy_setopt(curl, CURLOPT_URL, "http://aicomp.io/api/games/practice");
    } else {
      curl_easy_setopt(curl, CURLOPT_URL, "http://aicomp.io/api/games/search");
    }

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, AI::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    }

    *j = json::parse(read_buffer);
    std::cout << "Connected to the game! Hoo-rah!" << std::endl;
  }

  curl_easy_cleanup(curl);
}

void AI::StartPracticeGame(json* j) {
  AI::StartServerGame(j, true);
}

void AI::StartMatchmadeGame(json* j) {
  AI::StartServerGame(j, false);
}

void AI::PostMove(json* j, std::string move) {
  CURL* curl;
  CURLcode res;
  std::string read_buffer;

  std::string url = URL;
  url += (*j)["gameID"].get<std::string>();

  std::string data = "devkey=" DEV_KEY "&playerID=";
  data += (*j)["playerID"].get<std::string>();
  data += "&move=" + move;


  curl = curl_easy_init();
  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, AI::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    }

    *j = json::parse(read_buffer);
  }

  curl_easy_cleanup(curl);
}

void AI::hotseatGame() {
  while (this->realGame->running) {
    std::string pagmove = "";

    std::cout << "player" << this->realGame->currentPlayer->playerNum + 1 << ", enter a move: " << std::endl;
    std::cin >> pagmove;

    if (pagmove == "hint") {
      pagmove = this->chooseMove();
      std::cout << "Found a probably good move: " << pagmove << std::endl;
    } else {
      this->realGame->submit(this->realGame->currentPlayer, pagmove);
      this->realGame->render();
    }
  }
}

void AI::onePlayerGame() {
  int p = rand() % 2;
  this->verbose = true;
  while (this->realGame->running) {
    std::string pagmove = "";
    std::cout << "Player" << this->realGame->currentPlayer->playerNum + 1 << " to move." << std::endl;
    if (this->realGame->currentPlayer->playerNum == p) {
      std::cout << "Human Player, enter a move: " << std::endl;
      std::cin >> pagmove;
    } else {
      std::cout << "CPU player thinking..." << std::endl;
      pagmove = this->chooseMove();
      sleep(500);
    }

    this->realGame->submit(this->realGame->currentPlayer, pagmove);
    this->realGame->render();
  }
}

void AI::duckGame() {
  this->verbose = true;
  int p = rand() % 2;
  while (this->realGame->running) {
    std::string pagmove = "";

    std::cout << "Player" << this->realGame->currentPlayer->playerNum + 1 << " to move." << std::endl;
    if (this->realGame->currentPlayer->playerNum == p) {
      pagmove = "b";
      std::vector<std::string> am = this->realGame->filterPointlessMoves();
      while (pagmove == "b") {
        int r = rand() % am.size();
        pagmove = am[r];
      }
      std::cout << "move was " << pagmove << std::endl;
    } else {
      pagmove = this->chooseMove();
    }

    this->realGame->submit(this->realGame->currentPlayer, pagmove);
    if (this->verbose) {
      std::cout << "Move: " << this->realGame->moveNumber << std::endl;
    }

    this->realGame->render();
  }
}

void AI::cpuVsCpuGame() {
  this->verbose = true;
  while (this->realGame->running) {
    std::string pagmove = "";

    std::cout << "Player" << this->realGame->currentPlayer->playerNum + 1 << " to move." << std::endl;

    pagmove = this->chooseMove();
    std::cout << "Found a probably good move: " << pagmove << std::endl;

    this->realGame->submit(this->realGame->currentPlayer, pagmove);
    this->realGame->render();
  }
}


////
// The ML plan (probably only going to implement up to step 9)
////
// 1)  initialize a random population
// 2)  run each individual for Y1=3 "years"" (no mutations)
// 3)  prune all that didnt survive all Y1 [this can be done as 2 is happening]
// 4)  remove lowest until there is no more than 55% original population
// 5)  remove randomly until there is no more than 45% original population
// 6)  for each individual left, reproduce asexually (with mutations)
// 7)  fill the rest of the spaces randomly
// 8)  repeat 1-8 for G=1000 generations
// 9)  run each individual for Y2=5 "years"
// 10) prune each individual that didnt survive all Y2 [this can be done as 9 is happening]
// 11) analyze individuals for correlations in data (e.g. weight A high -> weight B low)
// 12) create linked genes
// 13) run traditional GA with linked gene crossover on a new random population

void AI::machineLearn() {
  // this is gonna be one big-ol' function
  this->learning = true;
  this->verbose = false;

  // initialize popluation
  std::vector<Creature*> population;
  for (size_t i = 0; i < POP_SIZE; i++) {
    population.push_back(new Creature()); // default constructor is random weights
  }

  for (size_t g = 0; g < GENERATIONS; g++) {
    std::cout << "Starting generation " << g+1 << std::endl;
    // run each individual for some number of games
    int creatNum = 0;
    for (auto creatureIt = population.begin(); creatureIt != population.end(); ) {
      bool dunzo = false; // whether this creature gets the boot
      Creature* curCreat = *creatureIt;
      std::cout << std::endl << "Starting creature #" << ++creatNum << " (age: " << curCreat->age() << ")" << "..." << std::endl;
      for (size_t k = 0; k < curCreat->weights.size(); k++) {
        std::cout << curCreat->weights[k] << ", ";
      }
      std::cout << std::endl;
      this->currentCreature = curCreat;

      bool ranOutOfTime = false;
      for (size_t j = 0; j < Y1; j++) {
        if (dunzo) {
          break;
        }

        // initialize a dummy creature to play against
        Creature* dummyCreature = new Creature();
        if (curCreat->playerNum == PLAYER1) {
          this->p1Creature = curCreat;
          this->p2Creature = dummyCreature;
          dummyCreature->playerNum = PLAYER2;
        } else {
          this->p1Creature = dummyCreature;
          this->p2Creature = dummyCreature;
          dummyCreature->playerNum = PLAYER1;
        }

        delete this->realGame;
        this->realGame = new Game();

        int sanity = HARDCUTOFF; // make sure game doesnt run too long
        while (this->realGame->running && sanity > 0) {
          if (sanity == SOFTCUTOFF) {
            if (curCreat->playerNum == PLAYER1
             && this->realGame->player1->coins <= 3
             && this->realGame->player1->bombRange == 3
             && this->realGame->player1->bombCount == 1
             && this->realGame->player1->bombPierce == 0)
            {
              std::cout << "This creature sucks, moving on" << std::endl;
              sanity = -100;
              break;
            }
          }

          std::string pagmove;
          if (curCreat->playerNum == this->realGame->currentPlayer->playerNum) {
            pagmove = this->chooseMove();
          } else {
            pagmove = "b";
            std::vector<std::string> am = this->realGame->filterPointlessMoves();
            while (pagmove == "b") {
              int r = rand() % am.size();
              pagmove = am[r];
            }
          }

          this->realGame->submit(this->realGame->currentPlayer, pagmove);
          sanity -= 1;
        }

        if (sanity == 0) {
          sanity = -50;
          ranOutOfTime = true;
        }

        // takin care of bizniss
        delete dummyCreature;
        dummyCreature = nullptr;

        if (this->realGame->winner == curCreat->playerNum) {
          curCreat->wins += 1;
          curCreat->score += sanity; // want to maximize score
        } else {
          // zero tolerance, prune losers
          dunzo = true;
        }
      }

      if (dunzo) {
        creatureIt = population.erase(creatureIt);
        delete curCreat;
        curCreat = nullptr;
        if (ranOutOfTime) {
          std::cout << "Cutoff, game took too long" << std::endl;
        } else {
          std::cout << "Cutoff, there was a loss" << std::endl;
        }
      } else {
        ++creatureIt;
      }
    }

    std::cout << "Done determining fitness, commencing natural selection" << std::endl;

    // sort creatures by fitness
    std::sort(population.begin(), population.end(), [](Creature* a, Creature* b) {
      // want b < a so larger vals are at the beginning
      int aval = a->avg() + std::min(50, a->age());
      int bval = b->avg() + std::min(50, b->age());
      return bval < aval;
    });

    if (g+1 != GENERATIONS) {
      // remove lowest until there is no more than 50% original population
      while (population.size() > POP_SIZE/2) {
        population.pop_back();
      }

      // remove randomly until there is no more than 40% original population
      while (population.size() > (POP_SIZE*2)/5) {
        int rm = std::max(rand() % population.size(), rand() % population.size());
        population.erase(population.begin() + rm);
      }

      // for each individual left, reproduce asexually (with mutations)
      int popsz = population.size();
      for (size_t i = 0; i < popsz; i++) {
        population.push_back(new Creature(population[i]));
      }

      // fill the rest of the spaces randomly
      while (population.size() < POP_SIZE) {
        population.push_back(new Creature());
      }
    }

    // print results from generation (to file)
    std::ofstream myfile;
    std::stringstream ss;
    ss << "out/generation_" << g << ".txt";
    myfile.open(ss.str());

    myfile << "Output from generation " << g << std::endl
           << "---------------------------" << std::endl;
    for (size_t i = 0; i < population.size(); i++) {
      Creature* c = population[i];
      myfile << "Creature " << i << " | age: " << c->age() << ", score: " << c->avg() << ")" << std::endl;
      for (size_t j = 0; j < MLCT; j++) {
        myfile << c->weights[j] << ", ";
      }
      myfile << std::endl << "-------------------------" << std::endl;
    }

    std::cout << "Finished generation " << g+1 << std::endl;
    std::cout << "-------------------------" << std::endl << std::endl;
  }

  std::cout << "Done learning!" << std::endl;
}

void AI::serverGame(bool practice) {
  json j;
  if (practice) {
    this->verbose = true;
    AI::StartPracticeGame(&j);
  } else {
    this->verbose = true;
    AI::StartMatchmadeGame(&j);
  }

  this->realGame = new Game();
  this->realGame->loadFromJSON(j);

  while (j["state"].get<std::string>() != "complete") {
    std::string probablyAGoodMove = this->chooseMove();
    std::cout << "Found a probably good move: " << probablyAGoodMove << std::endl;
    AI::PostMove(&j, probablyAGoodMove);
    delete this->realGame;
    this->realGame = new Game();
    try {
      this->realGame->loadFromJSON(j);
      this->realGame->render();
    } catch (std::domain_error&) {
      delete this->realGame;
      break;
    }
  }
}

void AI::practiceServerGame() {
  this->serverGame(true);
}

void AI::matchmadeServerGame() {
  this->serverGame(false);
}

void AI::hostServer() {
  std::cout << "Server not yet implemented" << std::endl;
}

// Who needs a constructor?
void AI::initGame(char* arg, bool continuous) {
  srand (time(NULL));

  this->realGame = new Game();
  this->learning = false;
  this->verbose = false;

  this->p1Creature = nullptr;
  this->p2Creature = nullptr;
  this->currentCreature = nullptr;

  // heuristic weights, hard-code in previously learned values
  /*
    10.0055, 28.9702, 3.37105, 0.15425, 20.3831,
    29.2975, 11.6823, 9.64621, 12.9612,
    17.0621, 6.24294,
    20.5329, 14.1068,
  */
  //this->A = 10.0055; // position
  this->A = 50.0055; // position
  this->B = 28.9702; // coins
  this->C = 3.37105; // bomb score
  this->D = 0.15425; // portal score
  this->E = 100.3831; // want to spend money

  this->G = 29.2975; // bomb tick multiplier
  //this->H = 11.6823; // max position score
  this->H = 18.6823; // max position score
  this->I = 9.64621; // max bomb tick score
  this->J = 12.9612; // bombs-out score multiplier

  this->K = 17.0621; // orange portal score multiplier
  this->L = 6.24294; // blue portal score multiplier

  this->N = 20.5329; // bomb x multiplier
  this->O = 14.1068; // bomb y multiplier

  // start one of the game modes based on cmd line args
  do {
    if (strcmp(arg, "-hotseat") == 0 || strcmp(arg, "-h") == 0) {
      this->hotseatGame();
    } else if (strcmp(arg, "-oneplayer") == 0 || strcmp(arg, "-1") == 0) {
      this->onePlayerGame();
    } else if (strcmp(arg, "-duck") == 0 || strcmp(arg, "-d") == 0) {
      this->duckGame();
    } else if (strcmp(arg, "-cpuvcpu") == 0 || strcmp(arg, "-c") == 0) {
      this->cpuVsCpuGame();
    } else if (strcmp(arg, "-learn") == 0 || strcmp(arg, "-l") == 0) {
      this->machineLearn();
    } else if (strcmp(arg, "-practice") == 0 || strcmp(arg, "-p") == 0) {
      this->practiceServerGame();
    } else if (strcmp(arg, "-search") == 0 || strcmp(arg, "-s") == 0) {
      this->matchmadeServerGame();
    } else if (strcmp(arg, "-host") == 0 || strcmp(arg, "-o") == 0) {
      this->hostServer();
    } else {
      std::cout << "Invalid arg: " << arg << std::endl;
    }

    sleep(3000);
  } while (continuous);

  std::cout << "Finished!" << std::endl;
}

void AI::loadWeights(Creature* creature) {
  this->A = creature->weights[0]; // position
  this->B = creature->weights[1]; // coins
  this->C = creature->weights[2]; // bomb score
  this->D = creature->weights[3]; // portal score
  this->E = creature->weights[4]; // want to spend money

  this->G = creature->weights[5]; // bomb tick multiplier
  this->H = creature->weights[6]; // max position score
  this->I = creature->weights[7]; // max bomb tick score
  this->J = creature->weights[8]; // bombs-out score multiplier

  this->K = creature->weights[9]; // orange portal score multiplier
  this->L = creature->weights[10]; // blue portal score multiplier

  this->N = creature->weights[11]; // bomb x multiplier
  this->O = creature->weights[12]; // bomb y multiplier
}

int AI::heuristic(Game* node) {
  Player *me, *you;

  bool isPlayer1 = (this->realGame->currentPlayer->playerNum == PLAYER1);

  if (isPlayer1) {
    me = node->player1;
    you = node->player2;
  } else {
    me = node->player2;
    you = node->player1;
  }

  int infinity = std::numeric_limits<int>::max();
  if (node->whoWon() != NO_PLAYER) {
    if (node->whoWon() == me->playerNum) {
      return infinity;
    } else if (node->whoWon() == you->playerNum){
      return -infinity;
    } else {
      return 0;
    }
  }

  if (this->learning) {
    if (this->realGame->currentPlayer->playerNum == PLAYER1) {
      this->loadWeights(this->p1Creature);
    } else {
      this->loadWeights(this->p2Creature);
    }
  }

  int M = 50; // max coin count cared about

  int pc1 = me->coins; // coins
  int br1 = me->bombRange; // bomb range
  int bp1 = me->bombPierce; // bomb pierce
  int bc1 = me->bombCount; // bomb count (want to encourage placing them)

  int p1x = me->location.x;
  int p1y = me->location.y;
  int p2x = you->location.x;
  int p2y = you->location.y;

  // player position
  float ps1 = H - std::abs(p1x - p2x + p1y - p2y);

  // bomb positions
  // want bombs to be close to the opponent on average
  // want to place bombs rather than hoard them
  std::vector<Bomb*> myBombs;
  for (auto it = node->bombMap.begin(); it != node->bombMap.end(); ++it) {
    if (it->second->owner == me) {
      myBombs.push_back(it->second);
    }
  }

  int tb1 = bc1 + myBombs.size(); // total number of bombs
  float agb = 0; // average bomb score
  for (size_t i = 0; i < myBombs.size(); i++) {
    int bpx = myBombs[i]->location.x;
    int bpy = myBombs[i]->location.y;

    float bps = (I-myBombs[i]->tick) * G * (H - std::abs(bpx - p2x + bpy - p2y));
    if (bpx == you->location.x) {
      agb += N;
    }

    if (bpy == you->location.y) {
      agb += O;
    }
    agb += bps;
  }

  float tbs = 0; //total bomb score
  tbs += J * myBombs.size();
  tbs += agb;

  // portal positions
  // want orange portal to be close to opponent
  // want blue portal to be close to center

  float ops = -1.0;
  if (me->orangePortal != nullptr) {
    int orx = me->orangePortal->location.x;
    int ory = me->orangePortal->location.y;

    ops = H - std::abs(orx - p2x + ory - p2y);
  }

  float bps = -1.0;
  if (me->bluePortal != nullptr) {
    int brx = me->bluePortal->location.x;
    int bry = me->bluePortal->location.y;

    bps = node->getBlockValue(brx, bry);
  }

  float tps = K*ops + L*bps; //total portal score

  return (int)(A*ps1 + B*std::min(M, pc1) + C*tbs + D*tps + std::min(1, pc1)*E*(tb1 + br1 + bp1));

}

bool AI::lessComp(const Game* firstElem, const Game* secondElem) {
  return firstElem->bombMap.size() < secondElem->bombMap.size();
}

bool AI::greatComp(const Game* firstElem, const Game* secondElem) {
  return firstElem->bombMap.size() > secondElem->bombMap.size();
}

void AI::qsortGames(std::vector<Game*>& games, int l, int r) {
  int p = (l + r) / 2; // pivot

  int x = l;
  int y = r;

  while (l < y & r < x) {
    while (AI::lessComp(games[x], games[p])) {
      x++;
    }

    while (AI::greatComp(games[y], games[p])) {
      y++;
    }

    if (x <= y) {
      Game* temp = games[x];
      games[x] = games[y];
      games[y] = temp;
      x++;
      y--;
    } else {
      if (l < y) {
        qsortGames(games, l, y);
      }

      if (x < r) {
        qsortGames(games, x, r);
      }
    }
  }
}

/*
  Figuring out the maximizing player is a little weird
  because of how turns are structured
*/
int AI::minimax(Game* node, int depth, int alpha, int beta) {
  bool maximizingPlayer = (this->realGame->currentPlayer->playerNum == node->currentPlayer->playerNum);

  if (depth == 0 || (node->whoWon() != NO_PLAYER)) {
    return this->heuristic(node);
  }

  timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  double msTaken = tsMs(timeElapsed(this->start, now));
  if (msTaken > TIMEOUT) {
    // panic, running out of time
    if (maximizingPlayer) {
      return 1000000;
    } else {
      return -1000000;
    }
  }

  std::vector<Game*> children = node->getChildren();
  qsortGames(children, 0, children.size()-1);

  int infinity = std::numeric_limits<int>::max();
  int bestValue = -infinity;
  if (!maximizingPlayer) {
    bestValue = infinity;
  }

  for (size_t i = 0; i < children.size(); i++) {
    if (maximizingPlayer) {
      int mmscore = this->minimax(children[i], depth-1, alpha, beta);

      bestValue = std::max(bestValue, mmscore);
      alpha = std::max(alpha, bestValue);
    } else {
      int mmscore = this->minimax(children[i], depth-1, beta, alpha);

      bestValue = std::min(bestValue, mmscore);
      beta = std::min(beta, bestValue);
    }

    //*
    if (beta <= alpha && node->currentTurn == 1) {
      break;
    }
    //*/

    /*
    if (beta <= alpha){
      if (this->realGame->currentPlayer->playerNum == PLAYER1 && node->currentTurn == 0) {
        break; // CUTOFF
      } else if (this->realGame->currentPlayer->playerNum == PLAYER2 && node->currentTurn == 1) {
        break;
      }
    }
    //*/
  }

  for (size_t i = 0; i < children.size(); i++) {
    delete children[i];
  }

  return bestValue;
}

void AI::doMinimax(int* out, Game* node, int depth) {
  // For multithreading (not currently used)
  int infinity = std::numeric_limits<int>::max();
  *out = this->minimax(node, depth, -infinity, infinity);
}

std::string AI::chooseMove() {
  //this->start = std::clock();
  clock_gettime(CLOCK_MONOTONIC, &(this->start));

  std::vector<Game*> children = this->realGame->getChildren();

  int infinity = std::numeric_limits<int>::max(); // need this for later (now)
  int max_score = -infinity;
  std::vector<std::string> goodMoves;

  if (this->verbose) {
    std::cout << "Possible moves: ";
    std::vector<std::string> moves = this->realGame->filterPointlessMoves();
    for (size_t i = 0; i < moves.size(); i++) {
      std::cout << moves[i] << ", ";
    }
    std::cout << std::endl;
  }

  int depth = SEARCH_DEPTH;
  if (this->learning) {
    if (this->currentCreature->playerNum != this->realGame->currentPlayer->playerNum) {
      depth = OPP_SEARCH_DEPTH;
      std::cout << "USING LOWER SEARCH DEPTH!!!" << std::endl; // this is important
    }
  }

  // Each move's analysis does get its own thread
  //std::vector<int> scores;
  std::vector<int*> scores;
  std::vector<std::thread> threads;
  for (size_t i = 0; i < children.size(); i++) {
    //int score = this->minimax(children[i], depth, -infinity, infinity);
    int* score = new int(-infinity);
    scores.push_back(score);
    threads.push_back(std::thread(&AI::doMinimax, this, score, children[i], depth));
  }

  for (size_t i = 0; i < children.size(); i++) {
    threads[i].join();
  }

  for (size_t i = 0; i < scores.size(); i++) {
    int *score = scores[i];
    if (children[i]->lastMove == "b" && *score != -infinity && *score != 0) {
      if (this->realGame->moveNumber <= 120) {
        *score = 1000000;
      } else if (this->realGame->moveNumber <= 200) {
        if (rand() % 3 == 0) {
          *score = 1000000;
        }
      } else {
        if (rand() % 2 == 0) {
          *score = 1000000;
        }
      }
    }
  }

  for (size_t i = 0; i < scores.size(); i++) {
    int score = *(scores[i]);
    if (score == max_score) {
      goodMoves.push_back(children[i]->lastMove);
    }

    if (score > max_score) {
      max_score = score;
      goodMoves.clear();
      goodMoves.push_back(children[i]->lastMove);
    }

    if (this->verbose) {
      std::cout << children[i]->lastMove << ": " << score << std::endl;
    }
  }

  if (max_score == -infinity) {
    for (size_t i = 0; i < children.size(); i++) {
      delete children[i];
    }

    return "tu";
  }

  std::string inTheCourt[16] = {"b", "ml", "mu", "mr", "md", "buy_count", "buy_range", "buy_pierce", "op", "bp", "buy_block", "", "tr", "tl", "td", "tu"}; // order
  if (this->realGame->currentPlayer->playerNum == PLAYER1) {
    inTheCourt[1] = "mr";
    inTheCourt[2] = "md";
    inTheCourt[3] = "ml";
    inTheCourt[4] = "mu";
  }

  std::string chosenMove = "";
  if (max_score != -infinity) {
    for (size_t i = 0; i < 16; i++) {
      chosenMove = inTheCourt[i];
      if (std::find(goodMoves.begin(), goodMoves.end(), chosenMove) != goodMoves.end()) {
        break;
      }
    }
  }

  for (size_t i = 0; i < children.size(); i++) {
    delete children[i];
    delete scores[i];
  }

  // how long did choosing this move take?
  if (this->verbose) {
    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double msTaken = tsMs(timeElapsed(this->start, now));
    std::cout << "Time: " << msTaken << " ms" << std::endl;

    if (msTaken >=  30000) {
      std::cout << "TIMEOUT NOOOOO" << std::endl;
      *(int*)0=0; // cause a segfault, my favorite way to exit a progam
    }
  }

  return chosenMove;
}
