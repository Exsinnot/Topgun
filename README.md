# Topgun

```bash
    sudo apt-get update

    sudo apt-get install build-essential cmake libssl-dev

    git clone https://github.com/eclipse/paho.mqtt.c.git
    cd paho.mqtt.c

    cmake -Bbuild -H.
    cmake --build build
    sudo cmake --install build
    sudo apt-get install libcurl4-openssl-dev
    sudo apt-get install libasound2-dev

    gcc -o testcode MQTTPUBANDSUB.c -lasound -lcurl -pthread -lpaho-mqtt3c

    export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
    echo 'export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
    source ~/.bashrc
    ldd ./testcode

```
