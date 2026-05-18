# Agora

> **Prototype warning:** Agora is experimental educational software. Do **not** use it for real funds, public fundraising, or production cryptocurrency deployment without major additional engineering, threat modeling, and independent security review.

A cryptocurrency node + wallet with:

- **Wallet & CLI** (create, view balance, send)
- **Pseudonyms** for recipients (human names → public keys)
- **Transactions + Mempool** with basic validation
- **Blocks + Mining** (CPU PoW) and chain validation
- **Fork support** via a **ChainSet** (selects the best chain)
- **P2P** broadcast/listen (simple TCP), optional peer file
- **Dynamic difficulty (ASERT)** per-block retarget (config in code)

---

## Build

### macOS (Homebrew)

```bash
brew install nlohmann-json openssl
make clean && make
```

If Homebrew is in a non-standard prefix, update `INCLUDES`/`LDFLAGS` in the Makefile or run:

```bash
make CXXFLAGS="-std=c++17 -Wall -Wextra -Wno-deprecated-declarations -pthread -I/opt/homebrew/include"      LDFLAGS="-L/opt/homebrew/lib -lssl -lcrypto"
```

### Linux

Install OpenSSL & JSON headers via your package manager (names vary):

```bash
# Ubuntu / Debian
sudo apt-get install -y build-essential libssl-dev nlohmann-json3-dev
make clean && make
```

---

## Binaries

After `make`, you’ll have:

- `agora` — wallet CLI (view balance, send by pseudonym)
- `create_wallet` — generate a new wallet
- `miner` — simple miner (reads mempool, mines blocks, broadcasts)
- `listener` — P2P listener/relay

---

## Files & directories

```

pseudonyms.json     # pseudonym -> public key (hex; 02/03 compressed OR 04 uncompressed)
mempool.json          # local mempool (created at runtime)
blockchains.json      # ChainSet (multiple forks); main chain chosen from here
ips.dat               # optional list of peers (ip:port, one per line)
wallet.dat            # your wallet (private/public key)
```

> Paths are **relative to your working directory**. Run from the repo root unless you pass absolute paths.

---

## Quick start

### 1) Create a wallet

```bash
./create_wallet
# => creates wallet.dat (or prompts for a path)
```

### 2) Add contacts (pseudonyms)

Edit `pseudonyms.json` (create the file if missing):

```json
{
  "george": "049188620ac9452a744add880083f2f1c538bde25af3a1804a08506cfbf7dd2667da7eba730e14e53214e9836291c5813c399968d0983b806bfc49bc4664516b86",
  "sara":   "045188620ac9452a744add880083f2f1c538bde25af3a1804a08506cfbf7dd2667da7eba730e14e53214e9836291c5813c399968d0983b806bfc49bc4664a12d92"
}
```

- Accepts **compressed** secp256k1 keys (`02..` or `03..`, 66 hex) **and** **uncompressed** (`04..`, 130 hex).
- The wallet UI **only accepts pseudonyms at the prompt**; raw pubkeys must be in this file.

> Tip: You can hot‑reload this file when sending if your code calls `resolver.reload()`; otherwise restart the app after editing.

### 3) Start the listener (P2P receiver)

```bash
./listener
```

This opens the TCP listener (default `DEFAULT_PORT` from code) and relays tx/blocks.  
If you have peers, list them in `ips.dat` (one `ip:port` per line).

### 4) Start the miner (optional, for a local chain)

```bash
./miner
```

The miner pulls transactions from `mempool.json`, mines blocks, and broadcasts them.  
Difficulty targets are computed per block (ASERT) using your consensus params.

### 5) Use the wallet

```bash
./agora wallet.dat
```

Menu:

```
1. View balance
2. Send transaction (by pseudonym)
3. Exit
```

- **View balance** scans the current main chain from `blockchains.json`.
- **Send**:
  - Enter a **pseudonym** (must exist in `pseudonyms.json`).
  - Enter an amount.
  - The wallet resolves the pseudonym → public key, builds & signs the tx, appends to `mempool.json`, and **broadcasts**.

List mempool contents without opening the wallet UI:

```bash
./agora --list
```

---

## Forks & chain selection

- The node stores **multiple forks** in `blockchains.json` via a `ChainSet`.
- On each new block, it validates and inserts into the appropriate fork.
- The **main chain** is selected by the “best” criterion (height / cumulative work—see your `ChainSet` logic).
- Wallet balance is computed against the **current main chain**.

> To test forks locally: run two miners pointing at different peers or temporarily block broadcasts so each mines its own next block, then reconnect. The `ChainSet` should keep both forks and pick the better tip when they meet.

---

## Configuration (in code)

- **Consensus**: block time target, ASERT half-life, max target, coinbase amount/maturity.
- **Networking**: default port, peer file (`ips.dat`).
- **Paths**: filenames shown above; change in code if you want a data dir.

---

## Troubleshooting

- **“Unknown pseudonym”**  
  Run from the repo root (so `pseudonyms.json` is found), ensure valid JSON. If you edit while running, call `resolver.reload()` before lookup or restart.

- **Can’t connect / no broadcasts**  
  Likely a firewall/NAT. On macOS you may need to allow incoming connections for your binary. For home networks, enable port forwarding or run everything on localhost.

- **Build errors about `nlohmann/json.hpp`**  
  Install the header: `brew install nlohmann-json` (macOS) or `apt install nlohmann-json3-dev` (Debian/Ubuntu). Ensure your Makefile includes the Homebrew include/lib paths.

- **OpenSSL link errors**  
  Make sure `-lssl -lcrypto` are linked, and paths include your Homebrew or system OpenSSL dirs. Using `pkg-config` can help:
  ```
  pkg-config --cflags --libs openssl
  ```

---

## Safety notes

- Wallet keys are stored locally in `wallet.dat` as plaintext key material. The code now tries to set restrictive POSIX permissions, but there is no passphrase/KDF encryption yet.
- Do not expose the listener to the public internet; the P2P layer is still a simple prototype protocol.
- Pseudonyms are local to **your** machine (not a global registry).

---

## License
MIT
