#ifndef FREE_SPACE_DATA_H
#define FREE_SPACE_DATA_H

#include <cstdint>
#include "common/opense_basic_type.h"
#pragma pack(push, 1)

struct AlgorithmResult
{
    uint32_t dataType;
    uint32_t dataSize;
    char data[0];

    enum ResultType {
        Flatness = 1,
        SmallObsLabel,
        JourneyLaneData
    };
};

struct FlatnessDataHead
{
    enum Identifier {
        HeatMap = 1,
        SingleMap,
        LabelMap,
        LeftRoi,
        WheelRegion,
        RoiHeight,
        Total
    };
    int type;
    int rows;
    int cols;
    int size;
    char data[0];
};

struct SmallObsLabel {
    opense::Rect<int> roi;
    int bpp;
    int rows;
    int cols;
    int size;
    char data[0];
};

struct FreespacePoint
{
    int property;
    float confidence;
    opense::Point<int> ics;
    opense::Point3<float> vcs;
};

#pragma pack(pop)

#endif
