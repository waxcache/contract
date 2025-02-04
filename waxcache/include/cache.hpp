/*
░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
  ░█████╗░░█████╗░░█████╗░██╗░░██╗███████╗
  ██╔══██╗██╔══██╗██╔══██╗██║░░██║██╔════╝
  ██║░░╚═╝███████║██║░░╚═╝███████║█████╗░░
  ██║░░██╗██╔══██║██║░░██╗██╔══██║██╔══╝░░
  ╚█████╔╝██║░░██║╚█████╔╝██║░░██║███████╗
  ░╚════╝░╚═╝░░╚═╝░╚════╝░╚═╝░░╚═╝╚══════╝  ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
        ██████╗░███████╗███████╗██╗███╗░░██╗██╗████████╗██╗░█████╗░███╗░░██╗░██████╗
        ██╔══██╗██╔════╝██╔════╝██║████╗░██║██║╚══██╔══╝██║██╔══██╗████╗░██║██╔════╝
        ██║░░██║█████╗░░█████╗░░██║██╔██╗██║██║░░░██║░░░██║██║░░██║██╔██╗██║╚█████╗░
        ██║░░██║██╔══╝░░██╔══╝░░██║██║╚████║██║░░░██║░░░██║██║░░██║██║╚████║░╚═══██╗
        ██████╔╝███████╗██║░░░░░██║██║░╚███║██║░░░██║░░░██║╚█████╔╝██║░╚███║██████╔╝
        ╚═════╝░╚══════╝╚═╝░░░░░╚═╝╚═╝░░╚══╝╚═╝░░░╚═╝░░░╚═╝░╚════╝░╚═╝░░╚══╝╚═════╝░
        ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
*/

#pragma once
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/eosio.hpp>
#include <eosio/transaction.hpp>
#include <eosio.system.hpp>
#include <string>
#include <eosio/singleton.hpp>
#include <eosio/crypto.hpp>
#include <vector>
#include <algorithm>
#include <math.h>
#include <string_view>
#include <stdlib.h>
#include <stdio.h>
#include <cmath>
#include <eosio/print.hpp>
#include <cstdlib>
#include <string.h>
#include <eosio/ignore.hpp>
#include <eosio/action.hpp>
#include <eosio/permission.hpp>
#include <eosio/fixed_bytes.hpp> 
#include <eosio/dispatcher.hpp>
#include <config.hpp>
#include <eosio.bios.hpp>
#include <eosio.msig.hpp>
#include <eosio.token.hpp>
#include <exchange_state.hpp>
#include <global_state2.hpp>

using namespace std;
using namespace eosio;

CONTRACT CONTRACT_CLASS_NAME : public contract {

  public:
    using contract::contract;

    CONTRACT_CLASS_NAME(name receiver, name code, datastream<const char*> ds) : contract(receiver, code, ds), _miningstats(_self, _self.value), _config(_self, _self.value){} 

    [[eosio::action]] void  bios(name contractissuer);
    [[eosio::action]] void  enlist(name miner); 
    [[eosio::on_notify("eosio.token::transfer")]] void incoming_transfer( name from, name to, asset quantity, string memo); 
    [[eosio::action]] void  reducemining(name miner, uint64_t bytes);
    [[eosio::action]] void  claim(name miner, uint64_t max_claims ); 
    [[eosio::action]] void  redeem(name miner, name receiver, asset requested_cache);
    [[eosio::action]] void  mineblock( name initiator, string memo);      
    [[eosio::action]] void  votefee(name miner, asset tx_fee); 
    [[eosio::action]] void  voteprop( name miner, uint64_t prop_timestamp);
    [[eosio::action]] void  proposemsig(name proposer, name proposal_name, string memorandum);
    [[eosio::action]] void  propreview( name initiator, uint64_t proposal  ) ;
    [[eosio::action]] void  transfer(name from, name to, asset quantity, string memo );
    [[eosio::action]] void  open(name owner, const symbol& symbol, name ram_payer);
    [[eosio::action]] void  close(name owner, const symbol& symbol);
    [[eosio::action]] void  whitelist(const std::vector<name>& accounts );
    [[eosio::action]] void  devclaim( asset quantity );
    [[eosio::action]] void  blocklog(name initiator,
                                    uint64_t block,
                                    uint64_t block_time, 
                                    asset issued_supply, 
                                    asset collected_tx_fees, 
                                    asset burned_tokens, 
                                    asset total_supply, 
                                    asset burn_supply, 
                                    uint64_t abc_miners, 
                                    uint64_t bytes, 
                                    uint64_t txs,
                                    asset tx_fee,
                                    uint64_t active_proposals,
                                    uint64_t leading_proposal,
                                    double ram_price,
                                    string block_memo);

  private:

    struct setabi {
      name  account;
      byte  abi;
    };

    struct setcode {
      name    account;
      uint8_t vmtype;
      uint8_t vmversion;
      byte    code;
    };
 
    /*
      ▀█▀ ▄▀█ █▄▄ █░░ █▀▀ █▀
      ░█░ █▀█ █▄█ █▄▄ ██▄ ▄█
    */

    //@multi: CACHE balance's table
    TABLE account {
      asset    balance;

      uint64_t primary_key()const { return balance.symbol.code().raw(); }
    };

    //@multi: tracks current CACHE supply
    TABLE currency_stats {
      asset    supply;                // circulating supply 
      asset    max_supply;            // total max supply ( including supply designated for burning )
      asset    max_mine_supply;       // max mintable supply
      asset    mined_supply;          // supply that was mined
      uint128_t unredeemed_supply;    // previously claimed but unredeemed tokens as a scaled up integer ( for precision )
      asset    burn_supply;           // # of tokens designated only for burning
      uint128_t fee_vt_weight;        // total adjusted miningstats weight for transaction fees
      asset    current_tx_fee;        // current block transaction fee ( cannot be modified during current block ) 
      asset    upcoming_tx_fee;       // next block transaction fee ( can only be adjusted prior to block ) 
      uint64_t block_txs;             // # of block_txs in current block 
      asset    block_fees;            // total accumulated in current block from transaction fees
      name     issuer;                // token issuer ( _self )
      vector<name> whtlst_xchng_accts;// List of whitelisted accounts ( outgoing and incoming ) 

      uint64_t primary_key()const { return supply.symbol.code().raw(); }
    };

    //@multi: Tracks the inflationary token dynamics of CACHE
    TABLE inflation {
      uint64_t block;                 // block number
      uint64_t era;                   // inflationary era
      uint64_t time_reached;          // time 'x' block was reached
      asset    block_subsidy;         // total block reward amount at 'x' block 
      asset    added_supply;          // supply added during era
      asset    total_supply;          // total updated supply after era

      uint64_t primary_key()      const { return block; }
      uint64_t by_era()           const { return era; }
    };

    //@multi: temporarily stores mined blocks for miners to claim
    TABLE tokensubsidy {
      uint64_t block_time;            // time block was logged
      uint64_t block;                 // block #
      uint64_t total_bytes;           // # of mining bytes at subsidy
      uint64_t miner_count;           // # of engaged miners at subsidy
      uint64_t unclaimed_miners;      // # of engaged miners who have not claimed 'x' subsidy
      asset    reward;                // total block rewards including ( transaction fees/ block subsidies )

      uint64_t primary_key()     const { return block_time; }
    };
    
    //@single: antelopeCACHE mining business logic
    TABLE miningstats {
      uint64_t  current_era;           // current inflationary era 
      uint64_t  current_block;         // current block #
      uint64_t  total_bytes;           // total # of mining bytes 
      uint64_t  miner_count;           // total # of engaged miners ( with BYTES > 1 )
      uint64_t  last_subsidy;          // last subsidy time
      uint64_t  nxt_block_time;        // next scheduled blocktime / subsidy 
      double    prev_ram_price;        // previously recorded ram price from latest block subsidy
      uint64_t  stored_subsidies;      // current # of stored token subsidies on the 'tokensubsidies' table 
      uint64_t  active_props;          // actively linked eosio.msigs proposals ( realized and unrealized ) 
      uint64_t  successful_props;      // # of previously linked eosio.msig proposals that have successfully been approved/executed
      uint64_t  genesis_bytes;         // # of bytes forfeited by genesis_miner ( one time occurrence that is always accounted for & allows for contract to remain in a constant mining state )
      name      genesis_miner;         // first miner who forfeited mining privileges
      asset     dev_claim_bal;         // amount of accumilated and claimable CORE tokens ( derived from miner ramcore increases ) 
    };

    //@single: antelopeCACHE business logic configurations
    TABLE  config {
      uint64_t  prop_vote_decay_start; // beginning decay UNIX time
      uint64_t  prop_vote_decay_end;   // ending decay UNIX time
      double    initial_vote_pct_req;  // required vote percentage at start of decay
      double    ending_vote_pct_req;   // required vote percentage at end of decay
      uint64_t  proposition_period;    // Amount of time a proposition is votable before it expires
      uint64_t  min_leading_blocks;    // # of blocks a proposal has successfully had more than required voting percentage ( per block )
      asset     max_fee_vote;          // Maximum votable CACHE transaction fee 
      uint64_t  block_interval;        // Amount of time between block rewards ( constant but variable by action execution ) 
      uint64_t  max_subsidy_storage;   // Max # of subsidies that are allowed to simultaneously exist on 'tokensubsidy' table ( erased on first in first out basis )
      asset     const_burn_rate;       // the antelopeCACHE burn rate if no change or RAMCORE tables are non existent
      name      ramcustodian;          // EOS Account that holds mining RAM aka miner RAMCORE (  no resources are utilized by this account ) 
      name      developer;             // dev
    };

    //@multi: individual miner account details
    TABLE miner {
      name     cache_miner;            // miner account name 
      uint64_t bytes;                  // total bytes used for mining
      uint64_t submitted_props;        // total # of submitted proposals 
      uint64_t successful_props;       // total # of approved proposals 
      asset    voted_fee;              // voted transaction fee
      uint64_t prop_vote;              // current proposal vote 
      uint64_t last_prop_vt_time;      // time you submitted your last proposal vote
      uint64_t last_byte_adj_time;     // time you last updated your bytes
      uint64_t start_time;             // mining start time
      uint64_t last_claim;             // last recorded subsidy claim time
      uint128_t unredeemed_shares;     /// unredeem mining shares represented as a scaled up integer ( for precision )

      auto     primary_key()          const { return cache_miner.value; }
      uint64_t by_bytes()             const { return bytes; }
      uint64_t by_submitted_props()   const { return submitted_props; }
      uint64_t by_successful_props()  const { return successful_props; }
      uint64_t by_voted_fee()         const { return voted_fee.symbol.code().raw(); }
      uint64_t by_prop_vote()         const { return prop_vote; }
      uint64_t by_last_prop_vt_time() const { return last_prop_vt_time; }
      uint64_t by_last_byte_adj_time()const { return last_byte_adj_time; }
      uint64_t by_start_time()        const { return start_time; }
      uint64_t by_last_claim()        const { return last_claim; }
      uint128_t by_unredeemed_shares() const { return unredeemed_shares; }
    };

    //@multi: tracks imported eosio.msig proposals that are pending approval to upgrade contract abi & wasm ( code )
    TABLE proposals {        
      uint64_t    prop_timestamp;      // time of proposal 
      uint64_t    votes;               // total number of votes represented as bytes cast toward 'x' proposal
      name        proposer;            // account proposing the linkage of an existing eosio.msig transaction 
      name        eosio_msig_name;     // msig name as listed on the eoio.msig account
      uint64_t    leading_blocks;      // # of blocks where proposal had more than the required voting percentage
      checksum256 sha256hash;          // SHA256 hash of original proposal linked to _self 
      string      memorandum;          // statement behind proposal
            
      auto        primary_key()         const { return prop_timestamp; }
      uint64_t    by_proposer()         const { return proposer.value; } 
      uint64_t    by_eosio_msig_name()  const { return eosio_msig_name.value; }     
      uint64_t    by_votes()            const { return votes; }
      uint64_t    by_leading_blocks()    const { return leading_blocks; }
      checksum256 by_account_propname() const { return _by_account_propname( proposer , eosio_msig_name ); }
      static checksum256 _by_account_propname( name proposer , name eosio_msig_name ) { return checksum256::make_from_word_sequence<uint64_t>( 0ULL, proposer.value ,eosio_msig_name.value );}
    };

    typedef eosio::singleton< "miningstats"_n, miningstats > miningstats_singleton;
            miningstats_singleton _miningstats;
    typedef eosio::singleton< "config"_n, config > config_singleton;
            config_singleton _config;
    typedef multi_index<name("inflation"), inflation,
            indexed_by<name("era"), const_mem_fun<inflation, uint64_t, &inflation::by_era>>> inflation_table;
    typedef multi_index<"tokensubsidy"_n, tokensubsidy> tokensubsidy_table;
    typedef multi_index<name("miner"), miner,
            indexed_by<"minerbytes"_n, const_mem_fun<miner, uint64_t, &miner::by_bytes>>,
            indexed_by<"propsubmit"_n, const_mem_fun<miner, uint64_t, &miner::by_submitted_props>>,
            indexed_by<"propsucces"_n, const_mem_fun<miner, uint64_t, &miner::by_successful_props>>,
            indexed_by<"votedfee"_n, const_mem_fun<miner, uint64_t, &miner::by_voted_fee>>,
            indexed_by<"propvote"_n, const_mem_fun<miner, uint64_t, &miner::by_prop_vote>>,
            indexed_by<"lastpropvtime"_n, const_mem_fun<miner, uint64_t, &miner::by_last_prop_vt_time>>,
            indexed_by<"lastbytetime"_n, const_mem_fun<miner, uint64_t, &miner::by_last_byte_adj_time>>,
            indexed_by<"starttime"_n, const_mem_fun<miner, uint64_t, &miner::by_start_time>>,
            indexed_by<"lastclaim"_n, const_mem_fun<miner, uint64_t, &miner::by_last_claim>>,
            indexed_by<"unredeemed"_n, const_mem_fun<miner, uint128_t, &miner::by_unredeemed_shares>>> miner_table;
    typedef eosio::multi_index< name("accounts"), account > accounts;
    typedef eosio::multi_index< name("stat"), currency_stats > stats;
    typedef multi_index<name("proposals"), proposals,
            indexed_by<"byproposer"_n, const_mem_fun<proposals, uint64_t,&proposals::by_proposer>>,
            indexed_by<"bypropname"_n, const_mem_fun<proposals, uint64_t,&proposals::by_eosio_msig_name>>,
            indexed_by<"byvotes"_n, const_mem_fun<proposals, uint64_t,&proposals::by_votes>>,
            indexed_by<"byleadprops"_n, const_mem_fun<proposals, uint64_t, &proposals::by_leading_blocks>>,
            indexed_by<"bychecksum"_n, const_mem_fun<proposals, checksum256,&proposals::by_account_propname>>> proposals_table;

    // regular non return functions 
    void sovereignize_account ();
    void sub_balance( name owner, asset value);
    void add_balance( name owner, asset value, name ram_payer );
    void redeem_tokens( name from, name claimer,  name beneficiary, asset quantity );
    void tokenomics();
    void miningstats_check(string type);
    void verify_miner_claims ( name miner );
    void weight_adjustment_fees(name miner, uint64_t old_fee_vote, uint64_t new_fee_vote, uint64_t old_miner_byte_balance, uint64_t updated_miner_byte_balance );
    void msigapprove( name proposer, name propname );
    void msigexecute( name proposer, name propname );
    void proposalchecker(name msig_proposer, name proposal_name );
    void checkBlockReward();
    void increaseramcore ( name miner, uint64_t bytes );

    // functions w/ return values
    asset algorithmic_burn();
    double get_ram_price();
    checksum256 hashofproposal ( name msig_proposer, name proposal_name );
    uint64_t net_purchased_bytes_after_eosio_fees( const asset& quant );
    uint64_t eosio_ram_fees( uint64_t eos );
    uint64_t core_tokens_post_dev_fees( uint64_t eos );
    uint64_t updated_ram_supply();
    asset direct_convert( const asset& from, const symbol& to );
    int64_t get_bancor_output( int64_t inp_reserve, int64_t out_reserve, int64_t inp );
    double required_vote_percentage_by_decay();
    std::pair<string, string> proposal_table_iterator( uint64_t proposal );
    void increase_proposal_record_vote(name miner, uint64_t proposal, uint64_t adjustment ) ; 
    void decrease_proposal_record_vote_if_exist(name miner, uint64_t proposal, uint64_t adjustment ) ; 
    void add_miner_proposal_vote ( name miner, uint64_t proposal );
    void remove_miner_proposal_vote (name miner );
    void delete_proposal_record_if_exist ( uint64_t proposal ) ;

    using bios_action = eosio::action_wrapper<"bios"_n, &CONTRACT_CLASS_NAME::bios>;
    using enlist_action = eosio::action_wrapper<"enlist"_n, &CONTRACT_CLASS_NAME::enlist>;
    using claim_action = eosio::action_wrapper<"claim"_n, &CONTRACT_CLASS_NAME::claim>;
    using redeem_action = eosio::action_wrapper<"redeem"_n, &CONTRACT_CLASS_NAME::redeem>;
    using mineblock_action = eosio::action_wrapper<"mineblock"_n, &CONTRACT_CLASS_NAME::mineblock>;
    using votefee_action = eosio::action_wrapper<"votefee"_n, &CONTRACT_CLASS_NAME::votefee>;
    using transfer_action = eosio::action_wrapper<"transfer"_n, &CONTRACT_CLASS_NAME::transfer>;
    using open_action = eosio::action_wrapper<"open"_n, &CONTRACT_CLASS_NAME::open>;
    using close_action = eosio::action_wrapper<"close"_n, &CONTRACT_CLASS_NAME::close>;
    using proposemsig_action = eosio::action_wrapper<"proposemsig"_n, &CONTRACT_CLASS_NAME::proposemsig>;
    using propreview_action = eosio::action_wrapper<"propreview"_n, &CONTRACT_CLASS_NAME::propreview>;
    using blocklog_action = eosio::action_wrapper<"blocklog"_n, &CONTRACT_CLASS_NAME::blocklog>;
    using reducemining_action = eosio::action_wrapper<"reducemining"_n, &CONTRACT_CLASS_NAME::reducemining>;
};