#pragma once

using namespace eosio;

class [[eosio::contract("eosio.system")]] system_contract : public contract {
   public:
      using contract::contract;
      system_contract( name s, name code, datastream<const char*> ds );
      ~system_contract();

      [[eosio::action]]
      void buyram( const name& payer, const name& receiver, const asset& quant );

      [[eosio::action]]
      void sellram( const name& account, int64_t bytes );

      using buyram_action = action_wrapper<"buyram"_n, &system_contract::buyram>;
      using sellram_action = action_wrapper<"sellram"_n, &system_contract::sellram>;
};