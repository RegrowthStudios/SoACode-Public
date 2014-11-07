#include "stdafx.h"
#include "FullQuadVBO.h"

void vg::FullQuadVBO::init(i32 attrLocation /*= 0*/) {
    glGenBuffers(2, _buffers);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ib);
    ui32 inds[6] = { 0, 1, 3, 0, 3, 2 };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(inds), inds, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, _vb);
    f32 points[8] = { -1, -1, 1, -1, -1, 1, 1, 1 };
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);
    glVertexAttribPointer(attrLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, _vb);
    glBindVertexArray(0);
}

void vg::FullQuadVBO::dispose() {
    glDeleteBuffers(2, _buffers);
    glDeleteVertexArrays(1, &_vao);
}

void vg::FullQuadVBO::draw() {
    glBindVertexArray(_vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ib);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(0);
    glBindVertexArray(0);
}
