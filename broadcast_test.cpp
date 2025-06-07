#include "p2p.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
using json = nlohmann::json;
using namespace std;


int main() {
    std::string message = R"({
        "type": "TX",
        "transaction": {
            "amount": 10.0,
            "id": "0b6ea6da820684ef798ddc82bef6907079194a35edc9d83718d67189996a24b4",
            "receiver": "10",
            "sender": "049188620ac9452a744add880083f2f1c538bde25af3a1804a08506cfbf7dd2667da7eba730e14e53214e9836291c5813c399968d0983b806bfc49bc4664516b86",
            "signature": "97e8937e95174db0b5f0f5365b8bd2cc71c480183bbd40f069c9b2b16e9490f0660ef957140e83b3c3979ab3b7d074670358b64e841c419c1f629b6100e298de",
            "timestamp": "2025-06-07 12:23:15"
        }
    })";


    //json j = json::parse(message);

    int port = 8080; // Replace with the actual port your listener is using
    broadcastMessage(message, port);
    std::cout << "Broadcasted message to all peers.\n";

    std::string message2 = R"({
        "type": "BLOCK",
        "block": {
                "hash": "0000008763488ee129e03c489483a0cdb22601e6acf06bde3fd3f744b9dd01c6",
                "index": 8,
                "nonce": 1134667,
                "previousHash": "00000074a6ea2048943b2c3c7a0d752d3816b79e37de6dda4a4f568aab6c9e5b",
                "timestamp": "2025-06-07 14:00:24",
                "transactions": [
                    {
                        "amount": 10.0,
                        "id": "c33002351a66669239911d9b9058ff55c754d353f6e294e41197fe686893374e",
                        "receiver": "049188620ac9452a744add880083f2f1c538bde25af3a1804a08506cfbf7dd2667da7eba730e14e53214e9836291c5813c399968d0983b806bfc49bc4664516b86",
                        "sender": "COINBASE",
                        "signature": "reward",
                        "timestamp": "2025-06-07 14:00:24"
                    },
                    {
                        "amount": 10.0,
                        "id": "0b6ea6da820684ef798ddc82bef6907079194a35edc9d83718d67189996a24b4",
                        "receiver": "10",
                        "sender": "049188620ac9452a744add880083f2f1c538bde25af3a1804a08506cfbf7dd2667da7eba730e14e53214e9836291c5813c399968d0983b806bfc49bc4664516b86",
                        "signature": "97e8937e95174db0b5f0f5365b8bd2cc71c480183bbd40f069c9b2b16e9490f0660ef957140e83b3c3979ab3b7d074670358b64e841c419c1f629b6100e298de",
                        "timestamp": "2025-06-07 12:23:15"
                    }
                ]
            }
    })";
    broadcastMessage(message2, port);
    std::cout << "Broadcasted message to all peers.\n";

    std::string message3 = R"({
        "type": "BLOCK",
        "block": {
                "hash": "000000171894d49c54118b59d756fd2b76259df9b3090942593d266b899abe18",
                "index": 8,
                "nonce": 85103713,
                "previousHash": "00000074a6ea2048943b2c3c7a0d752d3816b79e37de6dda4a4f568aab6c9e5b",
                "timestamp": "2025-06-07 14:55:31",
                "transactions": [
                    {
                        "amount": 10.0,
                        "id": "40a57341f458eb83cc3b2fbee09be378e17525c57c823bf8ce02be2cc41ea355",
                        "receiver": "042db172ff492f201106a9afe98870c1f2869459ab7b7770fb12b7fbb0de256b3d84542d1738bfa86a8000cf91eabee20b0d1320e3c075601dbec09f536c054792",
                        "sender": "COINBASE",
                        "signature": "reward",
                        "timestamp": "2025-06-07 14:55:31"
                    },
                    {
                        "amount": 10.0,
                        "id": "0b6ea6da820684ef798ddc82bef6907079194a35edc9d83718d67189996a24b4",
                        "receiver": "10",
                        "sender": "049188620ac9452a744add880083f2f1c538bde25af3a1804a08506cfbf7dd2667da7eba730e14e53214e9836291c5813c399968d0983b806bfc49bc4664516b86",
                        "signature": "97e8937e95174db0b5f0f5365b8bd2cc71c480183bbd40f069c9b2b16e9490f0660ef957140e83b3c3979ab3b7d074670358b64e841c419c1f629b6100e298de",
                        "timestamp": "2025-06-07 12:23:15"
                    }
                ]
            }
    })";
    broadcastMessage(message3, port);
    std::cout << "Broadcasted message to all peers.\n";

    return 0;
}

