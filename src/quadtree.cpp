#include "quadtree.hpp"
#include <algorithm>
#include <cmath>
using namespace std;

Quadtree::Quadtree(float x, float y, float w, float h, int cap, int maxDepth)
    : capacity_(cap), maxDepth_(maxDepth) 
    {
    root_ = make_unique<Node>(x, y, w, h);
    }

void Quadtree::clear() {
    root_ = make_unique<Node>(root_->x, root_->y, root_->w, root_->h);
}

bool Quadtree::insert(const BodyRef& b) {
    return insertRecursive(root_.get(), b, 0);
}

void Quadtree::update(const BodyRef& b) {insert(b);}

bool Quadtree::insertRecursive(Node* node, const BodyRef& b, int depth) {
    if (!contains(node, b)) {
        return false;
    }
    
    if (node->isLeaf) {
        if (node->bodies.size() < capacity_ || depth >= maxDepth_) {
            node->bodies.push_back(b);
            return true;
        } else {
            subdivide(node);
        }
    }
    
    // Insert into appropriate child
    for (int i = 0; i < 4; ++i) {
        if (insertRecursive(node->children[i].get(), b, depth + 1)) {
            return true;
        }
    }
    
    // Fallback: add to current node if subdivision failed
    node->bodies.push_back(b);
    return true;
}

void Quadtree::subdivide(Node* node) {
    float halfW = node->w * 0.5f;
    float halfH = node->h * 0.5f;
    float midX = node->x + halfW;
    float midY = node->y + halfH;
    
    // NW, NE, SW, SE
    node->children[0] = make_unique<Node>(node->x, node->y, halfW, halfH);
    node->children[1] = make_unique<Node>(midX, node->y, halfW, halfH);
    node->children[2] = make_unique<Node>(node->x, midY, halfW, halfH);
    node->children[3] = make_unique<Node>(midX, midY, halfW, halfH);
    
    // Redistribute bodies
    std::vector<BodyRef> oldBodies = node->bodies;
    node->bodies.clear();
    node->isLeaf = false;
    
    for (const auto& body : oldBodies) {
        bool inserted = false;
        for (int i = 0; i < 4; ++i) {
            if (contains(node->children[i].get(), body)) {
                node->children[i]->bodies.push_back(body);
                inserted = true;
                break;
            }
        }
        if (!inserted) {
            node->bodies.push_back(body);
        }
    }
}

void Quadtree::query(float qx, float qy, float qr, std::vector<int>& outIds) const {
    outIds.clear();
    queryRecursive(root_.get(), qx, qy, qr, outIds);
}

void Quadtree::queryRecursive(const Node* node, float qx, float qy, float qr, std::vector<int>& outIds) const {
    if (!intersects(node, qx, qy, qr)) {
        return;
    }
    
    if (node->isLeaf) {
        for (const auto& body : node->bodies) {
            float dx = body.x - qx;
            float dy = body.y - qy;
            float dist_sq = dx * dx + dy * dy;
            float r_sum = body.r + qr;
            if (dist_sq < r_sum * r_sum) {
                outIds.push_back(body.id);
            }
        }
    } else {
        for (int i = 0; i < 4; ++i) {
            if (node->children[i]) {
                queryRecursive(node->children[i].get(), qx, qy, qr, outIds);
            }
        }
    }
}

void Quadtree::queryAABB(float minX, float minY, float maxX, float maxY, std::vector<int>& outIds) const {
    outIds.clear();
    queryAABBRecursive(root_.get(), minX, minY, maxX, maxY, outIds);
}

void Quadtree::queryAABBRecursive(const Node* node, float minX, float minY, float maxX, float maxY, std::vector<int>& outIds) const {
    if (!intersectsAABB(node, minX, minY, maxX, maxY)) {
        return;
    }
    
    if (node->isLeaf) {
        for (const auto& body : node->bodies) {
            if (body.x - body.r < maxX && body.x + body.r > minX &&
                body.y - body.r < maxY && body.y + body.r > minY) {
                outIds.push_back(body.id);
            }
        }
    } else {
        for (int i = 0; i < 4; ++i) {
            if (node->children[i]) {
                queryAABBRecursive(node->children[i].get(), minX, minY, maxX, maxY, outIds);
            }
        }
    }
}

bool Quadtree::contains(const Node* node, const BodyRef& b) const {
    return b.x - b.r >= node->x && b.x + b.r <= node->x + node->w &&
           b.y - b.r >= node->y && b.y + b.r <= node->y + node->h;
}

bool Quadtree::intersects(const Node* node, float qx, float qy, float qr) const {
    float closestX = std::max(node->x, std::min(qx, node->x + node->w));
    float closestY = std::max(node->y, std::min(qy, node->y + node->h));
    float dx = qx - closestX;
    float dy = qy - closestY;
    return dx * dx + dy * dy < qr * qr;
}

bool Quadtree::intersectsAABB(const Node* node, float minX, float minY, float maxX, float maxY) const {
    return !(node->x + node->w < minX || node->x > maxX || node->y + node->h < minY || node->y > maxY);
}

void Quadtree::getBounds(float& x, float& y, float& w, float& h) const {
    x = root_->x;
    y = root_->y;
    w = root_->w;
    h = root_->h;
}

