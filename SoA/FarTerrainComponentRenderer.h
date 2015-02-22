#include <Vorb/VorbPreDecl.inl>

struct AxisRotationComponent;
struct NamePositionComponent;
struct SpaceLightComponent;
struct SphericalTerrainComponent;
struct FarTerrainComponent;
class Camera;

DECL_VG(class GLProgram);

class FarTerrainComponentRenderer {
public:
    ~FarTerrainComponentRenderer();
    void draw(FarTerrainComponent& cmp,
              const Camera* camera,
              const f64v3& lightPos,
              const SpaceLightComponent* spComponent,
              const AxisRotationComponent* arComponent);

private:
    void buildShaders();

    vg::GLProgram* m_farTerrainProgram = nullptr;
    vg::GLProgram* m_farWaterProgram = nullptr;
};
