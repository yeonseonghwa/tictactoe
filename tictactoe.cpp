#include <eosio/eosio.hpp>
#include <vector>

using namespace eosio;
using namespace std;

class [[eosio::contract("tictactoe")]] tictactoe : public eosio::contract {
	public:

		tictactoe(name receiver, name code, datastream<const char*> ds): contract(receiver, code, ds) {}

		[[eosio::action]]
		void start(name host, name challenger) {
			require_auth(host);
			check(host != challenger, "host should't be the same as challenger");
			game_repository game_repos(get_self(), get_first_receiver().value);
			auto iterator = game_repos.find(challenger.value);
			check(iterator == game_repos.end(), "game already exists");

			auto board = vector<uint8_t>(9, 0);
			
			game_repos.emplace(host, [&](auto& game) {
				game.host = host;
				game.challenger = challenger;
				game.turn = host;
				game.board = board;
			});
		}
		[[eosio::action]]
		void play(name& host, name& challenger, name player, uint16_t& row, uint16_t& column) {
			require_auth(player);
			game_repository game_repos(get_self(), get_first_receiver().value);
			auto iterator = game_repos.find(challenger.value);
			check(iterator != game_repos.end(), "game doesn't exists");
			check(iterator->winner == "none"_n, "the game has ended!");
			check(player == iterator->host || player == iterator->challenger, "this is not your game!");
			check(player == iterator->turn, "it's not your turn turn yet!");

			uint32_t location = ((row -1) * 3) + column - 1;
			auto board = iterator->board;
			check(is_valid_play(location, board), "not a valid play!");

			const uint8_t mark = iterator->turn == iterator->host ? 1 : 2;
			board[location] = mark;

			name winner = "none"_n;
			if(is_winning(mark, board))
				winner = iterator->turn;

			if(is_board_full(board))
				winner = "draw"_n;

			const auto next_turn = iterator->turn == iterator->host ? iterator->challenger : iterator->host;

			game_repos.modify(iterator, iterator->host, [&](auto& game) {
				game.board[location] = mark;
				game.turn = next_turn;
				game.winner = winner;
			});
		}
		[[eosio::action]]
		void restart(name& host, name& challenger, name& player) {
			require_auth(player);
			game_repository game_repos(get_self(), get_first_receiver().value);
			auto iterator = game_repos.find(challenger.value);
			check(iterator != game_repos.end(), "game doesn't exists");
			check(player == iterator->host || player == iterator->challenger, "this is not your game!");

			game_repos.modify(iterator, iterator->host, [&](auto& game) {
				game.board = vector<uint8_t>(9, 0);
				game.turn = host;
				game.winner = "none"_n;
			});
		}
		[[eosio::action]]
		void close(name& host, name& challenger) {
			require_auth(host);
			game_repository game_repos(get_self(), get_first_receiver().value);
			auto iterator = game_repos.find(challenger.value);
			check(iterator != game_repos.end(), "game doesn't exists");
			game_repos.erase(iterator);	
		}
	private:
		struct [[eosio::table]] game {
			name challenger;
			name host;
			name turn;
			name winner = "none"_n;
			vector<uint8_t> board;

			uint64_t primary_key() const { return challenger.value; }
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
			return false;
		}

		bool is_winning(const uint8_t& mark, const vector<uint8_t>& board) const {
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

		typedef eosio::multi_index<"games"_n, game> game_repository;
};