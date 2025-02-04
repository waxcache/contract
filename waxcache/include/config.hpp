/*
  ░█████╗░░█████╗░███╗░░██╗███████╗██╗░██████╗░
  ██╔══██╗██╔══██╗████╗░██║██╔════╝██║██╔════╝░
  ██║░░╚═╝██║░░██║██╔██╗██║█████╗░░██║██║░░██╗░
  ██║░░██╗██║░░██║██║╚████║██╔══╝░░██║██║░░╚██╗
  ╚█████╔╝╚█████╔╝██║░╚███║██║░░░░░██║╚██████╔╝
  ░╚════╝░░╚════╝░╚═╝░░╚══╝╚═╝░░░░░╚═╝░╚═════╝░
  @brief: update this file with all desired blockchain table & action parameters ( yes might be a bit extra but feels cleaner 😉) 
*/

// CACHE CONTRACT
#define NETWORK W //👋 update me ( must be upper case )
#define CONTRACT_CLASS_NAME CACHE // 👋 update me ( must be upper case )
#define CONTRACT_CLASS_NAME_lower_case cache // 👋 update me ( lower case )
#define PREFIX_ACCT_NAMES string(CONTRACT_CLASS_NAME)

// ANTELOPECACHE CONTRACT
#define ANTELOPE_CONTRACT_NAME eosio::name(_self)
#define ANTELOPE_CONTRACT_NAME_STRING string (ANTELOPE_CONTRACT_NAME.to_string())

// CONCAT CACHE SYM
#define CONCATENATE_IDENTIFIERS(a, b) a##b
#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)
#define CONCATENATE_TO_STRING(a, b) TO_STRING(CONCATENATE_IDENTIFIERS(a, b))
#define CONCAT_CACHE_SYM CONCATENATE_TO_STRING(NETWORK, CONTRACT_CLASS_NAME)

// CONCAT NAMES
#define CONCATENATE_DETAIL(prefix, name)
#define CONCATENATE(prefix, name) CONCATENATE_DETAIL(prefix, name)
#define MAKE_NAME(name) CONCATENATE(PREFIX_ACCT_NAMES, name)

// CONTRACTWIDE DEFINITIONS
#define SENDER_ID(X, Y)        ( (( uint128_t )X << 64) | Y )
#define NOW uint64_t(current_time_point().sec_since_epoch()) 
#define NOW_STRING string( "{" + std::to_string(NOW) + "}" ) 
#define NOW_TIME_POINT time_point_sec(current_time_point())
#define MINER_MEMO string("{" + NOW_STRING + "} : " + "<\\" + memo + " \\>") 
#define ERROR_LOCATION   string ("...SOURCE: 🚧 - File: " + std::string(__FILE__) + "; function: " + __func__ + "; line: " + std::to_string(__LINE__) + " 🚧 ")  

// core native contract values
#define NATIVE_SYSTEM_CONTRACT_NAME eosio::name("eosio")
#define NATIVE_SYSTEM_CONTRACT_NAME_VALUE NATIVE_SYSTEM_CONTRACT_NAME.value
#define NATIVE_MSIG_CONTRACT_NAME eosio::name("eosio.msig")
#define NATIVE_SYSTEM_CONTRACT_NAME_STRING string (NATIVE_SYSTEM_CONTRACT_NAME.to_string())
#define NATIVE_MSIG_CONTRACT_NAME_STRING string (NATIVE_MSIG_CONTRACT_NAME.to_string())
#define NATIVE_SYSTEM_TOKEN_CONTRACT_NAME eosio::name("eosio.token")

// contract symbols
#define RAMCORE_SYM symbol(symbol_code("RAMCORE"), 4) //👋 update me
#define RAM_SYM symbol(symbol_code("RAM"), 0) //👋 update me
#define CACHE_SYM symbol(symbol_code(CONCAT_CACHE_SYM), 4) 
#define CACHE_SYM_STRING (CONCAT_CACHE_SYM)
#define NATIVE_SYM symbol(symbol_code("WAX"), 8) //👋 update me 
#define NATIVE_SYM_STRING (NATIVE_SYM.code().to_string() + "," + std::to_string(NATIVE_SYM.precision()))

// contract RAW symbols
#define CACHE_RAW CACHE_SYM.code().raw()
#define RAMCORE_RAW RAMCORE_SYM.code().raw()
#define NATIVE_RAW NATIVE_SYM.code().raw()

// referenced contract accounts
#define RESERVE_ACCT name (CONCATENATE_TO_STRING(CONTRACT_CLASS_NAME_lower_case, reserve))  
#define NULL_ACCT name (CONCATENATE_TO_STRING(CONTRACT_CLASS_NAME_lower_case, nullify))  
#define CACHEDEV_ACCT name (CONCATENATE_TO_STRING(CONTRACT_CLASS_NAME_lower_case, devlopr))  

// msig variables
#define MINIMUM_PROPOSITION_ROW_ITR uint64_t (1) //⚖️ discretionary
#define EXPECTED_NUM_OF_MSIG_ACTIONS int(2) // "setabi" & "setcode" in any order
#define MINIMUM_CLAIMS uint64_t (1) //⚖️ discretionary
#define MAX_PROP_MEMO_SIZE uint64_t (512)  //⚖️ discretionary
#define MAX_BLOCK_MEMO_SIZE uint64_t (256)  //⚖️ discretionary
#define MAX_TRANSFER_MEMO_SIZE uint64_t (256)  //⚖️ discretionary
#define MINIMUM_BYTE_INCREASE uint64_t (1) //⚖️ discretionary
#define INITIAL_NATIVE_RAMCORE_NEW_BYTES_PER_BLOCK uint64_t(1024) //👋 update me
#define REQ_PROP_VOTE_AND_BYTE_ADJUSTMENT_TIME_INTERVAL uint64_t(2); //💡 suggested minimum: 1

// permissions
#define ANTELOPE_CONTRACT_ACTIVE_PERMISSION permission_level{get_self(), "active"_n}
#define ANTELOPE_CONTRACT_OWNER_PERMISSION permission_level{get_self(), "owner"_n}
#define RESERVE_ACCT_ACTIVE_PERMISSION {RESERVE_ACCT, "active"_n}

// BIOS Settings ( ready to deploy 08.25.2024 )
#define SET_MAX_SUPPLY asset( 20971480000 , CACHE_SYM)  // max_mine_supply x 2; ⚖️ discretionary
#define SET_BURN_SUPPLY asset( 10485740000 , CACHE_SYM) // equal to max_mine_supply ⚖️ discretionary
#define SET_MAX_MINE_SUPPLY asset (10485740000 , CACHE_SYM) // ⚖️ discretionary
#define SET_STARTING_BLOCK_REWARD uint64_t ( 262144 ) // ⚖️ discretionary
#define SET_BLOCKS_PER_HALVING uint64_t ( 20000 ) // ⚖️ discretionary
#define SET_CONST_BURN_RATE asset(106400, CACHE_SYM)  // gold melting point ( 1064°C ); ⚖️ discretionary
#define SET_DEVELOPER_PREMINE asset ( 1165080000, CACHE_SYM) // ≈ 10% of ending supply || 5% of max possible supply; ⚖️ discretionary
#define SET_NUM_OF_HALVINGS uint64_t(19) // ⚖️ discretionary 
#define SET_HALVING_DIVISOR (2) 
#define SET_PROPOSITION_PERIOD uint64_t ( 2629744 ) // 2629744 seconds in a month; ⚖️ discretionary
#define SET_MINIMUM_LEADING_BLOCK uint64_t ( 48 ) //  ≈ 365 hours in half a month ( aka blocks ); ⚖️ discretionary
#define SET_MAX_FEE_VOTE asset( 10000 , CACHE_SYM) // ⚖️ discretionary
#define SET_BLOCK_INTERVAL uint64_t ( 3600 ) //💡 suggested minimum is at least 60 seconds
#define SET_MAX_SUBSIDY_STORAGE uint64_t ( 1024 ) // a month and a few days ⚖️ discretionary

/* proposal voting variables */
#define INITIAL_VOTE_PCT_REQ double ( 1.00000000000000000 ) // 100%; ⚖️ discretionary
#define ENDING_VOTE_PCT_REQ double ( 0.51000000000000000 ) // 51%;  ⚖️ discretionary
#define REQ_VOTE_PCT_DECAY_PERIOD uint64_t ( 31536000 ) // // seconds in a year; ⚖️ discretionary 

#define WHITELIST_ACCT_LIMIT 21 // ⚖️ discretionary 
constexpr uint128_t SCALE_FACTOR = 1'000'000'000'000'000'000; // 10^18 ( for precision mining claims/redeemptions )
constexpr double PRECISIONFACTOR = 1e8; // 👋 update me ( core native token precision )