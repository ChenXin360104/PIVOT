
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

#include <omp.h>

using namespace std;

#include <iostream>
#include <unordered_map>
#include <chrono>
#include <string>
#include <iomanip>
#include <iostream>
#include <vector>
#include <cmath>
#include <map>
#include <string>
#include <limits>
#include <algorithm>

using namespace std;
using namespace std::chrono;
#include <iostream>
#include <vector>
#include <cmath>
#include <map>
#include <string>
#include <limits>
#include <algorithm>

#include <iostream>
#include <fstream>
#include <string>
//#include "ThreadPool.h" // 如果你把线程池放到了一个头文件里
#include <future>
#include "MemoryManager.h"

#include"TimerStats.h"
#include"SegmentTree.h"
#include"FunInfo.h"
#include"SCREEN_M4.h"
#include"Experiments.h"





//ThreadPool pool(MAX_THREADS_MAIN); // 最多16个线程
//std::vector<std::future<void>> futures;




string csvfiles;
string dataset;





SegmentTreeNodePool nodePool(1); // 预分配2000万个节点
    

#include <iostream>
#include <vector>
#include <queue>
#include <memory>




std::string getFilenameFromPath(const std::string &file_path)
{

    // 找到最后一个斜杠（文件名的起始位置）
    size_t lastSlash = file_path.find_last_of("/\\");
    size_t start = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;

    // 找到最后一个点（扩展名起始位置）
    size_t lastDot = file_path.find_last_of('.');

    //cout << file_path << endl;

    if (lastDot == std::string::npos || lastDot < start)
    {
        // 没有扩展名
        return file_path.substr(start);
    }

    return file_path.substr(start, lastDot - start);
}

int extractNumberAfterCSV(const std::string &filepath)
{
    // 找到最后一个 ".csv" 的位置
    size_t csvPos = filepath.find(".csv");
    // cout<< filepath << endl;
    if (csvPos != std::string::npos)
    {
        // 从".csv"后的位置开始提取
        // cout<< filepath.substr(csvPos + 5) << endl;
        return stoi(filepath.substr(csvPos + 5)); // ".csv" 后面是数字部分
    }

    return 0; // 如果没有找到 ".csv" 返回空字符串
}

int loaddatas()
{

    // 使用 vector 存储文件路径
    vector<string> filePaths;

    // 添加文件路径到 vector
    //filePaths.push_back("../data/om3data/synthetic10k2v.csv_10000");
    //filePaths.push_back("../data/om3data/nycdata.csv_4733280");
    //filePaths.push_back("../data/om3data/synthetic_4m_om3.csv_3944701");
    filePaths.push_back(csvfiles);

    // 输出所有的文件路径
    for (int i = 0; i < filePaths.size(); i++)
    {
        string path = filePaths[i];
        // cout << path << endl;
        int linecount = extractNumberAfterCSV(path);
        // 打开 CSV 文件
        ifstream file(path);
        if (!file.is_open())
        {
            cerr << "无法打开文件!" << endl;
            return 1;
        }

        string line;
        vector<string> rows;
        while (getline(file, line))
        {
            // 使用 stringstream 来解析 CSV 行
            rows.push_back(line);
        }

        file.close();

        // // 打印当前行的数据
        // for (const auto& row : rows) {
        //     cout << row << endl;
        // }

        // string table_name = getFilenameFromPath(path);
        SegmentTree *t = new SegmentTree();
        t->table_name = getFilenameFromPath(path);
        t->linecount = linecount;
        // t->root = new SegmentTreeNode()
        t->sTime = 0;
        t->eTime = t->linecount - 1;
        t->init(rows);

        //t->printout();
        trees[t->table_name] = t;

        cout << t->table_name << ',' << t->linecount << endl;
    }

    return 0;
}





#include <unistd.h>
#include <sys/stat.h> // for stat
bool fileExists2(const std::string& filePath) {
    struct stat buffer;
    return (stat(filePath.c_str(), &buffer) == 0);
  }

struct InteractionPlan {
    std::string dataset;
    std::string type;
    std::string columns;
    int startTime;
    int endTime;
    std::string symbol;
    int width;
};



int interactions(string plansfile){

    if (fileExists2(plansfile)) {
        std::cout << "文件存在 (相对路径): " << plansfile << std::endl;
      } else {
        std::cout << "文件不存在 (相对路径): " << plansfile << std::endl;
    }

    vector<InteractionPlan> Plans;

    // 打开文件
    std::ifstream file(plansfile);
    
    // 检查文件是否成功打开
    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    std::string line;
    
    // 按行读取文件
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string field;
        
        InteractionPlan plan;
        
        // 读取并分割每一行
        std::getline(ss, plan.dataset, ',');
        std::getline(ss, plan.type, ',');
        std::getline(ss, plan.columns, ',');
        ss >> plan.startTime;
        ss.ignore(1, ','); // 忽略逗号
        ss >> plan.endTime;
        ss.ignore(1, ','); // 忽略逗号
        std::getline(ss, plan.symbol, ',');
        ss >> plan.width;
        
        // 输出解析后的数据
        std::cout << "dataset: " << plan.dataset
                  << ", type: " << plan.type
                  << ", columns: " << plan.columns
                  << ", startTime: " << plan.startTime
                  << ", endTime: " << plan.endTime
                  << ", symbol: " << plan.symbol
                  << ", width: " << plan.width
                  << std::endl;
        Plans.push_back(plan);
    }


//return 0;
    //std::vector<double> errorbounds = {0.00000001};
    std::vector<double> errorbounds = {0.05, 0.00000001};

    for(const auto &errorBound : errorbounds){
        for(int i=0;i<Plans.size();i++){
            InteractionPlan plan = Plans[i];
            std::vector<std::string> estimates = {"d"};
            std::vector<std::string> ablationStudy = {"s1", "s2", "s3", "s4"};
            std::string experiment = "ours-cpp"; 
            //double errorBound = 0.05;
    
            std::string table_name = plan.dataset;
            //std::vector<std::string> columns = plan.columns;
            std::string columns_str = plan.columns;
            std::vector<std::string> columns;
            size_t pos = 0;
            while ((pos = columns_str.find('I')) != std::string::npos)
            {
                columns.push_back(columns_str.substr(0, pos));
                columns_str.erase(0, pos + 1);
            }
            columns.push_back(columns_str); // 添加最后一个字段
    
            // 获取数据计数
            int dataCount = getCount(table_name);
    
    
            if (trees.find(table_name) != trees.end())
            {
                cout<<table_name<<" exist tree"<< std::endl;
            }
            else
            {
                cout<<table_name<<" not exist tree" << std::endl;
    
                mmdata = new MemoryManager(table_name);
                mmdata->load_minmax_data(MAX_THREADS_MAIN);
                mmdatas[table_name] = mmdata;
        
                SegmentTree *t = new SegmentTree();
                t->linecount = mmdata->get_data_size();
                int maxT = t->linecount - 1;
        
                t->flagssize = (int)pow(2, ceil(log2(maxT+1)));
                t->sTime = 0;
                t->eTime = t->flagssize-1;
                trees[table_name] = t;
            }
    
    
            int width = plan.width;
            int height = 600; // 高度固定
            int startTime = plan.startTime;
            int endTime = plan.endTime;
            string symbol = getFuncName(plan.symbol);
            std::string symbolName = getSymbolName(symbol);
    
            // 创建屏幕对象
            SCREEN_M4 screen_m4(experiment, table_name, 0, symbolName, width, height, errorBound);
            screen_m4.M4_array.reserve(2000);
            cout<<"M4_array size:"<< screen_m4.M4_array.size() <<endl;
    
            // 设置交互类型
            screen_m4.interact_type = "interaction-" + plan.type;
            screen_m4.estimateType = "estimateType";
            screen_m4.sx = "sx";
    
            // 执行实验
            Experiments(experiment, startTime, endTime, table_name, dataCount, columns, symbol, width, height, "single", 1, errorBound, "null", screen_m4);    
        }

    }
    

    return 0;
}

int static_exp()
{
    std::vector<std::string> estimates = {"d"};
    std::vector<std::string> ablationStudy = {"s1", "s2", "s3", "s4"};
    std::vector<std::string> experiments = {"ours-cpp"};                  //{"ours-duck", "ours-pg","PG-F","OM3","PG-M4", "IFX-M4", "IFX-F"};
    //std::vector<std::string> datasets = {"nycdata_om3 7,9,1,2,3,4,5,6,8,10,11"}; //{"synthetic_1m 1,2,3,4,5", "synthetic_2m 1,2,3,4,5", "synthetic_4m 1,2,3,4,5", "synthetic_8m 1,2,3,4,5", "synthetic_16m 1,2,3,4,5", "synthetic_32m 1,2,3,4,5", "synthetic_64m 1,2,3,4,5", "synthetic_128m 1,2,3,4,5", "nycdata 7,9,1,2,3,4,5,6,8,10,11", "soccerdata 5,3,1,2,4,6", "stockdata 9,5,1,2,3,4,6,7,8,10", "traffic 1,2,3,4,5,6,7,8,9,10", "sensordata 5,4,3,2,1,7,6"};
    //std::vector<std::string> datasets = {"nycdata_om3 7,9,1,2,3,4,5,6,8,10,11", "synthetic10k2v_om3 2,1"};

    //std::vector<std::string> functions = {"func1", "boxcox_0", "+", "-", "*", "/", "func4", "mean", "variance", "max", "L2", "weightsum"};                          


    std::vector<std::string> datasets;
    datasets.push_back(dataset);
    std::vector<std::string> functions = {"func1", "boxcox_0", "+", "-", "*", "/", "mean", "variance", "max", "L2", "weightsum", "L22", "L2LN"};  
    std::vector<int> widths = {200, 400, 600, 800, 1000, 1200};
    std::vector<double> errorbounds = {0.1, 0.05, 0.01, 0.000001};
    bool isTreeCache = false;
    bool isParallel = false;

    // 迭代实验数据
    for (const auto &data : datasets)
    {
        // 解析数据集名称和列
        size_t space_pos = data.find(' ');
        std::string table_name = data.substr(0, space_pos);
        std::string columns_str = data.substr(space_pos + 1);
        std::vector<std::string> columns;
        size_t pos = 0;
        while ((pos = columns_str.find(',')) != std::string::npos)
        {
            columns.push_back(columns_str.substr(0, pos));
            columns_str.erase(0, pos + 1);
        }
        columns.push_back(columns_str); // 添加最后一个字段

        // 获取数据计数
        int dataCount = getCount(table_name);

        string sss;
        cout<<"before load:\n";
        printMemoryUsage(sss);
        mmdata = new MemoryManager(table_name);
        mmdata->load_minmax_data(MAX_THREADS_MAIN);
        mmdatas[table_name] = mmdata;
        cout<<"after load:\n";
        printMemoryUsage(sss);

        SegmentTree *t = new SegmentTree();
        t->linecount = mmdata->get_data_size();
        int maxT = t->linecount - 1;

        t->flagssize = (int)pow(2, ceil(log2(maxT+1)));
        t->sTime = 0;
        t->eTime = t->flagssize-1;
        trees[table_name] = t;

        // std::string experiment = ex.substr(0, ex.size()-1);;
        for (const auto &errorBound : errorbounds)
        {
            for (const auto &symbol : functions)
            {
                for (const auto &experiment : experiments)
                {
                    for (size_t i = 0; i < widths.size(); i++)
                    {
                        int width = widths[i];
                        int height = 600; // 高度固定
                        int startTime = 0, endTime = -1;
                        std::string symbolName = getSymbolName(symbol);

                        // 创建屏幕对象
                        SCREEN_M4 screen_m4(experiment, table_name, 0, symbolName, width, height, errorBound);
                        screen_m4.M4_array.reserve(2000);
                        cout<<"M4_array size:"<< screen_m4.M4_array.size() <<endl;

                        // 设置交互类型
                        screen_m4.interact_type = "static";
                        screen_m4.estimateType = "estimateType";
                        screen_m4.sx = "sx";

                        // 执行实验
                        Experiments(experiment, startTime, endTime, table_name, dataCount, columns, symbol, width, height, "single", 1, errorBound, "null", screen_m4);
                    }
                }
            }
        }
    }

    return 0;
}

void testxxx(){

    // for(int i=0;i<849772;i++){
    //     stats.timestart("new node1");
    //     SegmentTreeNode *leftChild = new (nodePool.allocate()) SegmentTreeNode(1,1,1,1,1,1,1,1,"ss",nullptr);
    //     stats.timeend("new node1");
    // }


    stats.timestart("new 内存池版本");
    for(int i=0;i<849772;i++){
        SegmentTreeNode *leftChild = new (nodePool.allocate()) SegmentTreeNode(1,1);
    }
    stats.timeend("new 内存池版本");


    stats.timestart("new 无内存池");
    for(int i=0;i<849772;i++){
        SegmentTreeNode *leftChild = new SegmentTreeNode(1,1);
    }
    stats.timeend("new 无内存池");

    for (const auto& [key, val] : stats.callCounts) {
        stats.timetotal(key);
    }

}



int main(int argc, char *argv[])
{

    // 创建线程池
    // ThreadPool pool(2); // 最多16个线程
    // std::vector<std::future<void>> futures;
    //futures.reserve(100);

    // todo:前后加一下内存监测

    //testxxx();return 0;


    // 输出命令行参数的数量
    std::cout << "命令行参数的数量: " << argc << std::endl;

    // 遍历所有命令行参数并输出
    for (int i = 0; i < argc; ++i)
    {
        std::cout << "参数 " << i << ": " << argv[i] << std::endl;
        std::cout << "参数 1213221" << i << ": " << argv[i] << std::endl;
    }

    // 获取实验类型
    string experimentType = argv[1];
    csvfiles = argv[2];
    dataset = argv[3];
    MAX_THREADS_MAIN = stoi(argv[4]);
    cout<< "MAX_THREADS_MAIN:"<<MAX_THREADS_MAIN<<endl;


//return 0;

    //loaddatas();
    //printMemoryUsage();

    // 根据实验类型选择不同的函数
    if (experimentType == "interactions")
    {
        interactions(csvfiles);
    }
    else if (experimentType == "static")
    {
        static_exp();
    }
    else if (experimentType == "agg")
    {
        // agg();
    }
    else if (experimentType == "linenum")
    {
        // linenum();
    }
    else if (experimentType == "tsnum")
    {
        // tsnum();
    }
    else
    {
        std::cerr << "Error: Unknown experiment type: " << experimentType << std::endl;
        return 1;
    }

    //printMemoryUsage();

    return 0;
}
