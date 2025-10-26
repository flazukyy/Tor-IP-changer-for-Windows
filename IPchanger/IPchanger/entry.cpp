#include <iostream>
#include <cstdlib>
#include <string>
#include <thread>
#include <chrono>
#include <regex>
#include <windows.h>

std::string execCommand(const std::string& cmd) {
    std::string result;
    char buffer[128];
    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe) return "ERROR";
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    _pclose(pipe);
    return result;
}

bool isTorRunning() {
    std::string output = execCommand("tasklist | findstr /I tor.exe");
    return !output.empty();
}

void startTor() {
    std::cout << "Starting Tor service...\n";

    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string exePath(buffer);
    std::string dir = exePath.substr(0, exePath.find_last_of("\\/"));
    std::string torPath = dir + "\\tor.exe";

    SHELLEXECUTEINFOA info = { 0 };
    info.cbSize = sizeof(info);
    info.fMask = SEE_MASK_NOCLOSEPROCESS;
    info.hwnd = NULL;
    info.lpVerb = "open";
    info.lpFile = torPath.c_str();
    info.lpParameters = "";
    info.lpDirectory = dir.c_str();
    info.nShow = SW_HIDE;  

    if (ShellExecuteExA(&info)) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    else {
        std::cerr << "Failed to start tor.exe\n";
    }
}

std::string getIp() {
    std::string output = execCommand("curl -s -x socks5h://127.0.0.1:9050 https://checkip.amazonaws.com");
    std::regex ipRegex("(\\d{1,3}(?:\\.\\d{1,3}){3})");
    std::smatch match;
    if (std::regex_search(output, match, ipRegex))
        return match[1];
    return "Unknown";
}

void changeIp() {
    std::cout << "Reloading Tor (sending NEWNYM signal)...\n";
    system("taskkill /IM tor.exe /F >nul 2>&1");
    startTor();
    std::cout << "\033[34mNew IP address: " << getIp() << "\033[0m\n";
}

int main() {
    system("cls");
    std::cout << R"(

     ________                  __              _          ________             __                               
   / ____/ /___ _____  __  __/ /____  ____  _( )_____   /  _/ __ \      _____/ /_  ____ _____  ____ ____  _____
  / /_  / / __ `/_  / / / / / //_/ / / / / / /// ___/   / // /_/ /_____/ ___/ __ \/ __ `/ __ \/ __ `/ _ \/ ___/
 / __/ / / /_/ / / /_/ /_/ / ,< / /_/ / /_/ / (__  )  _/ // ____/_____/ /__/ / / / /_/ / / / / /_/ /  __/ /    
/_/   /_/\__,_/ /___/\__,_/_/|_|\__, /\__, / /____/  /___/_/          \___/_/ /_/\__,_/_/ /_/\__, /\___/_/     
                               /____//____/                                                 /____/             
                                                                                                       
)";
    if (!isTorRunning()) {
        startTor();
    }

    int interval = 0, times = 0;
    std::cout << "\033[34mEnter time interval between changes in seconds (0 for random 10-20s): \033[0m";
    std::cin >> interval;

    std::cout << "Starting infinite IP changes...\n";
    while (true) {
        changeIp();
        int sleepTime = (interval == 0) ? (10 + rand() % 11) : interval;
        std::this_thread::sleep_for(std::chrono::seconds(sleepTime));
        }
    return 0;
}
