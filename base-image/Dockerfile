FROM ubuntu:latest

SHELL ["/bin/bash", "-c"]

WORKDIR /home

# APT dependencies
RUN apt-get update --fix-missing
RUN apt-get install -y \
    build-essential    \
    openssh-server     \
    cmake              \
    git                \
    pkg-config         \
    libssl-dev         \
    libsasl2-dev       \
    libc6-dbg          \
    gdb                \
    valgrind

RUN apt update
RUN apt install -y \
    curl           \
    wget           \
    netbase

RUN source ~/.bashrc

# Boost
RUN wget http://downloads.sourceforge.net/project/boost/boost/1.84.0/boost_1_84_0.tar.gz
RUN tar -zxvf boost_1_84_0.tar.gz
WORKDIR /home/boost_1_84_0
RUN cpuCores=`cat /proc/cpuinfo | grep "cpu cores" | uniq | awk '{print $NF}'`
RUN ./bootstrap.sh
RUN ./b2 --with=all -- -j $cpuCores install
WORKDIR /home
RUN rm boost_1_84_0.tar.gz
RUN rm -rf ./boost_1_84_0

# Mongocxx
RUN curl -OL https://github.com/mongodb/mongo-cxx-driver/releases/download/r3.10.1/mongo-cxx-driver-r3.10.1.tar.gz
RUN tar -xzf mongo-cxx-driver-r3.10.1.tar.gz
WORKDIR /home/mongo-cxx-driver-r3.10.1/build
RUN cmake ..                   \
    -DCMAKE_BUILD_TYPE=Release \
    -DBSONCXX_POLY_USE_BOOST=1 \
    -DMONGOCXX_OVERRIDE_DEFAULT_INSTALL_PREFIX=OFF
RUN cmake --build . --target install
WORKDIR /home
RUN rm mongo-cxx-driver-r3.10.1.tar.gz
RUN rm -rf ./mongo-cxx-driver-r3.10.1

# AMQP-CPP
RUN git clone https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git
RUN mkdir -p ./AMQP-CPP/build
WORKDIR /home/AMQP-CPP/build
RUN cmake .. -DAMQP-CPP_BUILD_SHARED=ON -DAMQP-CPP_LINUX_TCP=ON
RUN cmake --build . --target install
WORKDIR /home
RUN rm -rf ./AMQP-CPP

# CRYPTOPP
RUN git clone https://github.com/weidai11/cryptopp.git
WORKDIR /home/cryptopp
RUN make
RUN make test
RUN make install
WORKDIR /home
RUN rm -rf ./cryptopp

RUN ldconfig

