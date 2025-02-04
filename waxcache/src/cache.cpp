/*
  ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
  ██╗░░  ███╗░░░███╗██╗███╗░░██╗██╗███╗░░██╗░██████╗░ . ░█████╗░░█████╗░████████╗██╗░█████╗░███╗░░██╗░██████╗
  ╚██╗░  ████╗░████║██║████╗░██║██║████╗░██║██╔════╝░ . ██╔══██╗██╔══██╗╚══██╔══╝██║██╔══██╗████╗░██║██╔════╝
  ░╚██╗  ██╔████╔██║██║██╔██╗██║██║██╔██╗██║██║░░██╗░ . ███████║██║░░╚═╝░░░██║░░░██║██║░░██║██╔██╗██║╚█████╗░
  ░██╔╝  ██║╚██╔╝██║██║██║╚████║██║██║╚████║██║░░╚██╗ . ██╔══██║██║░░██╗░░░██║░░░██║██║░░██║██║╚████║░╚═══██╗
  ██╔╝░  ██║░╚═╝░██║██║██║░╚███║██║██║░╚███║╚██████╔╝ . ██║░░██║╚█████╔╝░░░██║░░░██║╚█████╔╝██║░╚███║██████╔╝
  ╚═╝░░  ╚═╝░░░░░╚═╝╚═╝╚═╝░░╚══╝╚═╝╚═╝░░╚══╝░╚═════╝░ . ╚═╝░░╚═╝░╚════╝░░░░╚═╝░░░╚═╝░╚════╝░╚═╝░░╚══╝╚═════╝░
  ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
  @brief: contract business logic actions/function
*/

#include "cachebios.cpp"
#include "cachetoken.cpp"
#include "msigcontract.cpp"
#include "inlineram.cpp"
#include "whitelist.cpp"

using namespace eosiosystem;

/*
  █▀▀ █▄░█ █░░ █ █▀ ▀█▀
  ██▄ █░▀█ █▄▄ █ ▄█ ░█░ 
  @brief: Register your antelope account as an antelopeCACHE miner
  @param miner: account registering as a miner 
*/

[[eosio::action]] void CONTRACT_CLASS_NAME::enlist( name miner ){
  require_auth( miner); 
  miningstats_check("miningstats_verification");
  checkBlockReward();

  miner_table _miner( get_self(), get_self().value );
  auto miner_itr = _miner.find( miner.value );

  check ( miner_itr == _miner.end(), "⚠️ " + miner.to_string() + " is already a miner  ⚠️" + ERROR_LOCATION );

  if ( miner_itr == _miner.end() ){
    _miner.emplace( miner , [&]( auto& mach ){  // ram bill
      mach.cache_miner = miner;
      mach.start_time = NOW + 1;
      mach.last_claim = NOW + 1;
      mach.voted_fee = asset (0, CACHE_SYM);
    });
    print ( "🚀 ", miner, " successfully registered as a miner. Welcome to ", ANTELOPE_CONTRACT_NAME_STRING , " 🚀 \n" );
  }

}

/*
  ████████████████████████████████████████████████████████
  █▄─▀█▀─▄█▄─▄█▄─▀█▄─▄█▄─▄▄─█▄─▄─▀█▄─▄███─▄▄─█─▄▄▄─█▄─█─▄█
  ██─█▄█─███─███─█▄▀─███─▄█▀██─▄─▀██─██▀█─██─█─███▀██─▄▀██
  ▀▄▄▄▀▄▄▄▀▄▄▄▀▄▄▄▀▀▄▄▀▄▄▄▄▄▀▄▄▄▄▀▀▄▄▄▄▄▀▄▄▄▄▀▄▄▄▄▄▀▄▄▀▄▄▀
  ████████████████████████████████████████████████████████
  @brief: mine a block
  @param initiator: account that processes block ( any antelope account )
  @param block_memo: initiator's message to chain
*/

[[eosio::action]] void  CONTRACT_CLASS_NAME::mineblock( name initiator, string memo){

  require_auth(initiator);
  miningstats_check("miningstats_verification");

  auto config_delta = _config.get();
  auto miningstats_delta = _miningstats.get();
  stats statstable( _self , CACHE_RAW  );  
  auto existing = statstable.find( CACHE_RAW );    
  const auto& st = *existing;
  tokensubsidy_table _tokensubsidy( get_self(), get_self().value );
  auto win_itr = _tokensubsidy.find( NOW );
  auto oldest_subsidy_record = _tokensubsidy.begin();
  auto largest_key_subsidyItr = _tokensubsidy.rbegin();
  uint64_t seconds_till_next_subsidy = miningstats_delta.nxt_block_time - NOW;
  inflation_table _inflation( get_self(), get_self().value );
  auto block_itr = _inflation.find( miningstats_delta.current_block );
  auto era_index = _inflation.get_index<"era"_n>();   
  auto current_or_next_era = era_index.lower_bound( miningstats_delta.current_era );   
  auto fetch_subsidy_mint_via_era_index = current_or_next_era -> block_subsidy.amount; 
  uint64_t local_block_current_tx = st.block_txs;
  asset local_current_tx_fee = st.current_tx_fee;;
  asset local_collected_block_fees = st.block_fees;
  
  asset organically_issued_supply_during_this_block_reward = asset( 0 , CACHE_SYM);
  asset totaled_reward =  asset( 0 , CACHE_SYM);                

  check ( miningstats_delta.genesis_bytes > 0, "⚠️ Pending Genesis Bytes ⚠️" + ERROR_LOCATION );
  check ( miningstats_delta.nxt_block_time <= NOW, "⚠️ Blocktime not reached.  Approximately "+ std::to_string(seconds_till_next_subsidy) + " seconds remaining till next block ⚠️" + ERROR_LOCATION );

  // Purge Existing Subsidies, ensure we don't have more recorded subsidies over max allowed ( to prevent _self RAM overdraft ) 
  if ( ( oldest_subsidy_record != _tokensubsidy.end() ) && (  miningstats_delta.stored_subsidies >= config_delta.max_subsidy_storage ) ) {
    _tokensubsidy.erase(oldest_subsidy_record);
    miningstats_delta.stored_subsidies -= 1;
  }

  // Determine block subsidy & log to Tokensubsidy table
  if ( st.max_mine_supply > st.mined_supply && miningstats_delta.current_block == 0 ){        // GENESIS BLOCK:  Verify if its the Genesis Block by checking the current block is equal to 0, & we have not reached max mintable supply.
    print ( "⛏ 🥳 ", initiator, " successfully mined the Genesis Block # ", miningstats_delta.current_block, " 🥳 ⛏ \n" );
  } else if ( st.max_mine_supply > st.mined_supply && miningstats_delta.current_block > 0 ){  // REGULAR BLOCK: Verify we are not in the Genesis Block by checking checking current block is greater than 0, & we have not reach max mintable supply.
    totaled_reward = ( asset( fetch_subsidy_mint_via_era_index + ( st.block_fees.amount ) , CACHE_SYM)); 
    organically_issued_supply_during_this_block_reward = asset( fetch_subsidy_mint_via_era_index, CACHE_SYM);
    print ( "⛏ " , initiator, " successfully mined block # ", miningstats_delta.current_block, "! ⛏ \n" );
    print ( "Block subsidy total: " ,asset( fetch_subsidy_mint_via_era_index , CACHE_SYM), ". Transaction fee total: " , st.block_fees , " \n" );
  } else if ( st.max_mine_supply == st.mined_supply && miningstats_delta.current_block > 0 ){ // POST INFLATION BLOCK: Verify we are not in the Genesis Block by checking current block is greater than 0, & we have reached max mintable supply.
    totaled_reward = st.block_fees; 
    print ( "⛏ " , initiator, " successfully mined block # ", miningstats_delta.current_block, "! ⛏ \n" );
    print ( "Transaction fee total: " , st.block_fees, " \n" );
  } else {   // do nothing 
  }

  // document after block console print
  asset total_block_burn = algorithmic_burn(); 

  // If inflation schedule has not ended yet , update time era was reached
  if ( block_itr != _inflation.end() ){
    _inflation.modify( block_itr , same_payer , [&]( auto& rwd ){ // ram bill
      rwd.time_reached = NOW;
    }); 
    miningstats_delta.current_era += 1;
  } 

  // create new tokensubsidy record
  if ( win_itr == _tokensubsidy.end() ){; 
    _tokensubsidy.emplace( initiator , [&]( auto& win ) { // ram bill
      win.block_time = NOW;
      win.block = miningstats_delta.current_block;
      win.total_bytes = miningstats_delta.total_bytes - miningstats_delta.genesis_bytes;  
      win.miner_count = miningstats_delta.miner_count; 
      win.unclaimed_miners = miningstats_delta.miner_count;
      win.reward = totaled_reward;
    });

    // update miningstats singleton
    miningstats_delta.current_block += 1;
    miningstats_delta.last_subsidy = largest_key_subsidyItr -> block_time;
    miningstats_delta.nxt_block_time =  config_delta.block_interval + NOW;  
    miningstats_delta.prev_ram_price = get_ram_price(); 
    miningstats_delta.stored_subsidies += 1;
  }

  // Burn Tokens from _self
  if ( total_block_burn >= asset( 0, CACHE_SYM )) {
      sub_balance( st.issuer, total_block_burn );
  }

  statstable.modify( st, _self, [&]( auto& s ) { // ram bill
      uint128_t totaled_reward_uint128 = static_cast<uint128_t>(totaled_reward.amount) * SCALE_FACTOR;
      // updated supply
      s.supply += organically_issued_supply_during_this_block_reward - total_block_burn; 
      s.mined_supply += organically_issued_supply_during_this_block_reward;
      s.unredeemed_supply += totaled_reward_uint128;
      s.burn_supply -= total_block_burn;
      // Update current block fees
      s.block_fees = asset(0,CACHE_SYM);
      s.block_txs = 0; 
      s.current_tx_fee = s.upcoming_tx_fee;
  });

  add_balance( st.issuer, organically_issued_supply_during_this_block_reward, _self );

  _miningstats.set(miningstats_delta, _self);

  //leading proposal logic
  proposals_table _proposals(get_self(), get_self().value);
  auto prop_vote_count_index = _proposals.get_index<"byvotes"_n>(); 
  auto highest_vote_count_itr = prop_vote_count_index.rbegin();
  uint64_t leading_proposal_with_required_vote_percentage = 0;

  if ( highest_vote_count_itr != prop_vote_count_index.rend() ) {
    auto prop_with_most_votes = highest_vote_count_itr -> votes;
    auto prop_itr = _proposals.find (highest_vote_count_itr -> prop_timestamp);
    double largest_count_count_as_double = prop_with_most_votes;
    double total_bytes_wihtout_genesis_bytes = miningstats_delta.total_bytes - miningstats_delta.genesis_bytes;
    double percentage_of_votes = largest_count_count_as_double / total_bytes_wihtout_genesis_bytes;
    if ( percentage_of_votes > required_vote_percentage_by_decay() ) {
      auto [existing_proposal_vote_status, existing_proposal_vote_message] = proposal_table_iterator(prop_itr -> prop_timestamp);
      if ( existing_proposal_vote_status == "not_legit" ) {
        print ( ": proposal ", prop_itr -> prop_timestamp, " not recorded as leading proposal. \n");
      } else {
      _proposals.modify(prop_itr, same_payer, [&](auto& prp) { // ram bill  
        prp.leading_blocks += 1;
      });
      leading_proposal_with_required_vote_percentage = prop_itr -> prop_timestamp;
      }
    }
  }

  // log block reward for later query
  eosio::action(ANTELOPE_CONTRACT_ACTIVE_PERMISSION, 
  (get_self()), eosio::name("blocklog"), 
    std::tuple(
      initiator, 
      miningstats_delta.current_block - 1,
      NOW,
      organically_issued_supply_during_this_block_reward,
      local_collected_block_fees,
      total_block_burn,
      st.supply,
      st.burn_supply,
      miningstats_delta.miner_count,
      miningstats_delta.total_bytes - miningstats_delta.genesis_bytes,
      local_block_current_tx,
      local_current_tx_fee,
      miningstats_delta.active_props,
      leading_proposal_with_required_vote_percentage,
      get_ram_price(),
      memo)
  ).send();

}

/*
  █▄▄ █░░ █▀█ █▀▀ █▄▀ █░░ █▀█ █▀▀
  █▄█ █▄▄ █▄█ █▄▄ █░█ █▄▄ █▄█ █▄█
  @brief: only actionable by _self, generates a queryable log of each subsidy
*/

[[eosio::action]]
void CONTRACT_CLASS_NAME::blocklog(name initiator, 
                      uint64_t block,
                      uint64_t block_time, 
                      asset issued_supply, 
                      asset collected_tx_fees, 
                      asset burned_tokens, 
                      asset total_supply, 
                      asset burn_supply, 
                      uint64_t cache_miners, 
                      uint64_t bytes, 
                      uint64_t txs,
                      asset tx_fee,
                      uint64_t active_proposals,
                      uint64_t leading_proposal,
                      double ram_price, 
                      string block_memo) {

  check(has_auth(get_self()), "⚠️ 'blocklog' is only actionable as an inline action by  " + _self.to_string() + " in the 'mineblock' action ⚠️" + ERROR_LOCATION );
  check( block_memo.size() <= MAX_BLOCK_MEMO_SIZE, "⚠️ block_memo limited to " + std::to_string(MAX_BLOCK_MEMO_SIZE ) + " bytes ⚠️" + ERROR_LOCATION );
}

/*
  █▀▀ █░░ ▄▀█ █ █▀▄▀█
  █▄▄ █▄▄ █▀█ █ █░▀░█
  @brief: save qualifying subsidies from 'tokensubsidy' table as shares ( stored as a 10^18 scaled up integer ) for future redemption
  @param miner: miner that searches for claimable rewards
  @param max_claims: max # of 'tokensubidy' rows to iterate in search of qualifying block subsidies ( limited only by cpu/net usage and/or stored rows)
*/

[[eosio::action]] void  CONTRACT_CLASS_NAME::claim( name miner, uint64_t max_claims ) {

  require_auth( miner );
  miningstats_check("miningstats_verification");
  checkBlockReward();

  auto miningstats_delta = _miningstats.get();
  miner_table _miner( get_self(), get_self().value );
  auto miner_itr = _miner.find( miner.value );
  tokensubsidy_table _tokensubsidy( get_self(), get_self().value );
  uint64_t access_miner_last = miner_itr -> last_claim;  
  auto next_blocktime_itr = _tokensubsidy.upper_bound( access_miner_last ); 
  bool max_claims_flag = false;
  uint128_t total_rewards = 0, shares = 0, local_shares = 0;
  uint64_t reward_count = 0, row_count = 0;
  string blocks;

  check ( next_blocktime_itr -> block_time > access_miner_last  , "⚠️ ️ You've claimed everything to latest block  ⚠️" + ERROR_LOCATION );
  check ( miner_itr -> bytes != 0 , "⚠️ Miner is not active ⚠️" + ERROR_LOCATION );
  check ( miner_itr != _miner.end(), "⚠️ Miner does not exist  ⚠️" + ERROR_LOCATION );
  check ( max_claims >= MINIMUM_CLAIMS, "⚠️ Minimum claim search is " + std::to_string(MINIMUM_CLAIMS) + " ⚠️" + ERROR_LOCATION );
  check ( miner != miningstats_delta.genesis_miner, "⚠️ Genesis Miner has no claiming privileges ⚠️" + ERROR_LOCATION );

  for (auto loop_next_blocktime_itr = _tokensubsidy.upper_bound( access_miner_last ); loop_next_blocktime_itr != _tokensubsidy.end() && !max_claims_flag; ++loop_next_blocktime_itr) {

    // Convert values to scaled integers
    uint128_t fetch_reward = static_cast<uint128_t>(loop_next_blocktime_itr->reward.amount * SCALE_FACTOR);
    uint128_t current_bytes = static_cast<uint128_t>(miner_itr->bytes);
    uint128_t total_bytes = static_cast<uint128_t>(loop_next_blocktime_itr->total_bytes);
    
    // Calculate miner's share of the reward
    uint128_t miner_cut_of_reward = (fetch_reward * current_bytes) / total_bytes;

    // Update tokensubsidy table to reduce unclaimed miners
    _tokensubsidy.modify(loop_next_blocktime_itr, same_payer, [&](auto& win) {
        win.unclaimed_miners -= 1;
    });

    blocks.append(" {" + std::to_string(loop_next_blocktime_itr->block_time) + "}");
    
    // Accumulate shares and rewards
    shares += miner_cut_of_reward;
    local_shares = miner_cut_of_reward;
    total_rewards += miner_cut_of_reward;
    reward_count += 1;

    // Update miner's record with new unredeemed shares and last claim
    _miner.modify(miner_itr, same_payer, [&](auto& mach) {
        mach.last_claim = loop_next_blocktime_itr->block_time;
        mach.unredeemed_shares += local_shares; 
    });

    row_count += 1;
    local_shares = 0;
    if (row_count == max_claims) { max_claims_flag = true; }
  }        

  check(row_count > 0, "⚠️ Nothing more to claim until next block ⚠️" + ERROR_LOCATION);

  // Print claim summary for the miner
  print(miner, " confirmed a total of ", row_count, " ", ANTELOPE_CONTRACT_NAME_STRING, " blocktimes : \n");
  print("[ ", blocks, " ] \n");

  if (shares > 0) {
    // Convert `total_rewards` from scaled integer back to double for display
    double display_rewards = static_cast<double>(total_rewards) / SCALE_FACTOR; // Scale back down
    print("And successfully identified ", reward_count, " ", ANTELOPE_CONTRACT_NAME_STRING, " blocks with earnings totaling: ", display_rewards, " shares ฅ^•ﻌ•^ฅ 💰🎉 \n");
  } else {
    print("📌 No claimable shares identified 📌 \n");
  }
}

/*
  █▀█ █▀▀ █▀▄ █▀▀ █▀▀ █▀▄▀█
  █▀▄ ██▄ █▄▀ ██▄ ██▄ █░▀░█
  @brief: converts previously saved mining shares to transferrable antelopeCACHE to specified account
  @param miner: miner entitled to antelopeCACHE 
  @param receiver: beneficiary to receive antelopeCACHE from shares
  @param requested_cache: total antelopeCACHE that is being requested ( shares are are stored as an integer that is 10^18 for precision )
*/

[[eosio::action]] void CONTRACT_CLASS_NAME::redeem(name miner, name receiver, asset requested_cache) {

  require_auth( miner);
  miningstats_check("miningstats_verification");
  checkBlockReward();

  auto miningstats_delta = _miningstats.get();
  miner_table _miner( get_self(), get_self().value );
  auto miner_itr = _miner.find( miner.value );
  stats statstable( _self , CACHE_RAW  );  
  auto existing = statstable.find( CACHE_RAW );
  uint128_t requested_shares = static_cast<uint128_t>(requested_cache.amount) * SCALE_FACTOR;

  // Calculate maximum redeemable shares and convert to asset format for display
  uint128_t max_available_miner_shares_scaled = miner_itr->unredeemed_shares / SCALE_FACTOR;
  uint64_t max_redeemable_amount = static_cast<uint64_t>(max_available_miner_shares_scaled);
  asset max_redeemable_asset(max_redeemable_amount, CACHE_SYM);

  check( requested_cache.is_valid(), "⚠️ Invalid quantity ⚠️" + ERROR_LOCATION );
  check( requested_cache.amount > 0, "⚠️ Must transfer positive quantity ⚠️" + ERROR_LOCATION );
  check( requested_cache.symbol ==  CACHE_SYM, "⚠️ Symbol precision mismatch ⚠️" + ERROR_LOCATION );
  check( miner != miningstats_delta.genesis_miner, "⚠️ Genesis Miner has no redemption privileges ⚠️" + ERROR_LOCATION);
  check( requested_shares <= miner_itr->unredeemed_shares,  "⚠️ Redeem amount exceeds available mining share balance. Max redeemable: " + max_redeemable_asset.to_string() + " ⚠️" + ERROR_LOCATION);
  check( miner_itr != _miner.end(), "⚠️ Miner does not exist ⚠️" + ERROR_LOCATION);

  // Proceed if there are sufficient shares available for redemption
  if (miner_itr->unredeemed_shares >= requested_shares) {
    // Update miner's unredeemed shares
    _miner.modify(miner_itr, same_payer, [&](auto& mach) {
        mach.unredeemed_shares -= requested_shares;
    });

    // Update the stats table's unredeemed supply
    statstable.modify(existing, _self, [&](auto& s) {
        s.unredeemed_supply -= requested_shares;
    });

    // Convert requested_shares to asset for redeeming tokens ( redundant )
    uint128_t shares_scaled_down_to_64 = requested_shares / SCALE_FACTOR;
    int64_t requested_shares_asset = static_cast<int64_t>( shares_scaled_down_to_64 );
    redeem_tokens(_self, miner, receiver, asset(requested_shares_asset, CACHE_SYM));

    print("🎉 Woohoo! ", miner, " just redeemed ", asset(requested_shares_asset, CACHE_SYM), "! 🚀💰 \n");
    print("ฅ^•ﻌ•^ฅ Keep it coming! Earnings credited to ", receiver, " \n");
  }
}

/*
  █░█ █▀█ ▀█▀ █▀▀ █▀▀ █▀▀ █▀▀
  ▀▄▀ █▄█ ░█░ ██▄ █▀░ ██▄ ██▄
  @brief: vote to modify the current antelopeCACHE transaction fee
  @param miner: miner voting for fee
  @param tx_fee: desired antelopeCACHE transaction fee in units of 0.0001
*/

[[eosio::action]] void  CONTRACT_CLASS_NAME::votefee( name miner , asset tx_fee ){

  require_auth ( miner);
  miningstats_check("miningstats_verification");
  checkBlockReward();

  auto miningstats_delta = _miningstats.get();
  auto config_delta = _config.get();
  auto total_mining_bytes_minus_genesis_bytes = miningstats_delta.total_bytes - miningstats_delta.genesis_bytes;
  miner_table _miner( get_self(), get_self().value );
  auto miner_itr = _miner.find( miner.value );
  stats statstable( _self , CACHE_RAW  );  
  auto existing = statstable.find( CACHE_RAW );
  const auto& st = *existing;

  check ( tx_fee.symbol ==  CACHE_SYM, "⚠️ Symbol precision mismatch ⚠️" + ERROR_LOCATION );
  check ( miner_itr != _miner.end(), "⚠️ Miner does not exist ⚠️" + ERROR_LOCATION );
  check ( tx_fee <= config_delta.max_fee_vote , "⚠️ Maximum votable fee is " + config_delta.max_fee_vote.to_string() + " ⚠️" + ERROR_LOCATION ); 
  check ( miner != miningstats_delta.genesis_miner && miningstats_delta.genesis_bytes != 0 , "⚠️ Genesis Miner has no voting privileges ⚠️" + ERROR_LOCATION );
  check ( total_mining_bytes_minus_genesis_bytes != 0 && miner_itr -> bytes != 0, "⚠️ Please vote again after increasing your byte balance ⚠️" + ERROR_LOCATION );

  uint128_t old_miner_weight = static_cast<uint128_t>(miner_itr->voted_fee.amount) * static_cast<uint128_t>(miner_itr->bytes);
  uint128_t new_miner_weight = static_cast<uint128_t>(tx_fee.amount) * static_cast<uint128_t>(miner_itr->bytes);
  uint128_t miningstats_weight_minus_old_miner_weight = st.fee_vt_weight - old_miner_weight;
  uint128_t new_miningstats_weight = miningstats_weight_minus_old_miner_weight + new_miner_weight;

  // Ensure division by zero is impossible
  uint64_t new_fee_for_next_block = 0;

  if (total_mining_bytes_minus_genesis_bytes > 0) {
    // Use uint128_t for the division to avoid precision loss
    uint128_t temp_fee_for_next_block = new_miningstats_weight / static_cast<uint128_t>(total_mining_bytes_minus_genesis_bytes);

    // Ensure the new fee does not exceed the maximum allowed fee
    if (asset(static_cast<uint64_t>(temp_fee_for_next_block), CACHE_SYM) >= config_delta.max_fee_vote) {
        print("max fee implemented: " + config_delta.max_fee_vote.to_string(), " \n");
        new_fee_for_next_block = config_delta.max_fee_vote.amount;
    } else {
        new_fee_for_next_block = static_cast<uint64_t>(temp_fee_for_next_block);
    }
  }

  statstable.modify( existing , _self , [&]( auto& s ){  // ram bill 
    s.fee_vt_weight = new_miningstats_weight;
    s.upcoming_tx_fee = asset (new_fee_for_next_block, CACHE_SYM);
  });

  _miner.modify ( miner_itr , same_payer , [&]( auto& f ){   // ram bill 
    f.voted_fee = tx_fee; 
  }); 

  print ( miner, " successfully modified transaction fee amount, next block transaction fees set to be: ", st.upcoming_tx_fee, " \n");

}

/*
    ████████╗██╗░░██╗███████╗   ██╗░░░██╗░█████╗░██╗██████╗░   ███████╗██╗░░░██╗███╗░░██╗░█████╗░████████╗██╗░█████╗░███╗░░██╗░██████╗
    ╚══██╔══╝██║░░██║██╔════╝   ██║░░░██║██╔══██╗██║██╔══██╗   ██╔════╝██║░░░██║████╗░██║██╔══██╗╚══██╔══╝██║██╔══██╗████╗░██║██╔════╝
    ░░░██║░░░███████║█████╗░░   ╚██╗░██╔╝██║░░██║██║██║░░██║   █████╗░░██║░░░██║██╔██╗██║██║░░╚═╝░░░██║░░░██║██║░░██║██╔██╗██║╚█████╗░
    ░░░██║░░░██╔══██║██╔══╝░░   ░╚████╔╝░██║░░██║██║██║░░██║   ██╔══╝░░██║░░░██║██║╚████║██║░░██╗░░░██║░░░██║██║░░██║██║╚████║░╚═══██╗
    ░░░██║░░░██║░░██║███████╗   ░░╚██╔╝░░╚█████╔╝██║██████╔╝   ██║░░░░░╚██████╔╝██║░╚███║╚█████╔╝░░░██║░░░██║╚█████╔╝██║░╚███║██████╔╝
    ░░░╚═╝░░░╚═╝░░╚═╝╚══════╝   ░░░╚═╝░░░░╚════╝░╚═╝╚═════╝░   ╚═╝░░░░░░╚═════╝░╚═╝░░╚══╝░╚════╝░░░░╚═╝░░░╚═╝░╚════╝░╚═╝░░╚══╝╚═════╝░
*/

/*
  █░█░█ █▀▀ █ █▀▀ █░█ ▀█▀   ▄▀█ █▀▄ ░░█ █░█ █▀ ▀█▀ █▀▄▀█ █▀▀ █▄░█ ▀█▀   █▀▀ █▀▀ █▀▀ █▀
  ▀▄▀▄▀ ██▄ █ █▄█ █▀█ ░█░   █▀█ █▄▀ █▄█ █▄█ ▄█ ░█░ █░▀░█ ██▄ █░▀█ ░█░   █▀░ ██▄ ██▄ ▄█
  @brief: adjust miningstats fee weight everytime a miner updates their mining balance
*/

void CONTRACT_CLASS_NAME::weight_adjustment_fees(name miner, uint64_t old_fee_vote, uint64_t new_fee_vote, uint64_t old_miner_byte_balance, uint64_t updated_miner_byte_balance  ){ 

  auto config_delta = _config.get();

  auto miningstats_delta = _miningstats.get();
  auto total_mining_bytes_minus_genesis_bytes = miningstats_delta.total_bytes - miningstats_delta.genesis_bytes;
  proposals_table _proposals(get_self(), get_self().value);
  stats statstable( _self , CACHE_RAW  );  
  auto existing = statstable.find( CACHE_RAW );
  const auto& st = *existing;

  uint64_t new_fee_for_next_block = 0;
  uint128_t old_miner_fee_weight = static_cast<uint128_t>(old_fee_vote) * static_cast<uint128_t>(old_miner_byte_balance);
  uint128_t new_miner_fee_weight = static_cast<uint128_t>(new_fee_vote) * static_cast<uint128_t>(updated_miner_byte_balance);
  uint128_t old_miningstats_fee_weight = static_cast<uint128_t>(st.fee_vt_weight) - old_miner_fee_weight;
  uint128_t new_miningstats_fee_weight = old_miningstats_fee_weight + new_miner_fee_weight;


  if (new_miningstats_fee_weight > 0) {
    if (total_mining_bytes_minus_genesis_bytes > 0) {
        uint128_t temp_fee_for_next_block = new_miningstats_fee_weight / static_cast<uint128_t>(total_mining_bytes_minus_genesis_bytes);

      // Ensure the new fee does not exceed the maximum allowed fee
      if (asset(static_cast<uint64_t>(temp_fee_for_next_block), CACHE_SYM) >= config_delta.max_fee_vote) {
          print("max fee implemented: " + config_delta.max_fee_vote.to_string(), " \n");
          new_fee_for_next_block = config_delta.max_fee_vote.amount;
      } else {
          new_fee_for_next_block = static_cast<uint64_t>(temp_fee_for_next_block);
      }
    }
  }

  statstable.modify( existing , _self , [&]( auto& s ){   // ram bill
    s.fee_vt_weight = new_miningstats_fee_weight;
    s.upcoming_tx_fee = asset(new_fee_for_next_block, CACHE_SYM);
  });

}

/*
  █░█ █▀▀ █▀█ █ █▀▀ █▄█   █▀▄▀█ █ █▄░█ █▀▀ █▀█   █▀▀ █░░ ▄▀█ █ █▀▄▀█ █▀▀ █▀▄
  ▀▄▀ ██▄ █▀▄ █ █▀░ ░█░   █░▀░█ █ █░▀█ ██▄ █▀▄   █▄▄ █▄▄ █▀█ █ █░▀░█ ██▄ █▄▀
  @brief: verify miner entitlement to block subsidies
*/

void CONTRACT_CLASS_NAME::verify_miner_claims ( name miner ) {

  auto miningstats_delta = _miningstats.get();
  check ( miner != miningstats_delta.genesis_miner, "⚠️ Genesis Miner " + miner.to_string() + " has no claiming privileges ⚠️" + ERROR_LOCATION );
  miner_table _miner( get_self(), get_self().value );
  auto miner_itr = _miner.find( miner.value );
  uint64_t access_miner_last = miner_itr -> last_claim;  
  tokensubsidy_table _tokensubsidy( get_self(), get_self().value );
  auto win = _tokensubsidy.upper_bound(0);

  if ( win != _tokensubsidy.end() ) { 
    auto largest_key_subsidyItr = _tokensubsidy.rbegin();
    uint64_t latest_block_number = largest_key_subsidyItr -> block;
    uint64_t latest_block_time = largest_key_subsidyItr -> block_time; 
    check ( miner_itr != _miner.end(), "⚠️ Miner does not exist ⚠️");

    // verify that miner has confirmed/claimed up to latest block
    if ( miner_itr -> bytes != 0 ) {
      check ( access_miner_last >= latest_block_time, "⚠️ " + miner.to_string() + " has potentially unclaimed " + CACHE_SYM.code().to_string() + ", please attempt to claim to latest block_time: " + std::to_string( latest_block_time ) + ", blockheight: " + std::to_string( latest_block_number ) + " ⚠️" + ERROR_LOCATION ); 
    }
  }

}

/*
  █▀ █▄█ █▀ ▀█▀ █▀▀ █▀▄▀█   █▀▀ █░█ █▀▀ █▀▀ █▄▀
  ▄█ ░█░ ▄█ ░█░ ██▄ █░▀░█   █▄▄ █▀█ ██▄ █▄▄ █░█
  @brief: Validate if antelopeCACHE contract is active
  @param type: type of validation ( true or false )
*/

void CONTRACT_CLASS_NAME::miningstats_check(string type){ 

  auto miningstats_delta = _miningstats.exists();
  if ( type == "bios_activation" ) {
    check ( miningstats_delta == false, "⚠️ Contract BIOS already activated ⚠️" + ERROR_LOCATION );
  } else if ( type == "miningstats_verification") {
    check ( miningstats_delta == true, "⚠️ Contract BIOS not activated yet ⚠️" + ERROR_LOCATION );
  }
}

/*
  █▀▀ █░█ █▀▀ █▀▀ █▄▀   █▄▄ █░░ █▀█ █▀▀ █▄▀   █▀█ █▀▀ █░█░█ ▄▀█ █▀█ █▀▄
  █▄▄ █▀█ ██▄ █▄▄ █░█   █▄█ █▄▄ █▄█ █▄▄ █░█   █▀▄ ██▄ ▀▄▀▄▀ █▀█ █▀▄ █▄▀
  @brief: check if block reward is overdue
*/

void CONTRACT_CLASS_NAME::checkBlockReward() {

    auto miningstats_delta = _miningstats.get();
    check(miningstats_delta.nxt_block_time > NOW || miningstats_delta.current_era == 0 , "⚠️ Block overdue, 'mineblock' first ⚠️" + ERROR_LOCATION );
}

/*
  ███████╗██╗░░░██╗███╗░░██╗░█████╗░░  ██╗░░░░░░░██╗░░░░██╗ . ██████╗░███████╗████████╗██╗░░░██╗██████╗░███╗░░██╗
  ██╔════╝██║░░░██║████╗░██║██╔══██╗░  ██║░░██╗░░██║░░░██╔╝ . ██╔══██╗██╔════╝╚══██╔══╝██║░░░██║██╔══██╗████╗░██║
  █████╗░░██║░░░██║██╔██╗██║██║░░╚═╝░  ╚██╗████╗██╔╝░░██╔╝░ . ██████╔╝█████╗░░░░░██║░░░██║░░░██║██████╔╝██╔██╗██║
  ██╔══╝░░██║░░░██║██║╚████║██║░░██╗░  ░████╔═████║░░██╔╝░░ . ██╔══██╗██╔══╝░░░░░██║░░░██║░░░██║██╔══██╗██║╚████║
  ██║░░░░░╚██████╔╝██║░╚███║╚█████╔╝░  ░╚██╔╝░╚██╔╝░██╔╝░░░ . ██║░░██║███████╗░░░██║░░░╚██████╔╝██║░░██║██║░╚███║
  ╚═╝░░░░░░╚═════╝░╚═╝░░╚══╝░╚════╝░░  ░░╚═╝░░░╚═╝░░╚═╝░░░░ . ╚═╝░░╚═╝╚══════╝░░░╚═╝░░░░╚═════╝░╚═╝░░╚═╝╚═╝░░╚══╝
*/

/*
  ▄▀█ █░░ █▀▀ █▀█ █▀█ █ ▀█▀ █░█ █▀▄▀█ █ █▀▀   █▄▄ █░█ █▀█ █▄░█
  █▀█ █▄▄ █▄█ █▄█ █▀▄ █ ░█░ █▀█ █░▀░█ █ █▄▄   █▄█ █▄█ █▀▄ █░▀█
  @brief: Burn antelopeCACHE methodically as a hedge against RAMCORE
  @return: burn amount as an antelopeCACHE asset
  @dev note: current settings are: no change in ramcore price = constant burn, increased ramcore price = no burn, decreased price = adjusted burn
*/

asset CONTRACT_CLASS_NAME::algorithmic_burn(){

  auto miningstats_delta = _miningstats.get();
  auto config_delta = _config.get();
  stats statstable( _self , CACHE_RAW  );  
  auto existing = statstable.find( CACHE_RAW ); 
  const auto& st = *existing;
  eosiosystem::rammarket exchange_state( NATIVE_SYSTEM_CONTRACT_NAME, NATIVE_SYSTEM_CONTRACT_NAME_VALUE );
  auto eosio_ramcore_itr = exchange_state.lower_bound( RAMCORE_RAW ); 
  auto constant_burn_rate = asset( config_delta.const_burn_rate.amount , CACHE_SYM);
  global_state2_singleton eosio_global_state2( NATIVE_SYSTEM_CONTRACT_NAME, NATIVE_SYSTEM_CONTRACT_NAME_VALUE );
  auto global2_state_or_default = eosio_global_state2.get_or_default();

  double burn_supply = st.burn_supply.amount;
  double ramcore_infl_multiplier = 0.0;
  double burn_supply_after_burn = 0.0;
  uint64_t burn_rate = 0;
  asset adjusted_burn = asset ( 0, CACHE_SYM );
  asset burn_return = asset ( 0, CACHE_SYM );

  // Ensure prev_ram_price is not 0 to avoid division by zero error
  if (miningstats_delta.prev_ram_price != 0) {
    burn_supply_after_burn = burn_supply * fabs( get_ram_price() / miningstats_delta.prev_ram_price ); 
  } else {
    burn_supply_after_burn = 0.0;
  }

  double burn_supply_quant_dif = burn_supply - burn_supply_after_burn;
  double new_ram_per_block = 0.0;

  // Ensure burn dif as a minimum 1.000000 otherwise if change is fractional then burn would just be constant
  if (burn_supply_quant_dif < 1.0) {
    burn_supply_quant_dif = 1.0;
  }

  // Determine ram inflation multiplier, burn rate & adjusted burn
  if ( global2_state_or_default.new_ram_per_block > 0) {
    ramcore_infl_multiplier = global2_state_or_default.new_ram_per_block / INITIAL_NATIVE_RAMCORE_NEW_BYTES_PER_BLOCK;
    burn_rate = std::abs(( floor( burn_supply_quant_dif ) + constant_burn_rate.amount ) * ramcore_infl_multiplier); // ensure burn rate is (+)
    adjusted_burn = asset ( burn_rate , CACHE_SYM);
  } else {
    ramcore_infl_multiplier = 1;
    burn_rate = std::abs(( floor( burn_supply_quant_dif ) + constant_burn_rate.amount )); // ensure burn rate is (+)
    adjusted_burn = asset ( burn_rate , CACHE_SYM);
  }

  // if everything has been burned then go to end of function
  if ( st.burn_supply == asset( 0 , CACHE_SYM) ) { 
    print ("🌅 \n"); 
    goto thereturn;
  }

  if ( eosio_ramcore_itr != exchange_state.end() ){ // if core ramcore table exists
    if ( miningstats_delta.prev_ram_price == get_ram_price() ){  // if no change in ram costs do the following 
      if ( st.burn_supply - constant_burn_rate > asset( 0 , CACHE_SYM)){  // Burn the constant burn rate if doing so will not cause an overdraft
        burn_return = constant_burn_rate;
        print( "🔥 RAMCORE price unchanged since last block. Burned at constant burn rate: " , burn_return.to_string(), " 🔥 \n");
      } else { // Burn the remaining burn balance if constant burn rate causes an overdraft
        burn_return = st.burn_supply;
        print( "🔥 RAMCORE price unchanged since last block. Burned remaining burn supply: " ,st.burn_supply , "... and a once beautiful fire has successfully been extinguished 🧯 \n");
      }
    } else if ( miningstats_delta.prev_ram_price < get_ram_price() ){ // if ram prices have increased since the last block,  then do the following 
        print( "❄️ RAMCORE price increased 📈 since last block. No Burn ❄️ \n" );
        burn_return =  asset( 0 , CACHE_SYM);
    } else { // if ram prices have decreased since the last block, then do the following
      if ( st.burn_supply - adjusted_burn > asset( 0 , CACHE_SYM)){   // if algorithmic burn does not cause and burn balance overdraft, then algorithmically burn
        burn_return = adjusted_burn;
        print( "🔥 RAMCORE price decreased 📉 since last block. Burned " , burn_return.to_string(), ". { Inflation burn multiplier: " , ramcore_infl_multiplier , " x,  Constant Burn Rate: ",constant_burn_rate, "} 🔥 \n");
      } else { // if algorithmic burn causes and overdraft in burn balance, burn all remaining supply
        burn_return = st.burn_supply; 
        print( "🔥 RAMCORE price decreased 📉 since last block. Burned remaining burn supply: " , st.burn_supply , "... and a once beautiful fire has successfully been extinguished 🧯 \n");
      }
    }
  } else { // if contract cannot locate the eosio:exchange state or global2 tables then do the following
    if ( st.burn_supply - constant_burn_rate > asset( 0 , CACHE_SYM)){  // Burn the constant rate if doing so does not cause an overdraft in burn balance 
      burn_return = constant_burn_rate; 
      print( "🔥 BP's changed rammarket ( exchanged_state ) dynamics. Burned at constant rate: " , constant_burn_rate, " 🔥 ");
    } else { // Burn the remaining burn balance if constant burn rate causes an overdraft
      burn_return = st.burn_supply;
      print( "🔥 BP's changed rammarket ( exchanged_state ) dynamics. Burned remaining burn supply: ", st.burn_supply ,"... and a once beautiful fire has successfully been extinguished 🧯 \n");
    }
  }
  thereturn:
  return burn_return;
}

/*
█▀▀ █▀▀ ▀█▀   █▀█ ▄▀█ █▀▄▀█   █▀█ █▀█ █ █▀▀ █▀▀
█▄█ ██▄ ░█░   █▀▄ █▀█ █░▀░█   █▀▀ █▀▄ █ █▄▄ ██▄
  @brief: Detects existence eosio 'rammarket' table
  @return: current ramcore price in core native token/byte || RETURN 0 IF EXCHANGE STATE DOES NOT EXIST
*/

double CONTRACT_CLASS_NAME::get_ram_price() {

  eosiosystem::rammarket exchange_state( NATIVE_SYSTEM_CONTRACT_NAME, NATIVE_SYSTEM_CONTRACT_NAME_VALUE );
  auto eosio_ramcore_itr = exchange_state.lower_bound( RAMCORE_RAW );

  if ( eosio_ramcore_itr != exchange_state.end() ) {
    double ram_reserve = eosio_ramcore_itr -> base.balance.amount;
    double core_reserve = eosio_ramcore_itr -> quote.balance.amount;
    double current_ram_price =  (core_reserve / ram_reserve ) / PRECISIONFACTOR; 
   return current_ram_price;
  } else {
    return 0; 
  } 

}




