// TimerStats.h

#ifndef TIMERSTATS_H
#define TIMERSTATS_H

#include <unordered_map>
#include <string>
#include <chrono>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace chrono;


class TimerStats
{
public:
    unordered_map<string, double> functionTimes;                         // 累计时间（毫秒）
    unordered_map<string, high_resolution_clock::time_point> startTimes; // 开始时间
    unordered_map<string, int> callCounts;                               // 调用次数

    void timeclear()
    {
        functionTimes.clear();
        startTimes.clear();
        callCounts.clear();
    }

    void timestart(const string &functionName)
    {
        startTimes[functionName] = high_resolution_clock::now();
    }

    double timeend(const string &functionName)
    {
        if (startTimes.find(functionName) == startTimes.end())
        {
            cerr << "No start time recorded for " << functionName << endl;
            return 0.0;
        }

        auto start = startTimes[functionName];
        auto end = high_resolution_clock::now();
        double timeSpent = duration<double, milli>(end - start).count(); // 毫秒

        functionTimes[functionName] += timeSpent;
        callCounts[functionName]++;

        startTimes.erase(functionName); // 清除开始时间

        return timeSpent;
    }

    double getTotalTime(const string &functionName)
    {
        if (functionTimes.find(functionName) != functionTimes.end())
        {
            return functionTimes[functionName] / 1000.0; // 返回秒
        }
        return 0.0;
    }

    int getTotalCount(const string &functionName)
    {
        if (callCounts.find(functionName) != callCounts.end())
        {
            return callCounts[functionName];
        }
        return 0;
    }

    double getAvgTime(const string &functionName)
    {
        if (functionTimes.find(functionName) != functionTimes.end())
        {
            int count = getTotalCount(functionName);
            return (count > 0) ? (functionTimes[functionName] / 1000.0 / count) : 0.0;
        }
        else
        {
            cout << "No timing data for " << functionName << endl;
            return 0.0;
        }
    }

    void timetotal(const string &functionName)
    {
        if (functionTimes.find(functionName) != functionTimes.end())
        {
            double totalTime = functionTimes[functionName];
            int count = getTotalCount(functionName);
            cout << fixed << setprecision(2);
            cout << "Total time for " << functionName << ": " << totalTime << " ms, called " << count << " times" << endl;
        }
        else
        {
            cout << "No timing data for " << functionName << endl;
        }
    }
};



#endif // TIMERSTATS_H
