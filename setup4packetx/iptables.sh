#!/bin/sh

echo 1 > /proc/sys/net/ipv4/ip_forward

iptables -P INPUT ACCEPT
iptables -P FORWARD ACCEPT
iptables -P OUTPUT ACCEPT
iptables -t nat -F
iptables -t mangle -F
iptables -F
iptables -X

iptables -t nat -A POSTROUTING -s 10.45.0.0/16 -o eth1 -j MASQUERADE
iptables -A FORWARD -s 10.45.0.0/16 -o eth1 -j ACCEPT
iptables -A FORWARD -d 10.45.0.0/16 -m state --state ESTABLISH,RELATED -i eth1 -j ACCEPT
