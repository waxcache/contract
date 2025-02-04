/*
   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
   ██╗░░  ████████╗░█████╗░██╗░░██╗███████╗███╗░░██╗  ░█████╗░░█████╗░████████╗██╗░█████╗░███╗░░██╗░██████╗
   ╚██╗░  ╚══██╔══╝██╔══██╗██║░██╔╝██╔════╝████╗░██║  ██╔══██╗██╔══██╗╚══██╔══╝██║██╔══██╗████╗░██║██╔════╝
   ░╚██╗  ░░░██║░░░██║░░██║█████═╝░█████╗░░██╔██╗██║  ███████║██║░░╚═╝░░░██║░░░██║██║░░██║██╔██╗██║╚█████╗░
   ░██╔╝  ░░░██║░░░██║░░██║██╔═██╗░██╔══╝░░██║╚████║  ██╔══██║██║░░██╗░░░██║░░░██║██║░░██║██║╚████║░╚═══██╗
   ██╔╝░  ░░░██║░░░╚█████╔╝██║░╚██╗███████╗██║░╚███║  ██║░░██║╚█████╔╝░░░██║░░░██║╚█████╔╝██║░╚███║██████╔╝
   ╚═╝░░  ░░░╚═╝░░░░╚════╝░╚═╝░░╚═╝╚══════╝╚═╝░░╚══╝  ╚═╝░░╚═╝░╚════╝░░░░╚═╝░░░╚═╝░╚════╝░╚═╝░░╚══╝╚═════
   ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░
   @brief: antelopeCACHE token actions/functions 
*/

#include <cache.hpp>

/*
   ▀█▀ █▀█ ▄▀█ █▄░█ █▀ █▀▀ █▀▀ █▀█
   ░█░ █▀▄ █▀█ █░▀█ ▄█ █▀░ ██▄ █▀▄
   @brief: send antelopeCACHE tokens with an optional message
   @param from: antelopeCACHE sender
   @param to: antelopeCACHE receiver
   @param quantity: antelopeCACHE amount being sent
   @param memo: optional message from sender
*/

[[eosio::action]] void CONTRACT_CLASS_NAME::transfer( name from, name to, asset quantity, string memo ) {

   require_auth( from );

   check( from != _self, "⚠️ " + _self.to_string() + " not authorized to transfer " + CACHE_SYM.code().to_string() + " ⚠️" + ERROR_LOCATION ); 
   check( to != _self, "⚠️ " + _self.to_string() + " does not accept incoming " + CACHE_SYM.code().to_string() + " transfers ⚠️" + ERROR_LOCATION );   
   check( from != to, "⚠️ Cannot transfer to self ⚠️" + ERROR_LOCATION );
   check( is_account( to ), "⚠️ " + to.to_string() + " account does not exist ⚠️" + ERROR_LOCATION );

   auto sym = quantity.symbol.code();
   stats statstable( _self, sym.raw() );
   const auto& st = statstable.get( sym.raw() );
   bool fee_flag = 0;

   require_recipient( from );
   require_recipient( to );

   check( quantity.is_valid(), "⚠️ Invalid quantity ⚠️" + ERROR_LOCATION );
   check( quantity.amount > 0, "⚠️ Must transfer positive quantity ⚠️" + ERROR_LOCATION );
   check( quantity.symbol ==  CACHE_SYM, "⚠️ Symbol precision mismatch ⚠️" + ERROR_LOCATION );
   check( memo.size() <= MAX_TRANSFER_MEMO_SIZE, "⚠️ Memo limited to " + std::to_string( MAX_TRANSFER_MEMO_SIZE) + " bytes ⚠️" + ERROR_LOCATION );

   auto payer = has_auth( to ) ? to : from;
   auto fees = st.current_tx_fee;
   
   bool is_from_whitelisted = std::find(st.whtlst_xchng_accts.begin(), st.whtlst_xchng_accts.end(), from) != st.whtlst_xchng_accts.end();
   bool is_to_whitelisted = std::find(st.whtlst_xchng_accts.begin(), st.whtlst_xchng_accts.end(), to) != st.whtlst_xchng_accts.end();

   statstable.modify( st, _self, [&]( auto& s ) { // ram bill
      s.block_txs  += 1;    
      if (!is_from_whitelisted && !is_to_whitelisted) {
         s.block_fees += fees;
         fee_flag = 1;
      }
   });

   // regular transaction 
   sub_balance( from, quantity );
   add_balance( to, quantity, payer );

   // fee transaction 
   if (!is_from_whitelisted && !is_to_whitelisted) {
      sub_balance( from, fees );
      add_balance( st.issuer, fees, payer );
   }
   if ( fee_flag == 0 ) {
      print ( "💸 ", from, " transferred " + quantity.to_string() + " to " , to , " 💸 \n"); 
   } else {
      asset total_transaction_cost = quantity + fees;
      print ( "💸 ", from, " transferred " + quantity.to_string() + " to " , to, " including " + fees.to_string() + " in transaction fees. Transaction total: " + total_transaction_cost.to_string() + " 💸 \n" ); 

   }

}

/*
   █▀█ █▀█ █▀▀ █▄░█
   █▄█ █▀▀ ██▄ █░▀█
   @brief: Create a new balance record for `owner` by ram expense of  `ram_payer`
   @param owner: new account balance record
   @param symbol: token symbol
   @param ram_payer: account paying for storage of record
*/

[[eosio::action]] void CONTRACT_CLASS_NAME::open( name owner, const symbol& symbol, name ram_payer ) { 

   require_auth( ram_payer );

   auto sym_code_raw = symbol.code().raw();

   stats statstable( _self, sym_code_raw );
   const auto& st = statstable.get(sym_code_raw, ("⚠️ Symbol does not exist ⚠️" + ERROR_LOCATION).c_str());
   check( st.supply.symbol == symbol, "⚠️ Symbol precision mismatch ⚠️" + ERROR_LOCATION );

   accounts acnts( _self, owner.value );
   auto it = acnts.find( sym_code_raw );
   check( it == acnts.end(), "⚠️ Balance record already exists ⚠️" + ERROR_LOCATION );

   if( it == acnts.end() ) {
      acnts.emplace( ram_payer, [&]( auto& a ){ // ram bill
        a.balance = asset{0, symbol};
      });
   }   
   print ( "📝 ", ram_payer, " opened a " , CACHE_SYM_STRING , " record for ", owner ," 📝 \n"); 
}

/*
   █▀▀ █░░ █▀█ █▀ █▀▀
   █▄▄ █▄▄ █▄█ ▄█ ██▄
   @brief: Close a balance record for `owner`
   @param owner: the owner account to execute the close action for,
   @param symbol: the symbol of the token to execute the close action for.
*/

[[eosio::action]] void CONTRACT_CLASS_NAME::close( name owner, const symbol& symbol ){

   require_auth( owner );
   
   accounts acnts( _self, owner.value );
   auto it = acnts.find( symbol.code().raw() );
   check( it != acnts.end(), "⚠️ Balance record already deleted or never existed ⚠️" + ERROR_LOCATION );
   check( it -> balance.amount == 0, "⚠️ Cannot close because the balance is not zero ⚠️" + ERROR_LOCATION );
   acnts.erase( it );
   print ( "🗑️ ", owner, " closed " , CACHE_SYM_STRING , " record 🗑️ \n" ); 

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
   █▀█ █▀▀ █▀▄ █▀▀ █▀▀ █▀▄▀█   █▀▀ ▄▀█ █▀▀ █░█ █▀▀   ▀█▀ █▀█ █▄▀ █▀▀ █▄░█ █▀
   █▀▄ ██▄ █▄▀ ██▄ ██▄ █░▀░█   █▄▄ █▀█ █▄▄ █▀█ ██▄   ░█░ █▄█ █░█ ██▄ █░▀█ ▄█
   @brief: Transfer action for converting claimed mining shares into actual transferable antelopeCACHE
   @param from: _self
   @param claimer: miner entitled to antelopeCACHE tokens
   @param to: designated recipient of antelopecacheCACHE tokens
   @param quantity: amount of antelopecacheCACHE tokens being redeemed
   @note: only feeless type of transaction in antelopeCACHE
*/   

void CONTRACT_CLASS_NAME::redeem_tokens( name from, name claimer, name beneficiary, asset quantity ){

   require_auth( claimer ); 

   check ( beneficiary != _self, "⚠️ Cannot transfer to " + _self.to_string() + " ⚠️" + ERROR_LOCATION ); 
   check( is_account( beneficiary ), "⚠️ " + beneficiary.to_string() + " account does not exist ⚠️" + ERROR_LOCATION );

   auto sym = quantity.symbol.code();
   stats statstable( _self, sym.raw() );
   const auto& st = statstable.get( sym.raw() );

   require_recipient( from );
   require_recipient( beneficiary );

   check( quantity.is_valid(), "⚠️ Invalid quantity ⚠️" + ERROR_LOCATION );
   check( quantity.amount > 0, "⚠️ Must transfer positive quantity ⚠️" + ERROR_LOCATION ); 
   check( quantity.symbol == st.supply.symbol, "⚠️ Symbol precision mismatch ⚠️" + ERROR_LOCATION );

   statstable.modify( st, _self, [&]( auto& s ) { // ram bill
      s.block_txs  += 1; 
   });

   auto payer = has_auth( beneficiary ) ? beneficiary : claimer;

   sub_balance( from, quantity );
   add_balance( beneficiary, quantity, payer );  

}

/*
   █▀ █░█ █▄▄   █▄▄ ▄▀█ █░░ ▄▀█ █▄░█ █▀▀ █▀▀
   ▄█ █▄█ █▄█   █▄█ █▀█ █▄▄ █▀█ █░▀█ █▄▄ ██▄   
   @brief: remove balance for owner acct
*/

void CONTRACT_CLASS_NAME::sub_balance( name owner, asset value ) {

   accounts from_acnts( _self, owner.value );
   const auto& from = from_acnts.get( value.symbol.code().raw(), ( "⚠️ No balance object found in accounts️ ⚠️️ " + ERROR_LOCATION).c_str() );

   check( from.balance >= value , "⚠️ From balance overdrawn ⚠️" + ERROR_LOCATION ); 

   from_acnts.modify( from, owner, [&]( auto& a ) { // ram bill
         a.balance -= value;
   });

}

/*
   ▄▀█ █▀▄ █▀▄   █▄▄ ▄▀█ █░░ ▄▀█ █▄░█ █▀▀ █▀▀
   █▀█ █▄▀ █▄▀   █▄█ █▀█ █▄▄ █▀█ █░▀█ █▄▄ ██▄
   @brief: add balance to owner acct 
   @param ram_payer: account paying for owner balance record
*/

void CONTRACT_CLASS_NAME::add_balance( name owner, asset value, name ram_payer ){

   accounts to_acnts( _self, owner.value );
   auto to = to_acnts.find( value.symbol.code().raw() );
   if ( to == to_acnts.end() ) {
      to_acnts.emplace( ram_payer, [&]( auto& a ){ // ram bill
        a.balance = value;
      });
   } else {
      to_acnts.modify( to, same_payer, [&]( auto& a ) { // ram bill
        a.balance += value;
      });
   }

}
