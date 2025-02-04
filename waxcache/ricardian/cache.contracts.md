<h1 class="contract">enlist</h1>

---
spec_version: "0.3.0"
title: Enlist as a Miner
summary: 'Register {{nowrap miner}} as a antelopcache miner'
icon: https://raw.githubusercontent.com/antelopebuilder/antelopcache-ricardian-icns/main/ricardian%20icons/enlist_icon.png#c22497d5e06e711fd9f8f15343efd24ed350afd323051a976aef1133aea46440
---

{{nowrap miner}} agrees to enlist as a miner in the antelopcache contract.

##### RAM Costs
Incurs a RAM cost to store miner record. It is the responsibility of {{miner}} to ensure sufficient resources are available to cover this cost.

##### DISCLAIMER 
By executing this action, user confirms they have read, understood, and agreed to all terms outlined in the UserAgreement clause.

<h1 class="contract">claim</h1>

---
spec_version: "0.3.0"
title: Claim Mining Shares
summary: '{{nowrap miner}} iterates {{nowrap max_claims}} tokensubsidy rows for rewards'
icon: https://raw.githubusercontent.com/antelopebuilder/antelopcache-ricardian-icns/main/ricardian%20icons/claim_icon.png#591ab9e9ab66e10a1e93f86953e11f3a07137d4963380e820804c1d8515ba021
---

{{miner}} agrees to review up to {{max_claims}} tokensubsidies to identify claimable mining shares based on the mining weight.

##### RAM Costs
This action does not incur any additional RAM cost to {{miner}}

##### DISCLAIMER 
By executing this action, user confirms they have read, understood, and agreed to all terms outlined in the UserAgreement clause.

<h1 class="contract">redeem</h1>

---
spec_version: "0.3.0"
title: Convert Mining Shares to tokens
summary: '{{nowrap miner}} converts mining shares to {{nowrap requested_cache}} in benefit to {{nowrap receiver}}'
icon: https://raw.githubusercontent.com/antelopebuilder/antelopcache-ricardian-icns/main/ricardian%20icons/redeem_icon.png#7322b3d0a57d8c0d137a2b833164181f02fd42c4eaea75619a77e0e296cd4ad3
---

{{miner}} agrees to redeem {{requested_cache}} to {{receiver}}. 

##### RAM Costs
This operation may incur RAM usage costs if {{receiver}} has not opened an existing antelopcache account, deducted from {{miner}}'s account resources. It is the responsibility of {{miner}} to ensure sufficient resources are available to cover these costs.

##### DISCLAIMER 
By executing this action, user confirms they have read, understood, and agreed to all terms outlined in the UserAgreement clause.

<h1 class="contract">votefee</h1>

---
spec_version: "0.3.0"
title: Vote for Block Transaction Fee
summary: '{{nowrap miner}} votes for a new transaction fee of {{nowrap tx_fee}}.'
icon: https://raw.githubusercontent.com/antelopebuilder/antelopcache-ricardian-icns/main/ricardian%20icons/votefee_icon.png#509115dc7e82ee561b5761737256a3df30173668a9d0486eec2399835c6cf134
---

{{miner}} agrees to vote for the upcoming block transaction fee to be {{tx_fee}}.

##### RAM Costs
This action does not incur any additional RAM cost to {{miner}}

##### DISCLAIMER 
By executing this action, user confirms they have read, understood, and agreed to all terms outlined in the UserAgreement clause.

<h1 class="contract">mineblock</h1>

---
spec_version: "0.3.0"
title: Mine a Block
summary: '{{nowrap initiator}} mines current block.'
icon: https://raw.githubusercontent.com/antelopebuilder/antelopcache-ricardian-icns/main/ricardian%20icons/mineblock_icon.png#19894b08670b13a95a796f319c43c82b68463201573296fd0236363beed4018e
---

{{initiator}} agrees to mine the current block.

##### RAM Costs
{{initiator}} will temporarily incur RAM costs to store block subsidy until erased at a future date. It is the responsibility of {{initiator}} to ensure sufficient resources are available to cover these costs.

##### DISCLAIMER 
By executing this action, user confirms they have read, understood, and agreed to all terms outlined in the UserAgreement clause.

<h1 class="contract">blocklog</h1>

---
spec_version: "0.3.0"
title: Log Block Results
summary: 'Log block details in state history.'
icon: https://raw.githubusercontent.com/antelopebuilder/antelopcache-ricardian-icns/main/ricardian%20icons/blocklog_icon.png#ce2855eb6b7afc329e6ba257a5c831590d64d4ab6bf8d9f5de2b265f10a3ceb1
---

The blocklog action only executable by antelopcache via an inline action at end of the mineblock action is designed to record the outcomes of each block mine, providing a transparent and immutable record of activities & relevant metrics associated with block production.

##### RAM Costs
This action does not incur any additional RAM cost.

##### DISCLAIMER 
By executing this action, user confirms they have read, understood, and agreed to all terms outlined in the UserAgreement clause.

<h1 class="contract">bios</h1>

---
spec_version: "0.3.0"
title: Initialize Contract
summary: 'Initialize the antelopcache contract, token, inflation, and sovereign permission structure'
icon: https://raw.githubusercontent.com/antelopebuilder/antelopcache-ricardian-icns/main/ricardian%20icons/bios_icon.png#816a5601c364d5c0410f1b316e52b1593ab0e95e8b42fb02944b48bd777900ce
---

{{contractissuer}} agrees to launch the antelopcache contract, establishing the business logic, tokenomics, inflation schedule, and permissions enabling the sovereign governance of antelopcache.

##### RAM Costs: 
The RAM cost for initializing the antelopcache contract is covered by {{contractissuer}}.

##### DISCLAIMER 
By executing this action, user confirms they have read, understood, and agreed to all terms outlined in the UserAgreement clause.

<h1 class="contract">transfer</h1>

---
spec_version: "0.3.0"
title: Transfer Tokens
summary: 'Send {{nowrap quantity}} from {{nowrap from}} to {{nowrap to}}'
icon: https://raw.githubusercontent.com/antelopebuilder/antelopcache-ricardian-icns/main/ricardian%20icons/transfer_icon.png#4ed6fd2f7ec58925636069525c497454722dbfb97bf1c6b48df8f7b72a6a78be
---

{{from}} agrees to send {{quantity}} *not including current transaction fee* to {{to}}

*Transaction fees are deducted from {{from}} balance in addition to {{quantity}}*

{{#if memo}}There is a memo attached to the transfer stating:
{{memo}}
{{/if}}

##### RAM Costs
The {{from}} account will be the RAM payer for the transaction fees deducted.

If {{to}} does not have an existing balance for {{asset_to_symbol_code quantity}}, {{from}} will be designated as the RAM payer for the newly created {{asset_to_symbol_code quantity}} balance for {{to}}. As a result, RAM will be deducted from {{from}}'s resources to create the necessary blockchain records.

##### DISCLAIMER 
By executing this action, user confirms they have read, understood, and agreed to all terms outlined in the UserAgreement clause.

<h1 class="contract">open</h1>

---
spec_version: "0.3.0"
title: Open Token Balance
summary: 'Establish a zero quantity balance for {{nowrap owner}}'
icon: https://raw.githubusercontent.com/antelopebuilder/antelopcache-ricardian-icns/main/ricardian%20icons/open_icon.png#5ee334300e10d480e5e50ebbff51d86af4adb4f277fc71fb74fb752562abc864
---

{{ram_payer}} agrees to create a new balance record for {{owner}} for the {{symbol_to_symbol_code symbol}} token symbol. This action establishes a balance record to enable the receipt of {{symbol_to_symbol_code symbol}}, ensuring that transactions involving the specified token can be properly processed and recorded on the blockchain.

##### RAM Costs
By executing this action, {{ram_payer}} accepts the responsibility for covering the RAM costs associated with storing the new balance record.

##### DISCLAIMER 
By executing this action, user confirms they have read, understood, and agreed to all terms outlined in the UserAgreement clause

<h1 class="contract">close</h1>

---
spec_version: "0.3.0"
title: Close Token Balance
summary: 'Remove {{nowrap owner}}’s zero quantity {{nowrap symbol_to_symbol_code symbol}} token balance'
icon: https://raw.githubusercontent.com/antelopebuilder/antelopcache-ricardian-icns/main/ricardian%20icons/close_icon.png#1f23faf16967867deb02285e119d3990b3ea26d481dc42242d83171248070a86
---

{{owner}} agrees to close their {{symbol_to_symbol_code symbol}} token balance record. This action removes the balance record from the blockchain, freeing the associated RAM resources. 

##### DISCLAIMER 
By executing this action, user confirms they have read, understood, and agreed to all terms outlined in the UserAgreement clause.

<h1 class="contract">reducemining</h1>

---
spec_version: "0.3.0"
title: Reduce Mining Capacity
summary: '{{nowrap miner}} reduces mining capacity by {{nowrap bytes}}'
icon: https://raw.githubusercontent.com/antelopebuilder/antelopcache-ricardian-icns/main/ricardian%20icons/reducemining_icon.png#46380e7f500bb348a22b078e3ff0138b9e13dce87e31fd46fd6d3ad34d0c3623
---

{{miner}} agrees to sell {{bytes}} bytes of their mining capacity in exchange for Antelope blockchain's native core token, minus any applicable fees related to current market conditions.

##### RAM Costs
This action does not incur and additional RAM cost to {{miner}}

##### DISCLAIMER 
By executing this action, user confirms they have read, understood, and agreed to all terms outlined in the UserAgreement clause.

<h1 class="contract">increasebytes</h1>

---
spec_version: "0.3.0"
title: On Notify Increase Mining Capacity
summary: 'Increase {{nowrap from}} byte mining capacity.'
icon: https://raw.githubusercontent.com/antelopebuilder/antelopcache-ricardian-icns/main/ricardian%20icons/increase_icon.png#08645064a4321b92e5190569763dfbf26e3855e932fc84cc0585d79e2ddf451d
---

Upon receiving an Antelope blockchain's native core token transfer from {{from}}, the antelopcache contract adjusts the byte mining capacity of {{from}} after contract/developer fees.

##### DISCLAIMER 
By executing this action, user confirms they have read, understood, and agreed to all terms outlined in the UserAgreement clause.

<h1 class="contract">proposemsig</h1>

---
spec_version: "0.3.0"
title: Propose an abi/code change
summary: '{{nowrap proposer}} proposes eosio.msig {{nowrap proposal_name}} on antelopcache.'
icon: https://raw.githubusercontent.com/antelopebuilder/antelopcache-ricardian-icns/main/ricardian%20icons/proposemsig_icon.png#eb18a88c591773e86d1f5cae6d0f30b01954ebb6d52c42f289f2e17cfb286b3b
---

{{proposer}} agrees to  to link an existing eosio.msig proposal named {{proposal_name}} to the antelopcache contract for consideration and potential abi/code modification of antelopcache

{{#if memorandum}} {{proposer}} cites the following reasoning for {{proposal_name}}:
{{memorandum}}
{{/if}}

##### RAM Costs: 
{{proposer}} will incur the RAM cost to store {{proposal_name}} through expiration, approval, denial, and/or execution. It is the responsibility of {{proposer}} to have determined or made possible for antelopcache to have sufficient RAM ( not including its miningbytes ) to cover any increased costs for abi/code modifications.

##### DISCLAIMER 
By executing this action, user confirms they have read, understood, and agreed to all terms outlined in the UserAgreement clause.

<h1 class="contract">voteprop</h1>

---
spec_version: "0.3.0"
title: Vote for Proposal
summary: '{{nowrap miner}} votes for proposal {{nowrap proposal}}.'
icon: https://raw.githubusercontent.com/antelopebuilder/antelopcache-ricardian-icns/main/ricardian%20icons/voteprop_icon.png#da891723ca8308efcab6c69f4fbb201c4adbf72e7b8de1b9f00bb760cd49f339
---

{{miner}} agrees to vote for proposal: {{prop_timestamp}} to modify antelopcache abi/code

##### RAM Costs
This action does not incur and additional RAM cost to {{miner}}

##### DISCLAIMER 
By executing this action, user confirms they have read, understood, and agreed to all terms outlined in the UserAgreement clause.

<h1 class="contract">propreview</h1>

---
spec_version: "0.3.0"
title: Proposal Review and Execution 
summary: '{{nowrap initiator}} potentially executes and/or erases proposal {{nowrap proposal}}'
icon: https://raw.githubusercontent.com/antelopebuilder/antelopcache-ricardian-icns/main/ricardian%20icons/propreview_icon.png#e01e1abf2fd5bc63e6e07fee32fb4d06c10959ffe3ede428ed104ca0997a9690
---

{{initiator}} agrees to review the proposal {{proposal}} and by extension approve/execute and/or erase the proposal.

##### RAM Costs
This action does not incur and additional RAM cost to {{initiator}}

##### DISCLAIMER 
By executing this action, user confirms they have read, understood, and agreed to all terms outlined in the UserAgreement clause.

<h1 class="contract">whitelist</h1>

---
spec_version: "0.3.0"
title: Whitelist Accounts
summary: 'Register {{nowrap accounts}} for feeless antelopcache transfers.'
icon: https://raw.githubusercontent.com/antelopebuilder/antelopcache-ricardian-icns/main/ricardian%20icons/whitelist_icon.png#3b2e2a12c51dfa426cd207a3c92de85c0c6b03ad2ed23cc3b1803a1b03f25f62
---

By executing this action, {{accounts}} commit to adhering to the rules and standards set forth by antelopcache for whitelisted transfers. This action is executed solely by the developer with the intention of fostering community growth and adoption via DEX/CEX partnerships.

##### RAM Costs
Covered by antelopcache

##### DISCLAIMER 
By executing this action, user confirms they have read, understood, and agreed to all terms outlined in the UserAgreement clause

<h1 class="contract">devclaim</h1>

---
spec_version: "0.3.0"
title: Developer Claims Collected Native Core Token Fees
summary: 'antelopcache developer withdraws {{nowrap quantity}} in mining fees.'
icon: https://raw.githubusercontent.com/antelopebuilder/antelopcache-ricardian-icns/main/ricardian%20icons/devclaim_icon.png#6f53ca079d4c8954efef3d36f2c4962b66a3c685e9d8659c7573e74a42d81f92
---

By executing this action, antelopcache developer claims and withdraws previously collected {{nowrap quantity}}. 

##### DISCLAIMER 
By executing this action, user confirms they have read, understood, and agreed to all terms outlined in the UserAgreement clause.