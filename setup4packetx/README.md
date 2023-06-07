Setup for PacketX
=================

Environment setup
-------------------------------------------------------
````
apt install -y python3-pip python3-setuptools python3-wheel ninja-build build-essential flex bison git cmake libsctp-dev libgnutls28-dev libgcrypt-dev libssl-dev libidn11-dev libmongoc-dev libbson-dev libyaml-dev libnghttp2-dev libmicrohttpd-dev libcurl4-gnutls-dev libnghttp2-dev libtins-dev libtalloc-dev meson curl gpg
````

install Mongodb
----------------------------------
````
#for some CPU only suport mongdb4.4
echo "deb http://security.ubuntu.com/ubuntu focal-security main" | sudo tee /etc/apt/sources.list.d/focal-security.list
apt update
apt-get install libssl1.1
rm /etc/apt/sources.list.d/focal-security.list

curl -fsSL https://pgp.mongodb.com/server-4.4.asc |    sudo gpg -o /usr/share/keyrings/mongodb-server-4.4.gpg    --dearmor
echo "deb [ arch=amd64,arm64 signed-by=/usr/share/keyrings/mongodb-server-4.4.gpg ] https://repo.mongodb.org/apt/ubuntu focal/mongodb-org/4.4 multiverse" | sudo tee /etc/apt/sources.list.d/mongodb-org-4.4.list
apt update
apt-get install -y mongodb-org
systemctl start mongod
systemctl enable mongod
````

build and install open5gs
-------------------------
````
cd open5gs
meson build --prefix=/usr/local/open5gs
ninja -C build
cd build
ninja install
````

install webui
--------------
````
curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
apt install -y nodejs
cp open5gs/webui /usr/local/open5gs/ -R
````

install as services
----------------
````
cp services/open5gs-* /lib/systemd/system/
````

ip addr setup
-------------
- modify /usr/local/open5gs/etc/open5gs/amf.yaml ngap addr 
- modify /usr/local/open5gs/etc/open5gs/ufp.yaml gtpu addr
- modify /lib/systemd/system/open5gs-webui.service HOSTNAME

plmn setup to 46666 for private test
------------------------------------
- modify /usr/local/open5gs/etc/open5gs/amf.yaml 
- modify /usr/local/open5gs/etc/open5gs/mme.yaml 

start services
---------------
````
systemctl start open5gs-nrfd.service
systemctl start open5gs-scpd.service
systemctl start open5gs-amfd.service
systemctl start open5gs-smfd.service
systemctl start open5gs-upfd.service
systemctl start open5gs-ausfd.service
systemctl start open5gs-udmd.service
systemctl start open5gs-pcfd.service
systemctl start open5gs-nssfd.service
systemctl start open5gs-bsfd.service
systemctl start open5gs-udrd.service
systemctl start open5gs-mmed.service
systemctl start open5gs-sgwcd.service
systemctl start open5gs-sgwud.service
systemctl start open5gs-hssd.service
systemctl start open5gs-pcrfd.service
systemctl start open5gs-webui.service
````

stop services
-------------
````
systemctl stop open5gs-nrfd.service
systemctl stop open5gs-scpd.service
systemctl stop open5gs-amfd.service
systemctl stop open5gs-smfd.service
systemctl stop open5gs-upfd.service
systemctl stop open5gs-ausfd.service
systemctl stop open5gs-udmd.service
systemctl stop open5gs-pcfd.service
systemctl stop open5gs-nssfd.service
systemctl stop open5gs-bsfd.service
systemctl stop open5gs-udrd.service
systemctl stop open5gs-mmed.service
systemctl stop open5gs-sgwcd.service
systemctl stop open5gs-sgwud.service
systemctl stop open5gs-hssd.service
systemctl stop open5gs-pcrfd.service
systemctl stop open5gs-webui.service
````

enable services
----------------
````
systemctl enable open5gs-nrfd.service
systemctl enable open5gs-scpd.service
systemctl enable open5gs-amfd.service
systemctl enable open5gs-smfd.service
systemctl enable open5gs-upfd.service
systemctl enable open5gs-ausfd.service
systemctl enable open5gs-udmd.service
systemctl enable open5gs-pcfd.service
systemctl enable open5gs-nssfd.service
systemctl enable open5gs-bsfd.service
systemctl enable open5gs-udrd.service
systemctl enable open5gs-mmed.service
systemctl enable open5gs-sgwcd.service
systemctl enable open5gs-sgwud.service
systemctl enable open5gs-hssd.service
systemctl enable open5gs-pcrfd.service
systemctl enable open5gs-webui.service
````

#tun setup
````
ip tuntap add name ogstun mode tun
ip addr add 10.45.0.200/16 dev ogstun
ip link set ogstun up
````

#nat
````
apt install -y iptables

echo 1 > /proc/sys/net/ipv4/ip_forward
iptables -t nat -A POSTROUTING -s 10.45.0.0/16 -o eth1 -j MASQUERADE
iptables -A FORWARD -s 10.45.0.0/16 -o eth1 -j ACCEPT
iptables -A FORWARD -d 10.45.0.0/16 -m state --state ESTABLISH,RELATED -i eth1 -j ACCEPT
````
