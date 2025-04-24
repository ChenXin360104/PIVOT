#ifndef SCREEN_M4_H
#define SCREEN_M4_H

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

#include <iostream>
#include <fstream>
#include <vector>
#include <iostream>
#include <thread> // for std::this_thread::sleep_for
#include <chrono> // for std::chrono::seconds
#include <iostream>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>
//#include <format>
#include <unordered_map>
#include <map>
#include <sstream>

#include"SegmentTree.h"
#include"FunInfo.h"
#include"Heap.h"


using namespace std;


class M4
{
public:
    double max;
    double min;
    int start_time;
    int end_time;
    double st_v;
    double et_v;

    std::vector<vector<SegmentTreeNode *>> innerNodes;
    std::vector<SegmentTreeNode *> stNodes;
    std::vector<SegmentTreeNode *> etNodes;

    int stNodeIndex;
    int etNodeIndex;

    Heap<CompareMin> alternativeNodesMin;
    Heap<CompareMax> alternativeNodesMax;

    bool isCompletedMax;
    bool isCompletedMin;


    int errorPixels;

    M4() : max(-INFINITY), min(INFINITY),
           start_time(0), end_time(0),
           st_v(0), et_v(0),
           stNodeIndex(-1), etNodeIndex(-1),
           // alternativeNodesMax(nullptr), alternativeNodesMin(nullptr),
           // currentComputingNodeMax(nullptr), currentComputingNodeMin(nullptr),
           // currentComputingIntervalMax(nullptr), currentComputingIntervalMin(nullptr),
           isCompletedMax(false), isCompletedMin(false),
           // stInterval(nullptr), etInterval(nullptr),
           // minDLL(nullptr), maxDLL(nullptr),
           errorPixels(0)
    {
    }
};


class SCREEN_M4
{
public:
    std::vector<vector<M4*>> M4_arrays;
    std::vector<double> min_values;
    std::vector<double> max_values;

    std::string sx;
    std::string estimateType;

    vector<M4*> M4_array;
    int screenEnd;
    int screenStart;
    int height;
    int width;
    std::string experiment;
    std::string datasetname;
    std::string symbolName;
    double errorBound;
    int quantity;
    FunInfo *func;
    double nodeReductionRatio;
    double SQLtime;
    double totalTime;

    double buildNodeRate;
    vector<SegmentTree *> segmentTrees;
    double dataReductionRatio;

    int maxNodeNum;
    int nodeCount;
    double memLimit;
    std::string interact_type;
    std::vector<string> columns;

    double exactMax;
    double exactMin;
    double avgtMax;
    double avgtMin;
    double candidateMax;
    double candidateMin;

    double preError;
    double deltaError;
    int count;

    int dataStartTime;
    int dataEndTime;
    int dataDelta;
    int dataCont;

    int globalIntervalStartTime;
    int globalIntervalEndTime;

    int screenStartTimestamp;
    int screenEndTimestamp;

    int globalIntervalStart;
    int globalIntervalEnd;

    int delta;
    int intervalLength;
    int iterations;

    SCREEN_M4(const std::string &experiment,
              const std::string &datasetname,
              int quantity,
              const std::string &symbolName,
              int width,
              int height,
              double errorBound,
              FunInfo *func = nullptr)
        : screenEnd(0), screenStart(0), height(height), width(width),
          experiment(experiment), datasetname(datasetname), symbolName(symbolName),
          errorBound(errorBound), quantity(quantity), func(func), nodeReductionRatio(0),
          SQLtime(0), totalTime(0), buildNodeRate(0),
          dataReductionRatio(0), maxNodeNum(0), nodeCount(0), memLimit(0),
          exactMax(-INFINITY), exactMin(INFINITY),
          avgtMax(-INFINITY), avgtMin(INFINITY),
          candidateMax(-INFINITY), candidateMin(INFINITY),
          preError(0), deltaError(0), count(0),
          dataStartTime(-1), dataEndTime(-1), dataDelta(-1), dataCont(-1),
          globalIntervalStartTime(1420041600), globalIntervalEndTime(1704038399),
          screenStartTimestamp(1420041600), screenEndTimestamp(1704038399),
          globalIntervalStart(-1), globalIntervalEnd(-1),
          delta(60), intervalLength(60), iterations(0) {}
};



#endif // SCREEN_M4_H
