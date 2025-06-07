CXX = g++
CXXFLAGS = -std=c++17 -w -pthread
LDFLAGS = -lssl -lcrypto
INCLUDES = -I./

SRC = \
	main.cpp \
	wallet.cpp \
	crypto.cpp \
	transaction.cpp \
	mempool.cpp \
	block.cpp \
	blockchain.cpp \
	chainset.cpp \
	validate.cpp \
	sync.cpp \
	message.cpp \
	p2p.cpp

CREATE_WALLET_SRC = \
	create_wallet.cpp \
	wallet.cpp \
	crypto.cpp \
	sync.cpp

MINER_SRC = \
	miner.cpp \
	mempool.cpp \
	block.cpp \
	blockchain.cpp \
	chainset.cpp \
	wallet.cpp \
	transaction.cpp \
	crypto.cpp \
	validate.cpp \
	sync.cpp \
	message.cpp \
	p2p.cpp \
	handlers.cpp

LISTENER_SRC = \
	listener.cpp \
	p2p.cpp \
	handlers.cpp \
	validate.cpp \
	transaction.cpp \
	block.cpp \
	chainset.cpp \
	mempool.cpp \
	crypto.cpp \
	blockchain.cpp \
	sync.cpp


OBJS = $(SRC:.cpp=.o)
CREATE_WALLET_OBJS = $(CREATE_WALLET_SRC:.cpp=.o)
MINER_OBJS = $(MINER_SRC:.cpp=.o)
LISTENER_OBJS = $(LISTENER_SRC:.cpp=.o)

all: agora create_wallet miner listener

agora: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

create_wallet: $(CREATE_WALLET_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

miner: $(MINER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

listener: $(LISTENER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f *.o agora create_wallet miner listener

.PHONY: all clean

