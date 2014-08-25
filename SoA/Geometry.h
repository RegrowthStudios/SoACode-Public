//#include "global.h"
//
//class IcoSphereCreator
//{
//private:
//
//    MeshGeometry3D geometry;
//    int index;
//    Dictionary<Int64, int> middlePointIndexCache;
//
//    struct TriangleIndices
//    {
//        int v1;
//        int v2;
//        int v3;
//
//        TriangleIndices(int V1, int V2, int V3)
//        {
//            v1 = V1;
//            v2 = V2;
//            v3 = V3;
//        }
//    };
//
//    // add vertex to mesh, fix position to be on unit sphere, return index
//    int addVertex(glm::vec3 p)
//    {
//        double length = sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
//        geometry.Positions.Add(new glm::vec3(p.x/length, p.y/length, p.z/length));
//        return index++;
//    }
//
//    int getMiddlePoint(int p1, int p2);
//
//};