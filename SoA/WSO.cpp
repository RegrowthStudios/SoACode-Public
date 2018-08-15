#include "stdafx.h"
#include "WSO.h"

WSO::WSO(const WSOData* wsoData, const f64v3& pos) :
position(pos),
data(wsoData) {
}
WSO::~WSO() {
    // We Don't Have Any Allocated Resources Yet
}
