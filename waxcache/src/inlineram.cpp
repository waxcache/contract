/*
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
██╗░░  ██╗███╗░░██╗██╗░░░░░██╗███╗░░██╗███████╗██████╗░░█████╗░███╗░░░███╗
╚██╗░  ██║████╗░██║██║░░░░░██║████╗░██║██╔════╝██╔══██╗██╔══██╗████╗░████║
░╚██╗  ██║██╔██╗██║██║░░░░░██║██╔██╗██║█████╗░░██████╔╝███████║██╔████╔██║
░██╔╝  ██║██║╚████║██║░░░░░██║██║╚████║██╔══╝░░██╔══██╗██╔══██║██║╚██╔╝██║
██╔╝░  ██║██║░╚███║███████╗██║██║░╚███║███████╗██║░░██║██║░░██║██║░╚═╝░██║
╚═╝░░  ╚═╝╚═╝░░╚══╝╚══════╝╚═╝╚═╝░░╚══╝╚══════╝╚═╝░░╚═╝╚═╝░░╚═╝╚═╝░░░░░╚═╝
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
@brief: ramcore purchasing actions/functions
*/

#include <cache.hpp>

/*
  █▀█ █▄░█   █▄░█ █▀█ ▀█▀ █ █▀▀ █▄█   ▀█▀ █▀█ ▄▀█ █▄░█ █▀ █▀▀ █▀▀ █▀█    █ █▄░█ █▀▀ █▀█ █▀▀ ▄▀█ █▀ █▀▀   █▄▄ █▄█ ▀█▀ █▀▀ █▀
  █▄█ █░▀█   █░▀█ █▄█ ░█░ █ █▀░ ░█░   ░█░ █▀▄ █▀█ █░▀█ ▄█ █▀░ ██▄ █▀▄    █ █░▀█ █▄▄ █▀▄ ██▄ █▀█ ▄█ ██▄   █▄█ ░█░ ░█░ ██▄ ▄█
    @brief: on_notify action that listens for incoming Core Native Token transactions, increases byte mining balances; and credits RAM to the RESERVE_ACCT account
    @param from: miner
    @param to: recipient ( _self )
    @param quantity: amount of Core Native Token
    @param memo: transactable action
*/

void CONTRACT_CLASS_NAME::incoming_transfer( name from, name to, asset quantity, string memo ) { 

  if ( to != get_self() || from == get_self()) return;
  if ( from == RESERVE_ACCT ) return; 
  miningstats_check("miningstats_verification");
  checkBlockReward();
  
  auto miningstats_delta = _miningstats.get();
  auto config_delta = _config.get();
  miner_table _miner( get_self(), get_self().value );
  auto miner_itr = _miner.find( from.value );

  check ( miner_itr != _miner.end(), "⚠️ Miner does not exist ⚠️" + ERROR_LOCATION );
  check ( quantity.symbol == NATIVE_SYM, " ⚠️ " + _self.to_string() + " only accepts: " + NATIVE_SYM_STRING + " transfers ⚠️" + ERROR_LOCATION  );
  check ( from != miningstats_delta.genesis_miner, "⚠️ Genesis Miner does not carry mining carry continued mining privileges ⚠️" + ERROR_LOCATION );

  if ( miningstats_delta.genesis_bytes == 0  ) {
    check ( memo == "genesis_bytes", "⚠️ First miner must be a genesis miner that willfully surrenders 'x' ram to " + _self.to_string() + " contract indefinitely, use memo 'genesis_bytes' to confirm ⚠️" + ERROR_LOCATION );
    miningstats_delta.genesis_bytes = net_purchased_bytes_after_eosio_fees(asset( core_tokens_post_dev_fees(quantity.amount) , NATIVE_SYM) );
    miningstats_delta.genesis_miner = from;
    system_contract::buyram_action BUY_RAMCORE( NATIVE_SYSTEM_CONTRACT_NAME, ANTELOPE_CONTRACT_ACTIVE_PERMISSION );
    BUY_RAMCORE.send(get_self(), RESERVE_ACCT , asset( core_tokens_post_dev_fees(quantity.amount) , NATIVE_SYM) );

  } else if ( miningstats_delta.genesis_bytes > 0 && miningstats_delta.genesis_miner != from ) {
    if ( from == RESERVE_ACCT ) {  
    // do nothing
    } else {
      if ( memo == "increase_bytes" ) {
        check ( miningstats_delta.current_era > 0, "⚠️ all miners welcome after genesis block ⚠️" + ERROR_LOCATION );
        system_contract::buyram_action BUY_RAMCORE( NATIVE_SYSTEM_CONTRACT_NAME, ANTELOPE_CONTRACT_ACTIVE_PERMISSION );
        increaseramcore(from, net_purchased_bytes_after_eosio_fees(asset( core_tokens_post_dev_fees(quantity.amount) , NATIVE_SYM)) ); 
        BUY_RAMCORE.send(get_self(), RESERVE_ACCT , asset( core_tokens_post_dev_fees(quantity.amount) , NATIVE_SYM) );
        print ( _self, " successfully purchased bytes on behalf of " ,from, " who was credited: ", net_purchased_bytes_after_eosio_fees(asset( core_tokens_post_dev_fees(quantity.amount) , NATIVE_SYM))," bytes for mining after developer fees \n"); 
        if ( miner_itr -> bytes != 0 ) {
          verify_miner_claims( from );   
        }
      } else {
        check ( false, "⚠️ Transfer 'memo' required: 'increase_bytes' ⚠️" + ERROR_LOCATION  );
      }
    }
  }

  // update miner last_claim 
  if (miner_itr -> bytes == 0 ){ 
    miningstats_delta.miner_count += 1;
    _miner.modify( miner_itr , same_payer , [&]( auto& mach ){ // ram bill
      mach.last_claim = NOW + 1;
    });
  }
  //  updateminer bytes
  _miner.modify( miner_itr , same_payer , [&]( auto& mach ){ // ram bill
    mach.bytes += net_purchased_bytes_after_eosio_fees(asset( core_tokens_post_dev_fees(quantity.amount) , NATIVE_SYM)); 
    mach.last_byte_adj_time = NOW;
  });

  // update miningstats
  miningstats_delta.total_bytes += net_purchased_bytes_after_eosio_fees(asset( core_tokens_post_dev_fees(quantity.amount) , NATIVE_SYM));  
  miningstats_delta.dev_claim_bal += quantity - asset( core_tokens_post_dev_fees(quantity.amount), NATIVE_SYM);
  _miningstats.set(miningstats_delta, _self);

  // update fee weight & voted proposal weight
  if ( miningstats_delta.genesis_bytes != 0 && from != miningstats_delta.genesis_miner ) {
    //update fees
    weight_adjustment_fees(from, miner_itr -> voted_fee.amount , miner_itr -> voted_fee.amount, miner_itr -> bytes - net_purchased_bytes_after_eosio_fees(asset( core_tokens_post_dev_fees(quantity.amount) , NATIVE_SYM))  , miner_itr -> bytes ); 

    // review old vote legitimacy
    if ( miner_itr -> prop_vote > 0 ) {
      // is current proposal that miner voted for still active?
      auto [existing_proposal_vote_status, existing_proposal_vote_message] = proposal_table_iterator(miner_itr -> prop_vote);
      if ( existing_proposal_vote_status == "not_legit" ) {
          remove_miner_proposal_vote( from );
          delete_proposal_record_if_exist( miner_itr -> prop_vote );
          print ( ":removed miner vote and updated proposal record \n" );
      } else {
          increase_proposal_record_vote( from , miner_itr -> prop_vote, net_purchased_bytes_after_eosio_fees(asset( core_tokens_post_dev_fees(quantity.amount) , NATIVE_SYM)));
          print ( "🗳️ adjusted vote weight for ", miner_itr -> prop_vote, " 🗳️ \n");
      }
    } 
  }

}

/*
  █▀ █▀▀ █░░ █░░   █▄▄ █▄█ ▀█▀ █▀▀ █▀
  ▄█ ██▄ █▄▄ █▄▄   █▄█ ░█░ ░█░ ██▄ ▄█
    @brief: allows `miner` to sell 'x' bytes on the `RESERVE_ACCT`, proceeds are sent to '_self' which in turn delivers funds to 'miner';'reserve' account must have the '_self@eosio.code' authority
    @param miner: active miner with a current mining balance
    @param bytes: number of mining bytes to sell
*/

[[eosio::action]] void CONTRACT_CLASS_NAME::reducemining(name miner, uint64_t bytes){

  require_auth(miner);

  miningstats_check("miningstats_verification");
  checkBlockReward();

  auto miningstats_delta = _miningstats.get();
  miner_table _miner( get_self(), get_self().value );               
  auto miner_itr = _miner.find( miner.value );
  uint64_t old_fee = miner_itr -> voted_fee.amount;
  asset raw_tokens_out = direct_convert( asset(bytes, RAM_SYM), NATIVE_SYM);
  bool is_there_any_remaining_bytes = true;

  check ( miner_itr != _miner.end(), "⚠️ Miner does not exist ⚠️" + ERROR_LOCATION );
  check ( miner != miningstats_delta.genesis_miner, "⚠️ Genesis bytes previously surrendered to " + _self.to_string() + " contract, Genesis Miner carries no mining privileges ⚠️" + ERROR_LOCATION ) ;
  check ( bytes <= miner_itr -> bytes, "⚠️ Overdrawn miner byte balance ⚠️" + ERROR_LOCATION );
  check ( bytes >= 1, "⚠️ Must sell more at least 1 or more bytes ⚠️" + ERROR_LOCATION );

  verify_miner_claims( miner );
  // sell ram and send proceeds to reserve acct
  system_contract::sellram_action sell( NATIVE_SYSTEM_CONTRACT_NAME, RESERVE_ACCT_ACTIVE_PERMISSION);
  sell.send(RESERVE_ACCT, bytes);  

  // transfer ram sale proceeds to CACHE
  token::transfer_action RESERVE_ACCT_TRANSFER( NATIVE_SYSTEM_TOKEN_CONTRACT_NAME, RESERVE_ACCT_ACTIVE_PERMISSION ); 
  RESERVE_ACCT_TRANSFER.send(RESERVE_ACCT, get_self(), asset( eosio_ram_fees( raw_tokens_out.amount ) , NATIVE_SYM), std::to_string(eosio_ram_fees( raw_tokens_out.amount ))  ); 
  
  // transfer ram sale proceeds from CACHE to MINER
  token::transfer_action CACHE_ACCT_TRANSFER(NATIVE_SYSTEM_TOKEN_CONTRACT_NAME, ANTELOPE_CONTRACT_ACTIVE_PERMISSION); 
  CACHE_ACCT_TRANSFER.send(get_self(), miner, asset(eosio_ram_fees(raw_tokens_out.amount) , NATIVE_SYM), std::to_string((eosio_ram_fees(raw_tokens_out.amount)))  );

  // update byte balance & associate vote/miner count if applicable
  _miner.modify( miner_itr , same_payer , [&]( auto& mach ){ // ram bill
    if ( mach.bytes - bytes == 0 ) {
      mach.voted_fee = asset(0,CACHE_SYM);
      miningstats_delta.miner_count -= 1;
    }
    mach.bytes -= bytes;
    mach.last_byte_adj_time = NOW;
    if ( mach.bytes == 0 ) {
      is_there_any_remaining_bytes = false; 
    }
  });

  // update miningstats
  miningstats_delta.total_bytes -= bytes;
  _miningstats.set(miningstats_delta, _self);

  // update fee weight 
  weight_adjustment_fees( miner, old_fee, miner_itr -> voted_fee.amount, miner_itr -> bytes + bytes, miner_itr -> bytes ); 

  // update proposal vote weight if applicable 
  if ( miner_itr -> prop_vote > 0 ) {
    // is current proposal that miner voted for still active?
    auto [existing_proposal_vote_status, existing_proposal_vote_message] = proposal_table_iterator(miner_itr -> prop_vote);
    if ( existing_proposal_vote_status == "not_legit" ) {
        remove_miner_proposal_vote(miner);
        delete_proposal_record_if_exist( miner_itr -> prop_vote );
        print ( ":removed miner vote and updated proposal record \n" );
    } else {
        decrease_proposal_record_vote_if_exist(miner, miner_itr -> prop_vote , bytes );
        print ( "🗳️ adjusted vote weight for  ", miner_itr -> prop_vote, " 🗳️ \n");
    }
    if ( is_there_any_remaining_bytes == false ) {
      remove_miner_proposal_vote(miner);
    }

  } 
  print ( miner, " reduced mining weight by " , bytes, " bytes. ", asset(eosio_ram_fees(raw_tokens_out.amount) , NATIVE_SYM), " credited to ", miner, " \n"  );

}

/*
  █ █▄░█ █▀▀ █▀█ █▀▀ ▄▀█ █▀ █▀▀   █▄▄ █▄█ ▀█▀ █▀▀ █▀
  █ █░▀█ █▄▄ █▀▄ ██▄ █▀█ ▄█ ██▄   █▄█ ░█░ ░█░ ██▄ ▄█
  @brief: Increase MINER's BYTE mining balance
*/

void CONTRACT_CLASS_NAME::increaseramcore ( name miner, uint64_t bytes ){

  auto config_delta = _config.get();
  miner_table _miner( get_self(), get_self().value );
  auto miner_itr = _miner.find( miner.value );
  check ( miner_itr != _miner.end(), "⚠️ Miner does not exist ⚠️" + ERROR_LOCATION );    
  check ( bytes >= MINIMUM_BYTE_INCREASE, "⚠️ Must purchase at least " + std::to_string(MINIMUM_BYTE_INCREASE) + " byte(s) if allowable by " + NATIVE_SYSTEM_CONTRACT_NAME_STRING + " contract ⚠️" + ERROR_LOCATION );
  if ( miner_itr != _miner.end() ){
    _miner.modify( miner_itr , same_payer , [&]( auto& mnr ){ // ram bill                                                
      mnr.bytes += bytes;
    });
  } 
  
}

/*
  █▄░█ █▀▀ ▀█▀   █▀█ █░█ █▀█ █▀▀ █░█ ▄▀█ █▀ █▀▀ █▀▄  █▄▄ █▄█ ▀█▀ █▀▀ █▀   ▄▀█ █▀▀ ▀█▀ █▀▀ █▀█   █▀▀ █▀█ █▀ █ █▀█  █▀▀ █▀▀ █▀▀ █▀
  █░▀█ ██▄ ░█░   █▀▀ █▄█ █▀▄ █▄▄ █▀█ █▀█ ▄█ ██▄ █▄▀  █▄█ ░█░ ░█░ ██▄ ▄█   █▀█ █▀░ ░█░ ██▄ █▀▄   ██▄ █▄█ ▄█ █ █▄█  █▀░ ██▄ ██▄ ▄█
  @brief: return 'x' BYTES after default EOSIO.RAM fees ( for byte purchase only ) 
*/

uint64_t CONTRACT_CLASS_NAME::net_purchased_bytes_after_eosio_fees( const asset& quant ){

  check( quant.amount > 0, "⚠️ must purchase a positive amount ⚠️" + ERROR_LOCATION );
  uint64_t bytes_out = direct_convert( asset(eosio_ram_fees(quant.amount), NATIVE_SYM), RAM_SYM).amount;
  check( bytes_out > 0, "⚠️ must reserve a positive amount ⚠️" + ERROR_LOCATION );
  return bytes_out;
  
}

/*
  █▀▀ █▀█ █▀ █ █▀█   █▀█ ▄▀█ █▀▄▀█   █▀▀ █▀▀ █▀▀ █▀
  ██▄ █▄█ ▄█ █ █▄█   █▀▄ █▀█ █░▀░█   █▀░ ██▄ ██▄ ▄█
  @brief: return 'x' core native tokens after default EOSIO.RAM fees
*/

uint64_t CONTRACT_CLASS_NAME::eosio_ram_fees( uint64_t eos ) {

  uint64_t default_fee = ( eos + 199 ) / 200; // .5% fee (round up)
  uint64_t net_eos = eos - default_fee; 
  return net_eos; 
  
}

/*
  █▀▄ █▀▀ █░█ █▀▀ █░░ █▀█ █▀█ █▀▀ █▀█   █▀▀ █▀▀ █▀▀ █▀
  █▄▀ ██▄ ▀▄▀ ██▄ █▄▄ █▄█ █▀▀ ██▄ █▀▄   █▀░ ██▄ ██▄ ▄█
  @brief: return 'x' EOS after developer fees ( designed to match eosio.ramfees ) 
*/
uint64_t CONTRACT_CLASS_NAME::core_tokens_post_dev_fees( uint64_t eos ) {

  uint64_t default_fee = ( eos + 199 ) / 200; // .5% fee (round up)
  uint64_t net_eos = eos - default_fee; 
  return net_eos;
  
}

/*
  █░█ █▀█ █▀▄ ▄▀█ ▀█▀ █▀▀ █▀▄   █▀█ ▄▀█ █▀▄▀█   █▀ █░█ █▀█ █▀█ █░░ █▄█
  █▄█ █▀▀ █▄▀ █▀█ ░█░ ██▄ █▄▀   █▀▄ █▀█ █░▀░█   ▄█ █▄█ █▀▀ █▀▀ █▄▄ ░█░
  @brief: Default eosio contract function
*/

uint64_t CONTRACT_CLASS_NAME::updated_ram_supply() {

  global_state2_singleton eosio_config_state2( NATIVE_SYSTEM_CONTRACT_NAME, NATIVE_SYSTEM_CONTRACT_NAME_VALUE );
  auto system_delta2 = eosio_config_state2.get();
  auto cbt = eosio::current_block_time();
  if( cbt <= system_delta2.last_ram_increase ) return 0;
  uint64_t new_ram = (cbt.slot - system_delta2.last_ram_increase.slot) * system_delta2.new_ram_per_block;
  return new_ram;
  
}

/*
  █▀▄ █ █▀█ █▀▀ █▀▀ ▀█▀   █▀▀ █▀█ █▄░█ █░█ █▀▀ █▀█ ▀█▀
  █▄▀ █ █▀▄ ██▄ █▄▄ ░█░   █▄▄ █▄█ █░▀█ ▀▄▀ ██▄ █▀▄ ░█░
  @brief: Default eosio contract function
*/

asset CONTRACT_CLASS_NAME::direct_convert( const asset& from, const symbol& to ){

  eosiosystem::rammarket exchange_state( NATIVE_SYSTEM_CONTRACT_NAME, NATIVE_SYSTEM_CONTRACT_NAME_VALUE );
  auto eosio_ramcore_itr = exchange_state.lower_bound( RAMCORE_RAW );
  const auto& sell_symbol  = from.symbol;
  const auto& base_symbol  = eosio_ramcore_itr -> base.balance.symbol;
  const auto& quote_symbol = eosio_ramcore_itr -> quote.balance.symbol;
  check( sell_symbol != to, "⚠️ cannot convert to the same symbol ⚠️" + ERROR_LOCATION );
  asset out( 0, to );
  if ( sell_symbol == base_symbol && to == quote_symbol ) {
      out.amount = get_bancor_output( eosio_ramcore_itr -> base.balance.amount + updated_ram_supply(), eosio_ramcore_itr -> quote.balance.amount, from.amount );
  } else if ( sell_symbol == quote_symbol && to == base_symbol ) {
      out.amount = get_bancor_output( eosio_ramcore_itr -> quote.balance.amount, eosio_ramcore_itr -> base.balance.amount + updated_ram_supply(), from.amount );
  } else {
    check( false, "⚠️ invalid conversion ⚠️" + ERROR_LOCATION );
  }
  return out;
  
}

/*
  █▀▀ █▀▀ ▀█▀   █▄▄ ▄▀█ █▄░█ █▀▀ █▀█ █▀█   █▀█ █░█ ▀█▀ █▀█ █░█ ▀█▀
  █▄█ ██▄ ░█░   █▄█ █▀█ █░▀█ █▄▄ █▄█ █▀▄   █▄█ █▄█ ░█░ █▀▀ █▄█ ░█░
  @brief: Default eosio contract function
*/

int64_t CONTRACT_CLASS_NAME::get_bancor_output( int64_t inp_reserve, int64_t out_reserve, int64_t inp ){

  const double ib = inp_reserve;
  const double ob = out_reserve;
  const double in = inp;
  int64_t out = int64_t( (in * ob) / (ib + in) );
  if ( out < 0 ) out = 0;
  return out;
  
}

/*
  █▀▄ █▀▀ █░█ █▀▀ █░░ ▄▀█ █ █▀▄▀█
  █▄▀ ██▄ ▀▄▀ █▄▄ █▄▄ █▀█ █ █░▀░█
  @brief: _self developer claims / withdraws previously collected core token fees ( collected from byte increases )
  @param quantity: native core token amount ( EOS, WAX, TLOS ) 
*/

[[eosio::action]] void  CONTRACT_CLASS_NAME::devclaim( asset quantity ) {

  require_auth ( CACHEDEV_ACCT );

  auto miningstats_delta = _miningstats.get();

  check( quantity.amount > 0, "⚠️ Must transfer positive quantity ⚠️" + ERROR_LOCATION );
  check( quantity <= miningstats_delta.dev_claim_bal, "⚠️ Overdrawn claim  ⚠️" + ERROR_LOCATION );

  miningstats_delta.dev_claim_bal -= quantity;
 
  token::transfer_action TRANSFER_CORE_TOKEN_FEES_TO_DEV(NATIVE_SYSTEM_TOKEN_CONTRACT_NAME, ANTELOPE_CONTRACT_ACTIVE_PERMISSION); 
  TRANSFER_CORE_TOKEN_FEES_TO_DEV.send(get_self(), CACHEDEV_ACCT, quantity, "dev fees"  );

  _miningstats.set(miningstats_delta, _self);
  
}