# Agora — Production Readiness Roadmap

> **Current assessment: ~15% production ready.**
> The consensus foundation is sound for a prototype. The project is not safe for real funds or public deployment without the work described here. Estimated time to production-grade: 6–18 months of focused engineering depending on team size and scope.

---

## Phase 0 — Foundation (COMPLETE)

Core data model, signing, and consensus in place.

- [x] `Transaction` and `Block` structs with `int64_t` amounts (no floating point)
- [x] secp256k1 ECDSA signing/verification via OpenSSL
- [x] SHA-256 block hashing with canonical preimage serialization
- [x] PoW consensus (leading-zero difficulty check)
- [x] Coinbase / block reward enforcement (exact amount, sentinel signature)
- [x] Transaction ID recomputed and verified during validation
- [x] Double-spend detection within a candidate block (cumulative sender spend tracking)
- [x] Full-chain validation including genesis block check
- [x] Cross-chain duplicate transaction ID detection
- [x] Fork management via `ChainSet`
- [x] Atomic JSON state writes
- [x] 24-test unit suite passing
- [x] GitHub Actions CI (build + test on Ubuntu)
- [x] ASAN/UBSAN sanitizer CI job

---

## Phase 1 — Security Hardening (2–4 months)

Nothing in this phase is optional before handling real funds.

### 1.1 Wallet key encryption
- [ ] KDF-based passphrase encryption for `wallet.dat` (Argon2id + AES-256-GCM recommended)
- [ ] Secure memory clearing of private key material after use
- [ ] Key derivation path or HD wallet support (BIP-32 or similar)

### 1.2 Input hardening
- [ ] P2P message size limits to prevent memory exhaustion
- [ ] Rate limiting / connection throttling on the listener
- [ ] Strict JSON schema validation before deserializing blocks/transactions from peers
- [ ] Malformed hex / invalid-length public key rejection (already partially done; needs fuzzing)

### 1.3 Replay and double-spend protection
- [ ] Switch to a UTXO model (eliminates replay and simplifies balance proof)
  — *or* — add explicit nonce/sequence number per sender address as a cheaper interim fix
- [ ] Verify timestamp ordering policy (currently timestamps are accepted unchecked)

### 1.4 Static and dynamic analysis
- [ ] clang-tidy clean (enable at CI level)
- [ ] cppcheck or equivalent
- [ ] libFuzzer targets on block/transaction deserialization paths
- [ ] Valgrind clean on test suite

---

## Phase 2 — Protocol Stability (2–4 months)

The wire protocol must be frozen before running a multi-node network.

### 2.1 Protocol versioning
- [ ] Add protocol version field to P2P messages
- [ ] Define upgrade/negotiation path for future versions
- [ ] Specify and enforce message framing (length prefix or delimited)

### 2.2 Consensus improvements
- [ ] Dynamic per-block difficulty retarget (simple moving average or ASERT)
- [ ] Merkle tree for transaction commitment within blocks
- [ ] Transaction fee field and fee-based mempool prioritization
- [ ] Coinbase maturity (mined rewards unspendable for N blocks)

### 2.3 Chain selection hardening
- [ ] Switch from longest-chain to most-cumulative-work selection
- [ ] Checkpointing for deep reorganization protection

### 2.4 Networking
- [ ] Peer discovery (DNS seeds or lightweight DHT)
- [ ] Persistent connection management (connection pool, reconnection backoff)
- [ ] Peer exchange messages (share known peers)
- [ ] Connection-level timeouts and dead peer eviction

---

## Phase 3 — Infrastructure (2–4 months)

### 3.1 Storage
- [ ] Replace JSON files with LevelDB or RocksDB
- [ ] Block index for fast hash → block lookup
- [ ] UTXO set cache for O(1) balance queries
- [ ] Chain pruning / snapshot support

### 3.2 RPC / API
- [ ] JSON-RPC or REST API for wallet operations, chain queries, mempool inspection
- [ ] Authentication for the RPC interface
- [ ] Client library or CLI that speaks the RPC API

### 3.3 Transport security
- [ ] TLS on P2P connections (or Noise Protocol Framework)
- [ ] Peer identity / pubkey pinning (optional but recommended for private networks)

### 3.4 Observability
- [ ] Structured logging (JSON lines, configurable level)
- [ ] Metrics endpoint (Prometheus or similar): block height, peer count, mempool size, hash rate
- [ ] Health check endpoint

### 3.5 Testing
- [ ] Property-based testing (e.g., libcheck or a C++ PBT library) for consensus invariants
- [ ] Network simulation tests (multi-node fork, partition/remerge)
- [ ] Benchmarks for block validation throughput

---

## Phase 4 — Security Audit and Graduated Rollout (2–4 months)

No amount of internal review replaces independent audit before handling real funds.

- [ ] Independent cryptographic implementation review (focus: signing, hashing, key handling)
- [ ] Independent consensus safety review (focus: double-spend, fork handling, DoS surface)
- [ ] Independent network security review (focus: P2P parsing, resource exhaustion)
- [ ] Penetration test against a deployed testnet
- [ ] Bug bounty program (private, then public)
- [ ] Testnet operation for ≥ 3 months before mainnet
- [ ] Graduated mainnet rollout with transaction value limits and circuit breakers
- [ ] On-call runbook and incident response process

---

## Summary

| Phase | Status | Estimated Duration |
|---|---|---|
| 0 — Foundation | Complete | — |
| 1 — Security Hardening | Not started | 2–4 months |
| 2 — Protocol Stability | Not started | 2–4 months |
| 3 — Infrastructure | Not started | 2–4 months |
| 4 — Audit & Rollout | Not started | 2–4 months |

**Realistic production timeline: 8–16 months** with a small dedicated team, assuming scope is limited to a private/permissioned deployment. A public, permissionless mainnet requires more.

The single highest-leverage action before any real-world use is **Phase 1.1 (wallet key encryption)** — plaintext private key storage is an unconditional blocker.
