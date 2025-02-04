#pragma once

#include <eosio/singleton.hpp>

using eosio::asset;
using eosio::symbol;

struct [[eosio::table("global2"), eosio::contract("eosio.system")]] eosio_global_state2 {
   eosio_global_state2(){}

   uint16_t          new_ram_per_block;
   eosio::block_timestamp   last_ram_increase;
   eosio::block_timestamp   last_block_num;
   double            total_producer_votepay_share;
   uint8_t           revision; 

   EOSLIB_SERIALIZE( eosio_global_state2, ( new_ram_per_block )( last_ram_increase )( last_block_num )
                     ( total_producer_votepay_share )( revision ) )
};

typedef eosio::singleton< "global2"_n, eosio_global_state2 > global_state2_singleton;

