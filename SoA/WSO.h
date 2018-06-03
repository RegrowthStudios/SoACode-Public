#pragma once
class WSOData;

class WSO {
public:
    WSO(const WSOData* wsoData, const f64v3& pos);
    ~WSO();

    const f64v3 position;
    const WSOData* const data;
};