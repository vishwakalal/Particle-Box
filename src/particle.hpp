#ifndef PARTICLE_HPP
#define PARTICLE_HPP

struct Particle {
    float x, y;      //position
    float vx, vy;    //velocity
    float r;         //radius
    int id;        
    bool collided;
    
    Particle() : x(0), y(0), vx(0), vy(0), r(0), id(-1), collided(false) {}
    Particle(float x, float y, float vx, float vy, float r, int id)
        : x(x), y(y), vx(vx), vy(vy), r(r), id(id), collided(false) {}
};

#endif // PARTICLE_HPP

