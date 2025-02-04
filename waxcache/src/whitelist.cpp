/*
██╗░░  ░██╗░░░░░░░██╗██╗░░██╗██╗████████╗███████╗██╗░░░░░██╗░██████╗████████╗
╚██╗░  ░██║░░██╗░░██║██║░░██║██║╚══██╔══╝██╔════╝██║░░░░░██║██╔════╝╚══██╔══╝
░╚██╗  ░╚██╗████╗██╔╝███████║██║░░░██║░░░█████╗░░██║░░░░░██║╚█████╗░░░░██║░░░
░██╔╝  ░░████╔═████║░██╔══██║██║░░░██║░░░██╔══╝░░██║░░░░░██║░╚═══██╗░░░██║░░░
██╔╝░  ░░╚██╔╝░╚██╔╝░██║░░██║██║░░░██║░░░███████╗███████╗██║██████╔╝░░░██║░░░
╚═╝░░  ░░░╚═╝░░░╚═╝░░╚═╝░░╚═╝╚═╝░░░╚═╝░░░╚══════╝╚══════╝╚═╝╚═════╝░░░░╚═╝░░░
@brief: developer specific actions/functions
*/

#include <cache.hpp>

[[eosio::action]] void CONTRACT_CLASS_NAME::whitelist(const std::vector<name>& accounts) {

    check(accounts.size() > 0, "⚠️ The accounts array cannot be empty ⚠️" + ERROR_LOCATION);
    for (const auto& acct : accounts) {
        check(is_account(acct), "⚠️ The account " + acct.to_string() + " does not exist ⚠️" + ERROR_LOCATION );
    }

    require_auth(CACHEDEV_ACCT);

    check(accounts.size() <= WHITELIST_ACCT_LIMIT, "⚠️ Exceeds account limit of " + std::to_string(WHITELIST_ACCT_LIMIT) + " ⚠️" + ERROR_LOCATION);

    auto sym = SET_MAX_SUPPLY.symbol;
    stats statstable(_self, sym.code().raw());
    auto existing = statstable.find(sym.code().raw());

    statstable.modify(existing, _self, [&](auto& s) { // ram bill
        s.whtlst_xchng_accts = accounts;
    });

    print("✳️ ", CACHEDEV_ACCT.to_string(), " whitelisted the following accts for feeless transfers: \n");
    print("{ \n");
    for (const auto& acct : accounts) {
        print(acct.to_string(), ", ");
    }
    print("\n");
    print("} ✳️ \n");
    
};







