#pragma once
#include <map>

#include <Vorb/types.h>
#include <Vorb/VorbPreDecl.inl>

DECL_VG(class GLProgram);

class ChunkGrid;
class PhysicsEngine;
struct ItemStack;

class EditorNode {
    i32v3 position;
};

enum class EDITOR_TOOLS { AABOX, LINE };

class VoxelEditor {
public:    
    void editVoxels(ChunkGrid& grid, ItemStack* block);

    void stopDragging();

    void setStartPosition(const i32v3& position) { m_startPosition = position; }
    void setEndPosition(const i32v3& position) { m_endPosition = position; }   
    void setEditorTool(EDITOR_TOOLS editorTool) { m_currentTool = editorTool; }

    bool isEditing();

    const i32v3& getStartPosition() const { return m_startPosition; }
    const i32v3& getEndPosition() const { return m_endPosition; }

    //Draws the guide lines
    void drawGuides(vg::GLProgram* program, const f64v3& cameraPos, const f32m4 &VP, int blockID);

private:
    
    void placeAABox(ChunkGrid& grid, ItemStack* block);
    void placeLine(ChunkGrid& grid, ItemStack* block);

    // v v v Just some ideas v v v
    // void placeBox();
    // void placeWireBox();
    // void placeSlab();
    // void placeWireSlab();
    
    i32v3 m_startPosition = i32v3(INT_MAX);
    i32v3 m_endPosition = i32v3(INT_MAX);

    std::vector<EditorNode> m_currentShape;
    bool m_isAxisAligned;
    EDITOR_TOOLS m_currentTool = EDITOR_TOOLS::AABOX;
};