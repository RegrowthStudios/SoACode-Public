#pragma once
#include <map>

class Item;

class EditorNode {
    i32v3 position;
};

enum class EDITOR_TOOLS { AABOX, LINE };

class VoxelEditor {
public:    
    VoxelEditor();

    void editVoxels(Item *block);

    void stopDragging();

    void setStartPosition(const i32v3& position) { _startPosition = position; }
    void setEndPosition(const i32v3& position) { _endPosition = position; }   
    void setEditorTool(EDITOR_TOOLS editorTool) { _currentTool = editorTool; }

    bool isEditing();

    const i32v3& getStartPosition() const { return _startPosition; }
    const i32v3& getEndPosition() const { return _endPosition; }

    //Draws the guide lines
    void drawGuides(const f64v3& cameraPos, const glm::mat4 &VP, int blockID);

private:
    
    void placeAABox(Item *block);
    void placeLine(Item *block);

    // v v v Just some ideas v v v
    // void placeBox();
    // void placeWireBox();
    // void placeSlab();
    // void placeWireSlab();
    
    i32v3 _startPosition;
    i32v3 _endPosition;

    std::vector<EditorNode> currentShape;
    bool _isAxisAligned;
    EDITOR_TOOLS _currentTool;
};