# Makefile for Agora (Linux / Mac)
CXX = g++
# -Wno-deprecated-declarations silences OpenSSL 3.0 EC_KEY deprecations.
# The legacy EC_KEY API still works; migrating to EVP_PKEY is future work.
CXXFLAGS = -std=c++17 -Wall -Wextra -Wno-deprecated-declarations -pthread
LDFLAGS = -lssl -lcrypto

# Detect Homebrew prefix if present (Mac)
HOMEBREW_PREFIX := $(shell brew --prefix 2>/dev/null)
INCLUDES = -I./
ifneq ($(HOMEBREW_PREFIX),)
  INCLUDES += -I$(HOMEBREW_PREFIX)/include
  LDFLAGS  += -L$(HOMEBREW_PREFIX)/lib
else
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
	sync.cpp \
	message.cpp

# Test sources compiled separately with AGORA_DIFFICULTY=1 for fast PoW in unit tests
TEST_SRC = \
	test_blockchain.cpp \
	blockchain.cpp \
	block.cpp \
	transaction.cpp \
	crypto.cpp \
	validate.cpp \
	chainset.cpp \
	mempool.cpp \
	wallet.cpp \
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

# Compile test binary fresh each time with AGORA_DIFFICULTY=1 to avoid .o conflicts
test_blockchain: $(TEST_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -DAGORA_DIFFICULTY=1 -o $@ $^ $(LDFLAGS)

test: test_blockchain
	./test_blockchain

# Sanitizer build — ASAN + UBSAN on the test suite only; never linked into release binaries.
# Uses a separate output binary so it does not conflict with normal test_blockchain .o files.
test-san: $(TEST_SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -DAGORA_DIFFICULTY=1 \
	    -fsanitize=address,undefined -fno-omit-frame-pointer \
	    -o test_blockchain_san $^ $(LDFLAGS)
	./test_blockchain_san
	rm -f test_blockchain_san

clean:
	rm -f *.o agora create_wallet miner listener test_blockchain test_blockchain_san

.PHONY: all test test-san clean
