#pragma once
#include <iostream>
#include <sstream>
#include <vector>
#include <set>
#include <unordered_set>
#include <chrono>
#include <cstdlib>
#include <string>
#include <regex>
#include <fstream>
#if defined(_WIN32)
#include <direct.h>  // For Windows _mkdir, _getcwd
#define GetCurrentDir _getcwd
#define MakeDir(path) mkdir(path,0777)
#else
#include <unistd.h>  // For POSIX getcwd
#include <sys/stat.h>  // For POSIX mkdir
#define GetCurrentDir getcwd
#define MakeDir(path) mkdir(path,0777)
#endif

#define PRINT_CURRENT_LOCATION() \
    std::cout << "File: " << __FILE__ << ", Line: " << __LINE__ << std::endl;

// 单位毫秒
#define CPU_TIME(accum_time, func_call) \
    { \
        auto start_time = std::chrono::high_resolution_clock::now(); \
        func_call; \
        auto end_time = std::chrono::high_resolution_clock::now(); \
        accum_time += std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count(); \
    }




#define PRINT_VECTOR(vec,str) do {\
    std::cout << "*******************************************************\n";\
    std::cout << (str) << std::endl;\
    for (const auto& elem : (vec)) {\
        std::cout << " " << elem;\
    }\
    std::cout << std::endl;\
    std::cout << "*******************************************************\n";\
}while(0)

#define PRINT_2VECTOR(vec,str) do {\
    std::cout << "*******************************************************\n";\
    std::cout << (str) << std::endl;\
    for (const auto& elem : (vec)) {\
        for(const auto& i : elem)\
        {\
            std::cout << " " << i; \
        }\
        std::cout << std::endl;\
    }\
    std::cout << std::endl;\
    std::cout << "*******************************************************\n";\
}while(0)



#define PRINT_VECTOR_LOGGER(vec, str, logger) do { \
    (logger) << "*******************************************************\n"; \
    (logger) << (str) << std::endl; \
    for (const auto& elem : (vec)) { \
        (logger) << " " << elem; \
    } \
    (logger) << std::endl; \
    (logger) << "*******************************************************\n"; \
} while(0)

#define PRINT_2VECTOR_LOGGER(vec, str, logger) do { \
    (logger) << "*******************************************************\n"; \
    (logger) << (str) << std::endl; \
    for (const auto& elem : (vec)) { \
        for(const auto& i : elem) { \
            (logger) << " " << i; \
        } \
        (logger) << std::endl; \
    } \
    (logger) << std::endl; \
    (logger) << "*******************************************************\n"; \
} while(0)




// expr need to be true
#define ASSERT_MSG(expr, msg) do{\
if (!(expr)) {\
    std::cerr << "Assertion failed: (" #expr "), function " << __FUNCTION__ \
        << ", file " << __FILE__ << ", line " << __LINE__ << "." << std::endl; \
        std::cerr << "Message: " << msg << std::endl; \
        std::abort(); \
    }\
}while(0)



float generate_random_float() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0, 1.0);
    return dis(gen);
}


// Generate a random int [a,b)
int generate_random_int(int a, int b) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(a, b - 1);

    return dis(gen);
}

struct SCENARIO_STRUCTURE {
    std::string str;
    std::vector<std::vector<float>> q_table;

    SCENARIO_STRUCTURE(std::string s)
        :str{ s }
    {}
};



// Extract the data to file for matlab
void Extract_to_File(const std::vector<int>& data, std::string filename, std::string foldername) {
    char currentPath[FILENAME_MAX];
    if (!GetCurrentDir(currentPath, sizeof(currentPath))) {
        std::cerr << "Error getting current directory" << std::endl;
        return;
    }
    currentPath[sizeof(currentPath) - 1] = '\0';  // Null terminate

    std::cout << "Current working directory: " << currentPath << std::endl;



    // output file
    std::string folder = "../data/" + foldername;
    std::string fn = folder + "/" + filename + ".txt";


    // Set Regular Expression pattern
    std::regex pattern(R"([\\:\*\?"<>\|\s])");
    // judge if filename is leagal
    if (std::regex_search(fn, pattern)) {
        std::cout << "Invalid Filename, filename cannot contain the following character:   \\ : * ? \" < > | and space(whether space or tab)" << std::endl;
        return;
    }
    
    // Create the target folder if it doesn't exist
    #if defined(_WIN32)
        _mkdir(folder.c_str());
    #else 
        mkdir(folder.c_str(), 0777);
    #endif

    // open file output stream
    std::ofstream outfile(fn);

    // check if file has successfully opened
    if (!outfile) {
        std::cerr << "There is sth wrong with file output stream" << std::endl;
    }


    for (auto& s : data) {
        outfile << s << " ";
    }

    outfile.close();

}





struct node {
    int begin;
    int end;
    int job_id;
    node() {};
    node(int _b, int pt, int _id)
        : begin{ _b }, job_id(_id) {
        end = begin + pt;
    }

    void update(int push) {
        begin += push;
        end += push;
    }
};






class DataLogger {
    std::string folder_path_;        // 文件夹路径
    std::string filename_;           // 完整文件路径
    std::ostringstream data_stream_; // 用于暂存数据的字符串流
    bool is_valid_ = true;           // 用于标识文件名是否合法

    void MakeDirIfNotExists(const std::string& folder) {
        #if defined(_WIN32)
                _mkdir(folder.c_str());
        #else 
                mkdir(folder.c_str(), 0777);
        #endif
    }
public:
    DataLogger() : is_valid_(false) {}
    DataLogger(const std::string& filename, const std::string& foldername = "Log") {
        folder_path_ = "../data/" + foldername;
        filename_ = folder_path_ + "/" + filename + ".txt";

        // 检查文件名合法性
        std::regex pattern(R"([\\:\*\?"<>\|\s])");
        if (std::regex_search(filename_, pattern)) {
            std::cerr << "Invalid filename. It cannot contain the following characters: \\ : * ? \" < > | and spaces." << std::endl;
            is_valid_ = false;
            return;
        }

        // 创建目标文件夹
        MakeDirIfNotExists(folder_path_);
    }

    DataLogger& operator<<(const std::string& data) {
        data_stream_ << data;
        return *this;
    }

    DataLogger& operator<<(const std::vector<int>& data) {
        for (auto i : data) {
            data_stream_ << i << " ";
        }

        data_stream_ << "\n";

        return *this;
    }

    DataLogger& operator<<(int data) {
        data_stream_ << data;

        return *this;
    }

    DataLogger& operator<<(float data) {
        data_stream_ << data;

        return *this;
    }
    DataLogger& operator<<(long long data) {
        data_stream_ << data;

        return *this;
    }

    //overload std::endl
    DataLogger& operator<<(std::ostream& (*manip)(std::ostream&)) {
        if (is_valid_ && manip == static_cast<std::ostream & (*)(std::ostream&)>(std::endl)) {
            data_stream_ << "\n";
        }
        return *this;
    }


    ~DataLogger() {
        if (!is_valid_) 
            return; 

        std::ofstream file(filename_);
        if (file.is_open()) {
            file << data_stream_.str(); 
            file.close();
            std::cout << "Data saved to " << filename_ << std::endl;
        }
        else {
            std::cerr << "Error: Cannot open file " << filename_ << std::endl;
        }
    }
    

};