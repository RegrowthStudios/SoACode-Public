#include "stdafx.h"
#include "TerrainGenerator.h"

#include "GameManager.h"
#include "Planet.h"
#include "SimplexNoise.h"
#include "BlockData.h"
#include "Errors.h"
#include "ImageLoader.h"
#include "TexturePackLoader.h"

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

float mountainBase = 600.0f;

nString TerrainFunctionHelps[NumTerrainFunctions];

#ifndef ABS
#define ABS(a) (((a) < 0) ?(-(a)):(a))
#endif

Biome blankBiome;

TerrainGenerator::TerrainGenerator()
{
    numFunctions = 0;
    noiseFunctions.reserve(100);
    temperatureNoiseFunction = NULL;
    rainfallNoiseFunction = NULL;
    biomeOffsetNoiseFunction = NULL;
    riverNoiseFunction = NULL;
    tributaryNoiseFunction = NULL;
    preturbNoiseFunction = NULL;
    defaultTemp = defaultRain = 128.0;
    heightModifier = 0.0;

    // This is important. Sets up biome as map 1 and water as map 2
    GameManager::texturePackLoader->getColorMap("biome");
    GameManager::texturePackLoader->getColorMap("water");
}

void TerrainGenerator::CalculateSurfaceDensity(double X, double Y, double Z, double *SurfaceDensity, int size, int offset, int oct, double pers, double freq)
{
    int i, j, k;
    double n1=0, n2=0;
    for (i = 0; i < size; i++){
        for (j = 0; j < 5; j++){
            for (k = 0; k < 5; k++){
            //    n1 = ridged_octave_noise_3d_1(2, 0.6, 0.003, (double)(X + k*8.0)*0.86, (double)(Y + i*4.0)*1.05, (double)(Z + j*8.0)*0.86);
                n2 = ridged_octave_noise_3d_2(oct, pers, freq, (double)(X + k*8.0 + offset), (double)(Y + i*4.0 + offset)*0.3, (double)(Z + j*8.0 +offset));
                if (n1 > n2){
                    SurfaceDensity[i*25 + j*5 + k] = n1;
                }else{
                    SurfaceDensity[i*25 + j*5 + k] = n2;
                }
            }
        }
    }
}

void TerrainGenerator::CalculateStalactiteDensity(glm::ivec3 pos, double *CaveDensity, int size, int offset, int oct, double pers, double freq)
{
    int i, j, k;
    double n1=0, n2=0;
    for (i = 0; i < size; i++){
        for (j = 0; j < 9; j++){
            for (k = 0; k < 9; k++){
        
                n2 = ridged_octave_noise_3d_2(oct, pers, freq, (double)(pos.x + k*4.0 + offset)*1.0, (double)(pos.y + i*4.0 + offset)*0.001, (double)(pos.z + j*4.0 +offset)*1.0);
                CaveDensity[i*81 + j*9 + k] = n2;

            }
        }
    }
}

void TerrainGenerator::CalculateCaveDensity(glm::ivec3 pos, double *CaveDensity, int size, int offset, int oct, double pers, double freq)
{
    int i, j, k;
    double n1=0, n2=0;
    for (i = 0; i < size; i++){
        for (j = 0; j < 5; j++){
            for (k = 0; k < 5; k++){
        //        n1 = ridged_octave_noise_3d_2(oct, pers, freq, (double)(pos.x + k*8.0 + offset+2000), (double)(pos.y + i*4.0 + offset+1000)*0.01, (double)(pos.z + j*8.0 +offset- 2000));
                n2 = ridged_octave_noise_3d_2(oct, pers, freq, (double)(pos.x + k * 8 + offset), (double)(pos.y + i * 4 + offset)*1.3, (double)(pos.z + j * 8 +offset));
            //    if (n1 > n2){
            //        CaveDensity[i*25 + j*5 + k] = n1;
            //    }else{
        //            if (ABS(n2 - 0.55) > ABS(n1 - 0.55)){
                        CaveDensity[i*25 + j*5 + k] = n2;
        //            }else{
        //                CaveDensity[i*25 + j*5 + k] = n1;
        //            }
            //    }
            }
        }
    }
}

//bool TerrainGenerator::IsCave(int x, int y, int z)
//{
//    double noiseHeight = ridged_octave_noise_3d(5, 0.60, 0.005, (double)x, (double)y, (double)z);
//    return (noiseHeight > 0.86);
//}

double TerrainGenerator::GetRiverNoiseHeight(double &noiseHeight, NoiseInfo *noisef, double x, double y, double z, GLuint &flags)
{
    double h = scaled_octave_noise_3d(noisef->octaves, noisef->persistence, noisef->frequency, noisef->lowBound, noisef->upBound, x, y, z);
    double hm = 1.0;
    if (noiseHeight > 500) hm = MAX(0, 1.0 - (noiseHeight-500)*0.00025);
    if (h >= 0.46){
        if (h < 0.5){
            if (noiseHeight > 0) noiseHeight = noiseHeight - noiseHeight*(1.0 - (0.5-h)*100.0/4.0)*hm;
        }else if (h < 0.545){
            if (h > 0.505){
                if (noiseHeight > 0) noiseHeight = noiseHeight - noiseHeight*((0.545-h)*100.0/4.0)*hm;
            }else{ //bottom of river
                if (noiseHeight > 0){
                    noiseHeight = noiseHeight - (noiseHeight + (30+noiseHeight/500.0)*(1.0 - ABS(h-.5025)*400.0))*hm;
                }else{
                    noiseHeight = noiseHeight - (30)*(1.0 - ABS(h-.5025)*400.0)*hm;
                }
            }
        }
    }
    return h;
}

void TerrainGenerator::GetTributaryNoiseHeight(double &noiseHeight, NoiseInfo *noisef, double x, double y, double z, GLuint &flags, double rh)
{
    double h = scaled_octave_noise_3d(noisef->octaves, noisef->persistence, noisef->frequency, noisef->lowBound, noisef->upBound, x, y, z);
    double hm = 1.0;
    double hm2;
    double slopeSize = 0.06; //width of slope
    double waterSize = 0.007; //with of water slope
    if (noiseHeight > 50) hm = MAX(0, 1.0 - (noiseHeight-50)*0.005);
    hm2 = MAX(0, (1.0 - (ABS(rh - .5025) - 0.0325)*100));
    hm = MIN(hm, hm2);
    //hm = MIN(hm, hm2);
    if (rh < 0.5025){
        if (rh > .5) hm = MIN(hm, (.5025-rh)*400);
        if (h >= 0.4){
            if (h < 0.4 + slopeSize){
                if (noiseHeight > 0) noiseHeight = noiseHeight - noiseHeight*(1.0 - (0.4 + slopeSize - h)/slopeSize)*hm;
            }else if (h < 0.4 + (slopeSize + waterSize)*2){
                if (h > 0.4 + slopeSize + waterSize*2){
                    if (noiseHeight > 0) noiseHeight = noiseHeight - noiseHeight*((0.4 + (slopeSize + waterSize)*2-h)/slopeSize)*hm;
                }else{ //bottom of river
                    if (noiseHeight > 0){
                        noiseHeight = noiseHeight - (noiseHeight + (10+noiseHeight/500.0)*(1.0 - ABS(h-(0.4 + slopeSize + waterSize))/waterSize))*hm;
                    }else{
                        noiseHeight = noiseHeight - (10)*(1.0 - ABS(h-(0.4 + slopeSize + waterSize))/waterSize)*hm;
                    }
                }
            }
        }
    }else{
        if (rh < .505) hm = MIN(hm, (rh-.5025)*400);
        if (h >= 0.50){
            if (h < 0.5 + slopeSize){
                if (noiseHeight > 0) noiseHeight = noiseHeight - noiseHeight*(1.0 - (0.5 + slopeSize - h)/slopeSize)*hm;
            }else if (h < 0.5 + (slopeSize + waterSize)*2){
                if (h > 0.5 + slopeSize + waterSize*2){
                    if (noiseHeight > 0) noiseHeight = noiseHeight - noiseHeight*((0.5 + (slopeSize + waterSize)*2-h)/slopeSize)*hm;
                }else{ //bottom of river
                    if (noiseHeight > 0){
                        noiseHeight = noiseHeight - (noiseHeight + (10+noiseHeight/500.0)*(1.0 - ABS(h-(0.5 + slopeSize + waterSize))/waterSize))*hm;
                    }else{
                        noiseHeight = noiseHeight - (10)*(1.0 - ABS(h-(0.5 + slopeSize + waterSize))/waterSize)*hm;
                    }
                }
            }
        }
    }
}

void TerrainGenerator::GetNoiseHeight(double &noiseHeight, NoiseInfo *noisef, double x, double y, double z, GLuint &flags)
{
    int type = noisef->type;
    double h;
    double md = 1.0, off;

    if (noisef == NULL){
        std::cout << "ERROR NULL NULL NULL";
        int a;
        std::cin >> a;
    }
    
    NoiseInfo *nf2 = noisef->modifier;
    if (nf2){
        md = (octave_noise_3d(nf2->octaves, nf2->persistence, nf2->frequency, x, y, z)+1.0)*0.5;
        if (md <= nf2->lowBound) return;
        md = (md-nf2->lowBound)*nf2->upBound;
        if (md > 1.0) md = 1.0;
    }

    switch (type){
        case 1: //small mountains
            h = scaled_ridged_octave_noise_3d(noisef->octaves, noisef->persistence, noisef->frequency, noisef->lowBound, noisef->upBound, x, y, z);
            if (h > 16.0){
                noiseHeight += md * pow((h-16.0) * noisef->scale, 1.25); 
            }
            break;
        case 2: // Large Mountains and lakes    
            h = scaled_octave_noise_3d(noisef->octaves, noisef->persistence, noisef->frequency, noisef->lowBound, noisef->upBound, x, y, z);
            if (h > 7.5){
                noiseHeight += md * pow((h-7.5) * noisef->scale, 1.50);
            }else if (h < 4.0){
                noiseHeight -= md * ABS(4.0-h)/4 * noisef->scale*40;
            }
            break;
        case 3: //hills and plateaus
            h = scaled_octave_noise_3d(noisef->octaves, noisef->persistence, noisef->frequency, noisef->lowBound, noisef->upBound, x, y, z);
            if (h > 7.42){ //plateaus
                flags |= PLATEAU;
                noiseHeight += md * ((h-7.42) * 10 + 0.015*4000 + 0.505*noisef->scale);
            }else if (h > 7.405){
                flags |= PLATEAU;
                noiseHeight += md * ((h-7.405) * 4000 + 0.505*noisef->scale);
            }else if (h > 6.9){
                flags |= PLATEAU;
                noiseHeight += md * (h-6.9) * noisef->scale;
            }
            break;
            //}else if (h > 2.0 && h < 4.0) { //uses cos function for wormy hills
            //    h = (h-1.0)*3.14159265359*7;
            //    noiseHeight += cos(h)*100 + 100;
            //}
        case 4: //Seaside Cliffs
            if (noiseHeight < 100 && noiseHeight > -10){
                h = scaled_octave_noise_3d(noisef->octaves, noisef->persistence, noisef->frequency, noisef->lowBound, noisef->upBound, x, y, z);
                if (noiseHeight > 0){
                    if (h > 2){
                        noiseHeight += md * (h-2.0)*(100 - noiseHeight);
                    }else if (h < 1.5){
                        noiseHeight -= md * ((noiseHeight*(125.0/128.0) - (noiseHeight)*(noiseHeight)*(noiseHeight)*(noiseHeight)*(noiseHeight)/102400000.0)*(1.5-h)*1.5);
                    }
                }else{
                    if (h > 2){
                        noiseHeight -= md * (h-2.0)*(11 + noiseHeight);
                    }
                }
            }
            break;
        case 5: //plain old ridged
            noiseHeight += md * scaled_ridged_octave_noise_3d(noisef->octaves, noisef->persistence, noisef->frequency, noisef->lowBound, noisef->upBound, x, y, z);
            break;
        case 6: //billowy hills
            noiseHeight += md * scaled_reverse_ridged_octave_noise_3d(noisef->octaves, noisef->persistence, noisef->frequency, noisef->lowBound, noisef->upBound, x, y, z);
            break;
        case 7: //jagged cliffs
            h = (octave_noise_3d(noisef->octaves, noisef->persistence, noisef->frequency, x, y, z)+1.0)*5.0; //number between 0 and 10
            //if h is 0.1 or less greater than the lowest integer value, it is a steep slope and we multiply by 10,
            //otherwise its a flat top
            if ((h > 1.1) && ((int)h > (int)(h-0.1))){ //steep part
                h = ((int)h-1)+(h-(int)h)*10.0;
            }else{ //flat part
                h = (int)h; 
            }
            noiseHeight += md * (h)*noisef->scale;
            break;
        case 8: //Rare Huge Mountains
            noiseHeight += md * scaled_ridged_octave_noise_3d(noisef->octaves, noisef->persistence, noisef->frequency, noisef->lowBound, noisef->upBound, x, y, z)*noisef->scale;
            break;
        case 9: //Volcano
            h = octave_noise_3d(noisef->octaves, noisef->persistence, noisef->frequency,  x, y, z);
            if (h > 0.77){
                noiseHeight *= md * MAX(0,(1.0 - (h-0.45)*2));
                flags |= VOLCANO;
                noiseHeight += noisef->scale*1.08 - (0.05)*noisef->scale*12.0 - (h-0.77)*noisef->scale*6.0;
            }else if (h > 0.72){
                noiseHeight *= md * MAX(0,(1.0 - (h-0.45)*2));
                flags |= VOLCANO;
                noiseHeight += noisef->scale*1.08 - (h-0.72)*noisef->scale*12.0;
            }else if (h > 0.45){
                noiseHeight *= md * MAX(0,(1.0 - (h-0.45)*2));
                flags |= VOLCANO;
                noiseHeight += md * (h-0.45)*noisef->scale*4;
            }
            break;
        case 10: //canyons
            h = (octave_noise_3d(noisef->octaves, noisef->persistence, noisef->frequency, x, y, z)+1.0)*50.0; //0-100
            off = 5.0;
            if (h < 40.0+off){ //gradual slope up
                //noiseHeight += (h-(20.0+off))*0.05*noisef->scale;
                noiseHeight += md * noisef->scale;
            }else if (h < 50.0+off){ //steep down
                if ((int)h > (int)(h-0.2)){ //steep part
                    h = ((int)h)+(h-(int)h)*2.5-(40.0+off);
                }else{ //flat part
                    h = ((int)h)+0.5-(40.0+off)+(h-(int)h-0.2)*0.625;
                }
                noiseHeight += md * (noisef->scale - (h)*noisef->scale*0.1);
            }else if (h < 55.0+off){ //gradual down
                noiseHeight += md * (-(h-(50.0+off))*0.025*noisef->scale);
            }else if (h < 60.0+off){ //gradual up
                noiseHeight += md * (-(5.0-(h-(55.0+off)))*0.025*noisef->scale);
            }else if (h < 70.0+off){ //steep up
                if ((int)h > (int)(h-0.2)){ //steep part
                    h = ((int)h)+(h-(int)h)*2.5-(60.0+off);
                }else{ //flat part
                    h = ((int)h)+0.5-(60.0+off)+(h-(int)h-0.2)*0.625;
                }
                noiseHeight += md * (h)*noisef->scale*0.1;
            }else if (h > 70.0+off){ //top
                noiseHeight += md * noisef->scale;
            }
            
            

        /*    off = -0.3;
            if (h > -0.2+off){
                if (h < 0.3+off){ //slope up
                    noiseHeight += (h-(-0.2+off))*2.0*noisef->scale;
                }else if (h < 0.35+off){ //steep down
                    noiseHeight += noisef->scale - (h-(0.3+off))*10*noisef->scale;
                }else if (h < 0.4+off){ //slight down
                    noiseHeight += noisef->scale - 0.5*noisef->scale - (h-(0.35+off))*2.0*noisef->scale;
                }else if (h < 0.45+off){ //flat bottom
                    noiseHeight += noisef->scale - 0.5*noisef->scale - 0.1*noisef->scale;
                }else if ( h < 0.5+off){ //slight up
                    noiseHeight += noisef->scale - 0.5*noisef->scale - (0.05-(h-(0.45+off)))*2.0*noisef->scale;
                }else if ( h < 0.55+off) { //steep up
                    noiseHeight += noisef->scale - (0.05-(h-(0.5+off)))*10*noisef->scale;
                }else if (h > 0.55+off){ //top
                    noiseHeight += noisef->scale;
                }
            } */
            break;
        case 11: //oceans
            h = md * scaled_ridged_octave_noise_3d(noisef->octaves, noisef->persistence, noisef->frequency, noisef->lowBound, noisef->upBound, x, y, z);
            if (h < 0) {
                noiseHeight += h * noisef->scale;
            }else{
                noiseHeight += h * noisef->scale * 0.5;
            }
            break;
        default: //base noise
            noiseHeight += md * scaled_octave_noise_3d(noisef->octaves, noisef->persistence, noisef->frequency, noisef->lowBound, noisef->upBound, x, y, z);
            break;
    }

    //}else if (h > 4.5f){ //independent mountins 
            //    noiseHeight += (1.25f - ABS(h - 5.75f)) * 250;
    //}
}

double TerrainGenerator::GetTemperature(double x, double y, double z)
{
    if (!temperatureNoiseFunction) return defaultTemp; 
    return GameManager::planet->baseTemperature + scaled_octave_noise_3d(temperatureNoiseFunction->octaves, temperatureNoiseFunction->persistence, 
        temperatureNoiseFunction->frequency, temperatureNoiseFunction->lowBound, temperatureNoiseFunction->upBound, x, y, z);
}

double TerrainGenerator::GetRainfall(double x, double y, double z)
{
    if (!rainfallNoiseFunction) return defaultRain;
    return GameManager::planet->baseRainfall + scaled_octave_noise_3d(rainfallNoiseFunction->octaves, rainfallNoiseFunction->persistence,
        rainfallNoiseFunction->frequency, rainfallNoiseFunction->lowBound, rainfallNoiseFunction->upBound, x, y, z) - 20.0;
}

void TerrainGenerator::GetBaseBiome(int x, int y, int z, Biome **b, int h)
{
    int temperature, rainfall;
    temperature = MAX(GetTemperature(x, y, z) - MAX(h - 500, 0)/50, 0);
    rainfall = MAX(GetRainfall(x, y, z) - MAX(h - 500, 0)/150, 0);
    *b = (GameManager::planet->baseBiomesLookupMap.find(BiomeMap[rainfall][temperature]))->second;
}

void TerrainGenerator::setVoxelMapping(vvox::VoxelMapData* voxelMapData, int Radius, int pScale) {
    int rdir;
    voxelMapData->getGenerationIterationConstants(iPos, jPos, rPos, iDir, jDir, rdir);
    radius = Radius * rdir;
    scale = pScale;
}

void TerrainGenerator::SetLODFace(int Ipos, int Jpos, int Rpos, int Radius, int idir, int jdir, float pscale)
{
    iPos = Ipos;
    jPos = Jpos;
    rPos = Rpos;
    iDir = idir;
    jDir = jdir;
    radius = Radius;
    scale = pscale;
}

void TerrainGenerator::GenerateHeightMap(HeightData *lodMap, int icoord, int jcoord, int size, int iend, int jend, int step, int iskip, bool *hasWater)
{
    double random = 0;
    double noiseHeight = 0;
    double probheight = 0;
    double rh, rh2;
    double lattitude;
    double tempMod;
    glm::dvec3 pos;
    int biomeColor;
    double temperature;
    double rainfall;
    Biome *mainBiome;
    int depth;

    double ic, jc;
    
    std::map <int, Biome *>::iterator biomeit;
    
    for (int i = 0; i < iend; i++){
        for (int j = 0; j < jend; j++){
            if (iskip && (j%iskip == 0) && (i%iskip == 0)) continue;

            ic = (double)(icoord+(i*step)) * scale * iDir;
            jc = (double)(jcoord+(j*step)) * scale * jDir;

            pos[iPos] = ic + scaled_octave_noise_2d(4, 0.6, 0.019, -7.0, 7.0, ic, jc);
            pos[jPos] = jc + scaled_octave_noise_2d(4, 0.6, 0.02, -7.0, 7.0, jc, ic);
            pos[rPos] = radius;

            lattitude = glm::normalize(pos).y;
            if (lattitude < 0.0) lattitude = -lattitude;
            
            tempMod = 110.0 - (lattitude*200.0);

            //BASE NOISE
            noiseHeight = heightModifier;
            probheight = 0;
            GLuint flags = 0;
            for (unsigned int n = 0; n < noiseFunctions.size(); n++){
                GetNoiseHeight(noiseHeight, &noiseFunctions[n], pos.x, pos.y, pos.z, flags);
            }

            temperature = GetTemperature(pos.x, pos.y, pos.z) - MAX((int)noiseHeight - 500.0, 0)*0.01 + scaled_octave_noise_3d(3, 0.45, 0.013, 0.0, 2.0, pos.x, pos.y, pos.z) + tempMod;
            rainfall = GetRainfall(pos.x, pos.y, pos.z) - MAX((int)noiseHeight - 500,0)*0.006667 + scaled_octave_noise_3d(3, 0.5, 0.01, 0.0, 2.0, pos.z, pos.y, pos.x);
        
            if (biomeOffsetNoiseFunction) GetNoiseHeight(probheight, biomeOffsetNoiseFunction, pos.x, pos.y, pos.z, flags);

            if (temperature < 0.0) temperature = 0.0;
            if (temperature > 255.0) temperature = 255.0;
            if (rainfall < 0.0) rainfall = 0;
            if (rainfall > 255.0) rainfall = 255.0;
            biomeColor = BiomeMap[(int)rainfall][(int)temperature];
            biomeit = GameManager::planet->baseBiomesLookupMap.find(biomeColor);
            if (biomeit == GameManager::planet->baseBiomesLookupMap.end()){
                lodMap[i*size + j].biome = &blankBiome;
            }
            else{
                lodMap[i*size + j].biome = biomeit->second;
            }

            double sandDepth = 0.0;
            for (unsigned int n = 0; n < GameManager::planet->mainBiomesVector.size(); n++){
                mainBiome = GameManager::planet->mainBiomesVector[n];
                if (rainfall >= mainBiome->lowRain && rainfall <= mainBiome->highRain && 
                    temperature >= mainBiome->lowTemp && temperature <= mainBiome->highTemp){

                    rh = (octave_noise_3d(mainBiome->distributionNoise.octaves, mainBiome->distributionNoise.persistence, mainBiome->distributionNoise.frequency, pos.x, pos.y, pos.z) + 1.0)*0.5;
                    if (rh >= mainBiome->distributionNoise.lowBound){
                        rh = (rh - mainBiome->distributionNoise.lowBound)/mainBiome->distributionNoise.upBound;
                        rh2 = ((mainBiome->maxHeight - noiseHeight))/mainBiome->maxHeightSlopeLength;
                        if (rh2 >= 0.0){
                            if (rh2 < rh) rh = rh2;
                            InfluenceTerrainWithBiome(mainBiome, &(lodMap[i*size+j]), pos.x, pos.y, pos.z, temperature, rainfall, noiseHeight, sandDepth, flags, rh);
                        }
                    }
                }
            }

            //rivers
            if (riverNoiseFunction){
                rh = GetRiverNoiseHeight(noiseHeight, riverNoiseFunction, pos.x, pos.y, pos.z, flags);
                if (rh > 0.4 && rh < 0.605){
                    GetTributaryNoiseHeight(noiseHeight, tributaryNoiseFunction, pos.x, pos.y, pos.z, flags, rh);
                }
            }
            if (preturbNoiseFunction) GetNoiseHeight(noiseHeight, preturbNoiseFunction, pos.x, pos.y, pos.z, flags);

            lodMap[i*size+j].height = (int)(noiseHeight + sandDepth);
            if (hasWater != NULL && lodMap[i*size+j].height < 0) *hasWater = 1;
            lodMap[i*size+j].sandDepth = MAX((int)sandDepth, 0);
            lodMap[i*size+j].snowDepth = MAX((FREEZETEMP - temperature), 0)*MAX((rainfall-60)/2, 0)/170;
            lodMap[i*size + j].flags = 0;
            if (temperature < 0) temperature = 0;
            if (temperature > 255.0) temperature = 255.0;
            if (rainfall < 0.0) rainfall = 0;
            if (rainfall > 255.0) rainfall = 255.0;
            lodMap[i*size+j].temperature = (int)temperature;
            lodMap[i*size+j].rainfall = (int)rainfall;

            //water depth
            depth = -lodMap[i*size + j].height / 5;
            if (depth > 255) {
                depth = 255;
            } else if (depth < 0) {
                depth = 0;
            }
            lodMap[i*size + j].depth = (ui8)(255 - depth);
        }
    }
}

void TerrainGenerator::InfluenceTerrainWithBiome(Biome *b, HeightData *hv, double x, double y, double z, double &temp, double &rain, double &height, double &sandDepth, GLuint &flags, double terrainMult)
{
    NoiseInfo *noisef;
    double tmp = 1.0, mult = 1.0;
    double newHeight = 0.0;
    Biome *b2;
    if (temp < b->lowTemp + b->tempSlopeLength){
        mult = (temp - b->lowTemp)/b->tempSlopeLength;
    }else if (temp > b->highTemp - b->tempSlopeLength){
        mult = (b->highTemp - temp)/b->tempSlopeLength;
    }
    if (rain < b->lowRain + b->rainSlopeLength){
        tmp = (rain - b->lowRain)/b->rainSlopeLength;
    }else if (rain > b->highRain - b->rainSlopeLength){
        tmp = (b->highRain - rain)/b->rainSlopeLength;
    }
    if (tmp < mult) mult = tmp;
    if (terrainMult < mult) mult = terrainMult;
    if (mult >= b->applyBiomeAt) hv->biome = b;
    tmp = (1.0-(mult)*(1.0-b->minTerrainMult));
    if (height < 0) tmp = 1.0; // TEMP: ************************************************************* NO BIOME CAN DESTROY OCEAN
    height *= tmp; //bring down the existing terrain influence
    sandDepth *= tmp;
    
    for (unsigned int n = 0; n < b->terrainNoiseList.size(); n++){
        noisef = &(b->terrainNoiseList[n]);
        if (noisef->composition == 1){ //sand
            tmp = 0.0;
            GetNoiseHeight(tmp, noisef, x, y, z, flags);
            sandDepth += tmp*mult;
        }else{ //other
            GetNoiseHeight(newHeight, noisef, x, y, z, flags);
        }
    }
    height += newHeight*mult;

    //temp = (MAX(GetTemperature(x, y, z) - MAX(height - 500.0, 0)/50.0, 0) + scaled_octave_noise_3d(3, 0.45, 0.013, 0.0, 2.0, x, y, z));
    //rain = (MAX(GetRainfall(x, y, z) - MAX(height - 500,0)/150, 0) + scaled_octave_noise_3d(3, 0.5, 0.01, 0.0, 2.0, z, y, x));

    for (unsigned int n = 0; n < b->childBiomes.size(); n++){
        b2 = b->childBiomes[n];
        if (rain >= b2->lowRain && rain <= b2->highRain && 
            temp >= b2->lowTemp && temp <= b2->highTemp){

            tmp = (octave_noise_3d(b2->distributionNoise.octaves, b2->distributionNoise.persistence, b2->distributionNoise.frequency, x, y, z) + 1.0)*0.5;
            if (tmp >= b2->distributionNoise.lowBound){
                tmp = (tmp - b2->distributionNoise.lowBound)/b2->distributionNoise.upBound;        
                if (mult < tmp) tmp = mult;
                InfluenceTerrainWithBiome(b2, hv, x, y, z, temp, rain, height, sandDepth, flags, tmp); //recursive
            }
        }
    }
}

double TerrainGenerator::GenerateSingleHeight(int x, int y, int z)
{
    GLuint flags = 0;
    double noiseHeight = heightModifier;
    NoiseInfo *noisef;
    for (unsigned int n = 0; n < noiseFunctions.size(); n++){
        noisef = &noiseFunctions[n];
        GetNoiseHeight(noiseHeight, noisef, x, y, z, flags);
    }
    if (riverNoiseFunction){
        double rh = GetRiverNoiseHeight(noiseHeight, riverNoiseFunction, x, y, z, flags);
        if (rh > 0.4 && rh < 0.605){
            GetTributaryNoiseHeight(noiseHeight, tributaryNoiseFunction, x, y, z, flags, rh);
        }
    }

    if (preturbNoiseFunction) GetNoiseHeight(noiseHeight, preturbNoiseFunction, x, y, z, flags);
    return noiseHeight;
}

//void TerrainGenerator::GenerateVoxelMap(vector <vector <HeightData > > *heightMap, int x, int y, int z, int width, int height, int iStart, int jStart){
//    double random = 0;
//    double noiseHeight;
//    double probheight = 0;
//    double rh, rh2;
//    double mx, my, mz;
//    Biome *mainBiome;
//    NoiseInfo *noisef;
//    double rainfall;
//    double temperature;
//    int biomeColor;
//    for (int i = iStart; i < iStart+height; i++){
//        for (int j = jStart; j < jStart+width; j++){
//            mx = ((double)(x+j)) + scaled_octave_noise_2d(4, 0.6, 0.02, -7.0, 7.0, x+j, z+i);
//            my = y;
//            mz = ((double)(z+i)) + scaled_octave_noise_2d(4, 0.6, 0.019, -7.0, 7.0, z+i, x+j);
//
//            GLuint flags = 0;
//            noiseHeight = heightModifier;
//            probheight = 0;
//            for (unsigned int n = 0; n < noiseFunctions.size(); n++){
//                noisef = &noiseFunctions[n];
//                GetNoiseHeight(noiseHeight, noisef, mx, my, mz, flags);
//            }
//
//            temperature = (MAX(GetTemperature(mx, my, mz) - MAX(noiseHeight - 500.0, 0)/50.0, 0) + scaled_octave_noise_2d(3, 0.45, 0.013, 0.0, 2.0, mx, mz));
//            rainfall = (MAX(GetRainfall(mx, my, mz) - MAX(noiseHeight - 500,0)/150.0, 0) + scaled_octave_noise_2d(3, 0.5, 0.01, 0.0, 2.0, mz, mx));
//
//            if (biomeOffsetNoiseFunction) GetNoiseHeight(probheight, biomeOffsetNoiseFunction, mx, my, mz, flags);
//
//            if (temperature < 0) temperature = 0;
//            if (temperature > 255.0) temperature = 255.0;
//            if (rainfall < 0) rainfall = 0;
//            if (rainfall > 255.0) rainfall = 255.0;
//            (*heightMap)[i][j].tempOff = probheight;
//            biomeColor = BiomeMap[(int)rainfall][(int)temperature];
//            (*heightMap)[i][j].biome = (GameManager::planet->baseBiomesLookupMap.find(biomeColor))->second;
//
//            double sandDepth = 0.0;
//            for (unsigned int n = 0; n < GameManager::planet->mainBiomesVector.size(); n++){
//                mainBiome = GameManager::planet->mainBiomesVector[n];
//                if (rainfall >= mainBiome->lowRain && rainfall <= mainBiome->highRain && 
//                    temperature >= mainBiome->lowTemp && temperature <= mainBiome->highTemp){
//
//                    rh = (octave_noise_3d(mainBiome->distributionNoise.octaves, mainBiome->distributionNoise.persistence, mainBiome->distributionNoise.frequency, x+j, my, z+i) + 1.0)*0.5;
//                    if (rh >= mainBiome->distributionNoise.lowBound){
//                        rh = (rh - mainBiome->distributionNoise.lowBound)/mainBiome->distributionNoise.upBound;
//                        rh2 = (mainBiome->maxHeight - noiseHeight)/mainBiome->maxHeightSlopeLength;
//                        if (rh2 >= 0){
//                            if (rh2 < rh) rh = rh2;
//                            InfluenceTerrainWithBiome(mainBiome, &((*heightMap)[i][j]), x+j, my, z+i, temperature, rainfall, noiseHeight, sandDepth, flags, rh);
//                        }
//                    }
//                }
//            }
//
//            if (riverNoiseFunction){
//                rh = GetRiverNoiseHeight(noiseHeight, riverNoiseFunction, mx, my, mz, flags);
//                if (rh > 0.4 && rh < 0.605){
//                    GetTributaryNoiseHeight(noiseHeight, tributaryNoiseFunction, mx, my, mz, flags, rh);
//                }
//            }
//            if (preturbNoiseFunction) GetNoiseHeight(noiseHeight, preturbNoiseFunction, mx, my, mz, flags);
//            
//            (*heightMap)[i][j].height = (int)noiseHeight;
//            if (sandDepth >= 0.0){
//                (*heightMap)[i][j].sandDepth = (int)(sandDepth + noiseHeight)-(int)noiseHeight;
//            }else{
//                (*heightMap)[i][j].sandDepth = 0;
//                (*heightMap)[i][j].height += (int)(sandDepth + noiseHeight) - (int)noiseHeight;
//            }
//            (*heightMap)[i][j].snowDepth = MAX((FREEZETEMP - temperature), 0)*MAX((rainfall-60)/2, 0)/170;
//            if (temperature < 0) temperature = 0;
//            if (temperature > 255.0) temperature = 255.0;
//            if (rainfall < 0.0) rainfall = 0;
//            if (rainfall > 255.0) rainfall = 255.0;
//            (*heightMap)[i][j].temperature = (int)temperature;
//            (*heightMap)[i][j].rainfall = (int)rainfall;
//            (*heightMap)[i][j].flags = flags;
//        }
//    }
//}

void TerrainGenerator::AddNoiseFunction(double persistence, double frequency, int octaves, double lowBound, double upBound, double Scale, int type, bool isModifier)
{
    NoiseInfo noisef;
    noisef.persistence = persistence;
    noisef.frequency = frequency;
    noisef.octaves = octaves;
    noisef.lowBound = lowBound;
    noisef.upBound = upBound;
    noisef.type = type;
    noisef.scale = Scale;
    if (isModifier){
        if (noiseFunctions.size()){
            noiseFunctions.back().modifier = new NoiseInfo; //freed in NoiseInfo destructor
            (*(noiseFunctions.back().modifier)) = noisef;
        }
    }else{
        numFunctions++;
        noiseFunctions.push_back(noisef);
    }
}

void TerrainGenerator::ClearFunctions()
{
    noiseFunctions.clear();
    numFunctions = 0;
}

void TerrainGenerator::SetTributaryNoiseFunction(double persistence, double frequency, int octaves, double lowBound, double upBound, double Scale, int type)
{
    if (!tributaryNoiseFunction) tributaryNoiseFunction = new NoiseInfo;
    tributaryNoiseFunction->persistence = persistence;
    tributaryNoiseFunction->frequency = frequency;
    tributaryNoiseFunction->octaves = octaves;
    tributaryNoiseFunction->lowBound = lowBound;
    tributaryNoiseFunction->upBound = upBound;
    tributaryNoiseFunction->type = type;
    tributaryNoiseFunction->scale = Scale;
}

void TerrainGenerator::SetPreturbNoiseFunction(double persistence, double frequency, int octaves, double lowBound, double upBound, double Scale, int type)
{
    if (!preturbNoiseFunction) preturbNoiseFunction = new NoiseInfo;
    preturbNoiseFunction->persistence = persistence;
    preturbNoiseFunction->frequency = frequency;
    preturbNoiseFunction->octaves = octaves;
    preturbNoiseFunction->lowBound = lowBound;
    preturbNoiseFunction->upBound = upBound;
    preturbNoiseFunction->type = type;
    preturbNoiseFunction->scale = Scale;
}

void TerrainGenerator::SetRiverNoiseFunction(double persistence, double frequency, int octaves, double lowBound, double upBound, double Scale, int type)
{
    if (!riverNoiseFunction) riverNoiseFunction = new NoiseInfo;
    riverNoiseFunction->persistence = persistence;
    riverNoiseFunction->frequency = frequency;
    riverNoiseFunction->octaves = octaves;
    riverNoiseFunction->lowBound = lowBound;
    riverNoiseFunction->upBound = upBound;
    riverNoiseFunction->type = type;
    riverNoiseFunction->scale = Scale;
}

void TerrainGenerator::SetBiomeOffsetNoiseFunction(double persistence, double frequency, int octaves, double lowBound, double upBound, double Scale, int type)
{
    if (!biomeOffsetNoiseFunction) biomeOffsetNoiseFunction = new NoiseInfo;
    biomeOffsetNoiseFunction->persistence = persistence;
    biomeOffsetNoiseFunction->frequency = frequency;
    biomeOffsetNoiseFunction->octaves = octaves;
    biomeOffsetNoiseFunction->lowBound = lowBound;
    biomeOffsetNoiseFunction->upBound = upBound;
    biomeOffsetNoiseFunction->type = type;
    biomeOffsetNoiseFunction->scale = Scale;
}

void TerrainGenerator::SetTemperatureNoiseFunction(double persistence, double frequency, int octaves, double lowBound, double upBound, double Scale, int type)
{
    if (!temperatureNoiseFunction) temperatureNoiseFunction = new NoiseInfo;
    temperatureNoiseFunction->persistence = persistence;
    temperatureNoiseFunction->frequency = frequency;
    temperatureNoiseFunction->octaves = octaves;
    temperatureNoiseFunction->lowBound = lowBound;
    temperatureNoiseFunction->upBound = upBound;
    temperatureNoiseFunction->type = type;
    temperatureNoiseFunction->scale = Scale;
}

void TerrainGenerator::SetRainfallNoiseFunction(double persistence, double frequency, int octaves, double lowBound, double upBound, double Scale, int type)
{
    if (!rainfallNoiseFunction) rainfallNoiseFunction = new NoiseInfo;
    rainfallNoiseFunction->persistence = persistence;
    rainfallNoiseFunction->frequency = frequency;
    rainfallNoiseFunction->octaves = octaves;
    rainfallNoiseFunction->lowBound = lowBound;
    rainfallNoiseFunction->upBound = upBound;
    rainfallNoiseFunction->type = type;
    rainfallNoiseFunction->scale = Scale;
}

void TerrainGenerator::postProcessHeightmap(HeightData heightData[CHUNK_LAYER]) {
    i32 minNearHeight; //to determine if we should remove surface blocks
    i32 heightDiffThreshold = 3;
    i32 tmp;
    i32 maph;
    Biome *biome;
    i32 sandDepth, snowDepth;
    for (i32 z = 0; z < CHUNK_WIDTH; z++) {
        for (i32 x = 0; x < CHUNK_WIDTH; x++) {

            //*************Calculate if it is too steep ***********************
            maph = heightData[z*CHUNK_WIDTH + x].height;
            biome = heightData[z*CHUNK_WIDTH + x].biome;
            sandDepth = heightData[z*CHUNK_WIDTH + x].sandDepth;
            snowDepth = heightData[z*CHUNK_WIDTH + x].snowDepth;

            minNearHeight = maph;
            if (x > 0) { //Could sentinalize this in the future
                minNearHeight = heightData[z*CHUNK_WIDTH + x - 1].height;
            } else {
                minNearHeight = maph + (maph - heightData[z*CHUNK_WIDTH + x + 1].height); //else use opposite side but negating the difference
            }
            if (x < CHUNK_WIDTH - 1) {
                tmp = heightData[z*CHUNK_WIDTH + x + 1].height;
            } else {
                tmp = maph + (maph - heightData[z*CHUNK_WIDTH + x - 1].height);
            }
            if (tmp < minNearHeight) minNearHeight = tmp;
            if (z > 0) {
                tmp = heightData[(z - 1)*CHUNK_WIDTH + x].height;
            } else {
                tmp = maph + (maph - heightData[(z + 1)*CHUNK_WIDTH + x].height);
            }
            if (tmp < minNearHeight) minNearHeight = tmp;
            if (z < CHUNK_WIDTH - 1) {
                tmp = heightData[(z + 1)*CHUNK_WIDTH + x].height;
            } else {
                tmp = maph + (maph - heightData[(z - 1)*CHUNK_WIDTH + x].height);
            }
            if (tmp < minNearHeight) minNearHeight = tmp;

            if (maph - minNearHeight >= heightDiffThreshold && heightData[z*CHUNK_WIDTH + x].biome->looseSoilDepth) {
                heightData[z*CHUNK_WIDTH + x].flags |= TOOSTEEP;
            }
            //*************End TOOSTEEP calculation ***********************

            //**************START SURFACE BLOCK CALCULATION******************
            GLushort surfaceBlock = DIRT;
            if (maph == 0) {
                surfaceBlock = biome->beachBlock;
            } else if (maph < 0) {
                surfaceBlock = biome->underwaterBlock;
            } else {
                surfaceBlock = biome->surfaceBlock;
                if (surfaceBlock == DIRTGRASS && (snowDepth || sandDepth)) surfaceBlock = DIRT;
            }
            heightData[z*CHUNK_WIDTH + x].surfaceBlock = surfaceBlock;

            //**************END SURFACE BLOCK CALCULATION******************
        }
    }
}