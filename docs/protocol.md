# Agora Protocol Specification

> **Status: PROTOTYPE / UNSTABLE**
> Every section of this document describes the current prototype implementation.
> Field names, encoding, and wire format may change without notice.
> Do not build production software that depends on this specification.

---

## 1. Overview

Agora is a simple account-balance blockchain with CPU Proof-of-Work (PoW) and peer-to-peer broadcasting over raw TCP.

Key parameters (compile-time constants, not negotiated at runtime):

| Parameter | Value | Source |
|---|---|---|
| PoW difficulty | 6 leading zero hex digits | `block.hpp` `DIFFICULTY` |
| Max transactions per block | 5 | `block.hpp` `MAX_TXS_PER_BLOCK` |
| Block reward (coinbase) | 10 (integer smallest-unit) | `chainset.hpp` `BLOCK_REWARD` |
| Default P2P port | 8080 | `p2p.hpp` `DEFAULT_PORT` |
| Currency unit | Integer (`int64_t`), no floating point | `transaction.hpp` |

---

## 2. Data Structures

### 2.1 Transaction

> **UNSTABLE**: JSON field names differ from in-memory struct field names. See note below.

```
fromPublicKeyHex  — sender secp256k1 public key, hex-encoded
                    compressed (02/03 prefix, 66 hex chars) or
                    uncompressed (04 prefix, 130 hex chars)
toPublicKeyHex    — recipient public key, same encoding
amount            — int64_t, strictly positive (> 0) for normal transactions
signatureHex      — DER-encoded ECDSA signature over the sign message, hex-encoded
ts                — timestamp string (wall-clock, not validated for ordering)
id                — SHA-256(fromPublicKeyHex || toPublicKeyHex || amount_string || ts)
```

**Serialized sign message** (what is actually signed):

```
fromPublicKeyHex + "|" + toPublicKeyHex + "|" + to_string(amount)
```

**Transaction ID computation:**

```
SHA-256( fromPublicKeyHex || toPublicKeyHex || to_string(amount) || ts )
```

**Wire / JSON encoding** (keys differ from struct field names):

```json
{
  "id":        "<sha256 hex>",
  "sender":    "<fromPublicKeyHex>",
  "receiver":  "<toPublicKeyHex>",
  "amount":    <int64>,
  "signature": "<signatureHex>",
  "timestamp": "<ts>"
}
```

**Coinbase transaction** (one per block, must be index 0):

```
fromPublicKeyHex = "COINBASE"
signatureHex     = "reward"
amount           = BLOCK_REWARD (10)
id               = SHA-256("COINBASE" || toPublicKeyHex || "10" || ts)
```

### 2.2 Block

```
index        — int, monotonically increasing from 0
timestamp    — string (wall-clock)
transactions — ordered list of Transaction; first must be coinbase for index > 0
previousHash — hex SHA-256 of the previous block; "0" for genesis
nonce        — int, incremented during mining
hash         — SHA-256 of the block preimage (see below)
```

**Block hash preimage** (concatenated as a string, no delimiter):

```
to_string(index) + timestamp + previousHash + to_string(nonce)
  + for each tx: fromPublicKeyHex + toPublicKeyHex + to_string(amount) + signatureHex
```

Hash is computed as `SHA-256` of that UTF-8 string (via OpenSSL `SHA256`).

**Wire / JSON encoding:**

```json
{
  "index":        <int>,
  "timestamp":    "<string>",
  "transactions": [ <Transaction>, ... ],
  "previousHash": "<hex>",
  "nonce":        <int>,
  "hash":         "<sha256 hex>"
}
```

---

## 3. Consensus Rules

> **UNSTABLE**: Consensus parameters are compile-time constants. There is no on-wire negotiation.

### 3.1 Proof of Work

A block is valid only if:

```
block.hash.substr(0, DIFFICULTY) == string(DIFFICULTY, '0')
```

At `DIFFICULTY = 6`, the hash must start with `000000`. Nonce is incremented until this holds.

### 3.2 Block Validation

**Genesis block (index == 0):**
- `previousHash` must be `"0"`
- Hash must satisfy PoW difficulty
- Must contain exactly one transaction, which must be a valid coinbase

**Subsequent blocks:**
- `index == previousBlock.index + 1`
- `previousHash == previousBlock.hash`
- Hash must satisfy PoW difficulty
- First transaction must be a valid coinbase (exactly one coinbase)
- All other transactions must be normal (non-coinbase)
- No duplicate transaction IDs within the block
- Each sender's cumulative spend in the block must not exceed their chain balance

### 3.3 Transaction Validation

- Transaction ID must match recomputed value
- For normal transactions: `amount > 0`, signature valid, sender balance >= amount
- For coinbase: `signatureHex == "reward"`, `amount == BLOCK_REWARD`

### 3.4 Chain Validation

Full-chain validation (`validateBlockchain`):
- Genesis block validated first
- Transaction IDs must be globally unique across the entire chain
- Each block validated against the prefix of the chain up to that point

### 3.5 Chain Selection

> **UNSTABLE**: Selection logic is minimal and may change.

The `ChainSet` stores multiple fork candidates. The "main chain" (index 0 in the set) is preferred. `tryReplaceMainChain` replaces the current main chain if the candidate is longer.

---

## 4. Cryptography

> All cryptographic operations go through OpenSSL.

- **Key type**: secp256k1 ECDSA (same curve as Bitcoin)
- **Signing**: DER-encoded ECDSA signature
- **Key format**: raw hex (not PEM, not WIF); both compressed (66 hex) and uncompressed (130 hex) public keys accepted
- **Hashing**: SHA-256 (OpenSSL `SHA256`)
- **Key storage**: plaintext in `wallet.dat` with restrictive POSIX permissions; **no passphrase encryption**

> **Security warning**: plaintext private key storage is not acceptable for production. KDF-based encryption (Argon2 + AES-256-GCM or similar) is required before handling real funds.

---

## 5. Wire Protocol

> **UNSTABLE**: The P2P protocol has no versioning, no authentication, and no message framing beyond a type prefix.

### 5.1 Transport

Raw TCP on `DEFAULT_PORT = 8080`. No TLS. No peer authentication.

### 5.2 Message Format

Each message is a type flag followed by a newline, then JSON:

```
TX\n<transaction JSON>\n
BK\n<block JSON>\n
```

The listener reads the full message and dispatches on the `TX` or `BK` prefix.

### 5.3 Peer Discovery

> **UNSTABLE**: No automatic peer discovery. Peers are configured manually.

Peers are listed in `ips.dat`, one `ip:port` per line. `broadcastMessage` iterates this list and opens a new TCP connection for each send. There is no persistent connection management, no gossip protocol, and no peer exchange.

---

## 6. State Files

All runtime state is persisted as JSON. Writes use atomic rename (`sync.cpp`).

| File | Contents |
|---|---|
| `blockchains.json` | `ChainSet` — array of chains, each an array of blocks |
| `mempool.json` | Array of pending transactions |
| `wallet.dat` | `{"private_key": "<hex>", "public_key": "<hex>"}` |
| `pseudonyms.json` | `{"name": "<pubkey hex>", ...}` — local only, not broadcast |
| `ips.dat` | Newline-separated `ip:port` peer list |

---

## 7. Known Limitations (Prototype)

These are not bugs — they are deliberate scope boundaries for this prototype:

- No UTXO model; balance is computed by scanning the entire chain for each query
- No Merkle tree; transaction commitment is via hash preimage concatenation
- No transaction fees or mempool fee prioritization
- No dynamic per-block difficulty retarget
- No replay protection beyond transaction ID uniqueness
- No message size limits or rate limiting on the P2P layer
- No peer discovery or peer exchange
- No versioning or protocol negotiation
- Timestamp ordering is not validated
- State is JSON files; not suitable for large chain lengths or concurrent access
