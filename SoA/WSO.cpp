#include "stdafx.h"
#include "WSO.h"

WSO::WSO(const WSOData* wsoData, const f64v3& pos) :
data(wsoData),
position(pos) {
}
WSO::~WSO() {
    // We Don't Have Any Allocated Resources Yet
}
