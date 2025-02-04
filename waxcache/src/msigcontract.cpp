/*
  ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
  ██╗░░  ███╗░░░███╗░██████╗██╗░██████╗░    ░█████╗░░█████╗░████████╗██╗░█████╗░███╗░░██╗░██████╗
  ╚██╗░  ████╗░████║██╔════╝██║██╔════╝░    ██╔══██╗██╔══██╗╚══██╔══╝██║██╔══██╗████╗░██║██╔════╝
  ░╚██╗  ██╔████╔██║╚█████╗░██║██║░░██╗░    ███████║██║░░╚═╝░░░██║░░░██║██║░░██║██╔██╗██║╚█████╗░
  ░██╔╝  ██║╚██╔╝██║░╚═══██╗██║██║░░╚██╗    ██╔══██║██║░░██╗░░░██║░░░██║██║░░██║██║╚████║░╚═══██╗
  ██╔╝░  ██║░╚═╝░██║██████╔╝██║╚██████╔╝    ██║░░██║╚█████╔╝░░░██║░░░██║╚█████╔╝██║░╚███║██████╔╝
  ╚═╝░░  ╚═╝░░░░░╚═╝╚═════╝░╚═╝░╚═════╝░    ╚═╝░░╚═╝░╚════╝░░░░╚═╝░░░╚═╝░╚════╝░╚═╝░░╚══╝╚═════╝░
  ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
  @brief: msig actions/functions 
*/

#include <cache.hpp>

/*
  █▀█ █▀█ █▀█ █▀█ █▀█ █▀ █▀▀
  █▀▀ █▀▄ █▄█ █▀▀ █▄█ ▄█ ██▄
  @brief: allows registered antelopeCACHE miner to link an existing eosio.msig proposal to modify contract
  @param proposer: registered miner 
  @param proposal_name: eosio.msig name as recorded on eosio.msig table
  @param memorandum: statement backing up proposal
*/

[[eosio::action]] void CONTRACT_CLASS_NAME::proposemsig(name proposer, name proposal_name, string memorandum ) { 

  require_auth ( proposer );
  miningstats_check("miningstats_verification");

  auto miningstats_delta = _miningstats.get();
  auto config_delta = _config.get();
  miner_table _miner( get_self(), get_self().value );
  auto miner_itr = _miner.find( proposer.value );
  proposals_table _proposals(get_self(), get_self().value);
  auto prop_itr = _proposals.find(NOW);

  auto proposer_propname_KEY = proposals::_by_account_propname( proposer, proposal_name );
  auto proposer_propname_IDX = _proposals.get_index<"bychecksum"_n>();
  auto proposer_propname_ITR = proposer_propname_IDX.find( proposer_propname_KEY );
  
  check ( proposer != miningstats_delta.genesis_miner, "⚠️ Genesis miner carries no proposal privileges ⚠️" + ERROR_LOCATION );
  check ( prop_itr == _proposals.end(), "⚠️ Duplicate Proposal Time, attempt proposal shortly️ ⚠️" + ERROR_LOCATION );
  check ( proposer_propname_ITR == proposer_propname_IDX.end(), "⚠️ Duplicate proposal: " + std::to_string( proposer_propname_ITR -> prop_timestamp ) + " on 'proposals' table  ⚠️" + ERROR_LOCATION );
  check ( miner_itr != _miner.end() , "⚠️ Proposer must be a registered miner ⚠️" + ERROR_LOCATION );
  check ( memorandum.size() <= MAX_PROP_MEMO_SIZE, "⚠️ memorandum is limited to " + std::to_string(MAX_PROP_MEMO_SIZE) + " characters ⚠️" + ERROR_LOCATION );

  proposalchecker( proposer, proposal_name ); 
  
  _proposals.emplace(proposer, [&](auto& prp) { // ram bill
    prp.prop_timestamp = NOW; 
    prp.proposer = proposer;
    prp.eosio_msig_name = proposal_name;
    prp.sha256hash = hashofproposal(proposer,proposal_name);
    prp.memorandum = memorandum;
  }); 

  _miner.modify(miner_itr, same_payer, [&](auto&mnr) { // ram bill
    mnr.submitted_props += 1;
  });

  miningstats_delta.active_props += 1;

  _miningstats.set(miningstats_delta, _self);

  print("📃 ", NATIVE_MSIG_CONTRACT_NAME_STRING ,": '", proposal_name, "' by ", proposer, " successfully linked to ", _self.to_string(), " with proposal timestamp ",NOW_STRING, " and up for review by its miners  📃 \n"  );
  
}

/* 
  █░█ █▀█ ▀█▀ █▀▀ █▀█ █▀█ █▀█ █▀█
  ▀▄▀ █▄█ ░█░ ██▄ █▀▀ █▀▄ █▄█ █▀▀
  @brief: miner can vote for an existing valid proposal
  @param miner: registered miner
  @param proposal: primary key on proposals table ( aka time eosio.msig was linked/added to antelopeCACHE `proposals` table )
*/

[[eosio::action]] void CONTRACT_CLASS_NAME::voteprop( name miner, uint64_t prop_timestamp ) { 

  miningstats_check("miningstats_verification");

  auto miningstats_delta = _miningstats.get();
  miner_table _miner( get_self(), get_self().value );
  auto miner_itr = _miner.find( miner.value );
  proposals_table _proposals(get_self(), get_self().value);
  auto prop_itr = _proposals.find(prop_timestamp);

  check ( miner != miningstats_delta.genesis_miner && miningstats_delta.genesis_bytes != 0 , "⚠️ Genesis Miner has no voting privileges ⚠️" + ERROR_LOCATION );
  check ( miner_itr -> bytes > 0, "⚠️ Insufficient mining balance to vote ⚠️ " + ERROR_LOCATION );

  if ( prop_timestamp != 0 ) {
    check ( prop_itr != _proposals.end(), "⚠️ Proposal not found ⚠️" + ERROR_LOCATION );
    check ( prop_timestamp != miner_itr -> prop_vote, "⚠️ Already voted for " + std::to_string(prop_timestamp) + " ⚠️" + ERROR_LOCATION );
  } else {
    check ( prop_timestamp != miner_itr -> prop_vote, "⚠️ Miner currently has not vote ⚠️" + ERROR_LOCATION );
  }

  // validate existing miner vote
  if ( miner_itr -> prop_vote > 0 ) {
    auto [existing_proposal_vote_status, existing_proposal_vote_message] = proposal_table_iterator(miner_itr -> prop_vote);
    if ( existing_proposal_vote_status == "not_legit" ) {
        remove_miner_proposal_vote(miner);
        delete_proposal_record_if_exist( miner_itr -> prop_vote );
    } else {
        decrease_proposal_record_vote_if_exist(miner, miner_itr -> prop_vote, miner_itr -> bytes);
    }
  } 
  // validate new miner vote
  if ( prop_timestamp != 0 ) {
    auto [new_proposal_vote_status, new_proposal_vote_message] = proposal_table_iterator(prop_timestamp);
    if ( new_proposal_vote_status == "not_legit" ) {
      check ( false, "⚠️ " + new_proposal_vote_message + " ⚠️" + ERROR_LOCATION );
    } else {  
      add_miner_proposal_vote( miner, prop_timestamp );
      increase_proposal_record_vote( miner,prop_timestamp, miner_itr -> bytes );
      print ( "☑️ Successfully voted for ", ANTELOPE_CONTRACT_NAME_STRING," proposal:", prop_timestamp ," ☑️ \n");
    }
  } else {
     remove_miner_proposal_vote(miner);
     print ( "☑️ Successfully cleaned your voting record  ☑️ \n");
  }
  
}

/*
  █▀█ █▀█ █▀█ █▀█ █▀█ █▀▀ █░█ █ █▀▀ █░█░█
  █▀▀ █▀▄ █▄█ █▀▀ █▀▄ ██▄ ▀▄▀ █ ██▄ ▀▄▀▄▀
  @brief: determine authenticity of an msig proposal and execute, approve, erase if appropriate
  @param initiator: account reviewing proposal
  @param proposal: prop_timestamp
*/

[[eosio::action]] void CONTRACT_CLASS_NAME::propreview( name initiator, uint64_t proposal  ) { 

  auto miningstats_delta = _miningstats.get();
  auto config_delta = _config.get();
  proposals_table _proposals(get_self(), get_self().value);
  auto prop_itr = _proposals.find(proposal);
  miner_table _miner( get_self(), get_self().value );
  auto miner_itr = _miner.find( prop_itr -> proposer.value );
  auto initiator_itr = _miner.find ( initiator.value );
  uint64_t precautionary_flash_loan_protection_interval_last_vt = initiator_itr -> last_prop_vt_time + REQ_PROP_VOTE_AND_BYTE_ADJUSTMENT_TIME_INTERVAL; 
  uint64_t precautionary_flash_loan_protection_interval_last_byte_adj = initiator_itr -> last_byte_adj_time + REQ_PROP_VOTE_AND_BYTE_ADJUSTMENT_TIME_INTERVAL; 

  check ( prop_itr != _proposals.end(), "⚠️ Proposal not found ⚠️" + ERROR_LOCATION );
  check ( initiator_itr != _miner.end(), "⚠️ Initiator must be a miner ⚠️" + ERROR_LOCATION );
  check ( NOW > precautionary_flash_loan_protection_interval_last_vt, "⚠️ " + initiator.to_string()  + "'s last_prop_vt_time must be a greater than " + std::to_string(precautionary_flash_loan_protection_interval_last_vt) + "  ⚠️" + ERROR_LOCATION );
  check ( NOW > precautionary_flash_loan_protection_interval_last_byte_adj, "⚠️ " + initiator.to_string()  + "'s last_byte_adj_time must be a greater than " + std::to_string(precautionary_flash_loan_protection_interval_last_byte_adj) + "  ⚠️" + ERROR_LOCATION );

  auto [existing_proposal_vote_status, existing_proposal_vote_message] = proposal_table_iterator(proposal);
  if ( existing_proposal_vote_status == "not_legit" ) {
      delete_proposal_record_if_exist( prop_itr -> prop_timestamp );
      print ( ":removed proposal record \n" );
  } else if ( existing_proposal_vote_status == "legit" ) {
    uint64_t proposal_time_remaining = ( prop_itr -> prop_timestamp + config_delta.proposition_period ) - NOW;
    double cache_max_votes = miningstats_delta.total_bytes - miningstats_delta.genesis_bytes;
    double proposal_vote_pct = ( prop_itr -> votes / cache_max_votes ) * 100;
    double current_required_vote_pct = required_vote_percentage_by_decay() * 100;
    check ( false, "⚠️ proposal currently carries " + std::to_string(proposal_vote_pct) + "% of the total mining vote and " + std::to_string(prop_itr -> leading_blocks) + " leading blocks. However executable proposals require at least " + std::to_string(current_required_vote_pct) + "% of votes at time of 'propreview' including at least " + std::to_string( config_delta.min_leading_blocks ) + " successful leading blocks for approval/execution. Proposal expires " + std::to_string(proposal_time_remaining) + " seconds ⚠️" + ERROR_LOCATION ); 
  } else if ( existing_proposal_vote_status == "unrealized" ) {
    _miner.modify( miner_itr , same_payer , [&]( auto& mach ){ // ram bill
      mach.successful_props += 1;
    });
    miningstats_delta.successful_props += 1;
    miningstats_delta.active_props -= 1;
     _miningstats.set(miningstats_delta, _self);
    print ( "🎉 📜 MSIG '", prop_itr -> eosio_msig_name ,"' successfully approved/executed and set into effect 📜 🎉 \n" );
    msigapprove ( prop_itr -> proposer, prop_itr -> eosio_msig_name ); 
    msigexecute ( prop_itr -> proposer, prop_itr -> eosio_msig_name ); 
    _proposals.erase( prop_itr );
  }
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
  █▀▄▀█ █▀ █ █▀▀ ▄▀█ █▀█ █▀█ █▀█ █▀█ █░█ █▀▀
  █░▀░█ ▄█ █ █▄█ █▀█ █▀▀ █▀▀ █▀▄ █▄█ ▀▄▀ ██▄
  @brief: approves msig transaction on eosio.msig contract ( does not execute msig ) 
*/

void CONTRACT_CLASS_NAME::msigapprove(name proposer, name propname ) {
  eosio::action(ANTELOPE_CONTRACT_ACTIVE_PERMISSION, 
  NATIVE_MSIG_CONTRACT_NAME,
  eosio::name("approve"), 
  std::tuple(
   proposer, 
   propname, 
   ANTELOPE_CONTRACT_ACTIVE_PERMISSION) ) 
   .send();
}

/*
  █▀▄▀█ █▀ █ █▀▀ █▀▀ ▀▄▀ █▀▀ █▀▀ █░█ ▀█▀ █▀▀
  █░▀░█ ▄█ █ █▄█ ██▄ █░█ ██▄ █▄▄ █▄█ ░█░ ██▄
  @brief: executes msig transaction on eosio.msig contract ( does not approve msig ) 
*/

void CONTRACT_CLASS_NAME::msigexecute(name proposer, name propname ) {
  eosio::action(ANTELOPE_CONTRACT_ACTIVE_PERMISSION, 
  NATIVE_MSIG_CONTRACT_NAME,
  eosio::name("exec"), 
  std::tuple(
   proposer, 
   propname, 
   _self) )
   .send();
}

/*
  █▀█ █▀█ █▀█ █▀█ █▀█ █▀ ▄▀█ █░░ █▀▀ █░█ █▀▀ █▀▀ █▄▀ █▀▀ █▀█
  █▀▀ █▀▄ █▄█ █▀▀ █▄█ ▄█ █▀█ █▄▄ █▄▄ █▀█ ██▄ █▄▄ █░█ ██▄ █▀▄
  @brief: utilized for linking existing eosio.msig proposals to antelopeCACHE 
*/

void CONTRACT_CLASS_NAME::proposalchecker( name msig_proposer, name proposal_name ) {

  auto config_delta = _config.get();
  proposals_table _proposals(get_self(), get_self().value);
  auto prop_itr = _proposals.find(NOW);

  // eosio.msig proposal && approvals2 tables
  multisig::proposals proptable( NATIVE_MSIG_CONTRACT_NAME , msig_proposer.value );
  multisig::approvals approvals2table( NATIVE_MSIG_CONTRACT_NAME, msig_proposer.value );

  // proposal & appprovals2 table iterators
  auto& prop = proptable.get(proposal_name.value, (std::string("⚠️ " + NATIVE_MSIG_CONTRACT_NAME_STRING + " proposal not found, original '" + NATIVE_MSIG_CONTRACT_NAME_STRING + "' proposer must be the same proposer on " + ANTELOPE_CONTRACT_NAME_STRING + " ⚠️") + ERROR_LOCATION).c_str());
  auto& prop_approvals2 = approvals2table.get(proposal_name.value, (std::string("⚠️ Proposal not found the on " + NATIVE_MSIG_CONTRACT_NAME_STRING + " approvals table  ⚠️") + ERROR_LOCATION).c_str());

  // transaction flags/counters
  uint64_t action_ctr = 0;
  bool abi_flag = 0;
  bool code_flag = 0;

  // unpack msig transaction variables ( no counters ) 
  auto msig_expiration = eosio::unpack<transaction_header>( prop.packed_transaction ).expiration; 
  auto msig_action_array = eosio::unpack<transaction>( prop.packed_transaction ).actions; 
  auto num_of_msig_actions = std::size(msig_action_array); 

  // requested 'approvals' ( aka requested permissions ), can only request/sign '_self' with 'active' as only the active permission will transfer to the new 'provided approvals column' ( requesting eosio.code will not work ( duh )) 
  auto key_requested_itr = prop_approvals2.requested_approvals;
  auto key_provided_itr = prop_approvals2.provided_approvals;
  auto num_of_approvals_requested = std::size( key_requested_itr );
  auto num_of_approvals_provided = std::size ( key_provided_itr );

  uint64_t proper_expiration = NOW + config_delta.proposition_period; 
  uint64_t expired_x_seconds_ago = NOW - msig_expiration.sec_since_epoch();
  check ( NOW_TIME_POINT + config_delta.proposition_period < msig_expiration, "⚠️ Proposed MSIG transaction expiration time too short, must be at greater than: " + std::to_string( proper_expiration ) +  "  ⚠️" + ERROR_LOCATION );
  check ( num_of_approvals_provided == 0, "⚠️ ️MSIG proposition should not carry an already provided approval ⚠️" + ERROR_LOCATION ); 
  check ( NOW_TIME_POINT < msig_expiration, "⚠️ MSIG transaction proposition expired " + std::to_string( expired_x_seconds_ago) + "seconds ago ⚠️" + ERROR_LOCATION );
  check ( num_of_msig_actions == EXPECTED_NUM_OF_MSIG_ACTIONS, "⚠️ Proposals require '" + std::to_string(EXPECTED_NUM_OF_MSIG_ACTIONS) + "' unique packed " + NATIVE_MSIG_CONTRACT_NAME_STRING + " transactions: 'setabi' & 'setcode' in any order ⚠️" + ERROR_LOCATION ); 
  check ( num_of_approvals_requested == 1 && key_requested_itr[0].level.actor == _self && key_requested_itr[0].level.permission == "active"_n, "⚠️ MSIG proposal requested approvals limited to 'actor': '" + _self.to_string() + "' 'permission' : 'active' ⚠️" + ERROR_LOCATION );

  // eosio.msig packed transaction checks: contract, action type, authorizations
  while ( action_ctr != num_of_msig_actions ) {
     // unpack action data within the msig transaction
    auto contract_name_for_action = eosio::unpack<transaction>( prop.packed_transaction ).actions[action_ctr].account;
    auto action_name = eosio::unpack<transaction>( prop.packed_transaction ).actions[action_ctr].name;
    auto action_auth_array = eosio::unpack<transaction>( prop.packed_transaction ).actions[action_ctr].authorization;
    auto num_of_authorizations = std::size(action_auth_array); 
    auto action_authorization = eosio::unpack<transaction>( prop.packed_transaction ).actions[action_ctr].authorization[0];
    auto msig_action_array_for_loop = eosio::unpack<transaction>( prop.packed_transaction ).actions; 
    auto abi_data_account = eosio::unpack<setabi>(msig_action_array_for_loop[action_ctr].data).account; 
    auto code_data_account = eosio::unpack<setcode>(msig_action_array_for_loop[action_ctr].data).account;  

    check ( contract_name_for_action == NATIVE_SYSTEM_CONTRACT_NAME, "⚠️️ ️Incorrect contract name '" + contract_name_for_action.to_string() + "', MSIG action contract must be '" + NATIVE_SYSTEM_CONTRACT_NAME_STRING + "' ⚠️" + ERROR_LOCATION );  
    check ( num_of_authorizations == 1 && action_authorization == ANTELOPE_CONTRACT_ACTIVE_PERMISSION, "⚠️ Only one '" + NATIVE_MSIG_CONTRACT_NAME_STRING + "' authorization allowed: " + _self.to_string() + " with 'active' permission  ⚠️" + ERROR_LOCATION );

    // abi & code flag checker                     
    if ( action_name == "setabi"_n ) {
      check ( abi_flag != 1, "⚠️ Cannot have duplicate 'setabi' actions  ⚠️" + ERROR_LOCATION );
      check ( abi_data_account == _self, "⚠️ MSIG " + NATIVE_SYSTEM_CONTRACT_NAME_STRING + "::setabi 'account' parameter currently read as: '" + abi_data_account.to_string() + "' must be '" + _self.to_string() +  "' ⚠️" + ERROR_LOCATION ); 
      abi_flag = 1;
    } else if ( action_name == "setcode"_n ) {
      check ( code_flag != 1, "⚠️ Cannot have duplicate 'setcode' actions  ⚠️" + ERROR_LOCATION );
      check ( code_data_account == _self, "⚠️ MSIG " + NATIVE_SYSTEM_CONTRACT_NAME_STRING + "::setcode 'account' parameter currently read as: '" + code_data_account.to_string() + "' must be '" + _self.to_string() +  "' ⚠️" + ERROR_LOCATION ); 
      code_flag = 1;
    } else {
      check ( false, "⚠️ Incorrect MSIG trx action '" + action_name.to_string() + "' must be both 'setabi' or 'setcode' from '" + NATIVE_SYSTEM_CONTRACT_NAME_STRING + "' contract in any order  ⚠️" + ERROR_LOCATION );
    };   
    action_ctr += 1;
  }

}

/*
█ █▄░█ █▀▀ █▀█ █▀▀ ▄▀█ █▀ █▀▀   █▀█ █▀█ █▀█ █▀█ █▀█ █▀ ▄▀█ █░░   █▀█ █▀▀ █▀▀ █▀█ █▀█ █▀▄   █░█ █▀█ ▀█▀ █▀▀
█ █░▀█ █▄▄ █▀▄ ██▄ █▀█ ▄█ ██▄   █▀▀ █▀▄ █▄█ █▀▀ █▄█ ▄█ █▀█ █▄▄   █▀▄ ██▄ █▄▄ █▄█ █▀▄ █▄▀   ▀▄▀ █▄█ ░█░ ██▄
@brief: adjust '+' proposal record
*/

void CONTRACT_CLASS_NAME::increase_proposal_record_vote(name miner, uint64_t proposal, uint64_t adjustment ) { 

  proposals_table _proposals(get_self(), get_self().value);
  auto prop_itr = _proposals.find(proposal);

  _proposals.modify( prop_itr, same_payer, [&](auto& prp) { // ram bill
    prp.votes += adjustment;
  }); 
}

/*
█▀▄ █▀▀ █▀▀ █▀█ █▀▀ ▄▀█ █▀ █▀▀   █▀█ █▀█ █▀█ █▀█ █▀█ █▀ ▄▀█ █░░   █▀█ █▀▀ █▀▀ █▀█ █▀█ █▀▄   █░█ █▀█ ▀█▀ █▀▀   █ █▀▀  █▀▀ ▀▄▀ █ █▀ ▀█▀
█▄▀ ██▄ █▄▄ █▀▄ ██▄ █▀█ ▄█ ██▄   █▀▀ █▀▄ █▄█ █▀▀ █▄█ ▄█ █▀█ █▄▄   █▀▄ ██▄ █▄▄ █▄█ █▀▄ █▄▀   ▀▄▀ █▄█ ░█░ ██▄   █ █▀░  ██▄ █░█ █ ▄█ ░█░
@brief: adjust '-' proposal record
*/

void CONTRACT_CLASS_NAME::decrease_proposal_record_vote_if_exist(name miner, uint64_t proposal, uint64_t adjustment ) { 

  proposals_table _proposals(get_self(), get_self().value);
  auto prop_itr = _proposals.find(proposal);

  if ( prop_itr != _proposals.end() ){
    _proposals.modify( prop_itr, same_payer, [&](auto& prp) { // ram bill 
      prp.votes -= adjustment;
    }); 
  } // else do nothing as proposal no longer exists
}

/*
▄▀█ █▀▄ █▀▄   █▀▄▀█ █ █▄░█ █▀▀ █▀█   █▀█ █▀█ █▀█ █▀█ █▀█ █▀ ▄▀█ █░░   █░█ █▀█ ▀█▀ █▀▀
█▀█ █▄▀ █▄▀   █░▀░█ █ █░▀█ ██▄ █▀▄   █▀▀ █▀▄ █▄█ █▀▀ █▄█ ▄█ █▀█ █▄▄   ▀▄▀ █▄█ ░█░ ██▄
@brief: change miners existing proposal 
*/

void CONTRACT_CLASS_NAME::add_miner_proposal_vote ( name miner, uint64_t proposal ){

    miner_table _miner( get_self(), get_self().value );
    auto miner_itr = _miner.find( miner.value );

  _miner.modify( miner_itr , same_payer , [&]( auto& mach ){ // ram bill
    mach.prop_vote = proposal;
    mach.last_prop_vt_time = NOW;
  });
}

/*
█▀█ █▀▀ █▀▄▀█ █▀█ █░█ █▀▀   █▀▄▀█ █ █▄░█ █▀▀ █▀█   █▀█ █▀█ █▀█ █▀█ █▀█ █▀ ▄▀█ █░░   █░█ █▀█ ▀█▀ █▀▀
█▀▄ ██▄ █░▀░█ █▄█ ▀▄▀ ██▄   █░▀░█ █ █░▀█ ██▄ █▀▄   █▀▀ █▀▄ █▄█ █▀▀ █▄█ ▄█ █▀█ █▄▄   ▀▄▀ █▄█ ░█░ ██▄
@brief: remove miners existing proposal 
*/

void CONTRACT_CLASS_NAME::remove_miner_proposal_vote (name miner ){

  miner_table _miner( get_self(), get_self().value );
  auto miner_itr = _miner.find( miner.value );

  _miner.modify( miner_itr , same_payer , [&]( auto& mach ){ // ram bill
    mach.prop_vote = 0;
    mach.last_prop_vt_time = NOW;
  });
}

/*
█▀▄ █▀▀ █░░ █▀▀ ▀█▀ █▀▀   █▀█ █▀█ █▀█ █▀█ █▀█ █▀ ▄▀█ █░░   █▀█ █▀▀ █▀▀ █▀█ █▀█ █▀▄   █ █▀▀   █▀▀ ▀▄▀ █ █▀ ▀█▀
█▄▀ ██▄ █▄▄ ██▄ ░█░ ██▄   █▀▀ █▀▄ █▄█ █▀▀ █▄█ ▄█ █▀█ █▄▄   █▀▄ ██▄ █▄▄ █▄█ █▀▄ █▄▀   █ █▀░   ██▄ █░█ █ ▄█ ░█░
@brief: erase antelopeCACHE msig
*/
void CONTRACT_CLASS_NAME::delete_proposal_record_if_exist ( uint64_t proposal ) {

  auto miningstats_delta = _miningstats.get();
  proposals_table _proposals(get_self(), get_self().value);
  auto prop_itr = _proposals.find(proposal);

  if ( prop_itr != _proposals.end() ) {
    _proposals.erase( prop_itr );
    miningstats_delta.active_props -= 1;
    _miningstats.set(miningstats_delta, _self);
  } // elso do nothing as proposal no longer exists
}

/*
  ███████╗██╗░░░██╗███╗░░██╗░█████╗░░  ██╗░░░░░░░██╗░░░░██╗  ██████╗░███████╗████████╗██╗░░░██╗██████╗░███╗░░██╗
  ██╔════╝██║░░░██║████╗░██║██╔══██╗░  ██║░░██╗░░██║░░░██╔╝  ██╔══██╗██╔════╝╚══██╔══╝██║░░░██║██╔══██╗████╗░██║
  █████╗░░██║░░░██║██╔██╗██║██║░░╚═╝░  ╚██╗████╗██╔╝░░██╔╝░  ██████╔╝█████╗░░░░░██║░░░██║░░░██║██████╔╝██╔██╗██║
  ██╔══╝░░██║░░░██║██║╚████║██║░░██╗░  ░████╔═████║░░██╔╝░░  ██╔══██╗██╔══╝░░░░░██║░░░██║░░░██║██╔══██╗██║╚████║
  ██║░░░░░╚██████╔╝██║░╚███║╚█████╔╝░  ░╚██╔╝░╚██╔╝░██╔╝░░░  ██║░░██║███████╗░░░██║░░░╚██████╔╝██║░░██║██║░╚███║
  ╚═╝░░░░░░╚═════╝░╚═╝░░╚══╝░╚════╝░░  ░░╚═╝░░░╚═╝░░╚═╝░░░░  ╚═╝░░╚═╝╚══════╝░░░╚═╝░░░░╚═════╝░╚═╝░░╚═╝╚═╝░░╚══╝
*/

/*
█▀█ █▀█ █▀█ █▀█ █▀█ █▀ ▄▀█ █░░   ▀█▀ ▄▀█ █▄▄ █░░ █▀▀   █ ▀█▀ █▀▀ █▀█ ▄▀█ ▀█▀ █▀█ █▀█
█▀▀ █▀▄ █▄█ █▀▀ █▄█ ▄█ █▀█ █▄▄   ░█░ █▀█ █▄█ █▄▄ ██▄   █ ░█░ ██▄ █▀▄ █▀█ ░█░ █▄█ █▀▄
  @brief: determines if previously voted proposal is still expired or deletable;
  @return: returns '0' if vote is deletable || '1' if not deletable.
  @note: does not actually delete or modify records, only prints relevant actions to console and returns 1 || 0
*/

std::pair<string, string> CONTRACT_CLASS_NAME::proposal_table_iterator( uint64_t proposal ) {

  auto config_delta = _config.get();
  auto miningstats_delta = _miningstats.get();

  proposals_table _proposals(get_self(), get_self().value);
  auto cache_prop_itr = _proposals.find(proposal);

  multisig::proposals proptable(NATIVE_MSIG_CONTRACT_NAME, cache_prop_itr -> proposer.value);
  auto eosio_msig_prop_itr = proptable.find(cache_prop_itr -> eosio_msig_name.value);

  double cache_max_votes = miningstats_delta.total_bytes - miningstats_delta.genesis_bytes;
  double proposal_vote_pct = cache_prop_itr -> votes / cache_max_votes;

  // all proposals are treated as legit until proven otherwise 
  string generated_message = "";
  string proposal_status = "legit";

  // validate proposals existence on _self `propsal` table
  if (cache_prop_itr == _proposals.end()) {
    print("❗Alert: ", proposal, " no longer exists on antelopcache❗ \n");
    proposal_status = "not_legit";
    generated_message = "❗Alert: " +  std::to_string( proposal ) + " no longer exists on antelopcache  ❗ \n";
    goto endoffunction;
  } else {
    // validate proposal is not expired on _self  `proposal` table
    if (cache_prop_itr -> prop_timestamp + config_delta.proposition_period <= NOW) {
      print("❗Alert: ", cache_prop_itr -> prop_timestamp, " has expired on antelopcache❗ \n");
      proposal_status = "not_legit";
      generated_message = "❗Alert: " + std::to_string(cache_prop_itr -> prop_timestamp) + " has expired on antelopcache❗ \n";
      goto endoffunction;
    } else {
      // validate proposal has sufficient votes
      if (proposal_vote_pct > required_vote_percentage_by_decay() && cache_prop_itr -> leading_blocks >= config_delta.min_leading_blocks ) {
        print("🪧 Notice: ", cache_prop_itr -> prop_timestamp, " currently has enough votes and has enough leading blocks to be approved & executed 🪧 \n" );
        proposal_status = "unrealized";
      }
    }
  }
  // validate proposal existence on eosio.msig 'proposals' table 
  if (eosio_msig_prop_itr == proptable.end()) {
    print("❗Alert: ", cache_prop_itr -> prop_timestamp, " no longer exists on ", NATIVE_MSIG_CONTRACT_NAME_STRING, "❗ \n");
    proposal_status = "not_legit";
    generated_message = "❗Alert: " + std::to_string(cache_prop_itr -> prop_timestamp) + " no longer exists on " + NATIVE_MSIG_CONTRACT_NAME_STRING + "❗ \n";
    goto endoffunction;
  } else {
    // validate proposal hasn't changed on eosio.msig proposals table vs _self via SHA256 ( miner + pckd trx )
    if (cache_prop_itr -> sha256hash != hashofproposal(cache_prop_itr -> proposer, cache_prop_itr -> eosio_msig_name)) {
      print("(ง'̀-'́)ง ❗Alert: A change to proposal: " , cache_prop_itr -> prop_timestamp,  " was detected via SHA256 comparison on antelopcache and " , NATIVE_MSIG_CONTRACT_NAME_STRING , " (ง'̀-'́)ง ❗ \n");
      proposal_status = "not_legit";
      generated_message = "(ง'̀-'́)ง ❗Alert: A change to proposal: " + std::to_string( cache_prop_itr -> prop_timestamp) +  " was detected via SHA256 comparison on antelopecache and " + NATIVE_MSIG_CONTRACT_NAME_STRING +  " (ง'̀-'́)ง ❗ \n";
      goto endoffunction;
    }
  }
  endoffunction:
  return std::make_pair(proposal_status, generated_message );

}

/*
█▀█ █▀▀ █▀█ █░█ █ █▀█ █▀▀ █▀▄   █░█ █▀█ ▀█▀ █▀▀   █▀█ █▀▀ █▀█ █▀▀ █▀▀ █▄░█ ▀█▀ ▄▀█ █▀▀ █▀▀   █▄▄ █▄█   █▀▄ █▀▀ █▀▀ ▄▀█ █▄█
█▀▄ ██▄ ▀▀█ █▄█ █ █▀▄ ██▄ █▄▀   ▀▄▀ █▄█ ░█░ ██▄   █▀▀ ██▄ █▀▄ █▄▄ ██▄ █░▀█ ░█░ █▀█ █▄█ ██▄   █▄█ ░█░   █▄▀ ██▄ █▄▄ █▀█ ░█░
@brief: returns required % of votes for proposal approval/consensus
*/

double CONTRACT_CLASS_NAME::required_vote_percentage_by_decay() {

  auto config_delta = _config.get();

  check(config_delta.prop_vote_decay_start < NOW, "⚠️ Start time must be in the past ⚠️" + ERROR_LOCATION );
  
  // Ensure we cap the elapsed time to a maximum of REQ_VOTE_PCT_DECAY_PERIOD
  uint64_t elapsed_time = std::min(NOW - config_delta.prop_vote_decay_start, REQ_VOTE_PCT_DECAY_PERIOD);
  double percentageDecay = config_delta.initial_vote_pct_req - config_delta.ending_vote_pct_req; // Total decay over the period
  double decayPerSecond = percentageDecay / REQ_VOTE_PCT_DECAY_PERIOD; // Decay rate per second
  
  // Calculate current percentage
  double currentPercentage = config_delta.initial_vote_pct_req - (decayPerSecond * elapsed_time);

  return currentPercentage;

}

/*
  █░█ ▄▀█ █▀ █░█ █▀█ █▀▀ █▀█ █▀█ █▀█ █▀█ █▀█ █▀ ▄▀█ █░░
  █▀█ █▀█ ▄█ █▀█ █▄█ █▀░ █▀▀ █▀▄ █▄█ █▀▀ █▄█ ▄█ █▀█ █▄▄
  @brief: generates sha256 hash of eosio.msig proposal 
  @param msig_proposer: eosio.msig proposer
  @param proposal_name: eosio.msig proposal name
*/

checksum256 CONTRACT_CLASS_NAME::hashofproposal (name msig_proposer, name proposal_name ) {

  multisig::proposals proptable( NATIVE_MSIG_CONTRACT_NAME, msig_proposer.value );
  auto& prop = proptable.get(proposal_name.value, (std::string("⚠️ " + NATIVE_SYSTEM_CONTRACT_NAME_STRING + " proposal not found, original " + NATIVE_SYSTEM_CONTRACT_NAME_STRING + " proposer must be the same proposer on ") + _self.to_string() + " ⚠️" + ERROR_LOCATION).c_str());
  checksum256 hash_result;
  return eosio::sha256(reinterpret_cast<const char*>(prop.packed_transaction.data()), prop.packed_transaction.size());
  
}




