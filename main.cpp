#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <cstdint>
using namespace std;

// Converts a string into a hash number.
uint64_t demoHash(const string& s) {
    uint64_t h = 1469598103934665603ULL;

    for (unsigned char c : s) {
        h = (h ^ c) * 1099511628211ULL;
    }

    return h;
}


const string pepper = "ChinchillaPepper";

// Stores the information for each user.
struct User {
    string name;
    string salt;
    uint64_t passwordHash;
};

class HashTable {
private:
   
    vector<list<User>> table;  // Each bucket contains a list of users.
    int count;

    // Calculates which bucket a username belongs in.
    int getIndex(string name) {
        int index = demoHash(name) % table.size();

        cout << "Hashing username: " << name << endl;
        cout << "Bucket index: " << index << endl;

        return index;
    }

    // Doubles the table size and moves users to new buckets.
    void rehash() {
        cout << "\nLoad factor reached 0.75. Resizing table...\n";

        vector<list<User>> oldTable = table;

        cout << "Old table size: " << table.size() << " buckets\n";

        table.clear();
        table.resize(oldTable.size() * 2);

        cout << "New table size: " << table.size() << " buckets\n";
        cout << "Moving users to their new buckets...\n";

        for (int i = 0; i < oldTable.size(); i++) {
            for (list<User>::iterator it = oldTable[i].begin();
                 it != oldTable[i].end(); it++) {

                int index = getIndex(it->name);
                table[index].push_back(*it);

                cout << it->name << " moved to bucket "
                     << index << endl;
            }
        }

        cout << "Rehash complete!\n\n";
    }

public:
    // Starts with four empty buckets.
    HashTable() {
        table.resize(4);
        count = 0;

        cout << "Hash table created with 4 buckets.\n";
    }

    // Calculates the number of users divided by bucket count.
    double loadFactor() {
        return (double)count / table.size();
    }

    // Adds a new user to the table.
    void insert(string name, string password, string salt) {
        cout << "\nAdding chinchilla user: " << name << endl;

        cout << "Current load factor: " << loadFactor() << endl;

        if (loadFactor() >= 0.75) {
            rehash();
        }

        User user;
        user.name = name;
        user.salt = salt;

        cout << "Adding salt and pepper to the password before hashing.\n";

        user.passwordHash = demoHash(salt + password + pepper);

        cout << "Password hash created.\n";

        int index = getIndex(name);

        // A bucket may already contain another user.
        if (!table[index].empty()) {
            cout << "Collision detected! Bucket " << index
                 << " already contains a user.\n";
            cout << "Using separate chaining to store the user in the list.\n";
        }

        table[index].push_back(user);
        count++;

        cout << name << " added successfully.\n";
        cout << "Total users: " << count << endl;
        cout << "New load factor: " << loadFactor() << endl;
    }

    // Searches for a username and returns its record if found.
    User* find(string name) {
        cout << "Searching for user: " << name << endl;

        int index = getIndex(name);

        cout << "Checking the list in bucket " << index << "...\n";

        for (list<User>::iterator it = table[index].begin();
             it != table[index].end(); it++) {

            cout << "Checking username: " << it->name << endl;

            if (it->name == name) {
                cout << "User found!\n";
                return &(*it);
            }
        }

        cout << "User was not found.\n";
        return NULL;
    }

    // Removes a user from the table.
    void remove(string name) {
        cout << "\nRemoving user: " << name << endl;

        int index = getIndex(name);

        for (list<User>::iterator it = table[index].begin();
             it != table[index].end(); it++) {

            if (it->name == name) {
                table[index].erase(it);
                count--;

                cout << name << " was removed successfully.\n";
                cout << "Total users: " << count << endl;
                cout << "New load factor: " << loadFactor() << endl;
                return;
            }
        }

        cout << "User not found. Nothing was removed.\n";
    }

    // Checks a username and password during login.
    void login(string name, string password) {
        cout << "\nLogin attempt for: " << name << endl;

        // Find the account first.
        User* user = find(name);

        if (user == NULL) {
            cout << "Login failed: unknown chinchilla.\n";
        }
        else {
            cout << "Username found. Checking password...\n";

            // Hash the submitted password using the user's salt.
            uint64_t passwordHash =
                demoHash(user->salt + password + pepper);

            cout << "Submitted password hash calculated.\n";
            cout << "Comparing submitted hash with stored hash...\n";

            if (passwordHash == user->passwordHash) {
                cout << "Login successful! Welcome to ChinChilla Club!\n";
            }
            else {
                cout << "Login failed: incorrect password.\n";
            }
        }
    }
};

int main() {
 
    cout << " Chinchilla Club Login\n";

    
    HashTable users; 

    // Add five chinchilla accounts.
    cout << "\n--- Creating Five Users ---\n";

    users.insert("Oso", "treats123", "salt1");
    users.insert("Juna", "dustbath", "salt2");
    users.insert("Goro", "hay456", "salt3");
    users.insert("Mochi", "fluffy789", "salt4");
    users.insert("Coco", "tinyPaws", "salt5");


    cout << "\n--- Test 1: Successful Login ---\n";
    users.login("Oso", "treats123");

    
    cout << "\n--- Test 2: Incorrect Password ---\n";
    users.login("Juna", "wrong");

    
    cout << "\n--- Test 3: Unknown Username ---\n";
    users.login("Snowball", "abc");

   
    cout << "\n--- Test 4: Remove User ---\n";
    users.remove("Goro");

   
    cout << "\n--- Test 5: Automatic Rehashing ---\n";

    users.insert("Pip", "paws1", "salt6");
    users.insert("Luna", "paws2", "salt7");
    users.insert("Boba", "paws3", "salt8");

    // Show the final table load factor.
    cout << "\n--- Final Results ---\n";
    cout << "Final load factor: " << users.loadFactor() << endl;

    return 0;
}
