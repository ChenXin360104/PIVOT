#ifndef FUNINFO_H
#define FUNINFO_H

#include <string>
#include <chrono>
#include <iostream>
#include <iomanip>

#include <iostream>
#include <vector>
#include <cmath>
#include <map>
#include <string>
#include"SegmentTree.h"

class FunInfo
{
public:
    std::string funName;
    std::string params;
    // std::vector<double> extremes;
    std::string mode = "multi";
    double intervalRange = 0.0;
    std::map<std::string, std::vector<double>> maxExtremes;
    std::map<std::string, std::vector<double>> minExtremes;
    std::vector<double> weights;

    FunInfo(std::string funName, std::string extremes = "", std::string params = "")
    {
        this->funName = funName;
        this->params = params;
        // this->extremes = extremes;
        init();
    }

    void init()
    {
        maxExtremes.clear();
        minExtremes.clear();

        maxExtremes["func1"] = {-31.62};
        minExtremes["func1"] = {31.62};

        maxExtremes["boxcox_0"] = {};
        minExtremes["boxcox_0"] = {};

        maxExtremes["boxcox_1_2"] = {};
        minExtremes["boxcox_1_2"] = {};

        maxExtremes["boxcox_1"] = {};
        minExtremes["boxcox_1"] = {};

        maxExtremes["boxcox_2"] = {};
        minExtremes["boxcox_2"] = {0};

        maxExtremes["func3"] = {};
        minExtremes["func3"] = {};

        maxExtremes["func4"] = {};
        minExtremes["func4"] = {};

        weights = weights = {
            0.58, 0.04, 0.94,
            0.57, 0.09, 0.03,
            0.27, 0.26,  0.6,
            0.01, 0.47, 0.99,
            
            0.58, 0.04, 0.94,
            0.57, 0.09, 0.03,
            0.27, 0.26,  0.6,
            0.01, 0.47, 0.99
        };
    }

    double weightsum(const std::vector<SegmentTreeNode *> &nodes, const std::string &destination, bool isLeaf)
    {
        double sum = 0.0;
        for (size_t i = 0; i < nodes.size(); ++i)
        {
            double weight = weights[i];
            if (destination == "min")
            {
                sum += weight > 0 ? weight * nodes[i]->min : weight * nodes[i]->max;
            }
            else
            {
                sum += weight < 0 ? weight * nodes[i]->min : weight * nodes[i]->max;
            }
        }
        return sum;
    }

    double L2LN(const std::vector<SegmentTreeNode *> &nodes, const std::string &destination, bool isLeaf)
    {
        double sum = 0.0;
        for (const auto &node : nodes)
        {
            if (destination == "min")
            {
                double x = std::log(node->min+1);
                double y = x * x;
                sum += y;
            }
            else
            {
                double x = std::log(node->max+1);
                double y = x * x;
                sum += y;
            }
        }
        return std::sqrt(sum);
    }

    double L2(const std::vector<SegmentTreeNode *> &nodes, const std::string &destination, bool isLeaf)
    {
        double sum = 0.0;
        for (const auto &node : nodes)
        {
            if (destination == "min")
            {
                if (node->min * node->max < 0)
                {
                    sum += 0;
                }
                else
                {
                    sum += std::min(node->min * node->min, node->max * node->max);
                }
            }
            else
            {
                sum += std::max(node->min * node->min, node->max * node->max);
            }
        }
        return std::sqrt(sum);
    }

    double getmax(const std::vector<SegmentTreeNode *> &nodes, const std::string &destination, bool isLeaf)
    {
        double maxofmax = -std::numeric_limits<double>::infinity();
        double maxofmin = -std::numeric_limits<double>::infinity();

        for (const auto &node : nodes)
        {
            maxofmax = std::max(maxofmax, node->max);
            maxofmin = std::max(maxofmin, node->min);
        }

        return destination == "min" ? maxofmin : maxofmax;
    }

    double func4(const std::vector<SegmentTreeNode *> &nodes, const std::string &destination, bool isLeaf)
    {
        double x_min = std::numeric_limits<double>::infinity();
        double x_max = -std::numeric_limits<double>::infinity();
        double y_min = 0;
        double y_max = 0;

        if (isLeaf)
        {
            double x = nodes[0]->min;
            double y = nodes[1]->min;
            return (1 - std::sin(x)) * std::pow(std::log(std::abs(y) + 1), 2);
        }

        auto xRange = nodes[0];
        auto yRange = nodes[1];

        if (xRange->max - xRange->min >= 2 * M_PI)
        {
            x_min = -1;
            x_max = 1;
        }
        else
        {
            std::vector<double> extreme = {xRange->min, xRange->max}; // simplified
            for (const auto &val : extreme)
            {
                double tmp = std::sin(val);
                x_min = std::min(x_min, tmp);
                x_max = std::max(x_max, tmp);
            }
        }

        y_max = std::max(std::abs(yRange->min), std::abs(yRange->max));
        y_min = (yRange->min * yRange->max < 0) ? 0 : std::min(std::abs(yRange->min), std::abs(yRange->max));

        return destination == "min"
                   ? (1 - x_max) * std::pow(std::log(y_min + 1), 2)
                   : (1 - x_min) * std::pow(std::log(y_max + 1), 2);
    }

    double compute(const std::string &funName, const std::vector<SegmentTreeNode *> &nodes, const std::string &destination)
    {
        const auto &ext = (destination == "min") ? minExtremes[funName] : maxExtremes[funName];
        std::vector<double> Xs = generateXs(nodes[0]->min, ext, nodes[0]->max);
        double result = (destination == "min") ? std::numeric_limits<double>::infinity() : -std::numeric_limits<double>::infinity();

        for (const auto &x : Xs)
        {
            double y = getFunc(funName, x);
            if (destination == "min")
            {
                result = std::min(result, y);
            }
            else
            {
                result = std::max(result, y);
            }
        }
        return result;
    }

    double compute_default(double x)
    {
        return x;
    }

    double getFunc(const std::string &funName, double x)
    {
        if (funName == "func1")
            return func1(x);
        if (funName == "boxcox_0")
            return boxcox_0(x);
        if (funName == "boxcox_1_2")
            return boxcox_1_2(x);
        if (funName == "boxcox_1")
            return boxcox_1(x);
        if (funName == "boxcox_2")
            return boxcox_2(x);
        return compute_default(x);
    }

    double func1(double x)
    {
        return 0.001 * std::pow(x, 3) - 3 * x;
    }

    double boxcox_0(double x)
    {
        return (x <= 0) ? 0 : std::log(x);
    }

    double boxcox_1_2(double x)
    {
        return (x <= 0) ? 0 : (std::sqrt(x) - 1) / 0.5;
    }

    double boxcox_1(double x)
    {
        return x - 1;
    }

    double boxcox_2(double x)
    {
        return (std::pow(x, 2) - 1) / 2;
    }

    std::vector<double> generateXs(double min, const std::vector<double> &extremes, double max)
    {
        std::vector<double> x;
        for (const auto &e : extremes)
        {
            if (e > min && e < max)
            {
                x.push_back(e);
            }
        }
        x.push_back(min);
        x.push_back(max);
        return x;
    }



    std::vector<double> computes_default(const std::vector<double> &Xs)
    {
        std::vector<double> Ys;
        for (const auto &x : Xs)
        {
            Ys.push_back(compute_default(x));
        }
        return Ys;
    }
};


#endif // FUNINFO_H
