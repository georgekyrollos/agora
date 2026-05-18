# CLAUDE.md

This file provides guidance to Claude Code when working with this repository.

## Project Overview

Agora is a prototype peer-to-peer cryptocurrency in C++17. It is educational/prototype software and is **not production-ready** for real funds.

## Build Commands

```bash
make
make clean
make test
make agora
make create_wallet
make miner
make listener
```

Use strict local checks when hardening consensus code:

```bash
make clean && make && make test
```

## Runtime Notes

- `./agora <wallet.dat>`: wallet/balance/send flow
- `./agora --list`: view mempool
- `./create_wallet`: create a local wallet file
- `./miner`: mine blocks from mempool/runtime state
- `./listener`: listen for peer messages

Do not run long-lived listeners/miners during automated checks unless wrapped in `timeout`.
Do not expose this prototype to the public internet.

## Architecture

- Core data model: `Transaction`, `Block`, `ChainSet`, `Wallet`
- Consensus/validation: `validate.cpp` / `validate.hpp`
- Transaction and hashing helpers: `transaction.cpp`, `crypto.cpp`
- Pseudonym resolution: `pseudonym_resolver.*`
- Local state: JSON files for chains, mempool, peers, pseudonyms
- P2P: simple TCP broadcast/listen layer
- Crypto: secp256k1 ECDSA through OpenSSL

## Hardening Priorities

- Do not use floating point for currency amounts.
- Do not suppress compiler warnings.
- Enforce positive transaction amounts.
- Enforce exact block rewards / coinbase rules.
- Use canonical serialization for transaction signing, transaction IDs, and block hashing.
- Recompute transaction IDs during validation.
- Check cumulative sender spends inside a candidate block.
- Validate genesis block during full-chain validation.
- Enforce block index progression.
- Validate malformed hex/signature/public-key inputs defensively.
- Use atomic writes or a real database for runtime state.
- Treat wallet private-key files as sensitive secrets.

## Git Expectations

- Keep generated binaries, object files, wallet files, and runtime JSON state out of git.
- Run build/tests before committing.
- Do not force-push or overwrite remote history without explicit operator instruction.
