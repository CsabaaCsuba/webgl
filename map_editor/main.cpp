#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <emscripten/fetch.h>
#include <string.h>

#include "../emscripten_fetch/fetch.hpp"

typedef enum TerrainType {
    PLAIN,
    HILL,
    MOUNTAIN,

    SWAMP,
    DESERT,
    STEPPE,
    FOREST,
    RAINFOREST,
    BEACH,

    CITY=255, //this is a flag, it can be flipped 
} TerrainType;

typedef enum BuildingType {
    AGRICULTURE,
    LIGHT_INDUSTRY,
    HEAVY_INDUSTRY,
    INFRASTRUCTURE,
    MILITARY,
    DEFENSE,

    ENTERTAINMENT, //like pubs, circuses
    RELIGION,

} BuildingType;

typedef struct {
    BuildingType type;


} Building;

typedef struct Province {
    uint32_t id;
    TerrainType terrain;
    uint32_t population;
    uint8_t building_limit;
    std::vector<Building> buildings;

    std::vector<float> vertices;
    
} Province;

int main(){

}