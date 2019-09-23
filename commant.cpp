#include <eosio/eosio.hpp>
#include <vector>

using namespace eosio;
using namespace std;



class [[eosio::contract("commant")]] commant : public eosio::contract {
	public:

		commant(name receiver, name code, datastream<const char*> ds): contract(receiver, code, ds) {}

		[[eosio::action]]
		void write(name account, uint64_t t,string user, string keyword, string url, uint64_t up) {
            require_auth(account);
            notice_repository notice_repos(get_self(),get_first_receiver().value);
            
			notice_repos.emplace(account, [&](auto& notice) {
			    notice.t = t;
				notice.user = user;
				notice.keyword = keyword;
				notice.url = url;
				notice.up = up;
			});
		}
		
	private:
		struct [[eosio::table]] notice {
		    uint64_t t;
			string user;
			string keyword;
			string url;
			uint64_t up;

			uint64_t primary_key() const { return t; }
			EOSLIB_SERIALIZE(notice, (t)(user)(keyword)(url)(up))
		};


		typedef eosio::multi_index<"notices"_n, notice> notice_repository;
};

