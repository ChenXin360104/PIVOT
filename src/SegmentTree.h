#ifndef SEGMENTTREE_H
#define SEGMENTTREE_H

#include <vector>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <string>
#include <cmath> 

using namespace std;



int *readFlagzToIntArray(const std::string &filepath, size_t &outSize)
{
    std::ifstream file(filepath, std::ios::binary);

    if (!file)
    {
        std::cerr << "无法打开文件: " << filepath << std::endl;
        outSize = 0;
        return nullptr;
    }

    // 获取文件大小（字节数）
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    outSize = size;

    // 申请 int 数组，存储每个字节的值（0~255）
    int *flags = new int[size];

    char byte;
    for (size_t i = 0; i < size; ++i)
    {
        if (!file.read(&byte, 1))
        {
            std::cerr << "读取失败，位置: " << i << std::endl;
            delete[] flags;
            outSize = 0;
            return nullptr;
        }
        // 将 unsigned char 转成 int（确保是非负）
        flags[i] = static_cast<unsigned char>(byte);
    }

    return flags;
}


std::string getFirstColumn(const std::string &str)
{
    // 找到第一个逗号的位置
    size_t pos = str.find(',');

    // 如果找到了逗号，返回逗号前的部分，否则返回整个字符串
    if (pos != std::string::npos)
    {
        return str.substr(0, pos);
    }
    else
    {
        return str; // 如果没有逗号，返回原始字符串
    }
}


std::vector<std::string> splitString(const std::string &input, char delimiter)
{
    std::vector<std::string> result;
    std::string item;
    std::stringstream ss(input);

    // 按指定的分隔符分割字符串
    while (std::getline(ss, item, delimiter))
    {
        // 去除引号
        if (item.front() == '\"' && item.back() == '\"')
        {
            item = item.substr(1, item.length() - 2);
        }
        result.push_back(item); // 将分割后的字符串添加到 vector 中
    }

    return result;
}


int count_fields(const std::string &input)
{
    std::stringstream ss(input);
    std::string field;
    int count = 0;

    // 逐个读取字段
    while (std::getline(ss, field, ','))
    {
        count++; // 每读取一个字段，计数加1
    }

    return count;
}



class SegmentTreeNode
{
public:
    int index;
    double min;
    double max;
    // 构造函数
    SegmentTreeNode(
        int index = 0,
        double min = 0,
        double max = 0
        ):

          index(index),

          min(min),
          max(max)
    {
    }
};


class SegmentTreeNodePool {
    public:
        SegmentTreeNodePool(size_t capacity = 1000000) {
            pool = static_cast<SegmentTreeNode*>(::operator new(capacity * sizeof(SegmentTreeNode)));
            for (size_t i = 0; i < capacity; ++i) {
                freeList.push_back(pool + i);
            }
        }
    
        ~SegmentTreeNodePool() {
            ::operator delete(pool);
        }
    
        SegmentTreeNode* allocate() {
            if (freeList.empty()) {
                cout<<"node pool is full !!!"<<endl;
                throw std::bad_alloc(); // 或者扩展内存池
            }
            auto obj = freeList.back(); // 存下来
            freeList.pop_back();        // 再删
            return obj;                 // 最后返回
            //return freeList.back(); freeList.pop_back();
        }
    
        void deallocate(SegmentTreeNode* node) {
            freeList.push_back(node);
        }
    
    private:
        SegmentTreeNode* pool;
        std::vector<SegmentTreeNode*> freeList;
};


class SegmentTree
{
public:
    string table_name;
    int v_count;
    int linecount;
    int nodeCount;

    int globalstarttime;
    int globalendtime;
    int sTime;
    int eTime;

    int max_i;
    double **maxvd_array;
    double **minvd_array;
    int **flags;
    size_t flagssize;

    SegmentTreeNode *root;
    SegmentTreeNode **head;

    SegmentTreeNode * addNode(int index, double min, double max){
        nodeCount ++;
        return new SegmentTreeNode(index,min,max);
    }

    int printout(){
    

        cout<<"flag:"<<endl;
        for(int i=0;i<flagssize;i++){
            cout<<i<<':'<<endl;
            for(int j=0;j<v_count;j++){
                cout<<flags[j][i]<<',';
            }
            cout<<endl;
        }

        return 0;
    }

    int init(vector<string> rows)
    {

        string header = rows[0];
        //cout << header << endl;
        this->v_count = (count_fields(header) - 1) / 3;
        //cout << this->v_count << endl;

        string tail = rows.back();
        //cout << tail << endl;

        this->max_i = std::stoi(getFirstColumn(tail));
        //cout << max_i << endl;

        this->maxvd_array = new double *[v_count];
        this->minvd_array = new double *[v_count];
        this->root = new SegmentTreeNode[v_count];
        this->head = new SegmentTreeNode *[v_count];
        flags = new int *[v_count];
        this->loadFlags();

        for (int i = 0; i < v_count; i++)
        {
            this->maxvd_array[i] = new double[max_i+1];
            this->minvd_array[i] = new double[max_i+1];
            // this->root[i] = new SegmentTreeNode[max_i];
            //  flags[i] = new double[max_i];
        }

        //cout << rows[1] << endl;
        for (int k = 1; k < rows.size(); k++)
        {
            vector<string> row = splitString(rows[k], ',');

            int i = std::stoi(row[0]);
            if (i == -1)
            {
                i = 0;
            }

            for (int v = 0; v < v_count; v++)
            {
                // cout << row[1 + v*3] << endl;
                if (row[1 + v * 3].empty())
                {
                    this->minvd_array[v][i] = std::nan("1");
                    // cout <<"min," << v<<','<<i << endl;
                }
                else
                {
                    double minvd = std::stod(row[1 + v * 3]);
                    this->minvd_array[v][i] = minvd;
                    //cout <<"min," << v<<','<<i << endl;
                }

                if (row[2 + v * 3].empty())
                {
                    this->maxvd_array[v][i] = std::nan("1");
                    // cout <<"max,"<< v<<','<<i << endl;
                }
                else
                {
                    double maxvd = std::stod(row[2 + v * 3]);
                    this->maxvd_array[v][i] = maxvd;
                }
                if (i == 0)
                {
                    this->head[v] = &(this->root[v]);

                    this->sTime = 0;
                    this->eTime = this->flagssize - 1;
                    this->root[v].min = this->minvd_array[v][i];
                    this->root[v].max = this->maxvd_array[v][i];
                }
            }
            // break;
        }

        // cout << this->maxvd_array[1][3] << endl;

        return 0;
    }

    int loadFlags()
    {

        for (int i = 0; i < v_count; i++)
        {
            std::ostringstream oss;
            oss << "../flags/" << table_name << "_om3_v" << i + 1 << ".flagz";
            string flagname = oss.str();
            //std::cout << flagname << std::endl;

            size_t size = 0;
            // int *flags = readFlagzToIntArray(flagname, flagssize);

            this->flags[i] = readFlagzToIntArray(flagname, size);
            this->flagssize = size;
            // if (this->flags[i])
            // {
            //     std::cout << "共读取 " << size << " 个标志位" << std::endl;

            //     // 示例：打印前10个
            //     for (size_t j = 0; j < std::min(size_t(10), size); ++j)
            //     {
            //         std::cout << "flags[" << j << "] = " << this->flags[i][j] << std::endl;
            //     }

            //     // 别忘了释放内存
            //     //delete[] flags;
            // }
        }

        return 0;
    }

 
};






#endif // SEGMENTTREE_H
