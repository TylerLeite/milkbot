#include "game.h"

/* COORD CLASS */
Coord::Coord() :
  x(0), y(0), dir(NODIR)
{}

Coord::Coord(int x, int y) :
  x(x), y(y), dir(NODIR)
{}

Coord::Coord(int x, int y, int dir) :
  x(x), y(y), dir(dir)
{}

Coord::Coord(const Coord& other) :
  x(other.x), y(other.y), dir(other.dir)
{}

std::string Coord::toString() {
  std::string out = "";
  out += std::to_string(this->x);
  out += std::to_string(this->y);
  out += std::to_string(this->dir);
  return out;
}

/* PORTAL CLASS */

Portal::Portal() :
  owner(nullptr), color(-1), location(Coord(-1, -1, NODIR))
{}

Portal::Portal(Player* owner, int color, Coord& location) :
  owner(owner), color(color), location(location)
{}

Portal::Portal(Portal* other, Player* newOwner) :
  owner(newOwner), color(other->color), location(Coord(other->location))
{}


/* TRAIL CLASS */

Trail::Trail(int owner, int tick, int type) {
  if (type == APOCALYPSE) {
    this->p1Exist = false;
    this->p1Tick = -1;
    this->p1Type = NODIR;

    this->p2Exist = false;
    this->p2Tick = -1;
    this->p2Type = NODIR;

    this->apocalypse = true;
  } else if (owner == PLAYER1) {
    this->p1Exist = true;
    this->p1Tick = tick;
    this->p1Type = type;

    this->p2Exist = false;
    this->p2Tick = -1;
    this->p2Type = NODIR;

    this->apocalypse = false;
  } else {
    this->p2Exist = true;
    this->p2Tick = tick;
    this->p2Type = type;

    this->p1Exist = false;
    this->p1Tick = -1;
    this->p1Type = NODIR;

    this->apocalypse = false;
  }
}

Trail::Trail(Trail* other) :
  p1Exist(other->p1Exist), p1Tick(other->p1Tick), p1Type(other->p1Type),
  p2Exist(other->p2Exist), p2Tick(other->p2Tick), p2Type(other->p2Type),
  apocalypse(other->apocalypse)
{}

void Trail::set(int owner, int tick, int type) {
  if (owner == PLAYER1) {
    this->p1Exist = true;
    this->p1Tick = tick;
    this->p1Type = type;
  } else {
    this->p2Exist = true;
    this->p2Tick = tick;
    this->p2Type = type;
  }
}

void Trail::apocalify() {
  this->p1Exist = false;
  this->p1Tick = -1;
  this->p1Type = NODIR;

  this->p2Exist = false;
  this->p2Tick = -1;
  this->p2Type = NODIR;

  this->apocalypse = true;
}

/* BOMB CLASS */

Bomb::Bomb(Player* owner, Coord& location) :
  owner(owner), location(location), tick(5)
{}

Bomb::Bomb(Bomb* other, Player* newOwner) :
  owner(newOwner), location(Coord(other->location)), tick(other->tick)
{}

/* PLAYER CLASS */

Player::Player(int playerNum) :
  alive(true), playerNum(playerNum),
  coins(0), bombCount(1), bombPierce(0), bombRange(3),
  lastMove("")
{
  this->orangePortal = nullptr;
  this->bluePortal = nullptr;

  int x = 1;
  int y = 1;
  int dir = WEST;

  if (playerNum == PLAYER2) {
    x = 9;
    y = 9;
  }

  this->location = Coord(x, y, dir);
}

Player::Player(Player* other) :
  alive(other->alive), playerNum(other->playerNum),
  coins(other->coins), bombCount(other->bombCount),
  bombPierce(other->bombPierce), bombRange(other->bombRange),
  location(Coord(other->location)),
  lastMove(other->lastMove)
{
  this->orangePortal = nullptr;
  this->bluePortal = nullptr;
}

void Player::kill() {
  this->location.x = -1;
  this->location.y = -1;
  this->alive = false;
}


/* GAME CLASS */

// A lot of this can be improved to better utilize the coord class and the fact
//  that I have access to pointers, but for now I will put in minimal effort on
//  the simulation in order to focus on the rest of the ai. Once I have a working
//  version, I will make improvements here so that I can have a faster bot.

std::vector<std::string> makeAllMoves() {
  std::vector<std::string> allMoves;
  allMoves.push_back("");
  allMoves.push_back("buy_count");
  allMoves.push_back("buy_block");
  allMoves.push_back("buy_pierce");
  allMoves.push_back("buy_range");
  allMoves.push_back("b");
  allMoves.push_back("mu");
  allMoves.push_back("mr");
  allMoves.push_back("ml");
  allMoves.push_back("md");
  //allMoves.push_back("tu");
  //allMoves.push_back("tl");
  //allMoves.push_back("td");
  //allMoves.push_back("tr");
  allMoves.push_back("op");
  allMoves.push_back("bp");
  return allMoves;
}

std::vector<std::string> Game::allMoves = makeAllMoves();

bool Game::isAMoveMove(std::string move) {
  if (move == "mu" || move == "md" ||
      move == "ml" || move == "mr")
  {
    return true;
  }

  return false;
}

std::string Game::getCounterMove(std::string move) {
  std::string countermove = "";
  if (move == "md") {
    countermove = "mu";
  } if (move == "mu") {
    countermove = "md";
  } else if (move == "ml") {
    countermove = "mr";
  } else if (move == "mr") {
    countermove = "ml";
  }

  return countermove;
}

int Game::getDirectionOf(std::string clm) {
  int dir = NODIR;
  if (clm == "mu") {
    dir = NORTH;
  } else if (clm == "mr") {
    dir = EAST;
  } else if (clm == "ml") {
    dir = WEST;
  } else if (clm == "md") {
    dir = SOUTH;
  }

  return dir;
}

Game::Game() :
  boardSize(11), winner(-1), currentTurn(0),
  running(true), p1First(true), mePlayer(0),
  lastMove("NaM"), lastPlayer(0), moveNumber(0),
  apocalypseIterator1(0, 10, NORTH), apocalypseIterator2(10, 0, SOUTH)
{
  this->player1 = new Player(0);
  this->player2 = new Player(1);
  this->currentPlayer = this->player1;

  // Initialize the hardBlockBoard and softBlockBoard
  for (int y = 0; y < this->boardSize; y++) {
    for (int x = 0; x < this->boardSize; x++) {
      // local iterators, because c++ is not grunt
      if (x == 0 || y == 0 || x == this->boardSize-1 || y == this->boardSize-1 || x%2 == 0 && y%2 == 0) {
        this->hardBlockBoard.push_back(1);
        this->softBlockBoard.push_back(0);
        continue;
      } else {
        this->hardBlockBoard.push_back(0);
      }

      if ((x == 1 || y == 1) && (x <= 2 && y <= 2)) {
        this->softBlockBoard.push_back(0);
        continue;
      }

      if ((x == this->boardSize - 2 || y == this->boardSize - 2)
       && (x >= this->boardSize - 3 && y >= this->boardSize - 3))
      {
        this->softBlockBoard.push_back(0);
        continue;
      }

      if (rand() % 100 > 70){
        this->softBlockBoard.push_back(0); // might fiddle with %
      } else {
        this->softBlockBoard.push_back(1);
      }
    }
  }

  // guaranteed soft block spaces
  this->softBlockBoard[3*this->boardSize + 1] = 1;
	this->softBlockBoard[1*this->boardSize + 3] = 1;
	this->softBlockBoard[(this->boardSize - 2) * this->boardSize + this->boardSize - 4] = 1;
	this->softBlockBoard[(this->boardSize - 4) * this->boardSize + this->boardSize - 2] = 1;
}

Game::Game(Game* other) :
  boardSize(other->boardSize), winner(other->winner), currentTurn(other->currentTurn),
  running(other->running), p1First(other->p1First), mePlayer(other->mePlayer),
  lastMove(other->lastMove), lastPlayer(other->lastPlayer), moveNumber(other->moveNumber)
{
  this->player1 = new Player(other->player1);
  this->player2 = new Player(other->player2);

  std::map<Player*, Player*> old2new;
  old2new[other->player1] = this->player1;
  old2new[other->player2] = this->player2;

  this->currentPlayer = old2new[other->currentPlayer];

  for (size_t i = 0; i < other->boardSize * (other->boardSize); i++) {
    this->hardBlockBoard.push_back(other->hardBlockBoard[i]);
    this->softBlockBoard.push_back(other->softBlockBoard[i]);
  }

  for (auto it = other->bombMap.begin(); it != other->bombMap.end(); ++it) {
    int i = it->first;
    Bomb* oldBomb = it->second;
    this->bombMap[i] = new Bomb(oldBomb, old2new[oldBomb->owner]);
  }

  for (auto it = other->trailMap.begin(); it != other->trailMap.end(); ++it) {
    int i = it->first;
    Trail* oldTrail = it->second;
    this->trailMap[i] = new Trail(oldTrail);
  }

  for (auto it1 = other->portalMap.begin(); it1 != other->portalMap.end(); ++it1) {
    int i = it1->first;
    std::map<int, Portal*>* oldPortalsOnTile = &(it1->second);

    for (auto it2 = oldPortalsOnTile->begin(); it2 != oldPortalsOnTile->end(); ++it2) {
      int direction = it2->first;
      //Portal* oldPortal = it2->second;
      Portal* newPortal = new Portal(it2->second, old2new[it2->second->owner]); // WHY IS THIS BREAKING
      this->portalMap[i][direction] = newPortal;

      if (it2->second->color == PC_ORANGE) {
        newPortal->owner->orangePortal = newPortal;
      } else {
        newPortal->owner->bluePortal = newPortal;
      }
    }
  }
}

Game::~Game() {
  // Free all that mammary
  delete this->player1;
  this->player1 = nullptr;
  delete this->player2;
  this->player2 = nullptr;

  for (auto it = this->bombMap.begin(); it != this->bombMap.end(); ++it) {
    delete it->second;
    it->second = nullptr;
  }
  this->bombMap.clear();

  for (auto it = this->trailMap.begin(); it != this->trailMap.end(); ++it) {
    delete it->second;
    it->second = nullptr;
  }
  this->trailMap.clear();

  for (auto it1 = this->portalMap.begin(); it1 != this->portalMap.end(); ++it1) {
    for (auto it2 = it1->second.begin(); it2 != it1->second.end(); ++it2) {
      if (it2->second != nullptr) {
        delete it2->second;
        it2->second = nullptr;
      }
    }
    it1->second.clear();
  }
  this->portalMap.clear();
}

void Game::loadFromJSON(const json& j) {

  //std::cout << j.dump(4) << std::endl;

  delete this->player1;
  this->player1 = nullptr;
  delete this->player2;
  this->player2 = nullptr;

  this->mePlayer = j["playerIndex"];
  this->currentTurn = j["moveIterator"];
  this->boardSize = j["boardSize"];
  this->moveNumber = j["moveNumber"];

  int first = j["moveOrder"].at(0);
  this->p1First = (first == 0); // Look how smart I am

  this->softBlockBoard.clear();
  for (size_t i = 0; i < (this->boardSize)*(this->boardSize); i++) {
    this->hardBlockBoard.push_back(j["hardBlockBoard"][i]);
    this->softBlockBoard.push_back(j["softBlockBoard"][i]);
  }

  Player *player, *opponent;

  if (this->mePlayer == PLAYER1) {
    player = new Player(0);
    opponent = new Player(1);

    this->player1 = player;
    this->player2 = opponent;

    this->currentPlayer = this->player1;
  } else {
    player = new Player(1);
    opponent = new Player(0);

    this->player2 = player;
    this->player1 = opponent;

    this->currentPlayer = this->player2;
  }

  // Player 1 info
  player->location = Coord(j["player"]["x"], j["player"]["y"], j["player"]["orientation"]);
  player->coins = j["player"]["coins"];
  player->bombCount = j["player"]["bombCount"];
  player->bombPierce = j["player"]["bombPierce"];
  player->bombRange = j["player"]["bombRange"];
  player->alive = j["player"]["alive"];

  // Player 2 info
  opponent->location = Coord(j["opponent"]["x"], j["opponent"]["y"], j["opponent"]["orientation"]);
  opponent->coins = j["opponent"]["coins"];
  opponent->bombCount = j["opponent"]["bombCount"];
  opponent->bombPierce = j["opponent"]["bombPierce"];
  opponent->bombRange = j["opponent"]["bombRange"];
  opponent->alive = j["opponent"]["alive"];

  // Populate bomb map
  for (json::const_iterator bombInfo = j["bombMap"].begin(); bombInfo != j["bombMap"].end(); ++bombInfo) {
    std::string position = bombInfo.key();
    int x = position.at(0) - '0';
    int y = position.at(2) - '0';
    int i = x*(this->boardSize) + y;

    int tick = bombInfo.value()["tick"];

    Coord location = Coord(x, y);
    Player* owner = this->player1;
    if (bombInfo.value()["owner"] == PLAYER2) {
      owner = this->player2;
    }

    this->bombMap[i] = new Bomb(owner, location);
    this->bombMap[i]->tick = tick;
  }

  // Populate trail map
  for (json::const_iterator trailInfo = j["trailMap"].begin(); trailInfo != j["trailMap"].end(); ++trailInfo) {
    std::string position = trailInfo.key();
    int x = position.at(0) - '0';
    int y = position.at(2) - '0';

    this->placeTrail(this->player1, x, y, 'h');
  }


  // Populate portal map
  for (json::const_iterator portalMapInfo = j["portalMap"].begin(); portalMapInfo != j["portalMap"].end(); ++portalMapInfo) {
    std::string position = portalMapInfo.key();
    int x = position.at(0) - '0';
    int y = position.at(2) - '0';

    for (json::const_iterator portalInfo = portalMapInfo.value().begin(); portalInfo != portalMapInfo.value().end(); ++portalInfo) {
      std::string pik = portalInfo.key();
      int direction = pik.at(0) - '0';
      Player* owner = this->player1;
      if (portalInfo.value()["owner"] == PLAYER2) {
        owner = this->player2;
      }

      int color = PC_ORANGE;
      if (portalInfo.value()["portalColor"] == "blue") {
        color = PC_BLUE;
      }

      Coord location = Coord(x, y, direction);
      Portal* portal = new Portal(owner, color, location);

      if (color = PC_ORANGE) {
        owner->orangePortal = portal;
      } else {
        owner->bluePortal = portal;
      }
    }
  }
}

std::string Game::hash() {
  /*
  Player *player1, *player2; // Player class also has person id
  */
  std::string out = "";
  out += std::to_string(this->p1First);
  out += std::to_string(this->winner);
  out += std::to_string(this->currentTurn);
  if (this->lastMove == "") {
    out += "n";
  } else {
    out += this->lastMove;
  }

  out += std::to_string(this->player1->coins);
  out += ".";
  out += std::to_string(this->player1->bombCount);
  out += std::to_string(this->player1->bombRange);
  out += std::to_string(this->player1->bombPierce);
  out += this->player1->location.toString();

  out += std::to_string(this->player2->coins);
  out += ".";
  out += std::to_string(this->player2->bombCount);
  out += std::to_string(this->player2->bombRange);
  out += std::to_string(this->player2->bombPierce);
  out += this->player2->location.toString();

  for (size_t i=0; i < this->softBlockBoard.size(); i++) {
    out += std::to_string(this->softBlockBoard[i]);
  }

  for (auto it = this->bombMap.begin(); it != this->bombMap.end(); ++it) {
    out += std::to_string(it->first);
    out += ".";
    out += std::to_string(it->second->tick);
    out += it->second->location.toString();
  }

  for (auto it = this->trailMap.begin(); it != this->trailMap.end(); ++it) {
    out += std::to_string(it->first);
    out += ".";
    out += std::to_string(it->second->p1Type);
    out += std::to_string(it->second->p1Type);
    out += std::to_string(it->second->p1Exist);
    out += std::to_string(it->second->p2Type);
    out += std::to_string(it->second->p2Type);
    out += std::to_string(it->second->p2Exist);
  }

  for (auto it = this->portalMap.begin(); it != this->portalMap.end(); ++it) {
    out += std::to_string(it->first);
    out += ".";
    for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
      out += std::to_string(it2->first);
      out += std::to_string(it2->second->color);
      out += std::to_string(it2->second->owner->playerNum);
      out += it2->second->location.toString();
    }
  }

  return out;
}

int Game::whoWon() {
  //check if any player is on a trail
  if (!(this->running)) {
    return this->winner;
  } else {
    bool p1Alive = true;
    bool p2Alive = true;

    int p1Loc = this->player1->location.x * (this->boardSize) + this->player1->location.y;
    if (this->trailMap.find(p1Loc) != this->trailMap.end()) {
      p1Alive = false;
    }

    int p2Loc = this->player2->location.x * (this->boardSize) + this->player2->location.y;
    if (this->trailMap.find(p2Loc) != this->trailMap.end()) {
      p2Alive = false;
    }

    if (p1Alive && p2Alive) {
      return NO_PLAYER;
    } else if (!p1Alive && !p2Alive) {
      return BOTH_PLAYER;
    } else if (p1Alive) {
      return PLAYER1;
    } else if (p2Alive) {
      return PLAYER2;
    }
  }
}

bool Game::isOutOfBounds(const Coord& loc) {
  if (loc.x < 0 || loc.x >= this->boardSize || loc.y < 0 || loc.y >= this->boardSize) {
    return true;
  }

  return false;
}

Coord Game::getNextSquare(int x, int y, int direction) {
  switch (direction) {
    case WEST:
      return Coord(x-1, y, direction);
    case NORTH:
      return Coord(x, y-1, direction);
    case EAST:
      return Coord(x+1, y, direction);
    case SOUTH:
      return Coord(x, y+1, direction);
  }
}

int Game::getBlockValue(int x, int y) {
  int rawScore = std::abs((this->boardSize-1 - x)*x * (this->boardSize-1 - y)*y);
  int scaledScore = std::floor(10*rawScore / ((this->boardSize-1) * (this->boardSize-1) * (this->boardSize-1) * (this->boardSize-1) / 16));
  if (scaledScore == 0) {
    return 1;
  } else {
    return scaledScore;
  }
}

int Game::querySpace(int x, int y) {
  // This function figures out what is on the specified square
  // If a player and bomb are on the same square, it returns bomb
  // This could be made easier with polymorphism, but OOP is bad
  int i = x*(this->boardSize) + y;
  if (x < 0 || x >= this->boardSize || y < 0 || y >= this->boardSize) {
    return SP_OUT;
  } else if (this->hardBlockBoard[i] == 1) {
    return SP_HARDBLOCK;
  } else if (this->softBlockBoard[i] == 1) {
    return SP_SOFTBLOCK;
  } else if (this->bombMap.find(i) != this->bombMap.end()) {
    return SP_BOMB;
  } else if (this->player1->location.x == x && this->player1->location.y == y) {
    return SP_PLAYER1;
  } else if (this->player2->location.x == x && this->player2->location.y == y) {
    return SP_PLAYER2;
  } else {
    return SP_AIR;
  }
}

Coord Game::simulateMovement(int x, int y, int direction) {
  Coord nextSquare = getNextSquare(x, y, direction);
  if (this->isOutOfBounds(nextSquare)) {
    return Coord(x, y, direction);
  }

  int nextSquareContents = this->querySpace(nextSquare.x, nextSquare.y);

  int sz = this->boardSize;
  if (nextSquareContents == SP_AIR) {
    return nextSquare;
  } else {
    int i = nextSquare.x*sz + nextSquare.y;
    if (this->portalMap.find(i) != this->portalMap.end()) {
      std::map<int, Portal*> portalsOnTile = this->portalMap[i];

      int side = (direction+2) % 4; // Can't go into portals from the wrong side
      if (portalsOnTile.find(side) != portalsOnTile.end()) {
        Portal* portal = portalsOnTile[side];
        Player* player = portal->owner;
        if (player->orangePortal != nullptr && player->bluePortal != nullptr) {
          Coord otherPortalBlock, throughPortalSquare;

          // Get the location of the other portal
          if (portal->color == PC_ORANGE) {
            otherPortalBlock = player->bluePortal->location;
          } else if (portal->color == PC_BLUE) {
            otherPortalBlock = player->orangePortal->location;
          }

          // This is where you will land
          throughPortalSquare = this->simulateMovement(otherPortalBlock.x,
                                                      otherPortalBlock.y,
                                                      otherPortalBlock.dir);

          // Make sure you don't portal into a block
          if (throughPortalSquare.x != otherPortalBlock.x ||
              throughPortalSquare.y != otherPortalBlock.y) {
            return throughPortalSquare;
          }
        }
      }
    }

    return Coord(x, y, direction); // No movement
  }
}

// This function needs no better name
void Game::trailResolveSquare(int x, int y) {
  int i = x*(this->boardSize) + y;
  if (this->trailMap.find(i) == this->trailMap.end()) {
    return;
  }

  int space = this->querySpace(x, y);
  if (space == SP_SOFTBLOCK) {
    this->softBlockBoard[i] = 0;
    this->deletePortal(x, y, NODIR); // NODIR means delete all portals


    // std::cout << "bv: " << getBlockValue(x, y) << std::endl;

    // Give coins to p1
    if (this->trailMap[i]->p1Exist) {
      this->player1->coins += getBlockValue(x, y);
    }

    // Also give coins to p2
    if (this->trailMap[i]->p2Exist) {
      this->player2->coins += getBlockValue(x, y);
    }
  } else if (space == SP_PLAYER1 || space == SP_PLAYER2) {
    Player* player;
    if (space == SP_PLAYER1) {
      this->player1->kill();
    } else if (space == SP_PLAYER2) {
      this->player2->kill();
    }
  }
}

//type is orientation (horizontal vs vertical)
void Game::placeTrail(Player* owner, int x, int y, int type) {
  int ownerNum = 0;
  if (owner == this->player2) {
    ownerNum = 1;
  }

  int i = x*(this->boardSize) + y;
  if (this->trailMap.find(i) == this->trailMap.end()) {
    Trail* trail;
    if (owner == nullptr || type == APOCALYPSE) {
      trail = new Trail(-1, 0, APOCALYPSE);
    } else {
      trail = new Trail(ownerNum, 2, type);
    }
    this->trailMap[i] = trail;
  } else {
    if (owner == nullptr || type == APOCALYPSE) {
      this->trailMap[i]->apocalify();
    } else {
      this->trailMap[i]->set(ownerNum, 2, type);
    }
  }
}

void Game::recursiveDetonate(Player* owner, int x, int y, int direction, int range, int pierce, bool pierceMode) {
  if (range == 0 || (pierceMode && pierce < 0)) {
    return;
  }

  // This isn't the actual output, but I'm keeping old naming conventions
  Coord output = getNextSquare(x, y, direction);
  int outputContents = this->querySpace(output.x, output.y);

  if (outputContents == SP_HARDBLOCK || outputContents == SP_SOFTBLOCK) {
    int i = output.x*(this->boardSize) + output.y;
    if (this->portalMap.find(i) != this->portalMap.end()) {
      int side = (direction+2) % 4;
      std::map<int, Portal*> portalsOnTile = this->portalMap[i];

      if (portalsOnTile.find(side) != portalsOnTile.end()) {
        Portal* portal = portalsOnTile[side];
        Player* player = portal->owner;
        if (player->orangePortal != nullptr && player->bluePortal != nullptr) { // then we're traveling through poooortals
          Coord otherPortalBlock, throughPortalSquare;
          Portal* otherPortal = nullptr;
          if (portal->color == PC_ORANGE) {
            otherPortal = player->bluePortal;
          } else {
            otherPortal = player->orangePortal;
          }

          this->recursiveDetonate(owner, otherPortal->location.x, otherPortal->location.y, otherPortal->location.dir, range, pierce, pierceMode);
          return;
        }
      }
    }
  }

  int type = VERTICAL;
  if (direction == EAST || direction == WEST) {
    type = HORIZONTAL;
  }

  // Player* owner = this->player1;
  // if (this->currentPlayer->playerNum == PLAYER2) {
  //   owner = this->player2;
  // }

  // try this
  this->detonate(output.x, output.y);

  if (outputContents != SP_OUT) {
    this->placeTrail(owner, output.x, output.y, type);
  } else {
    return;
  }

  if (outputContents == SP_BOMB) {
    this->detonate(output.x, output.y);
  }

  if (outputContents != SP_AIR) {
    pierceMode = true;
  }

  if (pierceMode) {
    pierce -= 1;
  }

  this->recursiveDetonate(owner, output.x, output.y, direction, range-1, pierce, pierceMode);
}

void Game::detonate(int x, int y) {
  int i = x*(this->boardSize) + y;
  if (this->bombMap.find(i) == this->bombMap.end()) {
    return; // no bomb here.
  }

  Bomb* bomb = this->bombMap[i];
  Player* owner = bomb->owner;
  owner->bombCount += 1; // operator++ is for posers

  // delete the bomb, make sure there aren't any mammary leaks
  delete this->bombMap[i];
  this->bombMap[i] = nullptr;
  this->bombMap.erase(i);

  this->placeTrail(owner, x, y, ORIGIN);

  for (int direction = WEST; direction < SOUTH+1; direction++) {
    this->recursiveDetonate(owner, x, y, direction, owner->bombRange, owner->bombPierce, false);
  }
}

void Game::deletePortal(int x, int y, int direction) {
  int i = x*(this->boardSize) + y;
  if (this->portalMap.find(i) == this->portalMap.end()) {
    return; // can't delete a portal that doesnt exist
  }

  if (direction == NODIR) { // delete all portals on the square
    // first time using crazy new c++ 11 things, maybe not so clean
    for (auto it = this->portalMap[i].begin(); it != this->portalMap[i].end(); ++it) {
      Portal* portal = it->second;
      if (portal->color == PC_ORANGE) {
        portal->owner->orangePortal = nullptr;
      } else {
        portal->owner->bluePortal = nullptr;
      }

      delete portal;
      portal = nullptr;
      it->second = nullptr;
      this->portalMap[i].erase(it->first);
    }
  } else if (this->portalMap[i].find(direction) != this->portalMap[i].end()) {
    Portal* portal = this->portalMap[i][direction];

    if (portal->color == PC_ORANGE) {
      portal->owner->orangePortal = nullptr;
    } else {
      portal->owner->bluePortal = nullptr;
    }

    delete portal;
    portal = nullptr;
    this->portalMap[i][direction] = nullptr;
    this->portalMap[i].erase(direction);
  }

  // collect the garbage
  if (this->portalMap[i].empty()) {
    this->portalMap.erase(i);
  }
}

void Game::shootPortal(Player* owner, int direction, int color) {
  Coord nextSquare = getNextSquare(owner->location.x, owner->location.y, owner->location.dir);
  // if (this->isOutOfBounds(nextSquare)) {
  //   return;
  // }
  int nextSquareContents = this->querySpace(nextSquare.x, nextSquare.y);

  while (nextSquareContents != SP_HARDBLOCK && nextSquareContents != SP_SOFTBLOCK) {
    nextSquare = getNextSquare(nextSquare.x, nextSquare.y, nextSquare.dir);
    // if (this->isOutOfBounds(nextSquare)) {
    //   return;
    // }
    nextSquareContents = this->querySpace(nextSquare.x, nextSquare.y);
  }

  // now nextsquare is guaranteed to be a block or something
  //  this means it can maybe have a portal
  int newPortalDirection = (direction + 2) % 4;
  Coord newPortalLocation = Coord(nextSquare.x, nextSquare.y, newPortalDirection);
  Portal* newPortal = new Portal(owner, color, newPortalLocation);

  Portal* oldPortal = nullptr;
  if (color == PC_ORANGE) {
    oldPortal = owner->orangePortal;
  } else {
    oldPortal = owner->bluePortal;
  }

  if (oldPortal != nullptr) {
    Coord oldPortalLocation  = oldPortal->location;
    this->deletePortal(oldPortalLocation.x, oldPortalLocation.y, oldPortalLocation.dir);
    oldPortal = nullptr;
  }


  // need reduplication of code here since deleteportal relies on the player still owning that portal
  if (color == PC_ORANGE) {
    owner->orangePortal = newPortal;
  } else {
    owner->bluePortal = newPortal;
  }

  // portals overwrite older ones in the same spot
  int i = nextSquare.x*(this->boardSize) + nextSquare.y;
  if (this->portalMap.find(i) != this->portalMap.end()) {
    std::map<int, Portal*>* portalsOnTile = &(this->portalMap[i]);
    if (portalsOnTile->find(newPortalDirection) != portalsOnTile->end()) {
      oldPortal = (*portalsOnTile)[newPortalDirection];
      Coord oldPortalLocation  = oldPortal->location;
      this->deletePortal(oldPortalLocation.x, oldPortalLocation.y, oldPortalLocation.dir);
    }
  }

  // finally, place the new portal on the map
  this->portalMap[i][newPortalDirection] = newPortal;
}

std::string Game::submit(Player* currentPlayer, const std::string& move) {
  std::string outputNote = "";
  if (currentPlayer != this->currentPlayer) {
    return "Not your turn.";
  }

  Coord output;

  this->lastMove = move;
  currentPlayer->lastMove = move;
  this->lastPlayer = this->currentPlayer->playerNum;

  this->moveNumber += 1;
  if (move == "ml") {
    output = this->simulateMovement(currentPlayer->location.x, currentPlayer->location.y, WEST);
    currentPlayer->location.x = output.x;
    currentPlayer->location.y = output.y;
    currentPlayer->location.dir = output.dir;
  } else if (move == "mu") {
    output = this->simulateMovement(currentPlayer->location.x, currentPlayer->location.y, NORTH);
    currentPlayer->location.x = output.x;
    currentPlayer->location.y = output.y;
    currentPlayer->location.dir = output.dir;
  } else if (move == "mr") {
    output = this->simulateMovement(currentPlayer->location.x, currentPlayer->location.y, EAST);
    currentPlayer->location.x = output.x;
    currentPlayer->location.y = output.y;
    currentPlayer->location.dir = output.dir;
  } else if (move == "md") {
    output = this->simulateMovement(currentPlayer->location.x, currentPlayer->location.y, SOUTH);
    currentPlayer->location.x = output.x;
    currentPlayer->location.y = output.y;
    currentPlayer->location.dir = output.dir;
  } else if (move == "tl") {
    currentPlayer->location.dir = WEST;
  } else if (move == "tu") {
    currentPlayer->location.dir = NORTH;
  } else if (move == "tr") {
    currentPlayer->location.dir = EAST;
  } else if (move == "td") {
    currentPlayer->location.dir = SOUTH;
  } else if (move == "") {
    // nothing to see here
  } else if (move == "b") {
    int i = currentPlayer->location.x * (this->boardSize) + currentPlayer->location.y;
    if (this->bombMap.find(i) == this->bombMap.end() &&
        currentPlayer->bombCount > 0)
    {
      currentPlayer->bombCount -= 1; // operator-- is also for posers
      Coord bombLocation = Coord(currentPlayer->location.x, currentPlayer->location.y);
      this->bombMap[i] = new Bomb(currentPlayer, bombLocation);
    }
  } else if (move == "buy_count") {
    if (currentPlayer->coins < 5) {
      outputNote = "Insufficient funds to buy bomb";
    } else {
      currentPlayer->bombCount += 1;
      currentPlayer->coins -= 5;
    }
  } else if (move == "buy_pierce") {
    if (currentPlayer->coins < 5) {
      outputNote = "Insufficient funds to buy pierce";
    } else {
      currentPlayer->bombPierce += 1;
      currentPlayer->coins -= 5;
    }
  } else if (move == "buy_range") {
    if (currentPlayer->coins < 5) {
      outputNote = "Insufficient funds to buy range";
    } else {
      currentPlayer->bombRange += 1;
      currentPlayer->coins -= 5;
    }
  } else if (move == "buy_block") {
    // Right now this sucks. The only time I would use it is if it would trap an
    //  opponent, which shouldn't happen if the opponent is any good
    int price = 1;
    Coord newBlockLocation = this->getNextSquare(currentPlayer->location.x, currentPlayer->location.y, currentPlayer->location.dir);
    if (!(this->isOutOfBounds(newBlockLocation))) {
      if (this->querySpace(newBlockLocation.x, newBlockLocation.y) == SP_AIR) {
        // can only place the block on an empty square
        int blockCost = getBlockValue(newBlockLocation.x, newBlockLocation.y);
        if (currentPlayer->coins < blockCost) {
          outputNote = "Insufficient funds to buy block, requires: " + std::to_string(blockCost);
        } else {
          this->softBlockBoard[newBlockLocation.x * (this->boardSize) + newBlockLocation.y] = 1;
          currentPlayer->coins -= blockCost;
        }
      }
    }
  } else if (move == "op") {
    this->shootPortal(currentPlayer, currentPlayer->location.dir, PC_ORANGE);
  } else if (move == "bp") {
    this->shootPortal(currentPlayer, currentPlayer->location.dir, PC_BLUE);
  } else {
    outputNote = "invalid move, submitting no move this turn";
  }

  if (this->currentTurn == 0) {
    if (this->currentPlayer == this->player1) {
      this->currentPlayer = this->player2;
    } else {
      this->currentPlayer = this->player1;
    }
    this->currentTurn = 1;
  } else if (this->currentTurn == 1) {
    this->currentTurn = 0;
    this->p1First = !(this->p1First);

    // 1. switch move order (first player is put to the back of the list)
    // 2. bombs are ticked down, bombs with tick = 0 generate trails
    // 3. trails are ticked, killing players/blocks etc
    // 4. check if the game's ended

    if (this->moveNumber > 400) {
      this->placeTrail(nullptr, this->apocalypseIterator1.x, this->apocalypseIterator1.y, APOCALYPSE);
      this->placeTrail(nullptr, this->apocalypseIterator2.x, this->apocalypseIterator2.y, APOCALYPSE);

      Coord nextSquare1 = this->getNextSquare(this->apocalypseIterator1.x, this->apocalypseIterator1.y, this->apocalypseIterator1.dir);
      int i = (this->boardSize)*nextSquare1.x + nextSquare1.y;
      if (nextSquare1.x < 0 || nextSquare1.x >= this->boardSize
       || nextSquare1.y < 0 || nextSquare1.y >= this->boardSize
       || ((this->trailMap.find(i) != this->trailMap.end()) && this->trailMap[i]->apocalypse))
      {
        // both sides always turn at the same time
        this->apocalypseIterator1.dir = (this->apocalypseIterator1.dir + 1) % 4;
        this->apocalypseIterator2.dir = (this->apocalypseIterator2.dir + 1) % 4;
        nextSquare1 = getNextSquare(this->apocalypseIterator1.x, this->apocalypseIterator1.y, this->apocalypseIterator1.dir);
      }

      Coord nextSquare2 = getNextSquare(this->apocalypseIterator2.x, this->apocalypseIterator2.y, this->apocalypseIterator2.dir);
      this->apocalypseIterator1.x = nextSquare1.x;
      this->apocalypseIterator1.y = nextSquare1.y;
      this->apocalypseIterator2.x = nextSquare2.x;
      this->apocalypseIterator2.y = nextSquare2.y;
    }

    std::vector<Coord> toDetonate;
    for (auto it = this->bombMap.begin(); it != this->bombMap.end(); ++it) {
      int i = it->first;
      Bomb* bomb = it->second;
      bomb->tick -= 1;
      if (bomb->tick == 0) {
        int y = i % this->boardSize;
        int x = (i - y) / this->boardSize;
        toDetonate.push_back(Coord(x, y));
      }
    }

    for (size_t i = 0; i < toDetonate.size(); i++) {
      int x = toDetonate[i].x;
      int y = toDetonate[i].y;
      this->detonate(x, y);
    }

    for (auto it = this->trailMap.begin(); it != this->trailMap.end(); /*Nothng here*/) {
      int i = it->first;
      int y = i % this->boardSize;
      int x = (i - y) / this->boardSize;
      Trail* trail = it->second;

      this->trailResolveSquare(x, y);

      if (trail->p1Exist) {
        trail->p1Tick -= 1;
        if (trail->p1Tick <= 0) {
          trail->p1Exist = false;
        }
      }

      if (trail->p2Exist) {
        trail->p2Tick -= 1;
        if (trail->p2Tick <= 0) {
          trail->p2Exist = false;
        }
      }

      if (!trail->p1Exist && !trail->p2Exist && !trail->apocalypse) {
        delete trail;
        trail = nullptr;
        it->second = nullptr;
        this->trailMap.erase(it++);
      } else {
        ++it;
      }
    }

    // check victory step. even though i've been going out of my way to make sure
    //  this is possible to generalize everything, this only works for 2 players /s

    bool p1Alive = this->player1->alive;
    bool p2Alive = this->player2->alive;

    if (!p1Alive && !p2Alive) {
      this->winner = BOTH_PLAYER;
      this->running = false;
    } else if (p1Alive && !p2Alive) {
      this->winner = PLAYER1;
      this->running = false;
    } else if (!p1Alive && p2Alive) {
      this->winner = PLAYER2;
      this->running = false;
    } // else both players are alive
  }

  return outputNote;
}

std::vector<std::string> Game::filterPointlessMoves() {
  std::vector<std::string> out;

  int moveMoves = 0;
  Player* me = this->currentPlayer;

  for (int i = 0; i < Game::allMoves.size(); i++) {
    std::string clm = Game::allMoves[i];

    if (clm == "") {
      if (me->coins >= 5 && this->moveNumber < 400) {
        continue;
      }
    } else if (clm == "buy_block") {
      if (me->coins < 10) {
        continue;
      }

      if (this->bombMap.empty() && this->moveNumber < 400) {
        // only look at buying a block when bombs are on the field
        continue;
      }
    } else if (clm == "buy_range" || clm == "buy_pierce" || clm == "buy_count") {
      if (me->coins < 5 || this->moveNumber > 400) {
        continue;
      }

      if (clm == "buy_pierce") {
        if (me->bombRange <= me->bombPierce) {
          continue;
        }
      } else if (clm == "buy_range") {
        if (me->bombRange >= 10 || me->bombRange > me->bombPierce) {
          continue;
        }
      } else if (clm == "buy_count") {
        int realBombCt = me->bombCount;
        for (auto it = this->bombMap.begin(); it != this->bombMap.end(); ++it) {
          if (it->second->owner == me) {
            realBombCt += 1;
          }
        }

        if (realBombCt > 1) {
          continue;
        }
      }
    } else if (clm == "op" || clm == "bp") {
      if (this->bombMap.empty() && this->moveNumber < 400) {
        // only look at portal moves when bombs are on the field
        continue;
      }
    } else if (clm == "b") {
      if (me->bombCount < 1) {
        continue;
      }

      int x = this->currentPlayer->location.x;
      int y = this->currentPlayer->location.y;
      int i = x * (this->boardSize) + y;
      if (this->bombMap.find(i) != this->bombMap.end()) {
        continue;
      }
    } else if (clm == "mr" || clm == "ml" || clm == "md" || clm == "mu") {
      int dir = Game::getDirectionOf(clm);

      Coord destination = this->simulateMovement(me->location.x, me->location.y, dir);
      if (this->isOutOfBounds(destination) || (destination.x == me->location.x && destination.y == me->location.y)) {
        continue;
      }

      int contents = this->querySpace(destination.x, destination.y);
      if (contents != SP_AIR) {
        continue;
      } else {
        moveMoves += 1;
      }
    }
    out.push_back(clm);
  }

  return out;
}

std::vector<Game*> Game::getChildren() {
  std::vector<std::string> moves = this->filterPointlessMoves();

  std::vector<Game*> out;
  if (!(this->running)) {
    return out;
  }
  // Might want to use a more compact representation, like the json given
  for (size_t i = 0; i < moves.size(); i++) {
    Game* newGame = new Game(this);
    newGame->submit(newGame->currentPlayer, moves[i]);
    out.push_back(newGame);
  }

  return out;
}

void Game::render() {
  char strings[] = {'?', '+', '#', 'B', '1', '2', '.'};
  for (int y = 0; y < 11; y++) {
    for (int x = 0; x < 11; x++) {
      int i = x * (this->boardSize) + y;
      if (this->trailMap.find(i) != this->trailMap.end()) {
        if (this->trailMap.find(i)->second->p2Exist) {
          std::cout << 't';
        } else {
          std::cout << 'T';
        }
      } else if (this->portalMap.find(i) != this->portalMap.end()) {
        if (this->portalMap[i].find(EAST) != this->portalMap[i].end()) {
          if (this->portalMap[i][EAST]->owner->playerNum == PLAYER1) {
            std::cout << 'E';
          } else {
            std::cout << 'e';
          }
        } else if (this->portalMap[i].find(WEST) != this->portalMap[i].end()) {
          if (this->portalMap[i][WEST]->owner->playerNum == PLAYER1) {
            std::cout << 'W';
          } else {
            std::cout << 'w';
          }
        } else if (this->portalMap[i].find(NORTH) != this->portalMap[i].end()) {
          if (this->portalMap[i][NORTH]->owner->playerNum == PLAYER1) {
            std::cout << 'N';
          } else {
            std::cout << 'n';
          }
        } else if (this->portalMap[i].find(SOUTH) != this->portalMap[i].end()) {
          if (this->portalMap[i][SOUTH]->owner->playerNum == PLAYER1) {
            std::cout << 'S';
          } else {
            std::cout << 's';
          }
        }
      } else {
        int t = this->querySpace(x, y);
        std::cout << strings[t];
      }
    }
    std::cout << std::endl;
  }

  std::cout << "p1: c=" << this->player1->coins
            << "; r=" << this->player1->bombRange
            << "; p=" << this->player1->bombPierce
            << "; b=" << this->player1->bombCount << std::endl;

  std::cout << "p2: c=" << this->player2->coins
            << "; r=" << this->player2->bombRange
            << "; p=" << this->player2->bombPierce
            << "; b=" << this->player2->bombCount << std::endl;
}
