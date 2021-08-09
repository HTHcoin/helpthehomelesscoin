Help The Homeless DIP3 MN setup to run on local machine or VPS

Open local wallets debug window and run these two commands:

bls generate   **Save this to text file**
masternode outputs  **Save this to text file**


Copy the results to a textfile
Add the secret bls key and your outbound IP to helpthehomeless.conf:
outbound IP can be found at whatsmyip.com & is your IPv4 Address
If you set up masternode on a VPS you use its IP and the secret bls key from your controlling wallet in the helpthehomeless.conf on VPS

```
masternodeblsprivkey=  secret key from bls generate
masternode=1
listen=1
externalip=       Outbound IP Address
port=65000
```

Restart the wallet, or daemon if it is on VPS. 
Make sure to open port 65000 in firewall.
For multiple masternodes on one VPS make a copy of the .helpthehomeless directory, edit the new directory's helpthehomeless.conf, change IP, bls key, RPC port and credentials. Add listen=0 and save. Currently each masternode need an unique IPV4 address.
Start the second daemon with flags helpthehomelessd -datadir=/home/USER/.helpthehomeless2 -conf=helpthehomeless.conf -daemon -shrinkdebugfile
Make sure to open ports in firewall.

----
You can also install on VPS using an easy script, https://github.com/HTHcoin/helpthehomelesscoin/blob/master/doc/masternode-script.md

----

Now lets register the node. 

Create an address for collateral, send 1 million HTH to it.
I recommend to generate a new payoutaddress for each masternode for easy tracking of their performance. If you send a little coins for fee to the payoutaddress you can skip the feeSourceAddress.

The first transaction set up like this:

protx register_prepare "collateralHash" collateralIndex "ipAndPort" "ownerKeyAddr" "operatorPubKey" "votingKeyAddr" operatorReward "payoutAddress" "(feeSourceAddress)"


   example

protx register_prepare "2b98808f23bee0c9f4d071163437b263e215d106fba2ee48a500f9f879d305cc" 1 "19.68.74.1:65000" "hH8zz6WnAv7mYHhLFPqF96KmXwUfuBi9Jn" "15ea4a1319cc1b0ec62b5602f086ad1b13bfa53d082e683eac427f45ef7b2cf22d3d5f43f9970fc90691a554f433ae5f" "hg8zz6WnAv7mYHhLFPqF96KmXwUfuBi9Jn"  "0" "hdY4qeyw4yrmJcagaDGSrK4JFCDMSMAbVp"


"collateralHash" & collateralIndex is from masternode outputs,
 generate a new address as ownerKeyAddr,
 "operatorPubKey" is the bls public key,
 "votingKeyAddr" can be the same as owner,
 operatorReward is usually 0.


Important to use "" as above, and usually mysterious errors can relate to a double space somewhere in the transaction.

Example Output of protx register_prepare 
 
{
 ~~~>>>>> "tx": "0300010001becd97e63ae77834f87903404b61cb593aa123ed5d405fa638925ea563932dcf0100000000feffffff01213e7a10f35a00001976a914ac720005ba3b5213cf9c739bccef4e2968588ae788ac00000000d1010000000000cc05d379f8f900a548eea2fb06d115e263b237341671d0f4c9e0be238f80982b0100000000000000000000000000ffffc0a84a01c34f00a411b16a0cd0e9d8cd4840290a709934c15a8b8d8c7e4b88ea82ca6cb7a1de2251635444bb8d7d8288b164837573a7a327a2bb8f450e04ec280f1714564fcaceb2eaef00a411b16a0cd0e9d8cd4840290a709934c15a8b00001976a914ac720005ba3b5213cf9c739bccef4e2968588ae788acd278f0ff0d5eac33417f4172f0c0a30dd084f6a0bf14a11841f5fccc7ccf94ad00",
 ~~~>>>>>> "collateralAddress": "hbXQVUdZtyFtWih9pQB25eAMhAPCF21wGj",
 ~~~>>>>>> "signMessage": "hc3Fe4bb9VEXojfgCVTxDqz24yJzFQVHap|0|hLNqPvbdbrwJYWnpqWn16WpHc73AvjXVWd|hLNqPvbdbrwJYWnpqWn16WpHc73AvjXVWd|a950b0dfffec425ead2173c6b1229dc59ab7963f7df96610bb0280325210d156"
}


The output of the above transaction gives a tx, collateralAddress and a signMessage for the step below.

signmessage collateraladdress "message"


  example

signmessage hbXQVUdZtyFtWih9pQB25eAMhAPCF21wGj "hc3Fe4bb9VEXojfgCVTxDqz24yJzFQVHap|0|hLNqPvbdbrwJYWnpqWn16WpHc73AvjXVWd|hLNqPvbdbrwJYWnpqWn16WpHc73AvjXVWd|a950b0dfffec425ead2173c6b1229dc59ab7963f7df96610bb0280325210d156"

Final step is to register it all with the signature from signmessage and tx from protx register_prepare:

protx register_submit "tx" "sig"


  example

protx register_submit "0300010001becd97e63ae77834f87903404b61cb593aa123ed5d405fa638925ea563932dcf0100000000feffffff01213e7a10f35a00001976a914ac720005ba3b5213cf9c739bccef4e2968588ae788ac00000000d1010000000000cc05d379f8f900a548eea2fb06d115e263b237341671d0f4c9e0be238f80982b0100000000000000000000000000ffffc0a84a01c34f00a411b16a0cd0e9d8cd4840290a709934c15a8b8d8c7e4b88ea82ca6cb7a1de2251635444bb8d7d8288b164837573a7a327a2bb8f450e04ec280f1714564fcaceb2eaef00a411b16a0cd0e9d8cd4840290a709934c15a8b00001976a914ac720005ba3b5213cf9c739bccef4e2968588ae788acd278f0ff0d5eac33417f4172f0c0a30dd084f6a0bf14a11841f5fccc7ccf94ad00" "H0fU8AgAwoRlQHTjKwUzUDmA7mBOFFNY4aUbfEy+0lWjFdvyjmY0ZZgG1ELKUlUF/k7Rjpdfdh8I6bdcRJFwQXE="
