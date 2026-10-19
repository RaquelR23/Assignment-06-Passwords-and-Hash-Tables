#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <cstdint>

using namespace std;

// Required educational hash function.
// This is not suitable for real password security.
uint64_t demoHash(const string& s) {
    uint64_t h = 1469598103934665603ULL;

    for (unsigned char c : s) {
        h = (h ^ c) * 1099511628211ULL;
    }

    return h;
}

const string pepper = "ChinchillaPepper";

// Stores one user's information.
struct User {
    string name;
    string salt;
    uint64_t passwordHash;
};

// The four collision mitigation strategies.
enum Strategy {
    CHAINING,
    LINEAR_PROBING,
    QUADRATIC_PROBING,
    DOUBLE_HASHING
};

class HashTable {
private:
    // Separate chaining uses a linked list for each bucket.
    vector<list<User>> chains;

    // Open addressing stores one user per slot.
    struct Slot {
        User user;
        bool occupied;
        bool deleted;

        Slot() {
            occupied = false;
            deleted = false;
        }
    };

    vector<Slot> slots;

    int count;
    Strategy strategy;

    int capacity() {
        return (int)slots.size();
    }

    // Returns the name of the selected strategy.
    string strategyName() {
        if (strategy == CHAINING) {
            return "Separate Chaining";
        }
        else if (strategy == LINEAR_PROBING) {
            return "Linear Probing";
        }
        else if (strategy == QUADRATIC_PROBING) {
            return "Quadratic Probing";
        }
        else {
            return "Double Hashing";
        }
    }

    // Calculates the original bucket for a username.
    int getIndex(const string& name) {
        return demoHash(name) % capacity();
    }

    // Calculates the next bucket to check for open addressing.
    int probeIndex(const string& name, int attempt) {
        int start = getIndex(name);

        if (strategy == LINEAR_PROBING) {
            return (start + attempt) % capacity();
        }

        if (strategy == QUADRATIC_PROBING) {
            // Triangular-number offsets are a quadratic probing variant.
            // With a power-of-two capacity, they visit every slot.
            int offset = attempt * (attempt + 1) / 2;
            return (start + offset) % capacity();
        }

        // Double hashing uses a second hash to calculate the step size.
        // The step is odd so it can visit every slot in a power-of-two table.
        uint64_t secondHash = demoHash(name + "secondary");
        int step = (secondHash % capacity()) | 1;

        return (start + attempt * step) % capacity();
    }

    // Finds an available slot for an open-addressing strategy.
    int findInsertSlot(const string& name) {
        int firstDeleted = -1;

        for (int i = 0; i < capacity(); i++) {
            int index = probeIndex(name, i);

            if (slots[index].occupied) {
                continue;
            }

            if (slots[index].deleted) {
                if (firstDeleted == -1) {
                    firstDeleted = index;
                }
            }
            else {
                if (firstDeleted != -1) {
                    return firstDeleted;
                }

                return index;
            }
        }

        return firstDeleted;
    }

    // Places an existing user into the table without triggering another resize.
    void placeUser(const User& user) {
        if (strategy == CHAINING) {
            int index = getIndex(user.name);
            chains[index].push_back(user);
        }
        else {
            int index = findInsertSlot(user.name);

            if (index != -1) {
                slots[index].user = user;
                slots[index].occupied = true;
                slots[index].deleted = false;
            }
        }

        count++;
    }

    // Doubles the capacity and reinserts all existing users.
    void rehash() {
        int oldCapacity = capacity();

        vector<list<User>> oldChains = chains;
        vector<Slot> oldSlots = slots;

        cout << "Rehashing table: "
             << oldCapacity << " -> " << oldCapacity * 2 << "\n";

        slots.clear();
        slots.resize(oldCapacity * 2);

        chains.clear();
        chains.resize(oldCapacity * 2);

        count = 0;

        if (strategy == CHAINING) {
            for (int i = 0; i < oldCapacity; i++) {
                for (list<User>::iterator it = oldChains[i].begin();
                     it != oldChains[i].end(); it++) {
                    placeUser(*it);
                }
            }
        }
        else {
            for (int i = 0; i < oldCapacity; i++) {
                if (oldSlots[i].occupied) {
                    placeUser(oldSlots[i].user);
                }
            }
        }

        cout << "Rehash complete. Users preserved: "
             << count << "\n";
    }

public:
    // Starts with eight buckets.
    HashTable(Strategy selectedStrategy) {
        strategy = selectedStrategy;
        slots.resize(8);
        chains.resize(8);
        count = 0;
    }

    // Returns the number of users divided by table capacity.
    double loadFactor() {
        return (double)count / capacity();
    }

    // Finds a user by username.
    // Returns a pointer to the record or NULL if not found.
    User* find(string name) {
        if (strategy == CHAINING) {
            int index = getIndex(name);

            for (list<User>::iterator it = chains[index].begin();
                 it != chains[index].end(); it++) {
                if (it->name == name) {
                    return &(*it);
                }
            }
        }
        else {
            for (int i = 0; i < capacity(); i++) {
                int index = probeIndex(name, i);

                // Stop when an unused slot proves the username is absent.
                if (!slots[index].occupied && !slots[index].deleted) {
                    break;
                }

                if (slots[index].occupied &&
                    slots[index].user.name == name) {
                    return &slots[index].user;
                }
            }
        }

        return NULL;
    }

    // Inserts a username and its salted password hash.
    void insert(string name, string password, string salt) {
        if (find(name) != NULL) {
            cout << "Duplicate username: " << name << "\n";
            return;
        }

        // Resize before insertion if the load factor has reached 0.75.
        if (loadFactor() >= 0.75) {
            rehash();
        }

        User user;
        user.name = name;
        user.salt = salt;
        user.passwordHash = demoHash(salt + password + pepper);

        int start = getIndex(name);

        if (strategy == CHAINING) {
            if (!chains[start].empty()) {
                cout << "Collision: " << name
                     << " added to linked list at bucket "
                     << start << "\n";
            }

            chains[start].push_back(user);
        }
        else {
            int index = findInsertSlot(name);

            // Resize if no slot is available.
            if (index == -1) {
                rehash();
                index = findInsertSlot(name);
            }

            if (index != -1) {
                if (index != start) {
                    cout << "Collision: " << name
                         << " placed at bucket " << index
                         << " instead of bucket " << start << "\n";
                }

                slots[index].user = user;
                slots[index].occupied = true;
                slots[index].deleted = false;
            }
        }

        count++;
        cout << "Inserted user: " << name << "\n";
    }

    // Removes a user from the table.
    void remove(string name) {
        if (strategy == CHAINING) {
            int index = getIndex(name);

            for (list<User>::iterator it = chains[index].begin();
                 it != chains[index].end(); it++) {
                if (it->name == name) {
                    chains[index].erase(it);
                    count--;

                    cout << "Removed user: " << name << "\n";
                    return;
                }
            }
        }
        else {
            for (int i = 0; i < capacity(); i++) {
                int index = probeIndex(name, i);

                if (!slots[index].occupied && !slots[index].deleted) {
                    break;
                }

                if (slots[index].occupied &&
                    slots[index].user.name == name) {
                    // A deleted marker preserves the probing sequence.
                    slots[index].occupied = false;
                    slots[index].deleted = true;
                    count--;

                    cout << "Removed user: " << name << "\n";
                    return;
                }
            }
        }

        cout << "User not found for removal: " << name << "\n";
    }

    // Looks up a username and checks the submitted password.
    void login(string name, string password) {
        User* user = find(name);

        if (user == NULL) {
            cout << "Login failed: unknown username.\n";
            return;
        }

        uint64_t submittedHash =
            demoHash(user->salt + password + pepper);

        if (submittedHash == user->passwordHash) {
            cout << "Login successful: " << name << "\n";
        }
        else {
            cout << "Login failed: incorrect password.\n";
        }
    }

    // Displays the final number of users and load factor.
    void displaySummary() {
        cout << "Users stored: " << count << "\n";
        cout << "Final load factor: " << loadFactor() << "\n";
    }
};

// Runs all required tests for one collision strategy.
void runDemo(Strategy strategy) {
    HashTable users(strategy);

    cout << "--------------|";

    if (strategy == CHAINING) {
        cout << " Separate Chaining ";
    }
    else if (strategy == LINEAR_PROBING) {
        cout << " Linear Probing ";
    }
    else if (strategy == QUADRATIC_PROBING) {
        cout << " Quadratic Probing ";
    }
    else {
        cout << " Double Hashing ";
    }

    cout << "|--------------\n";

    // Insert at least five user records.
    users.insert("Oso", "treats123", "salt1");
    users.insert("Juna", "dustbath", "salt2");
    users.insert("Goro", "hay456", "salt3");
    users.insert("Mochi", "fluffy789", "salt4");
    users.insert("Coco", "tinyPaws", "salt5");

    cout << "Load factor after initial users: "
         << users.loadFactor() << "\n";

    // Test successful login, incorrect password, and unknown username.
    cout << "\nLogin tests:\n";
    users.login("Oso", "treats123");
    users.login("Juna", "wrong");
    users.login("Snowball", "abc");

    // Test removal.
    cout << "\nRemoval test:\n";
    users.remove("Goro");

    // Insert more users to trigger automatic resizing.
    cout << "\nResize test:\n";
    users.insert("Pip", "paws1", "salt6");
    users.insert("Luna", "paws2", "salt7");
    users.insert("Boba", "paws3", "salt8");

    // Confirm the removed user is no longer found.
    cout << "\nRemoval verification:\n";
    if (users.find("Goro") == NULL) {
        cout << "Goro is no longer in the table.\n";
    }

    users.displaySummary();
}

int main() {
    cout << " Chinchilla Club\n";

    // Run the same tests with every collision strategy.
    runDemo(CHAINING);
    runDemo(LINEAR_PROBING);
    runDemo(QUADRATIC_PROBING);
    runDemo(DOUBLE_HASHING);

    return 0;
}
