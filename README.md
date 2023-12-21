# Black-Nurse-Attack
Traditional ICMP flooding attack floods target hosts with massive echo request packets [Type 8, Code 0]. In contrast, TDC Security Operations Center has discovered in 2016 another type of ICMP based attack known as Black-Nurse attack that explicitly targets firewalls and routers.

The BlackNurse attack is considered as a special type of ICMP attack that depends on sending low volume traffic of specific ICMP error messages [Type: 3 (Destination unreachable), Code: 3 (Port unreachable)]. This low volume DDoS attack is effective because the objective is not to flood the firewall with illegitimate traffic, but rather to craft packets that consume firewall resources and drive high CPU workload. Usually, firewalls with single CPU are more likely to be vulnerable to this attack than firewalls with multicore.

Practically, to generate the BlackNurse attack as illustrated in, any network packet generator tool can be used. For example, the following Hping3 tool’s online commands allow to generate the attack:
# hping3 -1 –C 3 –K 3 –i u20 dest-ip.
This command sends ICMP port unreachable message [Type: 3, Code: 3] to the target dest-ip, where –i u20 sends one packet every 20 ms.
# hping3 -1 –C 3 –K 3 –flood dest-ip
While this command floods the target dest-ip with ICMP port unreachable message [Type: 3, Code: 3].

Experiments are conducted to generate the BlackNurse attack using only 7K packets per second and show its impact on commercial grades, Juniper NetScreen SSG 20, and Cisco ASA 5540 firewalls.
