#include "stdafx.h"
#include "VoxelModelMesh.h"

void VoxelModelMesh::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
}