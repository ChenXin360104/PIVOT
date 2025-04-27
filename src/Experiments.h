#ifndef EXPERIMENTS_H
#define EXPERIMENTS_H

#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include <chrono>

#include <omp.h>

#include"SCREEN_M4.h"

#include "MemoryManager.h"
#include"TimerStats.h"
#include <regex>


#define MAXVS 20

u32 MAX_THREADS_MAIN = 1;

M4* M4_arrays[50][2000] ;
MemoryManager* mmdata;

// 使用示例
TimerStats stats;
std::unordered_map<std::string, SegmentTree *> trees;
std::unordered_map<std::string, MemoryManager *> mmdatas;






void printMemoryUsage(string& ss) {
    std::ifstream status_file("/proc/self/status");
    std::string line;

    while (std::getline(status_file, line)) {
        if (line.rfind("VmRSS:", 0) == 0) { // 以 VmRSS 开头的行
            std::cout << "[Memory] " << line << std::endl;
            break;
        }
    }

    // 使用正则表达式提取数字
    std::regex re(R"(\d+)");  // 匹配一个或多个数字
    std::smatch match;

    if (std::regex_search(line, match, re)) {
        // 输出第一个匹配的结果
        ss = match.str(0);
        std::cout << "Extracted number: " << match.str(0) << std::endl;
    } else {
        std::cout << "No number found." << std::endl;
    }

}

void genDatainfo(SCREEN_M4 &screen_m4)
{
    FunInfo *func = screen_m4.func;
//return;
    //  会core，后面优化
    if (screen_m4.datasetname == "nycdata")
    {

        // todo 后面根据数据选取的时间段进行调整。
        screen_m4.dataStartTime = 1420041600;
        screen_m4.dataEndTime = 1704038399;
        screen_m4.dataDelta = 60;
        screen_m4.dataCont = (screen_m4.dataEndTime-screen_m4.dataStartTime)/screen_m4.dataDelta + 1;

        cout<<"screen_m4.dataCont:"<<screen_m4.dataCont<<endl;

        if(screen_m4.screenEnd < 0){
            screen_m4.screenStart = 0;
            // screen_m4.screenEnd =screen_m4.dataCont-1;
        }
        screen_m4.screenStartTimestamp = screen_m4.dataStartTime+(screen_m4.screenStart-0)*screen_m4.dataDelta;

        // 检查 func 是否为 null 或者 func.funName 是否为空
        if (func->funName.empty())
        {
            return;
        }
        
        if (func->funName == "ave" || func->funName == "sum")
        {
            if (func->params == "hour")
            {
                screen_m4.intervalLength = 60;
            }
            else if (func->params == "day")
            {
                screen_m4.intervalLength = 60 * 24;
            }
            else if (func->params == "week")
            {
                screen_m4.intervalLength = 60 * 24 * 7;
            }
            else if (func->params == "month")
            {
                screen_m4.intervalLength = 60 * 24 * 30;
            }
            else if (func->params == "year")
            {
                screen_m4.intervalLength = 60 * 24 * 365;
            }
        }
    }
}

void ours_pg(const std::string &table_name, int dataCount, const std::vector<std::string> &columns, const std::string &symbol,
             const std::string &extra, int width, int height, const std::string &mode, int parallel, double errorBound,
             int startTime, int endTime, const std::string &interact_type, SCREEN_M4 &screen_m4)
{
    // Placeholder for "ours_pg" experiment
}


string removeEndChar(const string &str, const string &suffix)
{
    if (str.size() >= suffix.size() && str.substr(str.size() - suffix.size()) == suffix)
    {
        return str.substr(0, str.size() - suffix.size());
    }
    return str;
}

string mergeTables(string table_name, vector<string> &columns, const FunInfo &funInfo, const string &tableType)
{
    vector<string> order;
    order.push_back( to_string( stoi(columns[0])-1)); // 添加第一个列名

    for (size_t i = 1; i < columns.size(); ++i)
    {
        if (columns[i] == "" || columns[i] == "null")
        {
            continue;
        }

        if (funInfo.funName == "func1" || funInfo.funName == "boxcox_0" ||
            funInfo.funName == "boxcox_1_2" || funInfo.funName == "boxcox_1" || funInfo.funName == "boxcox_2")
        {
            // 一元计算，table_name_others不取
            if (order.size() > 0)
            {
                // 二元计算，table_name_others只取1个
                break;
            }
        }
        else if (funInfo.funName == "+" || funInfo.funName == "-" || funInfo.funName == "*" || funInfo.funName == "/" ||
                 funInfo.funName == "func3" || funInfo.funName == "func4"
                 || funInfo.funName == "L22")
        {
            if (order.size() > 1)
            {
                // 二元计算，table_name_others只取1个
                break;
            }
        }

        order.push_back(to_string( stoi(columns[i])-1)); // 添加其它列名
    }

    // 处理 table_name 的后缀
    if (tableType == "om3")
    {
        if (table_name.substr(table_name.size() - 4) != "_om3")
        {
            table_name = table_name + "_om3";
        }
    }
    else if (tableType == "tv")
    {
        if (table_name.substr(table_name.size() - 4) == "_om3")
        {
            table_name = removeEndChar(table_name, "_om3");
        }
    }

    // 更新 columns 为 order
    columns = order;

    // 计算列的数量
    size_t columnsNum = columns.size();
    // cout << "columns.length: " << columns.size() << endl;

    return table_name;
}

int getRealDataRowNum(SegmentTree *tree)
{
    return tree->linecount;
}

void computeM4TimeSE(int width, vector<int> &timeRange, vector<M4*> & M4_array)
{
    M4_array.clear();
    // 创建 width 个 M4 对象
    for (int i = 0; i < width; i++)
    {
        //stats.timestart("new M4");
        M4 * m4 = new M4();
        //stats.timeend("new M4");
        //cout << i << endl;

        //stats.timestart("push M4");
        M4_array.push_back(m4);
        //stats.timeend("push M4");
    }

    int globalStart = timeRange[0];
    int globalEnd = timeRange[1];

    // 计算 timeRangeLength
    int timeRangeLength = globalEnd - globalStart + 1;

    // 平均每个桶分配的点数
    double everyNum = static_cast<double>(timeRangeLength) / width;

    // 第一个 M4 对象，设置 start_time
    M4_array[0]->start_time = globalStart;

    // 循环设置每个 M4 的时间区间
    for (int i = 1; i < width; i++)
    {
        // 当前 M4 开始时间是 globalStart + 每个桶分配的点数，向上取整
        M4_array[i]->start_time = globalStart + static_cast<int>(ceil(i * everyNum));

        // 上一个 M4 结束时间是当前 M4 开始时间 - 1
        M4_array[i - 1]->end_time = M4_array[i]->start_time - 1;
    }

    // 最后一个 M4 以 globalEnd 结尾
    M4_array[width - 1]->end_time = globalEnd;

    //return res;
}

// bool isSingleLeaf(SegmentTreeNode *node)
// {
//     if (node->eTime - node->sTime < 1)
//     {
//         return true;
//     }

//     return false;
// }

bool isLeafNode(const SegmentTree *segmentTree, int index)
{
    return index >= segmentTree->flagssize - 1;
}

std::pair<int, int> getSETimeByIndex(SegmentTree *segmentTree, int index)
{
    int srange = segmentTree->sTime;
    int erange = segmentTree->eTime;

    int level = static_cast<int>(std::floor(std::log2(index + 1)));
    int i = index - static_cast<int>(std::pow(2, level)) + 1;

    int interval = (erange - srange + 1) / std::pow(2, level);
    int sTime = srange + i * interval;
    int eTime = sTime + interval - 1;

    return std::make_pair(sTime, eTime);
}

int isContain(SegmentTree *segmentTree, SegmentTreeNode *node, M4 *m4)
{

    auto [node_sTime, node_eTime] = getSETimeByIndex(segmentTree, node->index);
    //cout<<"getSETimeByIndex:"<<node->index<< "," <<node_sTime << ","<<node_eTime<<endl;

    // 是叶子节点
    if (isLeafNode(segmentTree, node->index))
    {
        if (node_eTime < m4->start_time)
        {
            return -1; // Node 在 M4 左边
        }
        else if (node_sTime == m4->start_time)
        {
            return -2; // Node 与 M4 左边界重合
        }
        else if (node_sTime > m4->start_time && node_sTime < m4->end_time)
        {
            return -3; // Node 在 M4 内部
        }
        else if (node_sTime == m4->end_time)
        {
            return -4; // Node 与 M4 右边界重合
        }
        else if (node_sTime > m4->end_time)
        {
            return -5; // Node 在 M4 右边
        }
        else
        {
            return 0;
        }
    }
    else
    { // 非叶子节点
        if (node_eTime < m4->start_time)
        {
            return 1; // Node 完全在 M4 的左边
        }
        else if (node_eTime == m4->start_time)
        {
            return 2; // Node 右边界与 M4 左边界重合
        }
        else if (node_sTime < m4->start_time && node_eTime > m4->start_time)
        {
            return 3; // Node 跨过 M4 左边界
        }
        else if (node_sTime == m4->start_time /* && node->eTime < m4->end_time */)
        {
            return 4; // Node 左边界与 M4 左边界重合
        }
        else if (node_sTime > m4->start_time && node_eTime < m4->end_time)
        {
            return 5; // Node 在 M4 内部
        }
        else if (/* node->sTime > m4->start_time && */ node_eTime == m4->end_time)
        {
            return 6; // Node 右边界与 M4 右边界重合
        }
        else if (/* node->sTime > m4->start_time && */ node_eTime > m4->end_time && node_sTime < m4->end_time)
        {
            return 7; // Node 跨过 M4 右边界
        }
        else if (node_sTime == m4->end_time)
        {
            return 8; // Node 左边界与 M4 右边界重合
        }
        else if (node_sTime > m4->end_time)
        {
            return 9; // Node 完全在 M4 的右边
        }
        else
        {
            return 0;
        }
    }
}

std::pair<int, int> getChildrenIndex(int index)
{
    int leftIndex = 2 * index + 1;
    int rightIndex = 2 * index + 2;
    return std::make_pair(leftIndex, rightIndex);
}



std::pair<SegmentTreeNode *, SegmentTreeNode *> getChildren(SegmentTree *segmentTree1, SegmentTreeNode *parent_node, string& column, int thread_id)
{
    // 获取左右孩子索引
    auto [leftIndex, rightIndex] = getChildrenIndex(parent_node->index);
    
    // SegmentTreeNode *leftChild = new (nodePool.allocate())SegmentTreeNode(leftIndex, 0, 0);
    // SegmentTreeNode *rightChild = new (nodePool.allocate())SegmentTreeNode(rightIndex, 0, 0);


    // SegmentTreeNode *leftChild = new SegmentTreeNode(leftIndex, 0, 0);
    // SegmentTreeNode *rightChild = new SegmentTreeNode(rightIndex, 0, 0);
    SegmentTreeNode *leftChild = segmentTree1->addNode(leftIndex, 0, 0) ;
    SegmentTreeNode *rightChild = segmentTree1->addNode(rightIndex, 0, 0) ;



    std::pair<std::pair<double, double>, std::pair<double, double>> a = mmdata->minmax_decode_index(parent_node->min, parent_node->max, parent_node->index+1, stoi(column), thread_id);
    leftChild->min = a.first.first;
    leftChild->max = a.first.second;
    rightChild->min=a.second.first;
    rightChild->max = a.second.second;



    // // 获取左孩子的时间范围
    // auto [sTime, eTime] = getSETimeByIndex(segmentTree1, leftIndex);
    // if(sTime == 6575828 && eTime==6575828){
    //     int debug = 0;

    

    // cout<<"column: "<<column 
    // <<", index: "<<leftIndex +1
    // <<",start: "<< sTime 
    // << ",end: " << eTime 
    // <<",min: "<<leftChild->min
    // <<",max:"<<leftChild->max
    // <<endl;
    // }

    // cout<<"column: "<<column 
    //     <<", index: "<<leftIndex +1
    //     <<",start: "<< sTime 
    //     << ",end: " << eTime 
    //     <<",min: "<<leftChild->min
    //     <<",max:"<<leftChild->max
    //     <<endl;

    // leftChild->sTime = sTime;
    // leftChild->eTime = eTime;
    // // 获取右孩子的时间范围
    // auto [sTime2, eTime2] = getSETimeByIndex(segmentTree1, rightIndex);
    // rightChild->sTime = sTime2;
    // rightChild->eTime = eTime2;


    // 返回 pair 结构
    return std::make_pair(leftChild, rightChild);
}

std::tuple<
    double, double, double,
    double, double, double>
genNodeInfo(double current_diff_min, double current_diff_max, SegmentTreeNode *parent_node, double current_diff_ave)
{
    double left_node_min, right_node_min;
    double left_node_max, right_node_max;
    double left_node_ave, right_node_ave;

    if (std::isnan(current_diff_min) && current_diff_max == 0)
    {
        // 左孩子空，右孩子不空
        left_node_min = std::numeric_limits<double>::quiet_NaN();
        left_node_max = std::numeric_limits<double>::quiet_NaN();
        left_node_ave = 0;
        right_node_min = parent_node->min;
        right_node_max = parent_node->max;
        //right_node_ave = parent_node->ave * 2.0;
    }
    else if (current_diff_min == 0 && std::isnan(current_diff_max))
    {
        // 左不空，右空
        left_node_min = parent_node->min;
        left_node_max = parent_node->max;
        //left_node_ave = parent_node->ave * 2.0;
        right_node_min = std::numeric_limits<double>::quiet_NaN();
        right_node_max = std::numeric_limits<double>::quiet_NaN();
        right_node_ave = 0;
    }
    else
    {
        // 左右diff都空的，表示左右孩子都是空，没有写进数据库；
        // 左右diff都是0的，相当于左右都有值，正常处理。
        if (current_diff_min <= 0)
        {
            left_node_min = parent_node->min;
            right_node_min = left_node_min - current_diff_min;
        }
        else
        {
            right_node_min = parent_node->min;
            left_node_min = right_node_min + current_diff_min;
        }

        if (current_diff_max <= 0)
        {
            right_node_max = parent_node->max;
            left_node_max = right_node_max + current_diff_max;
        }
        else
        {
            left_node_max = parent_node->max;
            right_node_max = left_node_max - current_diff_max;
        }

        if (!std::isnan(current_diff_ave))
        {
            //left_node_ave = parent_node->ave + current_diff_ave / 2.0;
            //right_node_ave = parent_node->ave - current_diff_ave / 2.0;
        }
        else
        {
            //left_node_ave = parent_node->ave * 2.0;
            //right_node_ave = parent_node->ave * 2.0;
        }
    }

    return std::make_tuple(
        left_node_min, left_node_max, left_node_ave,
        right_node_min, right_node_max, right_node_ave);
}

bool isLeftNode(int index)
{
    return index % 2 != 0;
}



int getPosition(const SegmentTree *segmentTree, int index)
{
    if (isLeafNode(segmentTree, index))
    {
        return index - segmentTree->flagssize + 1;
    }
    else
    {
        std::cerr << "This node whose index = " << index << " is not a leafnode." << std::endl;
        return -1;
    }
}

std::vector<int> readFlag(const SegmentTree *segmentTree, int *flag, int index)
{
    if (isLeafNode(segmentTree, index))
    {
        int position = getPosition(segmentTree, index);

        // flag[position] 为空的判断（我们假设 -1 代表空）
        if (flag[position] == -1)
        {
            return {}; // 返回空 vector 表示 null
        }

        if (isLeftNode(index))
        {
            return {flag[position], flag[position + 1]};
        }
        else
        {
            return {flag[position - 1], flag[position]};
        }
    }
    else
    {
        std::cerr << "This node whose index = " << index << " is not a leafnode." << std::endl;
        return {-1}; // 或者抛异常，根据你的容错逻辑来定
    }
}

void buildNode(SegmentTreeNode *parent_node, SegmentTreeNode *node, SegmentTree *segmentTree, string column)
{
    //stats.timestart("buildNode");

    int i = stoi(column);
    double *maxvd_array = segmentTree->maxvd_array[i];
    double *minvd_array = segmentTree->minvd_array[i];
    int *flags = segmentTree->flags[i];

    // SegmentTreeNode *parent_node = node->parent;

    int index = node->index;
    //auto [sTime, eTime] = getSETimeByIndex(segmentTree, index);
    //node->sTime = sTime;
    //node->eTime = eTime;

    if (!isLeafNode(segmentTree ,node->index))
    {
        double current_diff_min = minvd_array[parent_node->index + 1];
        double current_diff_max = maxvd_array[parent_node->index + 1];
        double current_diff_ave; //= tableb_map[parent_node->index + 1][3 + i * 2];

        auto [left_node_min, left_node_max, left_node_ave, right_node_min, right_node_max, right_node_ave] =
            genNodeInfo(current_diff_min, current_diff_max, parent_node, current_diff_ave);

        if (isLeftNode(index))
        {
            node->min = left_node_min;
            node->max = left_node_max;
            //node->ave = left_node_ave;
        }
        else
        {
            node->min = right_node_min;
            node->max = right_node_max;
            //node->ave = right_node_ave;
        }
    }
    else
    {
        auto flag = readFlag(segmentTree, flags, index);
        double left_node_min, left_node_max, right_node_min, right_node_max;

        if (flag[0] == 0 && flag[1] == 0)
        {
            left_node_min = parent_node->max;
            left_node_max = parent_node->max;
            //left_node_ave = parent_node->max;

            right_node_min = parent_node->min;
            right_node_max = parent_node->min;
            //right_node_ave = parent_node->min;
        }
        else if (flag[0] == 1 && flag[1] == 1)
        {
            left_node_min = parent_node->min;
            left_node_max = parent_node->min;
            //left_node_ave = parent_node->min;

            right_node_min = parent_node->max;
            right_node_max = parent_node->max;
            //right_node_ave = parent_node->max;
        }
        else if (flag[0] == 1 && flag[1] == 0)
        {
            left_node_min = parent_node->min;
            left_node_max = parent_node->max;
            //left_node_ave = parent_node->ave * 2.0;

            right_node_min = NULL;
            right_node_max = NULL;
            //right_node_ave = NULL;
        }
        else if (flag[0] == 0 && flag[1] == 1)
        {
            left_node_min = NULL;
            left_node_max = NULL;
            //left_node_ave = NULL;

            right_node_min = parent_node->min;
            right_node_max = parent_node->max;
            //right_node_ave = parent_node->ave * 2.0;
        }

        if (isLeftNode(index))
        {
            node->min = left_node_min;
            node->max = left_node_max;
            //node->ave = left_node_ave;
        }
        else
        {
            node->min = right_node_min;
            node->max = right_node_max;
            //node->ave = right_node_ave;
        }
    }

    // node->level = parent_node->level + 1;
    // node->sum = node->ave * (eTime - sTime + 1);
    //node->isBuild = true;


    //stats.timeend("buildNode");
}

void fenlie(std::vector<M4*> &M4_array, int screenStart, int screenEnd,
            vector<string> &columns, FunInfo func,
            SegmentTree *segmentTree)
{

    cout<<"fenlie "<<endl;

    std::vector<SegmentTreeNode *> stacks [MAXVS];
    std::vector<SegmentTreeNode *> currentNodes;
    //for (const auto &column : columns)
    for(int i=0;i<columns.size();i++)
    {
        stacks[i].reserve(32);
        SegmentTreeNode * root = segmentTree->addNode(0,0,0); //new SegmentTreeNode();

        // cout<<"root:"<<root->min<<root->max<<endl;
        // cout<<"mmdata:"<<mmdata<<endl;

        std::pair<std::pair<double, double>, std::pair<double, double>> a =  mmdata->minmax_decode_index(0,0,0,stoi(columns[i]), MAX_THREADS_MAIN-1);
        root->min=a.first.first;
        root->max=a.first.second;
        root->index = 0;

        currentNodes.push_back(root);
        //currentNodes.push_back(segmentTree->head[stoi(columns[i])]);
        //stacks[i].push_back(segmentTree->head[stoi(columns[i])]);


    }

    if (currentNodes.empty())
    {
        // error
        return;
    }

//cout<<"fenlie start "<<endl;

    size_t i = 0;
    while (i < M4_array.size())
    {
        M4 *m4 = (M4_array[i]);
        int type = isContain(segmentTree ,currentNodes[0], m4);

        //cout<<"type:"<<type<<endl;

        if(i==41){
            bool dd = true;
        }

        if (type == -1)
        {
            //currentNodes[j] = currentNodes[j]->nextNode;
            if(stacks[0].empty()){
                break;
            }else{
                for (size_t j = 0; j < columns.size(); ++j){

                    //第一个像素列左边的，要delete
                    if(i==0){
                        delete currentNodes[j];
                    }

                    currentNodes[j] = stacks[j].back();
                    stacks[j].pop_back();

                }
            }

            continue;
        }

        if (type == -2)
        {
            for (size_t j = 0; j < columns.size(); ++j)
            {
                m4->stNodes.push_back(currentNodes[j]);
            }

            //currentNodes[j] = currentNodes[j]->nextNode;
            if(stacks[0].empty()){
                break;
            }else{
                for (size_t j = 0; j < columns.size(); ++j){
                    currentNodes[j] = stacks[j].back();
                    stacks[j].pop_back();

                }
            }

            continue;
        }

        if (type == -3)
        {
            std::vector<SegmentTreeNode *> nodePairs;
            for (size_t j = 0; j < columns.size(); ++j)
            {
                nodePairs.push_back(currentNodes[j]);
            }
            m4->innerNodes.push_back(nodePairs);

            //currentNodes[j] = currentNodes[j]->nextNode;
            if(stacks[0].empty()){
                break;
            }else{
                for (size_t j = 0; j < columns.size(); ++j){
                    currentNodes[j] = stacks[j].back();
                    stacks[j].pop_back();
                }
            }

            continue;
        }

        if (type == -4)
        {
            for (size_t j = 0; j < columns.size(); ++j)
            {
                m4->etNodes.push_back(currentNodes[j]);
            }

            //currentNodes[j] = currentNodes[j]->nextNode;
            if(stacks[0].empty()){
                break;
            }else{
                for (size_t j = 0; j < columns.size(); ++j){
                    currentNodes[j] = stacks[j].back();
                    stacks[j].pop_back();
                }
            }
            continue;
        }

        if (type == -5)
        {
            ++i;            
            
            //最后一个像素列右边的，要delete
            if(i==M4_array.size()){
                for (size_t j = 0; j < columns.size(); ++j)
                {
                    delete currentNodes[j];
                }
            }

            continue;
        }

        if (type == 1)
        {
            //currentNodes[j] = currentNodes[j]->nextNode;
            if(stacks[0].empty()){
                break;
            }else{
                for (size_t j = 0; j < columns.size(); ++j){
                    //第一个像素列左边的，要delete
                    if(i==0){
                        delete currentNodes[j];
                    }

                    currentNodes[j] = stacks[j].back();
                    stacks[j].pop_back();
                }
            }
            continue;
        }

        if (type == 2 || type == 3 || type == 4 || type == 6 || type == 7 || type == 8)
        {
            for (size_t j = 0; j < columns.size(); ++j)
            {
                SegmentTreeNode *leftChild;
                SegmentTreeNode *rightChild;
                std::tie(leftChild, rightChild) = getChildren(segmentTree, currentNodes[j],columns[j], MAX_THREADS_MAIN-1);

                // buildNode(currentNodes[j],leftChild, segmentTree, columns[j]);
                // buildNode(currentNodes[j],rightChild, segmentTree, columns[j]);

                // currentNodes[j]->leftChild = leftChild;
                // currentNodes[j]->rightChild = rightChild;
                // leftChild->parent = currentNodes[j];
                // rightChild->parent = currentNodes[j];

                //leftChild->preNode = currentNodes[j]->preNode;
                // leftChild->nextNode = rightChild;
                // //rightChild->preNode = leftChild;
                // rightChild->nextNode = currentNodes[j]->nextNode;


                // cout<<leftChild->index<< ","<<leftChild->min<<","<<leftChild->max <<endl;
                // cout<<rightChild->index<< ","<<rightChild->min<<","<<rightChild->max <<endl;

                stacks[j].push_back(rightChild);

                // if (leftChild->preNode != nullptr)
                // {
                //     leftChild->preNode->nextNode = leftChild;
                // }
                // if (rightChild->nextNode != nullptr)
                // {
                //     rightChild->nextNode->preNode = rightChild;
                // }

                // if (segmentTree->head[stoi(columns[j])]->index == currentNodes[j]->index)
                // {
                //     segmentTree->head[stoi(columns[j])] = leftChild;
                // }

                delete currentNodes[j];

                currentNodes[j] = leftChild;

            }

            // TODO: avg_w 特殊处理略过
            continue;
        }

        if (type == 5)
        {
            std::vector<SegmentTreeNode *> nodePairs;
            for (size_t j = 0; j < columns.size(); ++j)
            {
                nodePairs.push_back(currentNodes[j]);
            }
            m4->innerNodes.push_back(nodePairs);

            //currentNodes[j] = currentNodes[j]->nextNode;
            if(stacks[0].empty()){
                break;
            }else{
                for (size_t j = 0; j < columns.size(); ++j){
                    currentNodes[j] = stacks[j].back();
                    stacks[j].pop_back();
                }
            }
            continue;
        }

        if (type == 9)
        {
            ++i;

            //最后一个像素列右边的，要delete
            if(i==M4_array.size()){
                for (size_t j = 0; j < columns.size(); ++j)
                {
                    delete currentNodes[j];
                }
            }
            continue;
        }
    }
}

double sympleCalculate(
    double min1,
    double max1,
    double min2,
    double max2,
    const std::string &op,
    const std::string &destination,
    bool isLeaf)
{
    if (destination == "min")
    {
        if (op == "+")
        {
            return std::min({min1 + min2, min1 + max2, max1 + min2, max1 + max2});
        }
        else if (op == "-")
        {
            return std::min({min1 - min2, min1 - max2, max1 - min2, max1 - max2});
        }
        else if (op == "*")
        {
            return std::min({min1 * min2, min1 * max2, max1 * min2, max1 * max2});
        }
        else if (op == "/")
        {
            if (min2 == 0.0)
            {
                if (isLeaf)
                    return 0.0;
                min2 = 1.0;
            }
            if (max2 == 0.0)
            {
                if (isLeaf)
                    return 0.0;
                max2 = 1.0;
            }
            return std::min({min1 / min2, min1 / max2, max1 / min2, max1 / max2});
        }
    }
    else if (destination == "max")
    {
        if (op == "+")
        {
            return std::max({min1 + min2, min1 + max2, max1 + min2, max1 + max2});
        }
        else if (op == "-")
        {
            return std::max({min1 - min2, min1 - max2, max1 - min2, max1 - max2});
        }
        else if (op == "*")
        {
            return std::max({min1 * min2, min1 * max2, max1 * min2, max1 * max2});
        }
        else if (op == "/")
        {
            if (min2 == 0.0)
            {
                if (isLeaf)
                    return 0.0;
                min2 = 1.0;
            }
            if (max2 == 0.0)
            {
                if (isLeaf)
                    return 0.0;
                max2 = 1.0;
            }
            return std::max({min1 / min2, min1 / max2, max1 / min2, max1 / max2});
        }
    }

    return 0.0; // fallback
}

double calMean(const std::vector<SegmentTreeNode *> &computingNodes, const std::string &destination)
{
    double sum = 0.0;

    if (destination == "min")
    {
        for (const auto &node : computingNodes)
        {
            sum += node->min;
        }
    }
    else
    {
        for (const auto &node : computingNodes)
        {
            sum += node->max;
        }
    }

    return sum / computingNodes.size();
}

double sympleMean(const std::vector<double> &computeData)
{
    double sum = 0.0;

    // 计算所有数据的总和
    for (double val : computeData)
    {
        sum += val;
    }

    // 返回均值
    return sum / computeData.size();
}

double sympleVariance(const std::vector<double> &computeData)
{
    double mean = sympleMean(computeData); // 调用 sympleMean 函数计算均值
    double sum = 0.0;

    // 计算方差
    for (double val : computeData)
    {
        sum += (val - mean) * (val - mean); // 对每个元素的偏差平方求和
    }

    return sum / computeData.size(); // 返回均方差（方差）
}

double calVarianceExact(const std::vector<SegmentTreeNode *> &computingNodes, const std::string &destination)
{
    double mean = calMean(computingNodes, destination); // 计算均值
    double sum = 0.0;                                   // 初始化总和

    // 遍历 computingNodes 计算方差
    for (size_t i = 0; i < computingNodes.size(); i++)
    {
        sum += (computingNodes[i]->min - mean) * (computingNodes[i]->min - mean);
    }

    // 返回方差
    return sum / computingNodes.size();
}

// 计算最大方差的函数
std::vector<std::vector<double>> computeMaxVariance(const std::vector<SegmentTreeNode *> &computingNodes)
{
    // 初始化 count 和 maxVarSums
    int count = 1;
    std::vector<double> maxVarSums = {computingNodes[0]->min, computingNodes[0]->max}; // 存储最初的 min 和 max 值

    // 创建 maxVarList，每个元素是一个包含单个值的 vector
    std::vector<std::vector<double>> maxVarList = {{computingNodes[0]->min}, {computingNodes[0]->max}};

    // 遍历 computingNodes，从第二个元素开始处理
    for (size_t i = 1; i < computingNodes.size(); ++i)
    {
        count++; // 增加 count

        // 遍历 maxVarSums 和 maxVarList，进行计算
        for (size_t j = 0; j < maxVarSums.size(); ++j)
        {
            const double addMin = maxVarSums[j] + computingNodes[i]->min; // 累加最小值
            const double addMax = maxVarSums[j] + computingNodes[i]->max; // 累加最大值

            // 选择更合适的值（最小值或最大值）来更新 maxVarSums 和 maxVarList
            if (std::abs(computingNodes[i]->min - addMin / count) > std::abs(computingNodes[i]->max - addMax / count))
            {
                maxVarSums[j] = addMin;                          // 更新 maxVarSums
                maxVarList[j].push_back(computingNodes[i]->min); // 更新 maxVarList，推入最小值
            }
            else
            {
                maxVarSums[j] = addMax;                          // 更新 maxVarSums
                maxVarList[j].push_back(computingNodes[i]->max); // 更新 maxVarList，推入最大值
            }
        }
    }

    // 返回结果
    return maxVarList;
}

double getClosestVal(const SegmentTreeNode *node, double val)
{
    if (val < node->min)
    {
        return node->min;
    }
    else if (val > node->max)
    {
        return node->max;
    }
    return val;
}

std::vector<double> mergeCompute(const std::vector<SegmentTreeNode *> &minmaxVals, int start, int end)
{
    if (end - start == 1)
    {
        return {minmaxVals[start]->min, minmaxVals[start]->max};
    }

    int mid = (start + end + 1) / 2;
    std::vector<double> resultL = mergeCompute(minmaxVals, start, mid);
    std::vector<double> resultR = mergeCompute(minmaxVals, mid, end);

    if (resultL.size() == 2 && resultR.size() == 2)
    {
        double left = std::max(resultL[0], resultR[0]);
        double right = std::min(resultL[1], resultR[1]);

        if (left <= right)
        {
            return {left, right};
        }
        else
        {
            if (resultL[0] == left)
            {
                return {left * (mid - start) + right * (end - mid)};
            }
            else
            {
                return {right * (mid - start) + left * (end - mid)};
            }
        }
    }
    else if (resultL.size() == 1 && resultR.size() == 1)
    {
        return {resultL[0] + resultR[0]};
    }
    else
    {
        if (resultL.size() == 2)
        {
            double avgR = resultR[0] / (end - mid);
            double closest = 0; // getClosestVal(SegmentTreeNode{ resultL[0], resultL[1] }, avgR);
            return {closest * (mid - start) + resultR[0]};
        }
        else
        {
            double avgL = resultL[0] / (mid - start);
            double closest = 0; // getClosestVal(SegmentTreeNode{ resultR[0], resultR[1] }, avgL);
            return {resultL[0] + closest * (end - mid)};
        }
    }
}

// computeMinVariance 主函数
std::vector<double> computeMinVariance(const std::vector<SegmentTreeNode *> &minmaxVals)
{
    std::vector<double> result = mergeCompute(minmaxVals, 0, minmaxVals.size());

    if (result.size() == 2)
    {
        // 存在交集，返回空向量代表方差为0
        return {};
    }
    else
    {
        double estimatedAvg = result[0] / minmaxVals.size();
        std::vector<double> minVarList;

        for (const auto &vals : minmaxVals)
        {
            minVarList.push_back(getClosestVal(vals, estimatedAvg));
        }

        return minVarList;
    }
}

// 计算方差估算值
double calVarianceEstimate(const std::vector<SegmentTreeNode *> &computingNodes, const std::string &destination)
{
    if (destination == "min")
    {
        std::vector<double> minArray = computeMinVariance(computingNodes);
        if (minArray.empty())
        {
            return 0; // 如果 minArray 为空，则返回 0
        }
        return sympleVariance(minArray); // 计算并返回最小方差
    }
    else
    {
        std::vector<std::vector<double>> maxArray = computeMaxVariance(computingNodes);
        return std::max(sympleVariance(maxArray[0]), sympleVariance(maxArray[1])); // 计算并返回最大方差
    }
}

double calVariance(const std::vector<SegmentTreeNode *> &computingNodes, const std::string &destination, bool isLeaf)
{
    if (isLeaf)
    {
        return calVarianceExact(computingNodes, destination);
    }
    else
    {
        return calVarianceEstimate(computingNodes, destination);
    }
}

std::pair<double, double> unifiedCalculate(
    const SegmentTree *tree,
    const std::vector<SegmentTreeNode *> &computingNodes,
    FunInfo &func,
    const std::string &mode,
    bool isLeaf)
{
    if (computingNodes.empty())
    {
        return {NAN, NAN};
    }

    double tmpmin = 0.0;
    double tmpmax = 0.0;

    const std::string &funName = func.funName;

    if (funName == "+" || funName == "-" || funName == "*" || funName == "/")
    {
        tmpmin = sympleCalculate(
            computingNodes[0]->min,
            computingNodes[0]->max,
            computingNodes[1]->min,
            computingNodes[1]->max,
            funName,
            "min", isLeaf);

        tmpmax = sympleCalculate(
            computingNodes[0]->min,
            computingNodes[0]->max,
            computingNodes[1]->min,
            computingNodes[1]->max,
            funName,
            "max", isLeaf);
    }
    else if (funName == "mean")
    {
        tmpmin = calMean(computingNodes, "min");
        tmpmax = isLeaf ? tmpmin : calMean(computingNodes, "max");
    }
    else if (funName == "variance")
    {
        tmpmin = calVariance(computingNodes, "min", isLeaf);
        tmpmax = isLeaf ? tmpmin : calVariance(computingNodes, "max", isLeaf);
    }
    else if (funName == "func4")
    {
        tmpmin = func.func4(computingNodes, "min", isLeaf);
        tmpmax = isLeaf ? tmpmin : func.func4(computingNodes, "max", isLeaf);
    }
    else if (
        funName == "func1" || funName == "boxcox_0" ||
        funName == "boxcox_1_2" || funName == "boxcox_1" ||
        funName == "boxcox_2")
    {
        tmpmin = func.compute(funName, computingNodes, "min");
        tmpmax = isLeaf ? tmpmin : func.compute(funName, computingNodes, "max");
    }
    else if (funName == "ave" || funName == "sum")
    {
        // auto [tmpmin1, tmpmax1] = intervalEstimate(trees, computingNodes, func, mode, isLeaf);
        // tmpmin = tmpmin1;
        // tmpmax = tmpmax1;
    }
    else if (funName == "weightsum")
    {
        tmpmin = func.weightsum(computingNodes, "min", isLeaf);
        tmpmax = isLeaf ? tmpmin : func.weightsum(computingNodes, "max", isLeaf);
    }
    else if (funName == "L2" || funName == "L22")
    {
        tmpmin = func.L2(computingNodes, "min", isLeaf);
        tmpmax = isLeaf ? tmpmin : func.L2(computingNodes, "max", isLeaf);
    }
    else if (funName == "max")
    {
        tmpmin = func.getmax(computingNodes, "min", isLeaf);
        tmpmax = isLeaf ? tmpmin : func.getmax(computingNodes, "max", isLeaf);
    }
    else if(funName == "L2LN"){
        tmpmin = func.L2LN(computingNodes, "min", isLeaf);
        tmpmax = isLeaf ? tmpmin : func.L2LN(computingNodes, "max", isLeaf);
    }
    else
    {
        // default fallback for custom function
        // std::vector<double> Xs = generateXs(computingNodes[0]->min, func.extremes, computingNodes[0]->max);
        // std::vector<double> Ys = func.computes(Xs);
        // tmpmin = *std::min_element(Ys.begin(), Ys.end());
        // tmpmax = *std::max_element(Ys.begin(), Ys.end());
    }

    return {tmpmin, tmpmax};
}

void computeM4ValueSE(M4 *m4,
                      const SegmentTree *segmentTree,
                      FunInfo &func,
                      const std::string &mode)
{

    // 计算起始节点值
    double t1 = 0.0, t2 = 0.0;
    std::tie(t1, t2) = unifiedCalculate(segmentTree, m4->stNodes, func, mode, true);
    m4->st_v = t1;

    // 计算结束节点值
    double t3 = 0.0, t4 = 0.0;
    std::tie(t3, t4) = unifiedCalculate(segmentTree, m4->etNodes, func, mode, true);
    m4->et_v = t3;
}

void initForUnary(std::vector<M4*> &M4_array, int i,
                  SegmentTree *segmentTree,
                  FunInfo &func, const std::string &mode)
{

    double MAX = -std::numeric_limits<double>::infinity();
    double MIN = std::numeric_limits<double>::infinity();

    for (size_t j = 0; j < M4_array[i]->innerNodes.size(); j++)
    {
        std::vector<SegmentTreeNode *> &nodePairs = M4_array[i]->innerNodes[j];
        double tmpmin = nodePairs[0]->min;
        double tmpmax = nodePairs[0]->max;

        MAX = std::max(MAX, tmpmax);
        MIN = std::min(MIN, tmpmin);
    }

    std::vector<SegmentTreeNode *> currentNodes;
    SegmentTreeNode *node = new SegmentTreeNode();
    node->min = MIN;
    node->max = MIN;
    currentNodes.push_back(node);

    auto [tmpmin1, tmpmax1] = unifiedCalculate(segmentTree, currentNodes, func, mode, true);

    if (tmpmin1 < M4_array[i]->min)
    {
        M4_array[i]->min = tmpmin1;
    }

    currentNodes.clear();
    SegmentTreeNode *node2 = new SegmentTreeNode();
    node2->min = MAX;
    node2->max = MAX;
    currentNodes.push_back(node2);

    auto [tmpmin2, tmpmax2] = unifiedCalculate(segmentTree, currentNodes, func, mode, true);

    if (tmpmax2 > M4_array[i]->max)
    {
        M4_array[i]->max = tmpmax2;
    }

     delete node2;
     delete node;

    // 注意：上面用 new 分配的内存没有 delete，如果你希望管理内存，可以用智能指针或记得释放
}

void initForMaxOrMin(std::vector<M4*> &M4_array, int i,
                     SegmentTree *segmentTree,
                     FunInfo &func, std::string mode)
{

    double MAX = -std::numeric_limits<double>::infinity();
    double MIN = std::numeric_limits<double>::infinity();

    for (size_t j = 0; j < M4_array[i]->innerNodes.size(); ++j)
    {
        std::vector<SegmentTreeNode *> &nodePairs = M4_array[i]->innerNodes[j];
        for (size_t k = 0; k < nodePairs.size(); ++k)
        {
            SegmentTreeNode *node = nodePairs[k];
            double tmpmin = node->min;
            double tmpmax = node->max;

            if (tmpmax > MAX)
            {
                MAX = tmpmax;
            }
            if (tmpmin < MIN)
            {
                MIN = tmpmin;
            }
        }
    }

    if (func.funName == "max")
    {
        if (MAX > M4_array[i]->max)
        {
            M4_array[i]->max = MAX;
        }
    }
    else
    {
        if (MIN < M4_array[i]->min)
        {
            M4_array[i]->min = MIN;
        }
    }
}

void initM4(SegmentTree *segmentTree, vector<string> &columns, std::vector<M4*> &M4_array,
            FunInfo &func, const std::string &mode, bool parallel, SCREEN_M4 &screen_m4)
{

    // std::vector<std::vector<int>> needQueryNodesTrees(segmentTrees.size());

    for (size_t i = 0; i < M4_array.size(); ++i)
    {
        // 初始化 M4
        // M4_array[i].alternativeNodesMax = MaxHeap();
        // M4_array[i].alternativeNodesMin = MinHeap();
        M4_array[i]->isCompletedMax = false;
        M4_array[i]->isCompletedMin = false;
        // M4_array[i].currentComputingNodeMax.clear();
        // M4_array[i].currentComputingNodeMin.clear();

        if (i == 339)
        {
            bool debug = true;
        }

        // 计算边界 node
        computeM4ValueSE(M4_array[i], segmentTree, func, mode);
        for(int j=0;j<M4_array[i]->stNodes.size();j++){
            delete M4_array[i]->stNodes[j];
            delete M4_array[i]->etNodes[j];
        }

        if (M4_array[i]->st_v < M4_array[i]->et_v)
        {
            M4_array[i]->min = M4_array[i]->st_v;
            M4_array[i]->max = M4_array[i]->et_v;
        }
        else
        {
            M4_array[i]->min = M4_array[i]->et_v;
            M4_array[i]->max = M4_array[i]->st_v;
        }

        if (M4_array[i]->min < screen_m4.exactMin)
        {
            screen_m4.exactMin = M4_array[i]->min;
        }
        if (M4_array[i]->max > screen_m4.exactMax)
        {
            screen_m4.exactMax = M4_array[i]->max;
        }

        if (M4_array[i]->innerNodes.empty())
        {
            M4_array[i]->isCompletedMax = true;
            M4_array[i]->isCompletedMin = true;
            continue;
        }

        // 对一元函数，极值大概率出现在边界
        if (columns.size() == 1)
        {
            initForUnary(M4_array, i, segmentTree, func, mode);
        }

        // 对 max 或 min 函数处理
        if (func.funName == "max" || func.funName == "min")
        {
            initForMaxOrMin(M4_array, i, segmentTree, func, mode);
        }

        // innerNodes 候选处理
        for (auto &nodePairs : M4_array[i]->innerNodes)
        {
            auto [tmpmin, tmpmax] = unifiedCalculate(segmentTree, nodePairs, func, mode, false);

            if (tmpmax > M4_array[i]->max)
            {
                
                //Element max_e(tmpmax, nodePairs);
                Element max_e;
                max_e.value=tmpmax;
                for(int i=0;i<nodePairs.size();i++){
                    SegmentTreeNode * node = new SegmentTreeNode();
                    node->index = nodePairs[i]->index;
                    node->min = nodePairs[i]->min;
                    node->max = nodePairs[i]->max;
                    max_e.nodePairs.push_back(node);
                }
                //Element max_e(111.1, nodePairs);
                // max_e.value = tmpmax;
                // max_e.nodePairs = nodePairs;
                M4_array[i]->alternativeNodesMax.add(max_e);
            }

            if (tmpmin < M4_array[i]->min)
            {
                //Element min_e(tmpmin, nodePairs);
                Element min_e;
                min_e.value=tmpmin;                
                for(int i=0;i<nodePairs.size();i++){
                    SegmentTreeNode * node = new SegmentTreeNode();
                    node->index = nodePairs[i]->index;
                    node->min = nodePairs[i]->min;
                    node->max = nodePairs[i]->max;
                    min_e.nodePairs.push_back(node);
                }
                // min_e.value = tmpmin;
                // min_e.nodePairs = nodePairs;
                M4_array[i]->alternativeNodesMin.add(min_e);
            }
        }

        // auto tt = huisuCompute(M4_array[i], segmentTree, parallel);
    }
}

void getCandidateMinMax(int i, SCREEN_M4& screen_m4) {
    // timestart("getCandidateMinMax");

    M4* m4 = screen_m4.M4_array[i];

    if (m4->alternativeNodesMin.empty()) {
        if (m4->min < screen_m4.candidateMin) {
            screen_m4.candidateMin = m4->min;
        }
    } else {
        auto Ele = m4->alternativeNodesMin.top();
        screen_m4.candidateMin = std::min({screen_m4.candidateMin, Ele.value, m4->min});
    }

    if (m4->alternativeNodesMax.empty()) {
        if (m4->max > screen_m4.candidateMax) {
            screen_m4.candidateMax = m4->max;
        }
    } else {
        auto Ele = m4->alternativeNodesMax.top();
        screen_m4.candidateMax = std::max({screen_m4.candidateMax, Ele.value, m4->max});
    }

    // timeend("getCandidateMinMax");
}

double getBoundary(int start_time, int end_time, int width, int i) {
    return (end_time - start_time + 1.0) / width * i;
}

class YRange {
    public:
        int Ymin;
        int Ymax;
    
        YRange() {
            Ymin = std::numeric_limits<int>::max();
            Ymax = std::numeric_limits<int>::min();
        }
    };

// YRange getYRangeInner(double valuemin, double valuemax, double ymin, double ymax, int height) {
//     YRange yRange;
//     yRange.Ymin = static_cast<int>(std::floor(((valuemin - ymin) / (ymax - ymin)) * height));
//     yRange.Ymax = static_cast<int>(std::floor(((valuemax - ymin) / (ymax - ymin)) * height));
//     return yRange;
// }

YRange* getYRangeInner(double valuemin, double valuemax, double ymin, double ymax, int height) {
    YRange* yRange = new YRange();
    yRange->Ymin = static_cast<int>(std::floor(((valuemin - ymin) / (ymax - ymin)) * height));
    yRange->Ymax = static_cast<int>(std::floor(((valuemax - ymin) / (ymax - ymin)) * height));
    return yRange;
}


YRange* getYRangePre(M4* m4_pre, double boundaryPre, M4* m4, double ymin, double ymax, int height) {
    if (m4_pre == nullptr) {
        return nullptr;
    }

    if (m4_pre->et_v >= m4->min && m4_pre->et_v <= m4->max) {
        return nullptr;
    }

    YRange* yRange = new YRange();
    double intersectionY = 0;

    if (m4_pre->et_v < m4->st_v) {
        intersectionY = m4_pre->et_v + (m4->st_v - m4_pre->et_v) * (boundaryPre - m4_pre->end_time) / (m4->start_time - m4_pre->end_time);

        yRange->Ymin = static_cast<int>(std::floor((intersectionY - ymin) / (ymax - ymin) * height));
        yRange->Ymax = static_cast<int>(std::floor((m4->st_v - ymin) / (ymax - ymin) * height));

        return yRange;

    } else if (m4_pre->et_v > m4->st_v) {
        intersectionY = m4->st_v + (m4_pre->et_v - m4->st_v) * (m4->start_time - boundaryPre) / (m4->start_time - m4_pre->end_time);

        yRange->Ymin = static_cast<int>(std::floor((m4->st_v - ymin) / (ymax - ymin) * height));
        yRange->Ymax = static_cast<int>(std::floor((intersectionY - ymin) / (ymax - ymin) * height));

        return yRange;
    }

    delete yRange;
    return nullptr;
}

YRange* getYRangeNext(M4* m4_next, double boundaryNext, M4* m4, double ymin, double ymax, int height) {
    if (m4_next == nullptr) {
        return nullptr;
    }

    if (m4_next->st_v >= m4->min && m4_next->st_v <= m4->max) {
        return nullptr;
    }

    YRange* yRange = new YRange();
    double intersectionY = 0;

    if (m4_next->st_v < m4->et_v) {
        intersectionY = m4_next->st_v + (m4->et_v - m4_next->st_v) * (m4_next->start_time - boundaryNext) / (m4_next->start_time - m4->end_time);
        yRange->Ymin = static_cast<int>(std::floor(((intersectionY - ymin) / (ymax - ymin)) * height));
        yRange->Ymax = static_cast<int>(std::floor(((m4->et_v - ymin) / (ymax - ymin)) * height));
        return yRange;
    } else if (m4_next->st_v > m4->et_v) {
        intersectionY = m4->et_v + (m4_next->st_v - m4->et_v) * (boundaryNext - m4->end_time) / (m4_next->start_time - m4->end_time);
        yRange->Ymin = static_cast<int>(std::floor(((m4->et_v - ymin) / (ymax - ymin)) * height));
        yRange->Ymax = static_cast<int>(std::floor(((intersectionY - ymin) / (ymax - ymin)) * height));
        return yRange;
    }

    delete yRange;
    return nullptr;  // In case the conditions do not meet
}

YRange* getUnion(std::vector<YRange*> yRanges, int height) {
    int max = std::numeric_limits<int>::min();
    int min = std::numeric_limits<int>::max();

    double testmax = -std::numeric_limits<double>::infinity();
    double testmin = INFINITY;

    for (auto& range : yRanges) {
        if (range == nullptr) {
            continue;
        }

        if (max < range->Ymax) {
            max = range->Ymax;
        }

        if (min > range->Ymin) {
            min = range->Ymin;
        }
    }

    YRange* yRange = new YRange();
    yRange->Ymin = std::max(min, 0);
    yRange->Ymax = std::min(max, height);

    return yRange;
}

void outputpix(const std::string& type, double min, double max, double exactMin, double exactMax, YRange* yRange) {
    std::cout << type << " min: " << min << " max: " << max
              << " ymin: " << exactMin << " ymax: " << exactMax
              << " range: (" << yRange->Ymin << ", " << yRange->Ymax << ")\n";
}

YRange* computeErrorPixelsExact2Exact(
    M4* m4_pre, double boundaryPre,
    M4* m4, M4* m4_next, double boundaryNext,
    double exactMax, double exactMin,
    double candidateMax, double candidateMin,
    int height, bool debug
) {
    auto yRangeInner = getYRangeInner(m4->min, m4->max, exactMin, exactMax, height);

    auto yRangePre = getYRangePre(m4_pre, boundaryPre, m4, exactMin, exactMax, height);
    auto yRangeNext = getYRangeNext(m4_next, boundaryNext, m4, exactMin, exactMax, height);

    // if (errorBoundSatisfyCount == 13 || errorBoundSatisfyCount == 14) {
    //     std::cout << "yRangeInner/yRangePre/yRangeNext debug output" << std::endl;
    // }

    auto yRange = getUnion({ yRangeInner, yRangePre, yRangeNext }, height);

    if (debug) {
        outputpix("e2e", m4->min, m4->max, exactMin, exactMax, yRange);
    }

    delete yRangeInner;
    delete yRangePre;
    delete yRangeNext;

    return yRange;
}

YRange* computeErrorPixelsExact2Candidate(M4* m4_pre, double boundaryPre, M4* m4, M4* m4_next, double boundaryNext,
    double exactMax, double exactMin, double candidateMax, double candidateMin, int height, bool debug) {

    // 获取 yRangeInner, yRangePre, yRangeNext
    YRange* yRangeInner = getYRangeInner(m4->min, m4->max, candidateMin, candidateMax, height);
    YRange* yRangePre = getYRangePre(m4_pre, boundaryPre, m4, candidateMin, candidateMax, height);
    YRange* yRangeNext = getYRangeNext(m4_next, boundaryNext, m4, candidateMin, candidateMax, height);

    // 获取 yRange 合并结果
    YRange* yRange = getUnion({yRangeInner, yRangePre, yRangeNext}, height);

    // 如果调试标志为 true，输出相关信息
    if (debug) {
        outputpix("e2c", m4->min, m4->max, candidateMin, candidateMax, yRange);
    }

    delete yRangeInner;
    delete yRangePre;
    delete yRangeNext;

    return yRange;
}

YRange* computeErrorPixelsCandidate2Exact(M4* m4_pre, double boundaryPre, M4* m4, M4* m4_next, double boundaryNext,
    double exactMax, double exactMin, double candidateMax, double candidateMin, int height, bool debug) {

    double min, max;

    // 判断 alternativeNodesMin 是否为空，计算 min 值
    if (m4->alternativeNodesMin.empty()) {
        min = m4->min;
    } else {
        auto Ele = m4->alternativeNodesMin.top();
        min = std::min(Ele.value, m4->min);
    }

    // 判断 alternativeNodesMax 是否为空，计算 max 值
    if (m4->alternativeNodesMax.empty()) {
        max = m4->max;
    } else {
        auto Ele = m4->alternativeNodesMax.top();
        max = std::max(Ele.value, m4->max);
    }

    // 获取 yRangeInner, yRangePre, yRangeNext
    YRange* yRangeInner = getYRangeInner(min, max, exactMin, exactMax, height);
    YRange* yRangePre = getYRangePre(m4_pre, boundaryPre, m4, exactMin, exactMax, height);
    YRange* yRangeNext = getYRangeNext(m4_next, boundaryNext, m4, exactMin, exactMax, height);

    // 获取 yRange 合并结果
    YRange* yRange = getUnion({yRangeInner, yRangePre, yRangeNext}, height);

    // 如果调试标志为 true，输出相关信息
    if (debug) {
        outputpix("c2e", min, max, exactMin, exactMax, yRange);
    }

    delete yRangeInner;
    delete yRangePre;
    delete yRangeNext;

    return yRange;
}

YRange* computeErrorPixelsCandidate2Candidate(M4* m4_pre, double boundaryPre, M4* m4, M4* m4_next, double boundaryNext,
    double exactMax, double exactMin, double candidateMax, double candidateMin, int height, bool debug) {

    double min, max;

    // 判断 alternativeNodesMin 是否为空，计算 min 值
    if (m4->alternativeNodesMin.empty()) {
        min = m4->min;
    } else {
        auto Ele = m4->alternativeNodesMin.top();
        min = std::min(Ele.value, m4->min);
    }

    // 判断 alternativeNodesMax 是否为空，计算 max 值
    if (m4->alternativeNodesMax.empty()) {
        max = m4->max;
    } else {
        auto Ele = m4->alternativeNodesMax.top();
        max = std::max(Ele.value, m4->max);
    }

    // 获取 yRangeInner, yRangePre, yRangeNext
    YRange* yRangeInner = getYRangeInner(min, max, candidateMin, candidateMax, height);
    YRange* yRangePre = getYRangePre(m4_pre, boundaryPre, m4, candidateMin, candidateMax, height);
    YRange* yRangeNext = getYRangeNext(m4_next, boundaryNext, m4, candidateMin, candidateMax, height);

    // 获取 yRange 合并结果
    YRange* yRange = getUnion({yRangeInner, yRangePre, yRangeNext}, height);

    // 如果调试标志为 true，输出相关信息
    if (debug) {
        outputpix("c2c", min, max, candidateMin, candidateMax, yRange);
    }

    delete yRangeInner;
    delete yRangePre;
    delete yRangeNext;

    return yRange;
}

std::vector<YRange*> unionRangesFn(const std::vector<YRange*>& ranges) {
    if (ranges.empty()) return {};

    // 克隆并排序 ranges 数组，按 Ymin 升序排列
    std::vector<YRange*> allRanges = ranges;
    std::sort(allRanges.begin(), allRanges.end(), [](YRange* a, YRange* b) {
        return a->Ymin < b->Ymin;
    });

    std::vector<YRange*> result;
    YRange* current = new YRange();
    current->Ymin = allRanges[0]->Ymin;
    current->Ymax = allRanges[0]->Ymax;

    // 遍历所有区间，进行合并
    for (size_t i = 1; i < allRanges.size(); i++) {
        YRange* r = allRanges[i];
        if (current->Ymax >= r->Ymin) {
            // 合并区间
            current->Ymax = std::max(current->Ymax, r->Ymax);
        } else {
            // 将当前区间加入结果，并开始新区间
            result.push_back(current);
            current = new YRange();
            current->Ymin = r->Ymin;
            current->Ymax = r->Ymax;
        }
    }

    // 添加最后一个区间
    result.push_back(current);
    return result;
}

std::vector<YRange*> differenceRangesFn(const std::vector<YRange*>& a, YRange* b, bool& is_a) {
    std::vector<YRange*> result;
    is_a = false;

    // 如果 a 为空，直接返回空
    if (a.empty()) {
        //cout<<"如果 a 为空，直接返回空" << endl;
        return {};
    }

    if (!b) {
        is_a = true;
        //cout<<"如果 b 为 nullptr，直接返回 a 的副本" << endl;
        return a;  // 如果 b 为 nullptr，直接返回 a 的副本
    }

    int bStart = b->Ymin;
    int bEnd = b->Ymax;

    for (auto ra : a) {
        // 如果 ra 在 b 的左侧或右侧，则完全保留
        if (ra->Ymax < bStart || ra->Ymin > bEnd) {
            YRange* newRange = new YRange();
            newRange->Ymax = ra->Ymax;
            newRange->Ymin = ra->Ymin;
            result.push_back(newRange);
        } else {
            // 有重叠部分，处理差集
            if (ra->Ymin < bStart) {
                YRange* newRange = new YRange();
                newRange->Ymax = bStart - 1;
                newRange->Ymin = ra->Ymin;
                result.push_back(newRange);
            }
            if (ra->Ymax > bEnd) {
                YRange* newRange = new YRange();
                newRange->Ymax = ra->Ymax;
                newRange->Ymin = bEnd + 1;
                result.push_back(newRange);
            }
        }
    }

    return result;
}

YRange* intersectRangesFn(const std::vector<YRange*>& ranges) {
    if (ranges.empty()) return nullptr;

    YRange* intersection = new YRange();
    intersection->Ymin = ranges[0]->Ymin;
    intersection->Ymax = ranges[0]->Ymax;

    for (size_t i = 1; i < ranges.size(); i++) {
        YRange* r = ranges[i];
        int newYmin = std::max(intersection->Ymin, r->Ymin);
        int newYmax = std::min(intersection->Ymax, r->Ymax);
        
        if (newYmin > newYmax) {
            return nullptr; // 没有公共交集，返回空指针
        }
        
        intersection->Ymin = newYmin;
        intersection->Ymax = newYmax;
    }

    return intersection; // 返回单一交集区间
}


int computeErrorPixels(
    M4* m4_pre, double boundaryPre,
    M4* m4, 
    M4* m4_next, double boundaryNext,
    double exactMax, double exactMin,
    double candidateMax, double candidateMin,
    int height, bool debug
) {
    auto e2ePixInterval = computeErrorPixelsExact2Exact(
        m4_pre, boundaryPre, m4, m4_next, boundaryNext,
        exactMax, exactMin, candidateMax, candidateMin, height, debug
    );

    auto e2cPixInterval = computeErrorPixelsExact2Candidate(
        m4_pre, boundaryPre, m4, m4_next, boundaryNext,
        exactMax, exactMin, candidateMax, candidateMin, height, debug
    );

    auto c2ePixInterval = computeErrorPixelsCandidate2Exact(
        m4_pre, boundaryPre, m4, m4_next, boundaryNext,
        exactMax, exactMin, candidateMax, candidateMin, height, debug
    );

    auto c2cPixInterval = computeErrorPixelsCandidate2Candidate(
        m4_pre, boundaryPre, m4, m4_next, boundaryNext,
        exactMax, exactMin, candidateMax, candidateMin, height, debug
    );

    // 合并所有区间
    auto unionRanges = unionRangesFn({
        e2ePixInterval, e2cPixInterval, c2ePixInterval, c2cPixInterval
    });

    // 计算交集区间
    auto intersectionRange = intersectRangesFn({
        e2ePixInterval, e2cPixInterval, c2ePixInterval, c2cPixInterval
    });

    // 差集 = 并集 - 交集
    bool is_a = false;
    auto diffRanges = differenceRangesFn(unionRanges, intersectionRange, is_a);

    int totalDiffNum = 0;
    for (const auto& diffRange : diffRanges) {
        totalDiffNum += diffRange->Ymax - diffRange->Ymin + 1;
    }

    if (debug) {
        std::cout << "union: ";
        for (const auto& r : unionRanges) {
            std::cout << "[" << r->Ymin << "," << r->Ymax << "] ";
        }
        std::cout << "\nintersection: ";
        //for (const auto& r : intersectionRange) {
            std::cout << "[" << intersectionRange->Ymin << "," << intersectionRange->Ymax << "] ";
        //}
        std::cout << std::endl;
    }

    delete e2ePixInterval;
    delete e2cPixInterval;
    delete c2ePixInterval;
    delete c2cPixInterval;
    for(size_t i = 0; i < unionRanges.size(); i++){
        YRange* r = unionRanges[i];
        delete r;
    }
    // cout<<unionRanges.size()  <<endl;
    // unionRanges.clear();
    // cout<<unionRanges.size()  <<endl;
    // delete intersectionRange;

    if(!is_a){
        for(size_t i = 0; i < diffRanges.size(); i++){
            //cout<<diffRanges.size() << ","<< i <<endl;
            YRange* r = diffRanges[i];
            delete r;
        }

    }


    return totalDiffNum;
}


void calErrorPixM4(int i, SCREEN_M4& screen_m4) {
    // timestart("calErrorPixM4");

    auto& M4_array = screen_m4.M4_array;
    M4* m4 = screen_m4.M4_array[i];
    M4* m4_pre = nullptr;
    M4* m4_next = nullptr;
    double boundaryPre = M4_array[0]->start_time;
    double boundaryNext = M4_array[M4_array.size() - 1]->end_time;

    if (i > 0) {
        m4_pre = M4_array[i - 1];
        boundaryPre = getBoundary(M4_array[0]->start_time, M4_array[M4_array.size() - 1]->end_time, screen_m4.width, i);
    }
    if (i < M4_array.size() - 1) {
        m4_next = M4_array[i + 1];
        boundaryNext = getBoundary(M4_array[0]->start_time, M4_array[M4_array.size() - 1]->end_time, screen_m4.width, i + 1);
    }

    // m4.errorPixels = ...
    m4->errorPixels = computeErrorPixels(
        m4_pre,
        boundaryPre,
        m4,
        m4_next,
        boundaryNext,
        screen_m4.exactMax,
        screen_m4.exactMin,
        screen_m4.candidateMax,
        screen_m4.candidateMin,
        screen_m4.height,
        false
    );

    // timeend("calErrorPixM4");
}


bool errorBoundSatisfy(SCREEN_M4& screen_m4, int width, int height, double errorBound, int &errorBoundSatisfyCount) {
    vector<M4*>& M4_array = screen_m4.M4_array;
    int totalPixels = width * height;
    int errorPixels = 0;

    for (size_t i = 0; i < M4_array.size(); i++) {
        if (M4_array[i]->min < screen_m4.exactMin) {
            screen_m4.exactMin = M4_array[i]->min;
        }
        if (M4_array[i]->max > screen_m4.exactMax) {
            screen_m4.exactMax = M4_array[i]->max;
        }
        getCandidateMinMax(i, screen_m4);
    }

    //cout<<screen_m4.exactMax << screen_m4.exactMin << screen_m4.candidateMax << screen_m4.candidateMin << endl;

    for (size_t i = 0; i < M4_array.size(); i++) {
        calErrorPixM4(i, screen_m4);
        errorPixels += screen_m4.M4_array[i]->errorPixels;


        if(i == 0){
            //cout << screen_m4.M4_array[i]->errorPixels << endl;
        }
    }

    errorBoundSatisfyCount++;
    cout << errorBoundSatisfyCount << " " << (double)errorPixels / totalPixels << endl;

    screen_m4.preError = (double)errorPixels / totalPixels;
    screen_m4.count = 1;

    if ((double)errorPixels / totalPixels <= errorBound) {
        return true;
    } else {
        return false;
    }
}

void fenlieChildrens(SegmentTree* segmentTree,
    vector<string> &columns
                     , std::vector<SegmentTreeNode *> &fenlieNodes
                     , std::vector<SegmentTreeNode *> &leftNodes
                     , std::vector<SegmentTreeNode *> &rightNodes
                     , M4 *m4, int thread_id)
{

    // if (fenlieNodes[0]->leftChild != nullptr && fenlieNodes[0]->rightChild != nullptr)
    // {
    //     return;
    // }

    for (int i = 0; i < fenlieNodes.size(); i++)
    {

        std::pair<SegmentTreeNode *, SegmentTreeNode *> children = getChildren(segmentTree, fenlieNodes[i], columns[i], thread_id);
        SegmentTreeNode *leftChild = children.first;
        SegmentTreeNode *rightChild = children.second;
        // buildNode(fenlieNodes[i], leftChild, segmentTree, columns[i]);
        // buildNode(fenlieNodes[i], rightChild, segmentTree, columns[i]);

        leftNodes.push_back(leftChild);
        rightNodes.push_back(rightChild);


        delete fenlieNodes[i];
        // cout<<"start:"<<i<<endl;
        // delete fenlieNodes[i];
        // cout<<"end:"<<i<<endl;

        // fenlieNodes[i]->leftChild = leftChild;
        // fenlieNodes[i]->rightChild = rightChild;

        // leftChild->parent = fenlieNodes[i];
        // rightChild->parent = fenlieNodes[i];

        // 更新双向链表
        //leftChild->preNode = fenlieNodes[i]->preNode;
        //leftChild->nextNode = rightChild;
        //rightChild->preNode = leftChild;
        //rightChild->nextNode = fenlieNodes[i]->nextNode;

        // if (leftChild->preNode != nullptr)
        // {
        //     leftChild->preNode->nextNode = leftChild;
        // }
        // if (rightChild->nextNode != nullptr)
        // {
        //     rightChild->nextNode->preNode = rightChild;
        // }

        // if (segmentTree->head[i]->index == fenlieNodes[i]->index)
        // {
        //     segmentTree->head[i] = leftChild;
        // }

        // 如果需要处理父节点删除的逻辑，可解开注释：
        // if (segmentTrees[i].patentDelete) {
        //     segmentTrees[i].deleteNode(fenlieNodes[i]);
        // }
    }

    // 如果需要控制最大节点数逻辑，可解开注释：
    // if (segmentTrees[0].nodeCount > segmentTrees[0].maxNodeNum) {
    //     reduceNodes(segmentTrees, m4);
    // }
}



void CurrentCompute(M4* m4,
                    SegmentTree* segmentTree,
                    vector<string> &columns,
                    FunInfo &func,
                    vector<SegmentTreeNode *> &nodePairs,
                    string destination,
                    string& mode,
                    SCREEN_M4& screen_m4, int thread_id) {


    // ====================for min======================
    if(destination == "min") {
        vector<SegmentTreeNode *> currentComputingNodePairs = nodePairs;

        // if (!currentComputingNodePairs[0]->isBuild) {
        //     cout<<"error "<<endl;
        //     return;
        // }


        while(!isLeafNode(segmentTree, currentComputingNodePairs[0]->index)){
            std::vector<SegmentTreeNode *> leftNodes, rightNodes;
            fenlieChildrens(segmentTree, columns, currentComputingNodePairs, leftNodes, rightNodes, m4, thread_id);
            // for(int i=0;i<currentComputingNodePairs.size();i++){
            //     delete currentComputingNodePairs[i];
            // }
            // for (size_t j = 0; j < currentComputingNodePairs.size(); ++j) {

            //     leftNodes.push_back(currentComputingNodePairs[j]->leftChild);
            //     rightNodes.push_back(currentComputingNodePairs[j]->rightChild);
            //     //segmentTrees[j].deleteNode(currentComputingNodePairs[j]);
            // }

            auto [minLeft, maxLeft] = unifiedCalculate(segmentTree, leftNodes, func, mode, false);
            auto [minRight, maxRight] = unifiedCalculate(segmentTree, rightNodes, func, mode, false);
            Element ele;

            if (minLeft < m4->min && minRight < m4->min) {
                if (minLeft < minRight) {
                    currentComputingNodePairs = leftNodes;
                    ele.nodePairs = rightNodes;
                    ele.value = minRight;
                } else {
                    currentComputingNodePairs = rightNodes;
                    ele.nodePairs = leftNodes;
                    ele.value = minLeft;
                }
                m4->alternativeNodesMin.add(ele);
            } else if (minLeft < m4->min || minRight < m4->min) {
                currentComputingNodePairs = (minLeft < minRight) ? leftNodes : rightNodes;
                for(int i=0;i<leftNodes.size();i++){
                    if(minLeft < minRight){
                        delete rightNodes[i];
                    }else{

                        delete leftNodes[i];
                    }
                }
                //todo !!!要delete 没进candidate 的node
            } else {
                //currentComputingNodePairs.clear();
                //两个节点都delete
                for(int i=0;i<leftNodes.size();i++){
                    delete leftNodes[i];
                    delete rightNodes[i];
                }
                
                return;
            }
            
            // if(!isLeafNode(segmentTree, currentComputingNodePairs[0]->index)){
            //     fenlieChildrens(segmentTree, columns,currentComputingNodePairs, m4);
            // }
        }


        if (isLeafNode(segmentTree, currentComputingNodePairs[0]->index)) {
            auto [tmpmin, tmpmax] = unifiedCalculate(segmentTree, currentComputingNodePairs, func, mode, true);
            if (tmpmin < m4->min) {
                m4->min = tmpmin;
            }

            if (tmpmax > m4->max) {
                m4->max = tmpmax;
            }
            //todo !!!已经计算完该节点，要delete 
            for(int i=0;i<currentComputingNodePairs.size();i++){
                delete currentComputingNodePairs[i];
            }
        }
    }

    // ====================for max======================
    if(destination == "max") {
        vector<SegmentTreeNode *> currentComputingNodePairs = nodePairs;

        // if (!currentComputingNodePairs[0]->isBuild) {
        //     cout<<"error "<<endl;
        //     return;
        // }


        while(!isLeafNode(segmentTree, currentComputingNodePairs[0]->index)){
            std::vector<SegmentTreeNode *> leftNodes, rightNodes;
            fenlieChildrens(segmentTree, columns, currentComputingNodePairs, leftNodes, rightNodes, m4, thread_id);
            // for (size_t j = 0; j < currentComputingNodePairs.size(); ++j) {
                
            //     leftNodes.push_back(currentComputingNodePairs[j]->leftChild);
            //     rightNodes.push_back(currentComputingNodePairs[j]->rightChild);
            //     //segmentTrees[j].deleteNode(currentComputingNodePairs[j]);
            // }

            auto [minLeft, maxLeft] = unifiedCalculate(segmentTree, leftNodes, func, mode, false);
            auto [minRight, maxRight] = unifiedCalculate(segmentTree, rightNodes, func, mode, false);
            Element ele;

            if (maxLeft > m4->max && maxRight > m4->max) {
                if (maxLeft > maxRight) {
                    currentComputingNodePairs = leftNodes;
                    ele.nodePairs = rightNodes;
                    ele.value = maxRight;
                } else {
                    currentComputingNodePairs = rightNodes;
                    ele.nodePairs = leftNodes;
                    ele.value = maxLeft;
                }
                m4->alternativeNodesMax.add(ele);
            } else if (maxLeft > m4->max || maxRight > m4->max) {
                currentComputingNodePairs = (maxLeft > maxRight) ? leftNodes : rightNodes;

                for(int i=0;i<leftNodes.size();i++){
                    if(maxLeft > maxRight){
                        delete rightNodes[i];
                    }else{
                        delete leftNodes[i];
                    }
                }
            } else {
                //currentComputingNodePairs.clear();
                //两个节点都delete
                for(int i=0;i<leftNodes.size();i++){
                    delete leftNodes[i];
                    delete rightNodes[i];
                }
                return;
            }

            // if(!isLeafNode(segmentTree, currentComputingNodePairs[0]->index)){
            //     fenlieChildrens(segmentTree, columns,currentComputingNodePairs, m4);
            // }
        }


        if (isLeafNode(segmentTree, currentComputingNodePairs[0]->index)) {
            auto [tmpmin, tmpmax] = unifiedCalculate(segmentTree, currentComputingNodePairs, func, mode, true);
            if (tmpmin < m4->min) {
                m4->min = tmpmin;
            }

            if (tmpmax > m4->max) {
                m4->max = tmpmax;
            }

            for(int i=0;i<currentComputingNodePairs.size();i++){
                delete currentComputingNodePairs[i];
            }
        }
    }

}



void columnComputeMax(M4 *m4,
                      SegmentTree *segmentTree,
                      vector<string> &columns,
                      FunInfo &func,
                      string mode,
                      SCREEN_M4 &screen_m4, int thread_id)
{

    if (m4->isCompletedMax == true || m4->alternativeNodesMax.empty())
    {
        return;
    }


    while (!m4->alternativeNodesMax.empty())
    {

        Element MaxEle = m4->alternativeNodesMax.top();
        m4->alternativeNodesMax.pop();
        if (MaxEle.value > m4->max)
        {
            // if (MaxEle.nodePairs[0]->isBuild == false)
            // {
            //     // 表示该节点已经被删除了
            //     m4->alternativeNodesMax.pop();
            //     cout<<"error!!!"<<endl;
            //     continue;
            // }

            vector<SegmentTreeNode *> &nodePairs = MaxEle.nodePairs;

            if (!isLeafNode(segmentTree, nodePairs[0]->index))
            {
                //fenlieChildrens(segmentTree, columns,nodePairs, m4);
                CurrentCompute(m4, segmentTree, columns, func, nodePairs, "max", mode, screen_m4, thread_id);
            }
            else
            {
                // step4
                CurrentCompute(m4, segmentTree, columns, func, nodePairs, "max", mode, screen_m4, thread_id);
            }
        }
        else
        {
            // 堆顶不如当前m4，那么alternativeNodesMax里其他的都fail了，把alternative 清空
            // 后续改为清空函数
            m4->isCompletedMax = true;
            break;
        }

        break;
    }

    
}

void columnComputeMin(M4 *m4,
                      SegmentTree *segmentTree,
                      vector<string> &columns,
                      FunInfo &func,
                      string mode,
                      SCREEN_M4 &screen_m4, int thread_id)
{

    if (m4->isCompletedMin == true || m4->alternativeNodesMin.empty())
    {
        return;
    }

    while (!m4->alternativeNodesMin.empty())
    {

        Element MinEle = m4->alternativeNodesMin.top();
        m4->alternativeNodesMin.pop();

        if (MinEle.value < m4->min)
        {
            vector<SegmentTreeNode *> &nodePairs = MinEle.nodePairs;

            if (!isLeafNode(segmentTree, nodePairs[0]->index))
            {
                //fenlieChildrens(segmentTree, columns, nodePairs, m4);
                CurrentCompute(m4, segmentTree, columns, func, nodePairs, "min", mode, screen_m4, thread_id);
            }
            else
            {
                // step4
                CurrentCompute(m4, segmentTree, columns, func, nodePairs, "min", mode, screen_m4, thread_id);
            }
        }
        else
        {
            // 堆顶不如当前m4，那么alternativeNodesMax里其他的都fail了，把alternative 清空
            // 后续改为清空函数
            m4->isCompletedMin = true;
            break;
        }
        break;
    }

}

void columnCompute(M4 *m4,
                   SegmentTree *segmentTree,
                   vector<string> &columns,
                   FunInfo &func,
                   string mode,
                   SCREEN_M4 &screen_m4, int thread_id)
{

    columnComputeMax(m4, segmentTree,columns, func, mode, screen_m4, thread_id);
    columnComputeMin(m4, segmentTree,columns, func, mode, screen_m4, thread_id);
}





void trunkColumnCompute(M4* M4_array[],
    int size,
    SegmentTree *segmentTree,
    vector<string> &columns,
    FunInfo &func,
    string mode,
    SCREEN_M4 &screen_m4, int thread_id){

    for(int i=0;i<size;i++){
        //stats.timestart("columnCompute");
        columnCompute(M4_array[i],segmentTree,columns, func, mode, screen_m4, thread_id);
        //stats.timeend("columnCompute");
    }


}


int runColumnComputeBatch(
    SegmentTree *segmentTree,
    vector<string> &columns,
    SCREEN_M4 &screen_m4,
    FunInfo &func,
    const std::string &mode,
    bool parallel,
    int width,
    int height,
    double errorBound
){

    // std::vector<std::future<void>> futures;
    // futures.reserve(100);

    int computedCount = 0;
    int M4_arrays_size[50];
    for(int i=0;i<MAX_THREADS_MAIN;i++){
        M4_arrays_size[i]=0;
    }

    for (int i = 0; i < screen_m4.M4_array.size(); i++)
    {
        if (screen_m4.M4_array[i]->isCompletedMin){
            computedCount++;
        }
        if (screen_m4.M4_array[i]->isCompletedMax){
            computedCount++;
        }
        if (screen_m4.M4_array[i]->isCompletedMin && screen_m4.M4_array[i]->isCompletedMax){
            screen_m4.M4_array[i]->errorPixels = 0;
        }
        else{

            M4_arrays[i%MAX_THREADS_MAIN][M4_arrays_size[i%MAX_THREADS_MAIN]] = screen_m4.M4_array[i];
            M4_arrays_size[i%MAX_THREADS_MAIN]++;

            // stats.timestart("columnCompute");
            // columnCompute(screen_m4.M4_array[i], segmentTree,columns, func, mode, screen_m4);
            // stats.timeend("columnCompute");
        }
    }
    
    omp_set_num_threads(MAX_THREADS_MAIN);
    #pragma omp parallel for
    for(int i=0;i<MAX_THREADS_MAIN;i++){
        trunkColumnCompute(M4_arrays[i], M4_arrays_size[i], segmentTree,columns, func, mode, screen_m4, i);
        //cout<<M4_arrays_size[i]<<endl;
        // stats.timestart("pool.enqueue");

        // auto fut = pool.enqueue(
        //     trunkColumnCompute,
        //     ref(M4_arrays[i]),
        //     M4_arrays_size[i],
        //     ref(segmentTree),
        //     std::ref(columns),   // 引用传递
        //     std::ref(func),      // 引用传递
        //     ref(mode),
        //     std::ref(screen_m4) // 引用传递
        // );

        // stats.timeend("pool.enqueue");
    
        // stats.timestart("emplace_back");
        // futures.emplace_back(std::move(fut));
        // stats.timeend("emplace_back");
    
    }

    // stats.timestart("等待所有线程完成");
    // // 等待所有线程完成
    // for (auto& fut : futures)
    // {
    //     fut.get();
    // }
    // stats.timeend("等待所有线程完成");



    //cout<<endl;
    return computedCount;
}




void Start_Multi_Compute(
    SegmentTree *segmentTree,
    vector<string> &columns,
    SCREEN_M4 &screen_m4,
    FunInfo &func,
    const std::string &mode,
    bool parallel,
    int width,
    int height,
    double errorBound)
{
    cout<<"Start_Multi_Compute "<<endl;
    //auto &M4_array = screen_m4.M4_array;

    // 初始化 M4 数据
    stats.timestart("initM4");
    initM4(segmentTree, columns, screen_m4.M4_array, func, mode, parallel, screen_m4);
    stats.timeend("initM4");
    //std::vector<int> needQueryIndex;

    // std::vector<std::vector<Node *>> needQueryNodesTrees(segmentTrees.size());

    int computedCount = 0;
    int errorBoundSatisfyCount = 0;
    while (computedCount < screen_m4.M4_array.size() * 2)
    {
        stats.timestart("errorBoundSatisfy");
        if (errorBoundSatisfy(screen_m4, width, height, errorBound, errorBoundSatisfyCount))
        {
            break;
        }
        stats.timeend("errorBoundSatisfy");

        stats.timestart("runColumnComputeBatch");
        computedCount = runColumnComputeBatch(
            segmentTree,
            columns,
            screen_m4,
            func,
             mode,
            parallel,
             width,
             height,
            errorBound
        );
        stats.timeend("runColumnComputeBatch");

        screen_m4.candidateMax = -INFINITY;
        screen_m4.candidateMin = INFINITY;
        screen_m4.iterations++;
    }

    if (computedCount >= screen_m4.M4_array.size() * 2)
    {
         errorBoundSatisfy(screen_m4, width, height, errorBound, errorBoundSatisfyCount);
    }
}

void aggregateCalculation(SCREEN_M4 &screen_m4, SegmentTree* &segmentTree, FunInfo &func, int width, int height, string mode, vector<string> &columns, bool parallel, bool errorBound){

    screen_m4.exactMin = 100000;
    screen_m4.exactMax = -100000;

    for(int i=0;i<screen_m4.M4_arrays.size();i++){
        vector<M4*> &M4_array = screen_m4.M4_arrays[i];
        fenlie(M4_array, 0, -1, columns, func,segmentTree);

        for(int j=0;j<M4_array.size();j++){
            M4* m4=M4_array[j];
            m4->st_v = m4->stNodes[i]->min;
            m4->et_v = m4->etNodes[i]->min;

            m4->min = std::min(m4->st_v, m4->et_v);
            m4->max = std::max(m4->st_v, m4->et_v);
            for(int k=0;k<m4->innerNodes.size();k++){
                double min = m4->innerNodes[k][i]->min;
                double max = m4->innerNodes[k][i]->max;
                m4->min = std::min(m4->min, min);
                m4->max = std::max(m4->max, max);
                m4->isCompletedMax = true;
                m4->isCompletedMin = true;
            }
            screen_m4.exactMin = std::min(m4->min, screen_m4.exactMin);
            screen_m4.exactMax = std::max(m4->max, screen_m4.exactMax);
            screen_m4.candidateMax=screen_m4.exactMin;
            screen_m4.candidateMin=screen_m4.exactMax;
        }


    }




}

void computeMultyOrSingle(SegmentTree* &segmentTree,
    string &table, int dataCount, vector<string> &columns,
    FunInfo &func, int width, int height, string mode,
    vector<string> &symble, bool parallel, double errorBound,
    int screenStart, int screenEnd, SCREEN_M4 &screen_m4)
{


cout<< "computeMultyOrSingle: "  << endl;
    // console.log(tables, func, width,height, mode, symble, parallel, errorBound,screenStart,screenEnd)

    // if(func.mode == 'single'){
    //     columns = [columns[0]]
    // }

    //SegmentTree *segmentTree;
    //stats.timestart("xxx");
    // Loop through columns and create segment trees

    if (trees.find(table) == trees.end())
    {
        cout << table << " not exists." << endl;
    }
    else
    {
        segmentTree = trees[table];
        mmdata = mmdatas[table];
    }
    //stats.timestart("xxx");
    // Get real data row number
    segmentTree->nodeCount = 0;
    int realDataRowNum = getRealDataRowNum(segmentTree);
cout<< "realDataRowNum: " << realDataRowNum << endl;
cout<< ":"<< segmentTree->flagssize  << endl;
    //stats.timeend("xxx");




    screen_m4.segmentTrees.push_back(segmentTree);
    screen_m4.height = height;
    screen_m4.width = width;
    screen_m4.errorBound = errorBound;
    screen_m4.deltaError = 0.05;




    if (isnan(screenStart) || screenStart < 0)
    {
        screenStart = 0;
    }
    if (isnan(screenEnd) || screenEnd < 0 || screenEnd > realDataRowNum - 1)
    {
        screenEnd = realDataRowNum - 1;
    }

    // If screen_m4 has a function, process M4 arrays
    vector<int> timeRange = {screenStart, screenEnd};
    if (screen_m4.func->funName == "" || screen_m4.func->funName == "show")
    {
        for (int i = 0; i < columns.size(); i++)
        {

            //vector<M4*> M4_array;
            //computeM4TimeSE(width, timeRange, M4_array);
            computeM4TimeSE(width, timeRange, screen_m4.M4_array);
            screen_m4.M4_arrays.push_back(screen_m4.M4_array);
            //screen_m4.M4_array.clear();
        }
        return aggregateCalculation(screen_m4, segmentTree, func, width, height, mode, columns, parallel, errorBound);
    }
    else
    {

    
        stats.timestart("computeM4TimeSE");
        computeM4TimeSE(width, timeRange, screen_m4.M4_array);

        stats.timeend("computeM4TimeSE");
        //screen_m4.M4_array = M4_array;
        screen_m4.M4_arrays.push_back(screen_m4.M4_array);
        cout<<"M4_array size:"<< screen_m4.M4_array.size() <<endl;
    }


    // 判断条件，决定是否需要 query 和 compute
    // if（判定条件），then compute（）// 最后考虑。

    // Process query and compute
    // 从 SegmentTree1 的最底层开始，找到 StartIndex 和 EndIndex

    if (screen_m4.func != nullptr && screen_m4.func->funName != "")
    {
        // return await aggregateCalculation(screen_m4, segmentTrees, func, width, height, mode, symble, parallel, errorBound);
    }

    stats.timestart("fenlie");
    fenlie(screen_m4.M4_array, screenStart, screenEnd, columns, func, segmentTree);
    stats.timeend("fenlie");

    stats.timestart("Start_Multi_Compute");
    Start_Multi_Compute(segmentTree, columns, screen_m4, func, mode, parallel, width, height, errorBound);
    stats.timeend("Start_Multi_Compute");

    screen_m4.segmentTrees.push_back(segmentTree);
    // screen_m4.dataReductionRatio = 1 - segmentTrees[0].nodeCount * 2 / segmentTrees[0].realDataNum;

    // return screen_m4;
}

void finalcal_min(SegmentTree* &segmentTree, std::vector<SegmentTreeNode*> currentComputingNodePairs, FunInfo& func, M4* m4) {
    if (isLeafNode(segmentTree, currentComputingNodePairs[0]->index)) {
        auto [tmpmin, tmpmax]  = unifiedCalculate(nullptr, currentComputingNodePairs, func, "", true);
        // double tmpmin = result.tmpmin;
        // double tmpmax = result.tmpmax;
        if (m4->min > tmpmin) {
            m4->min = tmpmin;
        }
    } else {
        auto [tmpmin, tmpmax]  = unifiedCalculate(nullptr, currentComputingNodePairs, func, "nullptr", false);
        // double tmpmin = result.tmpmin;
        // double tmpmax = result.tmpmax;
        if (m4->min > tmpmin) {
            m4->min = (m4->min + tmpmin) / 2;
        }
    }
}

void finalcal_max(SegmentTree* &segmentTree, std::vector<SegmentTreeNode*> currentComputingNodePairs, FunInfo& func, M4* m4) {
    if (isLeafNode(segmentTree, currentComputingNodePairs[0]->index)) {
        auto [tmpmin, tmpmax]  = unifiedCalculate(nullptr, currentComputingNodePairs, func, "nullptr", true);
        // double tmpmin = result.tmpmin;
        // double tmpmax = result.tmpmax;
        if (m4->max < tmpmax) {
            m4->max = tmpmax;
        }
    } else {
        auto [tmpmin, tmpmax]  = unifiedCalculate(nullptr, currentComputingNodePairs, func, "nullptr", false);
        // double tmpmin = result.tmpmin;
        // double tmpmax = result.tmpmax;
        if (m4->max < tmpmax) {
            m4->max = (m4->max + tmpmax) / 2;
        }
    }
}


void finalCompute(SegmentTree* &segmentTree, SCREEN_M4& screen_m4, FunInfo& func) {
    //auto M4_array = screen_m4.M4_array;
    // if (M4_array == nullptr) {
    //     return;
    // }

    for (size_t i = 0; i < screen_m4.M4_array.size(); i++) {
        M4* m4 = screen_m4.M4_array[i];

        bool debug = false;
        if (i == 8) {
            debug = true;
        }

        // 计算min
        if (!m4->isCompletedMin) {
            if (!m4->alternativeNodesMin.empty()) {
                auto ele = m4->alternativeNodesMin.top();
                if (m4->min > ele.value) {
                    finalcal_min(segmentTree, ele.nodePairs, func, m4);
                }
            }

            // for (size_t j = 0; j < m4.currentComputingNodeMin.size(); j++) {
            //     auto currentComputingNodePairs = m4.currentComputingNodeMin[j];
            //     finalcal_min(currentComputingNodePairs, func, *m4);
            // }
        }

        // 计算max
        if (!m4->isCompletedMax) {
            if ( !m4->alternativeNodesMax.empty()) {
                auto ele = m4->alternativeNodesMax.top();
                if (m4->max < ele.value) {
                    finalcal_max(segmentTree, ele.nodePairs, func, m4);
                }
            }

            // for (size_t j = 0; j < m4->currentComputingNodeMax.size(); j++) {
            //     auto currentComputingNodePairs = m4->currentComputingNodeMax[j];
            //     finalcal_max(currentComputingNodePairs, func, *m4);
            // }
        }

        // 更新exact Min和Max
        if (screen_m4.exactMax < m4->max) {
            screen_m4.exactMax = m4->max;
        }
        if (screen_m4.exactMin > m4->min) {
            screen_m4.exactMin = m4->min;
        }
    }
}


void CandidateAsValue(SCREEN_M4 &screen_m4, FunInfo &func)
{

    //std::vector<M4> &M4_array = screen_m4.M4_array;

    for (size_t i = 0; i < screen_m4.M4_array.size(); ++i)
    {
        M4 *m4 = screen_m4.M4_array[i];

        bool debug = (i == 108);

        // 计算 min
        if (!screen_m4.M4_array[i]->isCompletedMin)
        {
            if (!m4->alternativeNodesMin.empty())
            {
                auto ele = m4->alternativeNodesMin.top();
                if (m4->min > ele.value)
                {
                    m4->min = ele.value;
                }
            }

            // for (const auto &currentComputingNodePairs : m4.currentComputingNodeMin)
            // {
            //     CalculationResult result = unifiedCalulate(nullptr, currentComputingNodePairs, func, nullptr, false);
            //     if (m4.min > result.tmpmin)
            //     {
            //         m4.min = result.tmpmin;
            //     }
            // }
        }

        // 计算 max
        if (!screen_m4.M4_array[i]->isCompletedMax)
        {
            if (!m4->alternativeNodesMax.empty())
            {
                auto ele = m4->alternativeNodesMax.top();
                if (m4->max < ele.value)
                {
                    m4->max = ele.value;
                }
            }

            // for (const auto &currentComputingNodePairs : m4.currentComputingNodeMax)
            // {
            //     CalculationResult result = unifiedCalulate(nullptr, currentComputingNodePairs, func, nullptr, false);
            //     if (m4.max < result.tmpmax)
            //     {
            //         m4.max = result.tmpmax;
            //     }
            // }
        }

        // 更新 candidateMin / candidateMax
        if (screen_m4.candidateMax < m4->max)
        {
            screen_m4.candidateMax = m4->max;
        }
        if (screen_m4.candidateMin > m4->min)
        {
            screen_m4.candidateMin = m4->min;
        }
    }
}

void ours_cpp(string table_name, int dataCount, vector<string> columns, string symble, string params, int width, int height, string mode, int parallel, double errorBound, int startTime, int endTime, string interact_type, SCREEN_M4 &screen_m4)
{
    // procesStartTime = performance.now() / 1000.0;

    // 处理符号并分割参数
    vector<string> symbleParts;
    stringstream ss(symble);
    string item;
    while (getline(ss, item, ';'))
    {
        symbleParts.push_back(item);
    }
    // if (symbleParts.size() > 1) {
    //     stringstream paramStream(symbleParts[1]);
    //     string paramItem;
    //     while (getline(paramStream, paramItem, ',')) {
    //         params.push_back(paramItem);
    //     }
    // }

    FunInfo funInfo(symbleParts[0]);

    // 处理合并的表
    string table = mergeTables(table_name, columns, funInfo, "cpp");

    SegmentTree* segmentTree;
    //stats.timestart("computeMultyOrSingle");
    computeMultyOrSingle(segmentTree, table, dataCount, columns, funInfo, width, height, mode, symbleParts, parallel, errorBound, startTime, endTime, screen_m4);
    //stats.timeend("computeMultyOrSingle");

    vector<vector<string>> M4_arrays;
    vector<double> min_values;
    vector<double> max_values;

    if (screen_m4.estimateType == "a")
    {
        screen_m4.min_values.push_back(screen_m4.exactMin);
        screen_m4.max_values.push_back(screen_m4.exactMax);
    }
    else if (screen_m4.estimateType == "b")
    {
        screen_m4.min_values.push_back(screen_m4.exactMin);
        screen_m4.max_values.push_back(screen_m4.exactMax);
    }
    else if (screen_m4.estimateType == "c")
    {
        screen_m4.min_values.push_back(screen_m4.candidateMin);
        screen_m4.max_values.push_back(screen_m4.candidateMax);
    }
    else if (screen_m4.estimateType == "d")
    {
        finalCompute(segmentTree, screen_m4, funInfo);
        screen_m4.min_values.push_back(screen_m4.exactMin);
        screen_m4.max_values.push_back(screen_m4.exactMax);
    }
    else if (screen_m4.estimateType == "e")
    {
        CandidateAsValue(screen_m4, funInfo);
        screen_m4.min_values.push_back(screen_m4.exactMin);
        screen_m4.max_values.push_back(screen_m4.exactMax);
    }
    else if (screen_m4.estimateType == "f")
    {
        CandidateAsValue(screen_m4, funInfo);
        screen_m4.min_values.push_back(screen_m4.candidateMin);
        screen_m4.max_values.push_back(screen_m4.candidateMax);
    }
    else
    {
        finalCompute(segmentTree, screen_m4, funInfo);
        screen_m4.min_values.push_back(screen_m4.exactMin);
        screen_m4.max_values.push_back(screen_m4.exactMax);
    }

    // return screen_m4;
}

void ours_duck(const std::string &table_name, int dataCount, const std::vector<std::string> &columns, const std::string &symbol,
               const std::string &extra, int width, int height, const std::string &mode, int parallel, double errorBound,
               int startTime, int endTime, const std::string &interact_type, SCREEN_M4 &screen_m4)
{
    // Placeholder for "ours_duck" experiment
}

void aggregate(const std::string &table_name, const std::vector<std::string> &columns, int width, SCREEN_M4 &screen_m4)
{
    // Placeholder for "aggregate" experiment logic
}

void Case1(const std::string &table_name, const std::vector<std::string> &columns, const std::string &symbol,
           const std::string &extra, int width, int height, const std::string &mode, int parallel, double errorBound,
           int startTime, int endTime, const std::string &interact_type, SCREEN_M4 &screen_m4)
{
    // Placeholder for Case1
}

void Case2(const std::string &table_name, const std::vector<std::string> &columns, const std::string &symbol,
           const std::string &extra, int width, int height, const std::string &mode, int parallel, double errorBound,
           int startTime, int endTime, const std::string &interact_type, SCREEN_M4 &screen_m4)
{
    // Placeholder for Case2
}

void Case3(const std::string &table_name, int dataCount, const std::vector<std::string> &columns, const std::string &symbol,
           const std::string &extra, int width, int height, const std::string &mode, int parallel, double errorBound,
           int startTime, int endTime, const std::string &interact_type, SCREEN_M4 &screen_m4)
{
    // Placeholder for Case3
}

void Case4(const std::string &table_name, const std::vector<std::string> &columns, const std::string &symbol,
           const std::string &extra, int width, int height, const std::string &mode, int parallel, double errorBound,
           int startTime, int endTime, const std::string &interact_type, SCREEN_M4 &screen_m4)
{
    // Placeholder for Case4
}

void Case5(const std::string &table_name, const std::vector<std::string> &columns, const std::string &symbol,
           const std::string &extra, int width, int height, const std::string &mode, int parallel, double errorBound,
           int startTime, int endTime, const std::string &interact_type, SCREEN_M4 &screen_m4)
{
    // Placeholder for Case5
}

void Case6(const std::string &table_name, const std::vector<std::string> &columns, const std::string &symbol,
           const std::string &extra, int width, int height, const std::string &mode, int parallel, double errorBound,
           int startTime, int endTime, const std::string &interact_type, SCREEN_M4 &screen_m4)
{
    // Placeholder for Case6
}

void Case7(const std::string &table_name, const std::vector<std::string> &columns, const std::string &symbol,
           const std::string &extra, int width, int height, const std::string &mode, int parallel, double errorBound,
           int startTime, int endTime, const std::string &interact_type, SCREEN_M4 &screen_m4)
{
    // Placeholder for Case7
}

void Case6_test(const std::string &table_name, const std::vector<std::string> &columns, const std::string &symbol,
                const std::string &extra, int width, int height, const std::string &mode, int parallel, double errorBound,
                int startTime, int endTime, const std::string &interact_type, SCREEN_M4 &screen_m4)
{
    // Placeholder for Case6_test
}

std::string joinWithComma(const std::vector<std::string>& vec) {
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        oss << vec[i];
        if (i != vec.size() - 1) {
            oss << ",";
        }
    }
    return oss.str();
}

// Helper function to create directory
bool create_directory(const std::string& path) {
    // Using system command to create directory
    std::string cmd = "mkdir -p " + path;
    return system(cmd.c_str()) == 0;
}

// Helper function to check if directory exists
bool directory_exists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
}

void outputM4(const SCREEN_M4 &screen_m4)
{
    auto M4_array = screen_m4.M4_array;
    string memeoryuse;
    printMemoryUsage(memeoryuse);

    // Check if the path exists, create if it doesn't
    try {
        if (!directory_exists("../m4_result/")) {
            if (!create_directory("../m4_result/")) {
                std::cerr << "Error: Failed to create data directory." << std::endl;
            }
            std::cout << "Created directory: ../m4_result/" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error creating directory: " << e.what() << std::endl;
    }

    for (const auto& [key, val] : stats.callCounts) {
        stats.timetotal(key);
    }

    double SQLtime = stats.getTotalTime("SQL.query.time");
    double totalTime = stats.getTotalTime("totaltime");

    if (std::isnan(SQLtime))
    {
        SQLtime = 0;
    }

    string columnsstr = joinWithComma(screen_m4.columns);

    std::cout << "experiment: " << screen_m4.experiment<<"-"<<MAX_THREADS_MAIN
              << " ; table: " << screen_m4.datasetname
              << " ; interacttype: " << screen_m4.interact_type
              << " ; columns: " << columnsstr;
    // for (const auto &col : screen_m4.columns)
    //     std::cout << col << " ";
    std::cout //<< "; nodeCount: " << screen_m4.segmentTrees[0].nodeCount
        << " ; errorBound: " << screen_m4.errorBound
        << " ; symbolName: " << screen_m4.symbolName
        << " ; width: " << screen_m4.width
        << " ; startTime: " << screen_m4.screenStart
        << " ; endTime: " << screen_m4.screenEnd
        << " ; iterations: " << screen_m4.iterations
        << " ; totaltime: " << std::fixed << std::setprecision(3) << totalTime
        << " ; sqltime: " << std::fixed << std::setprecision(3) << SQLtime
        << " ; memeoryuse: " << memeoryuse
        << " ; error: " << std::fixed << std::setprecision(4) << screen_m4.preError
        << " ; nodeCount: " << screen_m4.segmentTrees[0]->nodeCount/screen_m4.columns.size()
        << " ; lastcolumn: " << screen_m4.columns[screen_m4.columns.size()-1]
        << std::endl;

    if (!screen_m4.M4_array.empty())
    {
        std::ostringstream writestr;
        writestr << "m4 info "
                //<< "; globalMin: " << screen_m4.min_values
                 //<< " ;globalMax: " << screen_m4.max_values
            << " ; experiment: " << screen_m4.experiment
            << " ;interacttype: " << screen_m4.interact_type
            << " ;table: " << screen_m4.datasetname
            << " ; columns: " << columnsstr;
        // for (const auto &col : screen_m4.columns)
        //     writestr << col << " ";
        writestr << " ;symbol: " << screen_m4.symbolName
                 << " ;width: " << screen_m4.width
                 << " ;height: " << screen_m4.height
                 << " ;startTime: " << screen_m4.screenStart
                 << " ; endTime: " << screen_m4.screenEnd
                 << " ;errorBound: " << screen_m4.errorBound
                 << " ; error: " << std::fixed << std::setprecision(4) << screen_m4.preError
                 << " ; nodeCount: " << screen_m4.segmentTrees[0]->nodeCount/screen_m4.columns.size()
                 << " ; totaltime: " << std::fixed << std::setprecision(3) << totalTime
                 << " ; sqltime: " << std::fixed << std::setprecision(3) << SQLtime
                 << " ; iterations: " << screen_m4.iterations
                 << "; memeoryuse: " << memeoryuse
            //<< " ; nodeCount: " << screen_m4.segmentTrees[0].nodeCount;
            ;
        for (size_t i = 0; i < M4_array.size(); ++i)
        {
            const auto &m4 = M4_array[i];
            writestr << "\nm4: " << i
                     << " sT: " << m4->start_time
                     << " ,eT: " << m4->end_time
                     << " ,sV: " << std::fixed << std::setprecision(3) << m4->st_v
                     << " ,eV: " << m4->et_v
                     << " ,min: " << m4->min
                     << " ,max: " << m4->max;
        }

        std::string filename = "../m4_result/" + screen_m4.experiment + "_" +
                               screen_m4.interact_type + "_" +
                               (screen_m4.columns.empty() ? "col" : columnsstr) + "_" +
                               std::to_string(screen_m4.width) + "_" +
                               std::to_string(static_cast<int>(screen_m4.memLimit)) + "_" +
                               screen_m4.datasetname + "_" +
                               std::to_string(static_cast<int>(screen_m4.screenStart)) + "_" +
                               std::to_string(static_cast<int>(screen_m4.screenEnd)) + "_" +
                               std::to_string(screen_m4.errorBound) + "_" +
                               screen_m4.symbolName + ".m4";

        std::ofstream file(filename);
        file << writestr.str();
        file.close();
    }
    else
    {
        // for (size_t m = 0; m < screen_m4.M4_arrays.size(); ++m)
        // {
        //     auto M4_array = screen_m4.M4_arrays[m];

        //     std::ostringstream writestr;
        //     writestr //<< "m4 info; globalMin: " << screen_m4.min_values
        //              //<< " ;globalMax: " << screen_m4.max_values
        //         << " ; experiment: " << screen_m4.experiment
        //         << " ,interacttype: " << screen_m4.interact_type
        //         << " ,table: " << screen_m4.datasetname
        //         << "; columns: " << columnsstr
        //         << " ;symbol: " << screen_m4.symbolName << "#";
        //     // for (const auto& p : screen_m4.func.params) writestr << p << ",";
        //     writestr << " ;width: " << screen_m4.width
        //              << " ;height: " << screen_m4.height
        //              << " ;startTime: " << screen_m4.screenStart
        //              << " ; endTime: " << screen_m4.screenEnd
        //              << " ;errorBound: " << screen_m4.errorBound
        //              << " ; error: " << std::fixed << std::setprecision(4) << screen_m4.preError
        //              << " ; memLimit: " << screen_m4.memLimit;

        //     for (size_t i = 0; i < M4_array.size(); ++i)
        //     {
        //         const auto &m4 = M4_array[i];
        //         writestr << "\nm4: " << i
        //                  << " sT: " << m4.start_time
        //                  << " ,eT: " << m4.end_time
        //                  << " ,sV: " << std::fixed << std::setprecision(3) << m4.st_v
        //                  << " ,eV: " << m4.et_v
        //                  << " ,min: " << m4.min
        //                  << " ,max: " << m4.max;
        //     }

        //     std::ostringstream filename;
        //     filename << "../m4_result/"
        //              << screen_m4.experiment << "_"
        //              << screen_m4.interact_type << "_"
        //              << columnsstr << "_"
        //              << screen_m4.width << "_"
        //              << static_cast<int>(screen_m4.memLimit) << "_"
        //              << screen_m4.datasetname << "_"
        //              << static_cast<int>(screen_m4.screenStart) << "_"
        //              << static_cast<int>(screen_m4.screenEnd) << "_"
        //              << screen_m4.errorBound << "_"
        //              << screen_m4.symbolName << "#";
        //     // for (const auto& p : screen_m4.func.params) filename << p << ","
        //     ;

        //     filename << ".m4";

        //     std::ofstream file(filename.str());
        //     file << writestr.str();
        //     file.close();
        // }
    }

    // 注释的 ablationStudy 区域略
}

void deleteAll(SCREEN_M4 &screen_m4){
    //delete M4
    for(int i=0;i<screen_m4.M4_array.size();i++){
        M4* m4 = screen_m4.M4_array[i];
        //delete innerNodes
        for(int j = 0;j<m4->innerNodes.size();j++){
            vector<SegmentTreeNode *> nodePair = m4->innerNodes[j];
            for(int k=0;k<nodePair.size();k++){
                delete nodePair[k];
            }
        }

        //delete alternativeNodesMax
        while(!m4->alternativeNodesMax.empty()){
            Element MaxEle = m4->alternativeNodesMax.top();
            m4->alternativeNodesMax.pop();
            vector<SegmentTreeNode *> nodePair = MaxEle.nodePairs;
            for(int k=0;k<nodePair.size();k++){
                delete nodePair[k];
            }
        }

        //delete alternativeNodesMin
        while(!m4->alternativeNodesMin.empty()){
            Element MinEle = m4->alternativeNodesMin.top();
            m4->alternativeNodesMin.pop();
            vector<SegmentTreeNode *> nodePair = MinEle.nodePairs;
            for(int k=0;k<nodePair.size();k++){
                delete nodePair[k];
            }
        }

        //delete m4
        //delete m4;
    }

}


std::string getFuncName(const std::string& symbol) {
    std::string symbolName = symbol.substr(0, symbol.find(';'));

    // 使用映射表来进行替换
    if (symbolName == "plus") {
        symbolName = "+";
    } else if (symbolName == "minus") {
        symbolName = "-";
    } else if (symbolName == "multi") {
        symbolName = "*";
    } else if (symbolName == "divide") {
        symbolName = "/";
    } else if (symbolName == "boxcox0") {
        symbolName = "boxcox_0";
    } else if (symbolName == "boxcox12") {
        symbolName = "boxcox_1_2";
    } else if (symbolName == "boxcox1") {
        symbolName = "boxcox_1";
    } else if (symbolName == "boxcox2") {
        symbolName = "boxcox_2";
    }
    
    return symbolName;
}


std::string getSymbolName(const std::string &symbol)
{
    // 用分号分隔符将符号字符串分割成多个部分
    std::istringstream ss(symbol);
    std::string part;
    std::getline(ss, part, ';'); // 获取第一个部分（符号本身）

    // 定义一个映射表
    std::map<std::string, std::string> symbolMap = {
        {"+", "plus"},
        {"-", "minus"},
        {"*", "multi"},
        {"/", "divide"},
        {"boxcox_0", "boxcox0"},
        {"boxcox_1_2", "boxcox12"},
        {"boxcox_1", "boxcox1"},
        {"boxcox_2", "boxcox2"}};

    std::string symbolName = symbolMap[part]; // 查找符号映射

    // 如果符号名为空，则返回原始符号（未在映射表中）
    if (symbolName.empty())
    {
        symbolName = part;
    }

    // 如果符号字符串中还有其他部分（例如：boxcox_0;1234），则附加到符号名称后面
    if (std::getline(ss, part))
    {
        symbolName += "#" + part;
    }

    return symbolName;
}

int getCount(string table_name)
{

    // std::cout << table_name << std::endl;
    if (trees.find(table_name) != trees.end())
    {
        std::cout << table_name << " count: " << trees[table_name]->linecount << std::endl;
        return trees[table_name]->linecount;
    }
    else
    {
        std::cout << table_name << " not exist." << std::endl;
        return 0;
    }

    return 0;
}






void Experiments(const std::string &experiment, int startTime, int endTime, const std::string &table_name, int dataCount,
    const std::vector<std::string> &columns, std::string symbol, int width, int height,
    const std::string &mode, int parallel, double errorBound, const std::string &interact_type, SCREEN_M4 &screen_m4)
{
    std::cout <<"Experiments:"<<endl;

// 打印所有参数，逗号分隔
std::cout << "experiment: " << experiment << ", "
          << "startTime: " << startTime << ", "
          << "endTime: " << endTime << ", "
          << "table_name: " << table_name << ", "
          << "dataCount: " << dataCount << ", "
          << "columns: ";

for (size_t i = 0; i < columns.size(); ++i) {
    std::cout << columns[i];
    if (i != columns.size() - 1) {
        std::cout << ", ";
    }
}

std::cout << ", symbol: " << symbol << ", "
          << "width: " << width << ", "
          << "height: " << height << ", "
          << "mode: " << mode << ", "
          << "parallel: " << parallel << ", "
          << "errorBound: " << errorBound << ", "
          << "interact_type: " << interact_type << std::endl;


//return;
// 数据集名称处理
//std::string datasetname = table_name;
cout<<"screen_m4.datasetname:"<<screen_m4.datasetname<<endl;
screen_m4.datasetname.erase(
std::remove(screen_m4.datasetname.begin(), screen_m4.datasetname.end(), '_'),
screen_m4.datasetname.end()
);
cout<<"screen_m4.datasetname:"<<screen_m4.datasetname<<endl;
// 获取符号名称
std::string symbolName = getSymbolName(symbol);

// 设置屏幕参数
screen_m4.estimateType = "estimateType";
screen_m4.sx = "sx";
screen_m4.screenStart = startTime;
screen_m4.screenEnd = endTime;
screen_m4.columns = columns;

// 处理符号，类似 JavaScript 中的 split
std::vector<std::string> s;
std::stringstream ss(symbol);
std::string temp;
while (std::getline(ss, temp, ';'))
{
s.push_back(temp);
}

if (s.size() > 1)
{
    symbol = s[0];
    std::vector<std::string> ag;
    std::stringstream agStream(s[1]);
    while (std::getline(agStream, temp, '#'))
    {
        ag.push_back(temp);
    }
    // 创建 FunInfo 对象并设置给 func
    FunInfo *func = new FunInfo(ag[0], "", ag[1]);
    screen_m4.func = func;
}
else
{
    screen_m4.func = nullptr;
}

screen_m4.func = new FunInfo(symbol);


// 调用 genDatainfo 函数
genDatainfo(screen_m4);

// 时间处理
stats.timeclear();
stats.timestart("totaltime");

// 根据实验类型调用不同的实验函数
if (experiment == "ours-cpp")
{
ours_cpp(table_name, dataCount, columns, symbol, "", width, height, mode, parallel, errorBound, startTime, endTime, interact_type, screen_m4);
}
else if (experiment == "ours-pg")
{
ours_pg(table_name, dataCount, columns, symbol, "", width, height, mode, parallel, errorBound, startTime, endTime, interact_type, screen_m4);
}
else if (experiment == "ours-duck")
{
ours_duck(table_name, dataCount, columns, symbol, "", width, height, mode, parallel, errorBound, startTime, endTime, interact_type, screen_m4);
}
else if (experiment == "aggregate")
{
aggregate(table_name, columns, width, screen_m4);
}
else if (experiment == "case1")
{
Case1(table_name, columns, symbol, "", width, height, mode, parallel, errorBound, startTime, endTime, interact_type, screen_m4);
}
else if (experiment == "PG-F")
{
Case2(table_name, columns, symbol, "", width, height, mode, parallel, errorBound, startTime, endTime, interact_type, screen_m4);
}
else if (experiment == "OM3")
{
Case3(table_name, dataCount, columns, symbol, "", width, height, mode, parallel, errorBound, startTime, endTime, interact_type, screen_m4);
}
else if (experiment == "PG-M4")
{
Case4(table_name, columns, symbol, "", width, height, mode, parallel, errorBound, startTime, endTime, interact_type, screen_m4);
}
else if (experiment == "IFX-F")
{
Case5(table_name, columns, symbol, "", width, height, mode, parallel, errorBound, startTime, endTime, interact_type, screen_m4);
}
else if (experiment == "IFX-M4")
{
Case6(table_name, columns, symbol, "", width, height, mode, parallel, errorBound, startTime, endTime, interact_type, screen_m4);
}
else if (experiment == "IFX-R")
{
Case7(table_name, columns, symbol, "", width, height, mode, parallel, errorBound, startTime, endTime, interact_type, screen_m4);
}
else if (experiment == "test")
{
Case6_test(table_name, columns, symbol, "", width, height, mode, parallel, errorBound, startTime, endTime, interact_type, screen_m4);
}

// 结束时间记录
stats.timeend("totaltime");

// 输出结果
outputM4(screen_m4);
deleteAll(screen_m4);
}





#endif // EXPERIMENTS_H
