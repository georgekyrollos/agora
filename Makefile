CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
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

OBJS = $(SRC:.cpp=.o)
CREATE_WALLET_OBJS = $(CREATE_WALLET_SRC:.cpp=.o)

all: agora create_wallet

agora: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

create_wallet: $(CREATE_WALLET_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f *.o agora create_wallet

.PHONY: all clean
