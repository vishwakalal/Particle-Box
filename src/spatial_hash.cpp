#include "spatial_hash.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <set>

SpatialHash::SpatialHash(float cellSize)
    : cellSize_(std::max(cellSize, 1.0f)), tableSize_(256), itemCount_(0) {
    table_.resize(tableSize_);
}

void SpatialHash::clear() {
    for (auto& cell : table_) {
        cell.bodies.clear();
        cell.occupied = false;
    }
    itemCount_ = 0;
}

uint64_t SpatialHash::splitmix64(uint64_t x) {
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27;
    x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}

uint64_t SpatialHash::hashKey(const HashKey& key) const {
    // Interleave i and j into a single uint64
    uint64_t x = static_cast<uint64_t>(key.i);
    uint64_t y = static_cast<uint64_t>(key.j);
    
    // Spread bits
    x = (x | (x << 32)) & 0x1F00000000FFFFULL;
    x = (x | (x << 16)) & 0x1F0000FF0000FFULL;
    x = (x | (x << 8)) & 0x100F00F00F00F00FULL;
    x = (x | (x << 4)) & 0x10C10C10C10C10C1ULL;
    
    y = (y | (y << 32)) & 0x1F00000000FFFFULL;
    y = (y | (y << 16)) & 0x1F0000FF0000FFULL;
    y = (y | (y << 8)) & 0x100F00F00F00F00FULL;
    y = (y | (y << 4)) & 0x10C10C10C10C10C1ULL;
    
    uint64_t combined = (x << 1) | y;
    return splitmix64(combined);
}

int SpatialHash::findSlot(uint64_t hash, const HashKey& key) const {
    int slot = hash % tableSize_;
    int startSlot = slot;
    
    // Linear probing - find slot with matching key or empty slot
    while (table_[slot].occupied) {
        if (table_[slot].key == key) {
            return slot;  // Found existing cell with this key
        }
        slot = (slot + 1) % tableSize_;
        if (slot == startSlot) {
            // Table full, should resize
            return -1;
        }
    }
    
    return slot;  // Empty slot
}

void SpatialHash::resize() {
    std::vector<Cell> oldTable = table_;
    int oldSize = tableSize_;
    
    tableSize_ *= 2;
    table_.clear();
    table_.resize(tableSize_);
    itemCount_ = 0;
    
    // Reinsert all items
    for (const auto& cell : oldTable) {
        if (cell.occupied) {
            for (const auto& body : cell.bodies) {
                insert(body);
            }
        }
    }
}

void SpatialHash::insert(const BodyRef& b) {
    // Check if resize needed
    if (itemCount_ >= tableSize_ * LOAD_FACTOR) {
        resize();
    }
    
    // Calculate grid cell
    int i = static_cast<int>(std::floor(b.x / cellSize_));
    int j = static_cast<int>(std::floor(b.y / cellSize_));
    
    HashKey key(i, j);
    uint64_t hash = hashKey(key);
    int slot = findSlot(hash, key);
    
    if (slot < 0) {
        resize();
        hash = hashKey(key);
        slot = findSlot(hash, key);
    }
    
    if (!table_[slot].occupied) {
        table_[slot].key = key;
        table_[slot].occupied = true;
        itemCount_++;
    }
    
    table_[slot].bodies.push_back(b);
}

void SpatialHash::query(float qx, float qy, float qr, std::vector<int>& outIds) const {
    outIds.clear();
    
    // Query 3x3 neighborhood around the query point
    int centerI = static_cast<int>(std::floor(qx / cellSize_));
    int centerJ = static_cast<int>(std::floor(qy / cellSize_));
    
    for (int di = -1; di <= 1; ++di) {
        for (int dj = -1; dj <= 1; ++dj) {
            int i = centerI + di;
            int j = centerJ + dj;
            
            HashKey key(i, j);
            uint64_t hash = hashKey(key);
            int slot = hash % tableSize_;
            int startSlot = slot;
            
            // Linear probe to find the cell with matching key
            while (table_[slot].occupied) {
                if (table_[slot].key == key) {
                    // Found the cell, check all bodies
                    for (const auto& body : table_[slot].bodies) {
                        float dx = body.x - qx;
                        float dy = body.y - qy;
                        float dist_sq = dx * dx + dy * dy;
                        float r_sum = body.r + qr;
                        if (dist_sq < r_sum * r_sum) {
                            outIds.push_back(body.id);
                        }
                    }
                    break;
                }
                
                slot = (slot + 1) % tableSize_;
                if (slot == startSlot) {
                    break;
                }
            }
        }
    }
    
    // Remove duplicates using set
    std::set<int> uniqueIds(outIds.begin(), outIds.end());
    outIds.clear();
    outIds.reserve(uniqueIds.size());
    for (int id : uniqueIds) {
        outIds.push_back(id);
    }
}
