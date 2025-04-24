#include "crow.h"
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <utility>


#include "Experiments.h"


using namespace std;

static_assert(sizeof(std::string) > 0, "System headers work");

// 查询参数结构体
struct ChartQueryParams {
    std::string table_name;
    std::string columns;
    std::string symbol;
    std::string mode;
    int width;
    int height;
    int64_t startTime;
    int64_t endTime;
    std::string interact_type;
    std::string experiment;
    int parallel;
    double errorBound;
    std::string aggregate;
};

// 字符串分割函数
std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// CORS中间件
struct CORSMiddleware {
    struct context {};
    
    void before_handle(crow::request& req, crow::response& res, context& ctx) {}

    void after_handle(crow::request& req, crow::response& res, context& ctx) {
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "*");
        
        if (req.method == crow::HTTPMethod::OPTIONS) {
            res.code = 204;
        }
    }
};

std::vector<std::string> processString(const std::string& input) {
    std::vector<std::string> result;
    std::stringstream ss(input);
    std::string temp;

    // 按逗号分割字符串
    while (std::getline(ss, temp, ',')) {
        // 去掉 'v' 字符
        temp.erase(std::remove(temp.begin(), temp.end(), 'v'), temp.end());
        result.push_back(temp);
    }

    return result;
}

int main() {
    crow::App<CORSMiddleware> app;
    cout<<"start"<<endl;

    
    string table_name = "nycdata";
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


    
    // 获取默认表和信息
    CROW_ROUTE(app, "/postgres/line_chart/getAllDefaultTableAndInfo")
    ([](const crow::request& req) {
        crow::json::wvalue response;
        response["code"] = 200;
        response["data"] = crow::json::wvalue{
            {"defaultTable", "stockdata"},
            {"defaultColumns", crow::json::wvalue::list({"v1", "v2"})}
        };
        return crow::response(response);
    });

    // 获取所有标志名称
    CROW_ROUTE(app, "/postgres/line_chart/getAllFlagNames")
    ([](const crow::request& req) {
        crow::json::wvalue response;
        response["code"] = 200;
        response["data"] = crow::json::wvalue::list({
            "flag1", "flag2", "flag3"
        });
        return crow::response(response);
    });
    

    // 获取列名
    CROW_ROUTE(app, "/postgres/line_chart/getcolumns")
    ([](const crow::request& req) {
        std::string table_name = req.url_params.get("table_name");
        crow::json::wvalue response;
        response["code"] = 200;
        response["columns"] = crow::json::wvalue{
            {crow::json::wvalue::list({"v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11"})}
        };
        return crow::response(response);
    });

    // 获取函数列表
    CROW_ROUTE(app, "/postgres/line_chart/getfunctions")
    ([](const crow::request& req) {
        crow::json::wvalue response;
        response["code"] = 200;
        response["functions"] = crow::json::wvalue{
            {crow::json::wvalue::list({
                "show", "func1", "boxcox_0", "+", "-", "*", "/", "mean", "variance", "max", "L2", "weightsum", "L22", "L2LN"
            })}
        };
        return crow::response(response);
    });

    // 获取实验列表
    CROW_ROUTE(app, "/postgres/line_chart/getexperiment")
    ([](const crow::request& req) {
        crow::json::wvalue response;
        response["code"] = 200;
        response["experiments"] = crow::json::wvalue{
            {crow::json::wvalue::list({
                "ours"
            })}
        };
        return crow::response(response);
    });

    // 获取表格列表
    CROW_ROUTE(app, "/postgres/line_chart/gettables")
    ([](const crow::request& req) {
        crow::json::wvalue response;
        response["code"] = 200;
        response["tables"] = crow::json::wvalue{
            {crow::json::wvalue::list({
                "nycdata"
            })}
        };
        return crow::response(response);
    });
    CROW_ROUTE(app, "/test")
    ([](const crow::request& req) {
        return "Test endpoint working";
    });
    CROW_ROUTE(app, "/")
    ([](const crow::request& req) {
        return "Server is running";
    });
    CROW_ROUTE(app, "/postgres/line_chart/getSingleFlag")
    .methods("GET"_method)
    ([](const crow::request& req) {
        try {
            // 添加调试日志
            CROW_LOG_INFO << "Received request to /postgres/line_chart/getSingleFlag";
            
            auto name = req.url_params.get("name");
            auto line_type = req.url_params.get("line_type");
            
            if (!name || !line_type) {
                CROW_LOG_ERROR << "Missing required parameters";
                return crow::response(400, "Missing required parameters");
            }
            
            CROW_LOG_INFO << "Parameters: name=" << name << ", line_type=" << line_type;

            crow::json::wvalue response;
            response["code"] = 200;
            response["tables"] = crow::json::wvalue{
                {crow::json::wvalue::list({
                    "nycdata"
                })}
            };
            
            return crow::response(response);
        } catch (const std::exception& e) {
            CROW_LOG_ERROR << "Error: " << e.what();
            return crow::response(500, e.what());
        }
    });
    // 核心路由start
    CROW_ROUTE(app, "/postgres/line_chart/start")
    ([](const crow::request& req) {
        try {
            // 解析查询参数
            ChartQueryParams params;
            
            if (!req.url_params.get("table_name")) {
                crow::json::wvalue error_response;
                error_response["code"] = 400;
                error_response["message"] = "Missing required parameter: table_name";
                crow::response res = error_response;
                res.code = 400;
                return res;
            }
        
            params.table_name = req.url_params.get("table_name") ? req.url_params.get("table_name") : "";
            params.columns = req.url_params.get("columns") ? req.url_params.get("columns") : "";
            params.symbol = req.url_params.get("symbol") ? req.url_params.get("symbol") : "";
            params.mode = req.url_params.get("mode") ? req.url_params.get("mode") : "";
            params.width = req.url_params.get("width") ? std::stoi(req.url_params.get("width")) : 600;
            params.height = req.url_params.get("height") ? std::stoi(req.url_params.get("height")) : 600;
            params.startTime = req.url_params.get("startTime") ? std::stoll(req.url_params.get("startTime")) : 0;
            params.endTime = req.url_params.get("endTime") ? std::stoll(req.url_params.get("endTime")) : -1;
            params.interact_type = req.url_params.get("interact_type") ? req.url_params.get("interact_type") : "";
            params.experiment = req.url_params.get("experiment") ? req.url_params.get("experiment") : "";
            params.parallel = req.url_params.get("parallel") ? std::stoi(req.url_params.get("parallel")) : 1;
            params.errorBound = req.url_params.get("errorBound") ? std::stod(req.url_params.get("errorBound")) : 0.05;
            params.aggregate = req.url_params.get("aggregate") ? req.url_params.get("aggregate") : "";

            if(params.experiment == "ours" || params.experiment == "" ){
                params.experiment = "ours-cpp";
            }

            std::cout << "table_name: " << params.table_name << ", "
              << "columns: " << params.columns << ", "
              << "symbol: " << params.symbol << ", "
              << "mode: " << params.mode << ", "
              << "width: " << params.width << ", "
              << "height: " << params.height << ", "
              << "startTime: " << params.startTime << ", "
              << "endTime: " << params.endTime << ", "
              << "interact_type: " << params.interact_type << ", "
              << "experiment: " << params.experiment << ", "
              << "parallel: " << params.parallel << ", "
              << "errorBound: " << params.errorBound << ", "
              << "aggregate: " << params.aggregate << std::endl;
              

            //std::string symbolName = getSymbolName(params.symbol);
              
            string symbol = getFuncName(params.symbol);
            std::string symbolName = getSymbolName(symbol);
              // 创建屏幕对象
              SCREEN_M4 screen_m4(params.experiment, params.table_name, 0, symbolName, params.width, params.height, params.errorBound);
              screen_m4.M4_array.reserve(2000);
              std::cout<<"M4_array size:"<< screen_m4.M4_array.size() <<endl;

              // 设置交互类型
              screen_m4.interact_type = "static";
              screen_m4.estimateType = "estimateType";
              screen_m4.sx = "sx";

              std::vector<std::string> columns = processString(params.columns);




            Experiments(params.experiment, params.startTime, params.endTime, params.table_name, 0, columns, symbol, params.width, params.height, "single", 1, params.errorBound, "null", screen_m4);





            std::vector<std::string> requestedColumns = split(params.columns, ',');
            std::vector<crow::json::wvalue> allSeries;

            int dataMaxLen=0;
            for(const auto& M4_array : screen_m4.M4_arrays){
                auto series_data = crow::json::wvalue::list();
                for(int i=0;i<M4_array.size();i++){
                    const auto& m4 = M4_array[i];
                    auto dp = crow::json::wvalue::object();
                    dp["i"] = i;
                    // if(i < 100){
                    //     cout<<"i:"<<i<<",start_time:"<<m4->start_time<<endl;
                    //     cout<<"j:"<<i<<",start_time:"<<screen_m4.M4_array[i]->start_time<<endl;
                    // }
                    dp["st"] = m4->start_time;
                    dp["sv"] = m4->st_v;
                    dp["min"] = m4->min;
                    dp["max"] = m4->max;
                    dp["et"] = m4->end_time;
                    dp["ev"] = m4->et_v;
                    dp["timestamp"] = screen_m4.screenStartTimestamp + (m4->start_time-screen_m4.screenStart)*screen_m4.delta;

                    series_data.push_back(std::move(dp));

                    dataMaxLen = std::max((double)dataMaxLen, (double)m4->end_time);
                }
                allSeries.push_back(std::move(series_data));
            }


            // // 示例数据
            // std::vector<std::pair<int, std::vector<double>>> data = {
            //     {1420041600, {0, 0, 9.001962272862446, 8.996528148090857, 9.004668301573977, 166, 9.000729834944558}},
            //     {1420051620, {1, 167, 9.00146947994869, 8.998878325435811, 9.004913941467132, 333, 9.002701007197938}},
            //     {1420061640, {2, 334, 9.002701007197938, 8.999989642460726, 9.004176840696656, 500, 9.002701007197938}}
            // };

            // for (const auto& column : requestedColumns) {
            //     auto series_data = crow::json::wvalue::list();
                
            //     for (const auto& point : data) {
            //         auto dp = crow::json::wvalue::object();
            //         dp["i"] = point.second[0];
            //         dp["st"] = point.second[1];
            //         dp["sv"] = point.second[2];
            //         dp["min"] = point.second[3];
            //         dp["max"] = point.second[4];
            //         dp["et"] = point.second[5];
            //         dp["ev"] = point.second[6];
            //         dp["timestamp"] = point.first;
                    
            //         series_data.push_back(std::move(dp));
            //     }
                
            //     allSeries.push_back(std::move(series_data));
            // }

            crow::json::wvalue response;
            response["code"] = 200;

            
            response["totaltime"] = 0.0;
            response["M4_arrays"] = std::move(allSeries);
            response["min_values"] = std::move(screen_m4.min_values);//std::vector<double>(requestedColumns.size(), 8.88875674784872);
            response["max_values"] = std::move(screen_m4.max_values);//std::vector<double>(requestedColumns.size(), 9.150165648940114);
            response["dataMaxLen"] = dataMaxLen;
            response["columns"] = requestedColumns;

            return crow::response(response);
        } catch (const std::exception& e) {
            crow::json::wvalue error_response;
            error_response["code"] = 500;
            error_response["message"] = std::string("Internal server error: ") + e.what();
            crow::response res = error_response;
            res.code = 500;
            return res;
        }
    });

    // 启动服务器
    std::cout << "Starting server on port 3000..." << std::endl;
    crow::logger::setLogLevel(crow::LogLevel::Debug); 
    CROW_LOG_INFO << "Starting server on port 3000";
    app.port(3000).multithreaded().run();
    
    return 0;
}
