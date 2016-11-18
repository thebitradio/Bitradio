
Bitradio is a PoS-based cryptocurrency.

Bitradio uses libsecp256k1, libgmp, Boost1.55, OR Boost1.57,  Openssl1.01p, Berkeley DB 4.8, QT5 to compile

Block Spacing: 2 minutes
Stake Minimum Age: 8 Hours

P2P Port: 32450
RPC Port: 32451


BUILD LINUX
-----------
1) git clone https://github.com/thebitradio/Bitradio

2) cd Bitradio/src

3) make -f makefile.unix            # Headless

(optional)

4) strip Bitradiod

5) sudo cp Bitradiod /usr/local/bin
