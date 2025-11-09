#pragma once

#include <vector>
#include <cstdint>
#include "body_ref.hpp"

class SpatialHash {
public:
    SpatialHash(float cellSize);
    
    void clear();
    void insert(const BodyRef& b);
    void query(float qx, float qy, float qr, std::vector<int>& outIds) const;
    
    float getCellSize() const { return cellSize_; }
    
private:
    struct HashKey {
        int i, j;
        
        HashKey(int i, int j) : i(i), j(j) {}
        bool operator==(const HashKey& other) const {
            return i == other.i && j == other.j;
        }
    };
    
    struct Cell {
        std::vector<BodyRef> bodies;
        bool occupied;
        HashKey key;  // Store the key for this cell
        
        Cell() : occupied(false), key(0, 0) {}
    };
    
    uint64_t hashKey(const HashKey& key) const;
    int findSlot(uint64_t hash, const HashKey& key) const;
    void resize();
    
    std::vector<Cell> table_;
    float cellSize_;
    int tableSize_;
    int itemCount_;
    static constexpr float LOAD_FACTOR = 0.75f;
    
    // Splitmix64 hash function
    static uint64_t splitmix64(uint64_t x);
};
