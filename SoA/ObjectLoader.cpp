#include "stdafx.h"
#include "ObjectLoader.h"

#include <SDL/SDL.h>

//***************************** ObjectLoader ***********************************

ObjectLoader::ObjectLoader(void) {
    _isMaterial = false;
    _hasNormals = false;
    _hasTexture = false;
    _hasVertexNormals = false;
}

Material::Material(const char *na, float a, float n, float ni2, float *d, float *am,
    float *s, int il, int t) {
    name = na;
    alpha = a;
    ns = n;
    ni = ni2;
    for (int i = 0; i < 3; i++) {
        dif[i] = d[i];
        spec[i] = s[i];
        amb[i] = am[i];
    }
    illum = il;
    texture = t;
}

TexCoord::TexCoord(float a, float b) {
    u = a;
    v = b;
}

int ObjectLoader::load(const cString filename, std::vector<ColorVertex>& vertices, std::vector<GLushort>& indices) {
    std::ifstream in(filename);
    std::vector<f32v3> tmpNorms;
    if (!(in.is_open())) {
        std::cout << "Not opened\n" << std::endl;
        return -1;
    }
    char buf[512];
    int curmat = 0;
    GLushort a, b, c, d, e, g;
    int count = 0;
    in.getline(buf, 256);
    while (!in.eof()) {
        std::string ins(buf);
        if (ins.size() < 4 || ins[0] == '#') {
            in.getline(buf, 256);
            continue;
        }

        if (ins[0] == 'v' && ins[1] == ' ') {
            glm::vec3 fv;
            sscanf(ins.c_str(), "v %f %f %f", &(fv.x), &(fv.y), &(fv.z));

            //        fv = glm::normalize(fv);

            vertices.push_back(ColorVertex());
            vertices.back().position.x = fv.x;
            vertices.back().position.y = fv.y;
            vertices.back().position.z = fv.z;
            vertices.back().color[0] = 255;
            vertices.back().color[1] = 255;
            vertices.back().color[2] = 255;
            vertices.back().color[3] = 255;

        } else if (ins[0] == 'v' && ins[1] == 'n') {
            glm::vec3 indx;
            sscanf(ins.c_str(), "vn %f %f %f", &(indx.x), &(indx.y), &(indx.z));
            tmpNorms.push_back(indx);
        } else if (ins[0] == 'f') {
            count = 0;
            for (size_t j = 0; j < ins.size(); j++) {
                if (ins[j] == ' ') count++;
            }
            if (count == 4) //quads
            {
                std::cout << "Quads in .obj file not supported!";
                exit(9939);
                /*if (ins.find("//")!=std::string::npos){
                    sscanf(ins.c_str(), "f %d//%d %d//%d %d//%d %d//%d",&a,&b,&c,&b,&d,&b,&e,&b);
                    faces.push_back(new face(b,a,c,d,e,0,0,0,0,curmat));
                    }else if(ins.find("/")!=std::string::npos){
                    int t[4];
                    sscanf(ins.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",&a, &t[0], &b,&c, &t[1], &b,&d, &t[2], &b,&e, &t[3], &b);
                    faces.push_back(new face(b,a,c,d,e,t[0],t[1],t[2], t[3],curmat));
                    }else{
                    sscanf(ins.c_str(), "f %d %d %d %d", &a, &b, &c, &d);
                    faces.push_back(new face(-1, a, b, c, d, 0, 0, 0, 0, curmat));
                    }*/
            } else { //triangles
                if (ins.find("//") != std::string::npos) {
                    sscanf(ins.c_str(), "f %hu//%hu %hu//%hu %hu//%hu", &a, &b, &c, &d, &e, &g);
                    indices.push_back(a - 1);
                    indices.push_back(c - 1);
                    indices.push_back(e - 1);
                    //vertices[a-1].normal = tmpNorms[b-1];
                    //vertices[c-1].normal = tmpNorms[d-1];
                    //vertices[e-1].normal = tmpNorms[g-1];
                } else {
                    sscanf(ins.c_str(), "f %hu %hu %hu", &a, &b, &c);
                    indices.push_back(a - 1);
                    indices.push_back(b - 1);
                    indices.push_back(c - 1);
                }
            }
        }
        in.getline(buf, 256);
    }
    in.close();
    return 0;
}
