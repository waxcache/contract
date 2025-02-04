#pragma once

namespace eosiosystem {

using eosio::asset;
using eosio::symbol;

   struct [[eosio::table, eosio::contract("eosio.system")]] exchange_state {
      asset    supply;

      struct connector {
         asset balance;
         double weight;

         EOSLIB_SERIALIZE( connector, (balance)(weight) )
      };

      connector base;
      connector quote;

      uint64_t primary_key()const { return supply.symbol.raw(); }

      EOSLIB_SERIALIZE( exchange_state, (supply)(base)(quote) )
   };

   typedef eosio::multi_index< "rammarket"_n, exchange_state > rammarket;

}
