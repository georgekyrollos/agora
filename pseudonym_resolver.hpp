#pragma once
#include <string>
#include <unordered_map>
#include <mutex>

using std::string;
using std::unordered_map;
using std::mutex;

class PseudonymResolver {
public:
    explicit PseudonymResolver(string path);

    string resolveToPubkey(const string& to) const;

    void set(const string& pseudonym, const string& hex_pubkey);
    bool erase(const string& pseudonym);
    unordered_map<string, string> list() const;
    void reload();

private:
    string               path_;
    mutable mutex        mu_;
    unordered_map<string, string> map_;

    static string trim(const string& s);
    static string normalize_name(string s);
    static bool   isHexPubkey(string s);    // accepts 02/03 (compressed) OR 04 (uncompressed)
    static string normalize_pubkey(string s);

    void load_unlocked();
    void persist_unlocked() const;
};
