#pragma once

#include <vector>
#include <memory>
#include "body_ref.hpp"
using namespace std;
class Quadtree {
public:
    Quadtree(float x, float y, float w, float h, int cap = 8, int maxDepth = 12);
    
    void clear();
    bool insert(const BodyRef& b);
    void update(const BodyRef& b);
    void query(float qx, float qy, float qr, std::vector<int>& outIds) const;
    void queryAABB(float minX, float minY, float maxX, float maxY, std::vector<int>& outIds) const;
    void getBounds(float& x, float& y, float& w, float& h) const;
    
private:
    struct Node {
        float x, y, w, h;
        vector<BodyRef> bodies;
        unique_ptr<Node> children[4];
        bool isLeaf;
        
        Node(float x, float y, float w, float h) 
            : x(x), y(y), w(w), h(h), isLeaf(true) {}
    };
    
    bool insertRecursive(Node* node, const BodyRef& b, int depth);
    void subdivide(Node* node);
    void queryRecursive(const Node* node, float qx, float qy, float qr, std::vector<int>& outIds) const;
    void queryAABBRecursive(const Node* node, float minX, float minY, float maxX, float maxY, std::vector<int>& outIds) const;
    bool contains(const Node* node, const BodyRef& b) const;
    bool intersects(const Node* node, float qx, float qy, float qr) const;
    bool intersectsAABB(const Node* node, float minX, float minY, float maxX, float maxY) const;
    
    unique_ptr<Node> root_;
    int capacity_;
    int maxDepth_;
};

