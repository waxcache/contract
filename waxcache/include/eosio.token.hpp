#pragma once

using namespace eosio;
using std::string;

class [[eosio::contract("eosio.token")]] token : public contract {
   public:
      using contract::contract;
      token( name s, name code, datastream<const char*> ds );
      ~token();
      [[eosio::action]]
      void transfer( const name&    from,
                     const name&    to,
                     const asset&   quantity,
                     const string&  memo);

      using transfer_action = eosio::action_wrapper<"transfer"_n, &token::transfer>;

};


