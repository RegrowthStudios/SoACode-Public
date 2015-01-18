#pragma once
class Player;
class Chunk;

const i32 PLAYER_COLLISION_STEPS = 20;

//bool RaySphere(float xc, float yc, float zc, float xd, float yd, float zd, float xs, float ys, float zs, float r, float *dist=NULL, coordinate3lf *point=NULL);
//bool RayPlane(float nx, float ny, float nz, float xs, float ys, float zs, float xd, float yd, float zd, coordinate3lf &p1, coordinate3lf &p2, coordinate3lf &p3, coordinate3lf &p4, float *dist=NULL, coordinate3lf *point=NULL);
//bool SphereSphere(coordinate3lf &p1, coordinate3lf p2, float radius1, float radius2);
//bool SpherePlane(coordinate3lf &sp, coordinate3lf &vn, coordinate3lf &p1, coordinate3lf &p2, coordinate3lf &p3, coordinate3lf &p4, float r);
//float TriangleArea(coordinate3lf &p1, coordinate3lf &p2, coordinate3lf &p3);
//bool AABBCollision(coordinate3lf &b1, coordinate3lf &b2, coordinate3lf &r1, coordinate3lf &r2, coordinate3lf &vn);

void aabbChunkCollision(Player* player, f64v3* playerPos, Chunk** chunks, ui8 size);