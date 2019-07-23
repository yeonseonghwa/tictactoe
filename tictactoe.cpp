#include <eosiolib/eosio.hpp>
#include <vector>
 
using namespace eosio;
using namespace std;
 
class tictactoe_service : public contract {
  public:
    tictactoe_service(account_name self)
      :contract(self),
      game_repository(self, self)
      {}
 
    // @abi action
    void start(const account_name host, const account_name challenger) {
      require_auth(host);
      eosio_assert(host != challenger, "host shouldn't be the same as chanllenger");
      auto itr = game_repository.find(challenger);
      eosio_assert(itr == game_repository.end(), "game already exists");
 
      auto board = vector<uint8_t>(9, 0);
 
      game_repository.emplace(_self, [&](auto& game) {
        game.host  = host;
        game.challenger = challenger;
        game.turn = host;
        game.board = board;
      });
    }
 
    // @abi action
    void play(const account_name& host, const account_name& challenger, const account_name& player, const uint16_t& row, const uint16_t& column ) {
      require_auth(player);
 
      auto itr = game_repository.find(challenger);
      eosio_assert(itr != game_repository.end(), "game doesn't exists");
      eosio_assert(itr->winner == N(none), "the game has ended!");
      eosio_assert(player == itr->host || player == itr->challenger, "this is not your game!");
      eosio_assert(player == itr->turn, "it's not your turn yet!");
 
      uint32_t location = ((row -1) * 3) + column - 1;
      auto board = itr->board;
      eosio_assert(is_valid_play(location, board), "not a valid play!");
 
      const uint8_t mark = itr->turn == itr->host ? 1 : 2;
      board[location] = mark;
 
      account_name winner = N(none);
      if(is_winning(mark, board))
        winner = itr->turn;
 
      if(is_board_full(board))
        winner = N(draw);
 
      const auto next_turn = itr->turn == itr->host ? itr->challenger : itr->host;
 
      game_repository.modify(itr, itr->host, [&](auto& game) {
        game.board[location] = mark;
        game.turn = next_turn;
        game.winner = winner;
      });
    }
 
    // @abi action
    void restart(const account_name& host, const account_name& challenger, const account_name& player) {
      require_auth(player);
      auto itr = game_repository.find(challenger);
      eosio_assert(itr != game_repository.end(), "game doesn't exists");
      eosio_assert(player == itr->host || player == itr->challenger, "this is not your game!");
 
      game_repository.modify(itr, itr->host, [&](auto& game) {
            game.board = vector<uint8_t>(9, 0);
            game.turn = host;            
            game.winner = N(none);
      });
    }
 
    // @abi action
    void close(const account_name& host, const account_name& challenger) {
      require_auth(host);
      auto itr = game_repository.find( challenger );
      eosio_assert(itr != game_repository.end(), "game doesn't exists");
      game_repository.erase(itr);
    }
 
  private:
    // @abi table games i64
    struct game {
      account_name challenger;
      account_name host;
      account_name turn; 
      account_name winner = N(none);
      vector<uint8_t> board; 
 
      uint64_t primary_key() const { return challenger; }
      EOSLIB_SERIALIZE(game, (host)(challenger)(turn)(winner)(board))
    };
 
    bool is_valid_play(const uint32_t& location, const vector<uint8_t>& board) {
      bool is_valid = location < board.size() && board[location]==0;
      return is_valid;
    }
 
    bool is_board_full(const vector<uint8_t>& board) {
      for(auto mark : board){
        if(mark==0)
          return false;
      }
      return true;
    }
 
    bool is_winning(const uint8_t& mark, const vector<uint8_t>& board) const
    {
      return 
        (board[0] == mark && board[1] == mark && board[2] == mark) ||
        (board[3] == mark && board[4] == mark && board[5] == mark) ||
        (board[6] == mark && board[7] == mark && board[8] == mark) ||
        (board[0] == mark && board[4] == mark && board[8] == mark) ||
        (board[2] == mark && board[4] == mark && board[6] == mark) ||
        (board[0] == mark && board[3] == mark && board[6] == mark) ||
        (board[1] == mark && board[4] == mark && board[7] == mark) ||
        (board[2] == mark && board[5] == mark && board[8] == mark);
    }
 
    multi_index<N(games), game> game_repository;
};
 
EOSIO_ABI(tictactoe_service, (start)(play)(restart)(close))