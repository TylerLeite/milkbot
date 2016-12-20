# Milkbot: A "Bomberman" AI

This is a program that plays a turn-based variation of bomberman with portals. Milkbot was made for RPI UPE's AI competition `aicomp`.

The strategy is a slight variation on the minimax algorithm. Because this is not a traditional turn-based game and players essentially move twice in a row, I had to account for which player was currently maximizing within the minimax function, rather than just alternating every move. In addition, I had to alter when alpha and beta cutoffs occurred, since a player might cut off its own move (not good).

Rather than coming up with my own heuristic function, I used a machine learning algorithm similar to a GA to find one. The chromosomes where weights of various attributes of the game state. The weights were optimized based on how quickly they caused a victory and how consistently they caused one. When evolving, the bot plays against a handicapped version of itself which had a lower search depth.

### Installation
* `git clone https://github.com/TylerLeite/milkbot.git`
* `cd milkbot`
* `make`
* run with `bin/milkbot -ARGS`

### Dependencies
* Milkbot depends on `libcurl`
* `json.h` is included in the repository.

### Usage
* `bin/milkbot -h` runs a hotseat game <b>Not help, don't get confused</b>
* `bin/milkbot -1` runs a human vs cpu game
* `bin/milkbot -d` runs a cpu vs duck game. The duck makes random moves.
* `bin/milkbot -c` runs a cpu vs cpu game
* `bin/milkbot -l` runs the machine learning algorithm
* `bin/milkbot -p` runs a practice game on the server <b>No longer works since server is down!</b>
* `bin/milkbot -s` searches for a rated game on the server <b>No longer works since server is down!</b>
* `bin/milkbot -o` hosts a LAN game <b>Never implemented this</b>
* By default, one game will be run. Adding an additional `-c` flag will run until the user terminates the program with `ctrl-c`

* <b>Example:</b> `bin/milkbot -1 -c` will run human versus cpu games continuously

### Post-mortem
I achieved second place in the competition for a prize of $1,000. I am happy with this result, but feel there is still a lot of room for improvement on this bot. After I lost in grand finals, I found a bug in my Game::loadFromJSON function which caused my bot to be unaware of the apocalypse flames. While I cannot say for certain that I would have won if I had caught the bug earlier, it certainly made the games end faster than they otherwise would have.

There were also some bugs which I cannot identify the source of, nor replicate. On occasion, The bot would value a quick draw over a forced victory. I believe this was caused by cutoffs due to time constraint, but am unsure. Additionally, I encountered a segfault once, but only once and seemingly at random.

As for improvements other than bug fixes, I believe that execution time could be considerably sped up through use of a transposition table as similar positions are encountered frequently. Using a transposition table would also allow me to utilize a dynamic search depth, since cutting off minimax early causes a great deal of problems. Using multiple threads for minimax evaluation helped lessen the effect of these cutoffs, but the effect was still clear.

Additionally, pattern matching based on previous moves to avoid redundant sequences such as `move_up, move_down, move_up, move_down` could have been beneficial. However I believe that a better heuristic function could also fix this problem, perhaps one that takes into account current move number. The reason I prioritize bombing so highly (the bot always places a bomb if it will not lead to assured death) is because without that prioritization the bot often got stuck in a loop moving back and forth, even when assigning a high value to placing bombs and having coins / powerups.
