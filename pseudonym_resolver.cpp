#include "pseudonym_resolver.hpp"
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <utility>              // std::move
#include <nlohmann/json.hpp>

using std::lock_guard;
using json = nlohmann::json;

PseudonymResolver::PseudonymResolver(string path)
    : path_(std::move(path)) { load_unlocked(); }

string PseudonymResolver::resolveToPubkey(const string& to) const {
    lock_guard<mutex> lk(mu_);
    string trimmed = trim(to);
    if (trimmed.empty()) throw std::runtime_error("empty recipient");

    // If it already looks like a pubkey, accept it (we normalize case/prefix).
    if (isHexPubkey(trimmed)) return normalize_pubkey(trimmed);

    // Otherwise, treat as pseudonym
    string key = normalize_name(trimmed);
    auto it = map_.find(key);
    if (it == map_.end())
        throw std::runtime_error("unknown pseudonym: '" + trimmed + "'");
    if (!isHexPubkey(it->second))
        throw std::runtime_error("invalid pubkey for pseudonym: '" + trimmed + "'");
    return normalize_pubkey(it->second);
}

void PseudonymResolver::set(const string& pseudonym, const string& hex_pubkey) {
    if (!isHexPubkey(hex_pubkey)) throw std::runtime_error("invalid public key hex");
    lock_guard<mutex> lk(mu_);
    map_[normalize_name(pseudonym)] = normalize_pubkey(hex_pubkey);
    persist_unlocked();
}

bool PseudonymResolver::erase(const string& pseudonym) {
    lock_guard<mutex> lk(mu_);
    bool removed = (map_.erase(normalize_name(pseudonym)) > 0);
    if (removed) persist_unlocked();
    return removed;
}

unordered_map<string, string> PseudonymResolver::list() const {
    lock_guard<mutex> lk(mu_);
    return map_;
}

void PseudonymResolver::reload() {
    lock_guard<mutex> lk(mu_);
    load_unlocked();
}

string PseudonymResolver::trim(const string& s) {
    auto a = s.find_first_not_of(" \t\r\n");
    auto b = s.find_last_not_of(" \t\r\n");
    if (a == string::npos) return "";
    return s.substr(a, b - a + 1);
}

string PseudonymResolver::normalize_name(string s) {
    s = trim(s);
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){
        if (c >= 'A' && c <= 'Z') return char(c + 32);
        if (c == ' ') return '_';
        return char(c);
    });
    return s;
}

static inline bool is_hex_char(char c){
    return (c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F');
}

bool PseudonymResolver::isHexPubkey(string s) {
    // Strip 0x prefix
    if (s.rfind("0x",0)==0||s.rfind("0X",0)==0) s = s.substr(2);

    // Accept compressed (33 bytes => 66 hex): 02/03
    if (s.size() == 66 && s[0]=='0' && (s[1]=='2'||s[1]=='3')) {
        for(char c: s) if(!is_hex_char(c)) return false;
        return true;
    }

    // Accept uncompressed (65 bytes => 130 hex): 04
    if (s.size() == 130 && s[0]=='0' && s[1]=='4') {
        for(char c: s) if(!is_hex_char(c)) return false;
        return true;
    }

    return false;
}

string PseudonymResolver::normalize_pubkey(string s) {
    // Strip 0x and lowercase hex (we preserve the leading 02/03/04)
    if (s.rfind("0x",0)==0||s.rfind("0X",0)==0) s = s.substr(2);
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){
        return (c>='A'&&c<='Z') ? char(c+32) : char(c);
    });
    return s;
}

void PseudonymResolver::load_unlocked() {
    map_.clear();
    std::ifstream in(path_);
    if (!in.good()) return; // no file yet is fine
    json j; in >> j;
    if (!j.is_object()) throw std::runtime_error("pseudonyms: invalid JSON root");
    for (auto it=j.begin(); it!=j.end(); ++it) {
        const string pseudo = normalize_name(it.key());
        const string pk     = it.value().get<string>();
        if (!isHexPubkey(pk))
            throw std::runtime_error("pseudonyms: invalid pubkey for '"+it.key()+"'");
        map_.emplace(pseudo, normalize_pubkey(pk));
    }
}

void PseudonymResolver::persist_unlocked() const {
    json j = json::object();
    for (auto& kv : map_) j[kv.first] = kv.second;
    std::ofstream out(path_, std::ios::trunc);
    if (!out.good()) throw std::runtime_error("pseudonyms: cannot write file");
    out << j.dump(2) << "\n";
}
