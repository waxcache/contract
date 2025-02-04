<div style="text-align:center;">
    <img src="https://github.com/waxcache/branding/blob/main/token/waxcache_token_logo.png?raw=true" alt="wax{cache}" style="width:50%;"/>
</div>
<h2 style="text-align:center;color:#F8691A;font-size: 150px;"><b>wax{cache}</b></h2>


<h2 style="text-align:center;color:orange;font-size: 58px;">a sovereign ram mining protocol</h2>
<br>

<h3 style="text-align:center;color:orange;"><b>specification & overview</b></h3>

<header>
an experimental protocol antelope{cache} and associated contract `antelopcache` that monopolizes the financial properties of antelope's native resource ram which already emulates the physical and financial dynamics of hardware mining found with bitcoin. please review the antelope{cache} whitepaper for a better introduction.

<br>

this serves as a brief manual/glossary that covers **table** and **action** properties for antelope{cache} participants (the values listed here are random and do not represent real contract values).

<h6 style="text-align:center;"><i>"ram is to antelope{cache} what mining hardware is to bitcoin, cpu is to antelope{cache} what electricity is to bitcoin, and ram fees are to antelope{cache} what sales tax and restocking fees are to mining hardware".</i></h6>

</header>

<h2 style="text-align:center;color:orange;font-size: 30px;"><b><u>tables glossary</u></b></h2>

<br>
<h4 style="color:orange;">table - <b><i>account</i></b></h4>
<p>stores ECACHE balance for each account.</p>
<ul>
</ul>

| account name |     balance      |
| :----------- | :--------------: |
| user1        | 1000.0000 ECACHE |
| user2        | 500.0000 ECACHE  |
| user3        | 250.0000 ECACHE  |

<br>
<h4 style="color:orange;">table - <b><i>currency_stats</i></b></h4>
<p>tracks circulating supply, maximum supply, and other aspects of the ECACHE token.</p>

| supply              |     max supply      |   max mine supply   |    mined supply     |  unclaimed supply  |    burn supply     |   fee vt weight    | current tx fee | upcoming tx fee | block txs |  block fees   | issuer  | whitelisted accounts |
| :------------------ | :-----------------: | :-----------------: | :-----------------: | :----------------: | :----------------: | :----------------: | :------------: | :-------------: | :-------: | :-----------: | :-----: | :------------------: |
| 1000000.0000 ECACHE | 2400000.0000 ECACHE | 2000000.0000 ECACHE | 1500000.0000 ECACHE | 500000.0000 ECACHE | 200000.0000 ECACHE | 1.1234567890123456 | 0.0100 ECACHE  |  0.0150 ECACHE  |    123    | 0.5000 ECACHE | issuer1 | [account1, account2] |

<br>
<h4 style="color:orange;">table - <b><i>inflation</i></b></h4>
<p>details the inflationary token dynamics of ECACHE, including block subsidies and era-based supply adjustments.</p>

| block |  era  | time reached | block subsidy  |   added supply   |    total supply     |
| :---- | :---: | :----------: | :------------: | :--------------: | :-----------------: |
| 1     |   1   |  1609459200  | 50.0000 ECACHE | 1000.0000 ECACHE | 1000000.0000 ECACHE |
| 2     |   1   |  1609462800  | 50.0000 ECACHE | 1000.0000 ECACHE | 1001000.0000 ECACHE |
| 3     |   1   |  1609466400  | 50.0000 ECACHE | 1000.0000 ECACHE | 1002000.0000 ECACHE |

<br>
<h4 style="color:orange;">table - <b><i>tokensubsidy</i></b></h4>
<p>temporarily stores mined blocks for miners to claim rewards.</p>

| block time | block | total bytes | miner count | unclaimed miners |     reward      |
| :--------- | :---: | :---------: | :---------: | :--------------: | :-------------: |
| 1609459200 |   1   |    1000     |     10      |        2         | 150.0000 ECACHE |
| 1609462800 |   2   |    2000     |     15      |        3         | 300.0000 ECACHE |
| 1609466400 |   3   |    3000     |     20      |        4         | 450.0000 ECACHE |

<br>
<h4 style="color:orange;">table - <b><i>miningstats</i></b></h4>
<p>singleton table containing key metrics and statistics about the mining process and the ECACHE ecosystem.</p>


| current era | current block | total bytes | miner count | last subsidy | next block time | prev ram price | stored subsidies | active proposals | successful proposals | genesis bytes | genesis miner | dev claim bal |
| :---------: | :-----------: | :---------: | :---------: | :----------: | :-------------: | :------------: | :--------------: | :--------------: | :------------------: | :-----------: | :-----------: | :-----------: |
|      1      |       3       |    3000     |     20      |  1609466400  |   1609470000    |     0.1230     |        3         |        1         |          1           |     1000      |     user1     |  0.1000 WAX   |

<br>
<h4 style="color:orange;">table - <b><i>config</i></b></h4>
<p>singleton table with configurations and settings guiding the contract logic.</p>

| prop vote decay start | prop vote decay end | initial vote pct req | ending vote pct req | proposition period | min leading blocks | max fee vote  | block interval | max subsidy storage | const burn rate | ram custodian | developer |
| :-------------------: | :-----------------: | :------------------: | :-----------------: | :----------------: | :----------------: | :-----------: | :------------: | :-----------------: | :-------------: | :-----------: | :-------: |
|      1609459200       |     1612137600      |        1.0000        |       0.5000        |       86400        |         10         | 0.1000 ECACHE |      600       |         10          |  0.0100 ECACHE  | ramcustodian  | developer |

<br>
<h4 style="color:orange;">table - <b><i>miner</i></b></h4>
<p>tracks individual miners, including their mining power, contributions, and rewards.</p>

| miner  | blocks mined | bytes | submitted proposals | successful proposals |   voted fee   | proposal vote | last prop vote time | last byte adj time | start time | last claim | unredeemed shares |
| :----- | :----------: | :---: | :-----------------: | :------------------: | :-----------: | :-----------: | :-----------------: | :----------------: | :--------: | :--------: | :---------------: |
| miner1 |      10      | 1000  |          1          |          1           | 0.0100 ECACHE |       1       |     1609459200      |     1609462800     | 1609459200 | 1609462800 |       10.0        |
| miner2 |      20      | 2000  |          2          |          2           | 0.0200 ECACHE |       2       |     1609462800      |     1609466400     | 1609462800 | 1609466400 |       20.0        |

<br>
<h4 style="color:orange;">table - <b><i>proposals</i></b></h4>
<p>tracks imported eosio.msig proposals that are pending approval to upgrade contract abi & wasm (code).</p>

| prop timestamp | votes | proposer  | eosio msig name | leading blocks |                     sha256 hash                      |      memorandum       |
| :------------- | :---: | :-------: | :-------------: | :------------: | :--------------------------------------------------: | :-------------------: |
| 1609459200     | 1000  | proposer1 |      msig1      |       10       | 2c26b46b68ffc68ff99b453c1d30413413422d706e6f9b5b94e7 | upgrade contract code |

<br><br><br>
<h2 style="text-align:center;color:orange;font-size: 30px;"><b><u>actions glossary</u></b></h2>

<br>
<h4 style="color:orange;">action - <b><i>bios</i></b></h4>
<p>initializes the antelopECACHE system, setting foundational configurations and permissions.</p>

<h4 style="color:orange;">action - <b><i>enlist</i></b></h4>
<p>allows a miner to enlist in the ECACHE mining process.</p>

<h4 style="color:orange;">action - <b><i>incoming_transfer</i></b></h4>
<p>handles incoming WAX token transfers, adjusting ECACHE mining balances accordingly.</p>

<h4 style="color:orange;">action - <b><i>reducemining</i></b></h4>
<p>enables miners to reduce their mining power by relinquishing bytes.</p>

<h4 style="color:orange;">action - <b><i>claim</i></b></h4>
<p>allows miners to claim their earned ECACHE rewards.</p>

<h4 style="color:orange;">action - <b><i>devclaim</i></b></h4>
<p>allows the developer to claim core tokens derived from mining.</p>

<h4 style="color:orange;">action - <b><i>redeem</i></b></h4>
<p>converts miners' earned shares into transferable ECACHE tokens.</p>

<h4 style="color:orange;">action - <b><i>mineblock</i></b></h4>
<p>triggers the mining of a new block, distributing rewards to miners.</p>

<h4 style="color:orange;">action - <b><i>votefee</i></b></h4>
<p>allows miners to vote on the transaction fee structure.</p>

<h4 style="color:orange;">action - <b><i>voteprop</i></b></h4>
<p>enables miners to vote for or against active proposals.</p>

<h4 style="color:orange;">action - <b><i>proposemsig</i></b></h4>
<p>allows miners to propose modifications to the contract via eosio.msig proposals.</p>

<h4 style="color:orange;">action - <b><i>propreview</i></b></h4>
<p>reviews and executes a scheduled proposal based on votes.</p>

<h4 style="color:orange;">action - <b><i>transfer</i></b></h4>
<p>enables the transfer of ECACHE tokens between accounts.</p>

<h4 style="color:orange;">action - <b><i>open</i></b></h4>
<p>opens a new account for managing ECACHE balances, with ram costs covered by the ram_payer.</p>

<h4 style="color:orange;">action - <b><i>close</i></b></h4>
<p>closes an ECACHE account, refunding the ram costs.</p>

<h4 style="color:orange;">action - <b><i>whitelist</i></b></h4>
<p>adds a list of accounts to the whitelist, allowing them to interact with the contract.</p>

<h4 style="color:orange;">action - <b><i>blocklog</i></b></h4>
<p>generates a log for each mined block, recording various metrics and data.</p>

<br>
<h6 style="color:orange;font-size: 18px;">with caffeinated ❤️,</h6>

<h3 style="color:orange;"><b><i>-lc</i></b></h3>
<br>

