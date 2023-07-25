#include<iostream>
#include<map>
#include<string>
#include<vector>
#include<json.hpp>
using namespace std;
using json = nlohmann::json;

int main()
{

    json js;
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    vec.push_back(4);
    vec.push_back(5);

    map<int, string> m;
    m.insert(make_pair<int,string>(1,"泰山")); 
    m.insert(make_pair<int,string>(2,"黄山")); 
    m.insert(make_pair<int,string>(3,"华山")); 
    js["list"] = vec;
    js["path"] = m; 
    string sendmsg = js.dump();
    
    cout << js << std::endl;
    cout << sendmsg << std::endl;

    string recvmsg = js.dump();    
    json js2 = json::parse(recvmsg);
    vector<int> vec2 = js2["list"];
    map<int, string> m2 = js2["path"];
    for(auto& elem : vec2){
        std::cout << elem << " ";
    }
    std::cout << std::endl;
    for(const auto& ptr : m2){
        std::cout << "first: " << ptr.first <<" second: " << ptr.second <<"  ";
    }
    std::cout << std::endl;

    return 0;
}