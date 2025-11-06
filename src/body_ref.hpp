#pragma once

struct BodyRef {
    int id;
    float x, y, r;
    
    BodyRef() : id(-1), x(0), y(0), r(0) {}
    BodyRef(int id, float x, float y, float r) : id(id), x(x), y(y), r(r) {}
};