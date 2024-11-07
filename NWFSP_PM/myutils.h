#pragma once
#include <iostream>
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
#else
#include <unistd.h>  // For POSIX getcwd
#include <sys/stat.h>  // For POSIX mkdir
#define GetCurrentDir getcwd
#endif

#define PRINT_CURRENT_LOCATION() \
    std::cout << "File: " << __FILE__ << ", Line: " << __LINE__ << std::endl;

// µ¥Î»ºÁÃë
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
void Extract_to_File(const std::vector<int>& data, std::string filename) {
    char currentPath[FILENAME_MAX];
    if (!GetCurrentDir(currentPath, sizeof(currentPath))) {
        std::cerr << "Error getting current directory" << std::endl;
        return;
    }
    currentPath[sizeof(currentPath) - 1] = '\0';  // Null terminate

    std::cout << "Current working directory: " << currentPath << std::endl;



    // output file
    std::string folder = "../data";
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


