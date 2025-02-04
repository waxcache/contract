#pragma once

#include <eosio/binary_extension.hpp>

namespace eosio {

   class [[eosio::contract("eosio.msig")]] multisig : public contract {
      public:
         using contract::contract;

         /**
          * Approve proposal
          * 
          * @details Approves an existing proposal
          * Allows an account, the owner of `level` permission, to approve a proposal `proposal_name`
          * proposed by `proposer`. If the proposal's requested approval list contains the `level` 
          * permission then the `level` permission is moved from internal `requested_approvals` list to 
          * internal `provided_approvals` list of the proposal, thus persisting the approval for 
          * the `proposal_name` proposal.
          * Storage changes are billed to `proposer`.
          * 
          * @param proposer - The account proposing a transaction
          * @param proposal_name - The name of the proposal (should be unique for proposer)
          * @param level - Permission level approving the transaction
          * @param proposal_hash - Transaction's checksum
          */
         [[eosio::action]]
         void approve( name proposer, name proposal_name, permission_level level,
                       const eosio::binary_extension<eosio::checksum256>& proposal_hash );
         /**
          * Execute proposal
          * 
          * @details Allows an `executer` account to execute a proposal.
          * 
          * Preconditions: 
          * - `executer` has authorization, 
          * - `proposal_name` is found in the proposals table, 
          * - all requested approvals are received, 
          * - proposed transaction is not expired, 
          * - and approval accounts are not found in invalidations table. 
          * 
          * If all preconditions are met the transaction is executed as a deferred transaction,
          * and the proposal is erased from the proposals table.
          * 
          * @param proposer - The account proposing a transaction
          * @param proposal_name - The name of the proposal (should be an existing proposal)
          * @param executer - The account executing the transaction
          */
         [[eosio::action]]
         void exec( name proposer, name proposal_name, name executer );

         using approve_action = eosio::action_wrapper<"approve"_n, &multisig::approve>;
         using exec_action = eosio::action_wrapper<"exec"_n, &multisig::exec>;

         struct [[eosio::table]] proposal {
            name                            proposal_name;
            std::vector<char>               packed_transaction;

            uint64_t primary_key()const { return proposal_name.value; }
         };

         typedef eosio::multi_index< "proposal"_n, proposal > proposals;

         struct [[eosio::table]] old_approvals_info {
            name                            proposal_name;
            std::vector<permission_level>   requested_approvals;
            std::vector<permission_level>   provided_approvals;

            uint64_t primary_key()const { return proposal_name.value; }
         };
         typedef eosio::multi_index< "approvals"_n, old_approvals_info > old_approvals;

         struct approval {
            permission_level level;
            time_point       time;
         };

         struct [[eosio::table]] approvals_info {
            uint8_t                 version = 1;
            name                    proposal_name;
            std::vector<approval>   requested_approvals;
            std::vector<approval>   provided_approvals;

            uint64_t primary_key()const { return proposal_name.value; }
         };
         typedef eosio::multi_index< "approvals2"_n, approvals_info > approvals;

   };

}