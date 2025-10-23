# Makefile for Macs
CXX = g++
CXXFLAGS = -std=c++17 -w -pthread
LDFLAGS = -lssl -lcrypto

# Detect Homebrew prefix if present
HOMEBREW_PREFIX := $(shell brew --prefix 2>/dev/null)
INCLUDES = -I./
ifneq ($(HOMEBREW_PREFIX),)
  INCLUDES += -I$(HOMEBREW_PREFIX)/include
  LDFLAGS  += -L$(HOMEBREW_PREFIX)/lib
else
  # Fallbacks (common defaults)
  INCLUDES += -I/opt/homebrew/include -I/usr/local/include
  LDFLAGS  += -L/opt/homebrew/lib -L/usr/local/lib
endif

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
	p2p.cpp \
	pseudonym_resolver.cpp

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
