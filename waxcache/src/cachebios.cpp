/*
  ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
  ██╗░░  ░█████╗░░█████╗░░█████╗░██╗░░██╗███████╗██████╗░██╗░█████╗░░██████╗
  ╚██╗░  ██╔══██╗██╔══██╗██╔══██╗██║░░██║██╔════╝██╔══██╗██║██╔══██╗██╔════╝
  ░╚██╗  ██║░░╚═╝███████║██║░░╚═╝███████║█████╗░░██████╦╝██║██║░░██║╚█████╗░
  ░██╔╝  ██║░░██╗██╔══██║██║░░██╗██╔══██║██╔══╝░░██╔══██╗██║██║░░██║░╚═══██╗
  ██╔╝░  ╚█████╔╝██║░░██║╚█████╔╝██║░░██║███████╗██████╦╝██║╚█████╔╝██████╔╝
  ╚═╝░░  ░╚════╝░╚═╝░░╚═╝░╚════╝░╚═╝░░╚═╝╚══════╝╚═════╝░╚═╝░╚════╝░╚═════╝░
  ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
  @brief: launch contract action ( could later be recycled to modify contract )
*/

#include <cache.hpp>

/*
  █▄▄ █ █▀█ █▀
  █▄█ █ █▄█ ▄█
  @brief: first action that launches contract, token, inflation, & sovereign permission structure
  @param contractissuer: _self which pays for contract creation
*/

[[eosio::action]] void CONTRACT_CLASS_NAME::bios( name contractissuer ){

    require_auth ( _self );
    miningstats_check("bios_activation");

    auto sym = SET_MAX_SUPPLY.symbol;
    stats statstable( _self, sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    auto config_delta = _config.get_or_create( _self , config{} ) ;
    auto miningstats_delta = _miningstats.get_or_create (_self, miningstats{} );

    check( sym.is_valid(), "⚠️ invalid symbol name ⚠️" + ERROR_LOCATION );
    check( SET_MAX_SUPPLY.is_valid(), "⚠️invalid supply ⚠️" + ERROR_LOCATION );
    check( SET_MAX_SUPPLY.amount > 0, "⚠️ max-supply must be positive ⚠️" + ERROR_LOCATION );
    check( existing == statstable.end(), "⚠️ token with symbol already exists ⚠️" + ERROR_LOCATION );

    // Create antelopeCACHE token
    statstable.emplace( _self, [&]( auto& s ) { // ram bill
        s.supply                         = SET_BURN_SUPPLY + SET_DEVELOPER_PREMINE;
        s.max_supply                     = SET_MAX_SUPPLY + SET_DEVELOPER_PREMINE;
        s.max_mine_supply                = SET_MAX_MINE_SUPPLY;
        s.mined_supply                   = asset( 0, CACHE_SYM);
        s.unredeemed_supply              = 0;
        s.burn_supply                    = SET_BURN_SUPPLY;  
        s.issuer                         = ANTELOPE_CONTRACT_NAME;
        s.block_txs                      = 0;    
        s.block_fees                     = asset(0, CACHE_SYM);   
        s.fee_vt_weight                  = 0; 
        s.current_tx_fee                 = asset(0, CACHE_SYM); 
        s.upcoming_tx_fee                = asset(0, CACHE_SYM); 
        //s.whtlst_xchng_accts;
    });

    // add burn balance to _self
    accounts self_acct( _self, contractissuer.value);
    auto contract_account = self_acct.find( CACHE_RAW ); 
    if ( contract_account == self_acct.end() ) {
      self_acct.emplace( _self, [&]( auto& a ){ // ram bill
      a.balance = SET_BURN_SUPPLY;
      });
    }

    accounts the_cachedev_acct( _self, CACHEDEV_ACCT.value);
    auto cachedev = the_cachedev_acct.find( CACHE_RAW );
    if ( cachedev == the_cachedev_acct.end() ) {
      the_cachedev_acct.emplace( _self, [&]( auto& a ){ // ram bill
      a.balance = SET_DEVELOPER_PREMINE;
      });
    }

    // config
    config_delta.prop_vote_decay_start    = NOW;
    config_delta.prop_vote_decay_end      = NOW + REQ_VOTE_PCT_DECAY_PERIOD;
    config_delta.initial_vote_pct_req     = INITIAL_VOTE_PCT_REQ;
    config_delta.ending_vote_pct_req      = ENDING_VOTE_PCT_REQ;
    config_delta.proposition_period       = SET_PROPOSITION_PERIOD;  
    config_delta.min_leading_blocks       = SET_MINIMUM_LEADING_BLOCK;
    config_delta.max_fee_vote             = SET_MAX_FEE_VOTE; 
    config_delta.block_interval           = SET_BLOCK_INTERVAL;      
    config_delta.max_subsidy_storage      = SET_MAX_SUBSIDY_STORAGE;
    config_delta.const_burn_rate          = SET_CONST_BURN_RATE;
    config_delta.ramcustodian             = RESERVE_ACCT;
    config_delta.developer                = CACHEDEV_ACCT;

    //miningstats  
    miningstats_delta.miner_count         = 0;
    miningstats_delta.last_subsidy        = 0;
    miningstats_delta.nxt_block_time      = 0;
    miningstats_delta.prev_ram_price      = get_ram_price();
    miningstats_delta.stored_subsidies    = 0;
    miningstats_delta.dev_claim_bal       = asset( 0, NATIVE_SYM );

    tokenomics();
    sovereignize_account();

    _config.set(config_delta, _self);
    _miningstats.set(miningstats_delta, _self);

    print ( "{ logic initiated 💥 } \n");
    print ( "⛓ 🖖 Hello AntelopeSpace! 🖖 ⛓ \n" ); 

}   

/*
  ▀█▀ █▀█ █▄▀ █▀▀ █▄░█ █▀█ █▀▄▀█ █ █▀▀ █▀
  ░█░ █▄█ █░█ ██▄ █░▀█ █▄█ █░▀░█ █ █▄▄ ▄█
  @brief: One time void function utilzied to establish infflectionary schedule
*/

void CONTRACT_CLASS_NAME::tokenomics(){

  require_auth( _self );

  inflation_table _inflation( get_self(), get_self().value );
  auto rwd_itr = _inflation.find( 0 );
  uint64_t num_of_halvings = SET_NUM_OF_HALVINGS;  
  uint64_t reward =  SET_STARTING_BLOCK_REWARD;
  uint64_t blocks_between_halvings = SET_BLOCKS_PER_HALVING;
  int halving_divisor = SET_HALVING_DIVISOR;
  uint64_t new_supply = reward * blocks_between_halvings; 
  uint64_t added_supply = new_supply;
  uint64_t total_supply = added_supply;
  uint64_t max_subsidized_blocks = ( num_of_halvings * blocks_between_halvings ); 
  uint64_t era = 1; 

  check ( rwd_itr == _inflation.end(), "⚠️ The tokenomics and contract are already in motion ⚠️" + ERROR_LOCATION  );

  if ( rwd_itr == _inflation.end() ){;
    for ( int tokenomics_itr = 0; tokenomics_itr != max_subsidized_blocks; tokenomics_itr += blocks_between_halvings , reward /= halving_divisor , added_supply /= halving_divisor , total_supply += added_supply , era +=1 ) {
      _inflation.emplace( _self , [&]( auto& rwd ){ // ram bill
        rwd.block         = tokenomics_itr;
        rwd.era           += era;
        rwd.block_subsidy = asset ( reward, CACHE_SYM);
        rwd.added_supply  = asset ( added_supply, CACHE_SYM );
        rwd.total_supply  = asset ( total_supply, CACHE_SYM );
      });
    } 
    // add one more row for post inflation era
    auto largest_primary_key = _inflation.rbegin();
      _inflation.emplace( _self , [&]( auto& rwd ){ // ram bill
        rwd.block         = largest_primary_key -> block + blocks_between_halvings;
        rwd.era           = largest_primary_key -> era + 1;
        rwd.block_subsidy = asset ( 0 , CACHE_SYM);
        rwd.added_supply  = asset ( 0, CACHE_SYM );
        rwd.total_supply  = largest_primary_key -> total_supply;
      });
    print ("{ tokenomics initialized 💥 } \n"); 
  }

}

/*
█▀ █▀█ █░█ █▀▀ █▀█ █▀▀ █ █▀▀ █▄░█ █ ▀█ █▀▀   ▄▀█ █▀▀ █▀▀ █▀█ █░█ █▄░█ ▀█▀
▄█ █▄█ ▀▄▀ ██▄ █▀▄ ██▄ █ █▄█ █░▀█ █ █▄ ██▄   █▀█ █▄▄ █▄▄ █▄█ █▄█ █░▀█ ░█░
*/
void CONTRACT_CLASS_NAME::sovereignize_account(){

  // Define the account permission structure
  eosiobios::authority ownauth;
  eosiobios::permission_level_weight account_permission{{get_self(), eosio::name("eosio.code")}, 1};
  ownauth.accounts.push_back(account_permission);
  ownauth.threshold = 1;

  // owner permission modification 
  eosio::action(ANTELOPE_CONTRACT_OWNER_PERMISSION, 
  NATIVE_SYSTEM_CONTRACT_NAME,
  eosio::name("updateauth"), 
  std::tuple(
   eosio::name(get_self()),
   eosio::name("owner"), 
   eosio::name(""), 
   ownauth) )
   .send();

  // active permission modification 
  eosio::action(ANTELOPE_CONTRACT_OWNER_PERMISSION, 
  NATIVE_SYSTEM_CONTRACT_NAME,
  eosio::name("updateauth"), 
  std::tuple(
   eosio::name(get_self()),
   eosio::name("active"), 
   eosio::name("owner"), 
   ownauth) )
   .send();

  print ( "{ contract soverignized 💥 } \n" );
}
