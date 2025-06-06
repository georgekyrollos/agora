CXX = g++
CXXFLAGS = -std=c++17 -w
LDFLAGS = -lssl -lcrypto
INCLUDES = -I./

SRC = \
	main.cpp \
	wallet.cpp \
	crypto.cpp \
	transaction.cpp \
	mempool.cpp \
	block.cpp \
	blockchain.cpp

CREATE_WALLET_SRC = \
	create_wallet.cpp \
	wallet.cpp \
	crypto.cpp

MINER_SRC = \
	miner.cpp \
	mempool.cpp \
	block.cpp \
	blockchain.cpp \
	wallet.cpp \
	transaction.cpp \
	crypto.cpp

VALIDATE_SRC = \
	validate.cpp \
	blockchain.cpp \
	block.cpp \
	transaction.cpp \
	crypto.cpp

OBJS = $(SRC:.cpp=.o)
CREATE_WALLET_OBJS = $(CREATE_WALLET_SRC:.cpp=.o)
MINER_OBJS = $(MINER_SRC:.cpp=.o)
VALIDATE_OBJS = $(VALIDATE_SRC:.cpp=.o)

all: agora create_wallet miner validate

agora: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

create_wallet: $(CREATE_WALLET_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

miner: $(MINER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

validate: $(VALIDATE_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f *.o agora create_wallet miner

.PHONY: all clean
