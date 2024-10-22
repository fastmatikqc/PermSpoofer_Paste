#include <iostream>
#include <fstream>
#include <Windows.h>
#include <filesystem>
#include <string>
#include <thread>
#include <curl.h>

#include "zipconf.h"
#include "zip.h"

#include "auth/auth.hpp"
#include "color.hpp"
#include "auth/utils.hpp"
#include "skCrypt.h"

using namespace KeyAuth;

auto name = skCrypt("");
auto ownerid = skCrypt("");
auto secret = skCrypt("");
auto version = skCrypt("1.0");
auto url = skCrypt("https://keyauth.win/api/1.2/");

api KeyAuthApp(name.decrypt(), ownerid.decrypt(), secret.decrypt(), version.decrypt(), url.decrypt());

std::string tm_to_readable_time(tm ctx);
static std::time_t string_to_timet(std::string timestamp);
static std::tm timet_to_tm(time_t timestamp);
const std::string compilation_date = (std::string)skCrypt(__DATE__);
const std::string compilation_time = (std::string)skCrypt(__TIME__);

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* ofs = static_cast<std::ofstream*>(userp);
    ofs->write(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

bool downloadFile(const std::string& url, const std::string& outputFile) {
    CURL* curl;
    CURLcode res;
    std::ofstream ofs(outputFile, std::ios::binary);

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ofs);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        ofs.close();
        return (res == CURLE_OK);
    }
    return false;
}

bool extractZIP(const std::string& zipPath, const std::string& outputPath) {
    int err = 0;
    zip_t* zip = zip_open(zipPath.c_str(), 0, &err);
    if (!zip) {
        std::cerr << "Failed to open ZIP file: " << zipPath << std::endl;
        return false;
    }

    zip_int64_t numEntries = zip_get_num_entries(zip, 0);
    for (zip_int64_t i = 0; i < numEntries; i++) {
        const char* name = zip_get_name(zip, i, 0);
        if (name) {
            std::string outputFilePath = outputPath + "\\" + name;
            // Ensure the directory exists
            std::filesystem::create_directories(outputPath + "\\" + std::filesystem::path(name).parent_path().string());
            zip_file_t* zf = zip_fopen(zip, name, 0);
            if (zf) {
                std::ofstream ofs(outputFilePath, std::ios::binary);
                char buffer[8192];
                zip_int64_t bytesRead;
                while ((bytesRead = zip_fread(zf, buffer, sizeof(buffer))) > 0) {
                    ofs.write(buffer, bytesRead);
                }
                zip_fclose(zf);
                ofs.close();
            }
            else {
                std::cerr << "Failed to open file in ZIP: " << name << std::endl;
            }
        }
    }

    zip_close(zip);
    return true;
}

void runBatchFile(const std::string& batchFilePath) {
    // Full command to execute the batch file in cmd
    std::string command = "cmd.exe /c \"" + batchFilePath + "\"";

    // Structures for process creation
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    // Initialize the STARTUPINFO structure
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Create the process to run the batch file
    if (!CreateProcessA(
        nullptr,                    // No module name (use command line)
        (LPSTR)command.c_str(),     // Command line
        nullptr,                    // Process handle not inheritable
        nullptr,                    // Thread handle not inheritable
        FALSE,                      // Set handle inheritance to FALSE
        0,                          // No creation flags
        nullptr,                    // Use parent's environment block
        nullptr,                    // Use parent's starting directory
        &si,                        // Pointer to STARTUPINFO structure
        &pi                         // Pointer to PROCESS_INFORMATION structure
    )) {
        std::cerr << "CreateProcess failed. Error: " << GetLastError() << std::endl;
    }
    else {
        // Wait for the batch file execution to finish (optional)
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process and thread handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

bool runExeFile(const std::string& exeFilePath) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    // Initialize the STARTUPINFO structure
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Create the process for the .exe file
    if (!CreateProcessA(
        exeFilePath.c_str(),  // Application name
        NULL,                 // Command line arguments
        NULL,                 // Process handle not inheritable
        NULL,                 // Thread handle not inheritable
        FALSE,                // Set handle inheritance to FALSE
        0,                    // No creation flags
        NULL,                 // Use parent's environment block
        NULL,                 // Use parent's starting directory
        &si,                  // Pointer to STARTUPINFO structure
        &pi                   // Pointer to PROCESS_INFORMATION structure
    )) {
        std::cerr << "CreateProcess failed. Error: " << GetLastError() << std::endl;
        return false;
    }

    // Wait until the process has finished executing
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return true;
}

void generateRandomByte(std::string& randomData) {
    int byte = rand() % 256;
    char hexValue[3];
    sprintf_s(hexValue, "%02X", byte);
    randomData += hexValue;
}

void mac_changer() {

    // Execute commands without administrative privileges
    system("netsh advfirewall reset");
    system("netsh winsock reset");
    system("netsh winhttp reset autoproxy");
    system("netsh winhttp reset proxy");
    system("netsh winhttp reset tracing");
    system("netsh interface ipv4 reset");
    system("netsh interface portproxy reset");
    system("netsh interface httpstunnel reset");
    system("netsh interface tcp reset");
    system("netsh interface teredo set state disabled");
    system("netsh interface ipv6 6to4 set state state=disabled undoonstop=disabled");
    system("netsh interface ipv6 isatap set state state=disabled");
    system("arp -d");

    std::string downloadUrl = "https://files.catbox.moe/mmhtxo.zip"; // download url
    std::string tempDir = std::filesystem::temp_directory_path().string() + "\\5485";
    std::string zipFilePath = tempDir + "\\mmhtxo.zip"; // insert archive name here
    std::string macchanger = tempDir + "\\Mac_changer.exe"; // Spoofer

    // Check if the tempDir exists and delete it if it does
    if (std::filesystem::exists(tempDir)) {
        //std::cout << "[+] Deleting existing temp directory.\n";
        std::filesystem::remove_all(tempDir);
    }
    // Create temp dir
    std::filesystem::create_directory(tempDir);

    // Download file if it does not exist
    if (!std::filesystem::exists(zipFilePath)) {
        if (downloadFile(downloadUrl, zipFilePath)) {
        }

        // Unpack archive into tempDir
        if (extractZIP(zipFilePath, tempDir)) {

            // Check if .bat exists
            if (std::filesystem::exists(macchanger)) {
                runExeFile(macchanger);

                // Clean temp files
                std::filesystem::remove_all(tempDir);
            }
        }
    }
    system("cls");
}

void registry_clean() {
    system(skCrypt("rmdir /s /q C:\\Windows\\temp >nul").decrypt());
    system(skCrypt("rmdir /s /q C:\\Users\\%username%\\AppData\\Local\\Temp >nul").decrypt());
    system(skCrypt("rmdir /s /q C:\\Windows\\Prefetch >nul").decrypt());
    system(skCrypt("reg add HKLM\\SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ComputerName /v ComputerName /t REG_SZ /d %random% /f >nul").decrypt());
    system(skCrypt("reg add HKLM\\SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName /v ComputerName /t REG_SZ /d %random% /f >nul").decrypt());
    system(skCrypt("reg add HKLM\\SYSTEM\\CurrentControlSet\\Control\\SystemInformation /v ComputerHardwareId /t REG_SZ /d {%random%%random%-%random%%random%-%random%%random%} /f >nul").decrypt());
    system(skCrypt("reg add HKLM\\SYSTEM\\CurrentControlSet\\Control\\SystemInformation /v ComputerHardwareIds /t REG_SZ /d %random%%random%-%random%%random%-%random%%random% /f >nul").decrypt());
    system(skCrypt("reg add HKLM\\SYSTEM\\CurrentControlSet\\Control\\IDConfigDB\\Hardware\" \"Profiles\\0001 /v HwProfileGuid /t REG_SZ /d {%random%%random%-%random%%random%-%random%%random%} /f >nul").decrypt());
    system(skCrypt("reg add HKLM\\SOFTWARE\\Microsoft\\SQMClient /v MachineId /t REG_SZ /d {%random%%random%-%random%%random%-%random%%random%} /f >nul").decrypt());
    system(skCrypt("reg add HKLM\\SOFTWARE\\Microsoft\\Cryptography /v MachineGuid /t REG_SZ /d %random%%random%-%random%%random%-%random%%random% /f >nul").decrypt());
    system(skCrypt("reg add HKLM\\SOFTWARE\\Microsoft\\Windows\" \"NT\\CurrentVersion /v InstallTime /t REG_QWORD /d %random%%random%-%random%%random%-%random%%random% /f >nul").decrypt());
    system(skCrypt("reg add HKLM\\SOFTWARE\\Microsoft\\Windows\" \"NT\\CurrentVersion /v InstallDate /t REG_QWORD /d %random%%random%-%random%%random%-%random%%random% /f >nul").decrypt());
    system(skCrypt("reg add HKLM\\SOFTWARE\\Microsoft\\Windows\" \"NT\\CurrentVersion /v ProductId /t REG_SZ /d %random%%random%-%random%%random%-%random%%random% /f >nul >nul").decrypt());
    system(skCrypt("reg add HKLM\\SOFTWARE\\Microsoft\\Windows\" \"NT\\CurrentVersion /v BuildGUID /t REG_SZ /d %random%%random%-%random%%random%-%random%%random% /f >nul >nul").decrypt());
    system(skCrypt("reg add HKLM\\SOFTWARE\\Microsoft\\Cryptography /v GUID /t REG_SZ /d %random%%random%-%random%%random%-%random%%random% /f >nul").decrypt());
    system("cls");
}

int loadspoofer() {
    std::string downloadUrls = "https://files.catbox.moe/0gtqws.zip"; // download url
    std::string extractDir = "C:\\Windows\\Logss"; // Target extraction directory
    std::string zipFilePathz = extractDir + "\\0gtqws.zip"; // insert archive name here
    std::string extractedBatchg = extractDir + "\\load_driver.bat"; // Spoofer

    // Create temp dir
    if (!std::filesystem::create_directory(extractDir) && !std::filesystem::exists(extractDir)) {
        std::cerr << "Failed to create extraction directory." << std::endl;
        return -1; // Error code for directory creation failure
    }

    // Download file if it does not exist
    if (!std::filesystem::exists(zipFilePathz)) {
        if (!downloadFile(downloadUrls, zipFilePathz)) {
            std::cerr << "Failed to download the file." << std::endl;
            return -2; // Error code for download failure
        }
    }

    // Unpack archive into tempDir
    if (!extractZIP(zipFilePathz, extractDir)) {
        std::cerr << "Failed to extract the ZIP file." << std::endl;
        return -3; // Error code for extraction failure
    }

    // Check if .bat exists
    if (std::filesystem::exists(extractedBatchg)) {
        runBatchFile(extractedBatchg); // Ensure this function handles errors too

        // Clean temp files
        std::filesystem::remove_all(extractDir);
        return 0; // Success
    }
    else {
        std::cerr << "Batch file not found." << std::endl;
        return -4; // Error code for batch file not found
    }
    system("cls");
}
void executeCommand(const std::string& command, std::stringstream& output)
{
    FILE* pipe = _popen(command.c_str(), "r");
    if (pipe)
    {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            output << buffer;
        }
        _pclose(pipe);
    }
}

void displaySectionTitle(const std::string& title, bool found)
{
    const int width = 50;
    const char lineChar = '-';
    std::string sectionLine(width, lineChar);
    std::string centeredTitle = " " + title + " ";
    const int titlePadding = (width - centeredTitle.length()) / 2;

    std::cout << sectionLine << "\n";
    if (found)
        std::cout << "| \x1b[32m" << std::string(titlePadding, ' ') << centeredTitle << std::string(titlePadding, ' ') << "\x1b[0m|\n";
    else
        std::cout << "| \x1b[31m" << std::string(titlePadding, ' ') << centeredTitle << std::string(titlePadding, ' ') << "\x1b[0m|\n";
    std::cout << sectionLine << "\n";
}

void check()
{
    system("cls");

    // Check Disk Drive C:
    system("echo Disk Drive");
    Sleep(200);
    system("wmic diskdrive get model,serialnumber");
    Sleep(200);
    system("echo BaseBoard:");
    Sleep(200);
    system("wmic baseboard get serialnumber");
    Sleep(200);
    system("echo System UUID:");
    Sleep(200);
    system("wmic path win32_computersystemproduct get uuid");
    Sleep(200);
    system("echo BIOS:");
    Sleep(200);
    system("wmic bios get serialnumber");
    Sleep(200);
    system("echo CPU:");
    Sleep(200);
    system("wmic cpu get serialnumber");
    Sleep(200);
    system("echo Mac Address:");
    Sleep(200);
    system("getmac");
    Sleep(200);
    std::cout << ("  ") << '\n';
    Sleep(200);
    system("echo -----------------------------------------------");
    Sleep(200);
    system("echo Returning In 5 Seconds");
    Sleep(200);
    system("echo -----------------------------------------------");
    Sleep(5000);
    system("cls");
}

void showLoadingBar(int duration) {
    const int barWidth = 50; // Width of the loading bar
    std::cout << dye::grey("[") << dye::red("+") << dye::grey("] ") << "Loading Driver...\n" << std::endl;

    for (int i = 0; i <= barWidth; ++i) {
        // Calculate progress percentage
        float progress = static_cast<float>(i) / barWidth;

        // Create the loading bar string
        std::string bar = "["; // Start of the bar
        int pos = barWidth * progress; // Current position in the bar
        for (int j = 0; j < barWidth; ++j) {
            if (j < pos) {
                bar += "="; // Fill character
            }
            else {
                bar += " "; // Empty character
            }
        }
        bar += "] " + std::to_string(static_cast<int>(progress * 100)) + "%"; // End of the bar and percentage

        // Clear the current line and print the new loading bar
        std::cout << "\r" << bar << std::flush;

        // Sleep to simulate loading time
        std::this_thread::sleep_for(std::chrono::milliseconds(duration / barWidth));
    }

    std::cout << std::endl; // Move to the next line after loading is complete
}

void centerConsoleWindow() {
    // Get the console handle
    HWND consoleWindow = GetConsoleWindow();

    // Get the dimensions of the console window
    RECT consoleRect;
    GetWindowRect(consoleWindow, &consoleRect);
    int consoleWidth = consoleRect.right - consoleRect.left;
    int consoleHeight = consoleRect.bottom - consoleRect.top;

    // Get the dimensions of the screen
    RECT screenRect;
    GetWindowRect(GetDesktopWindow(), &screenRect);
    int screenWidth = screenRect.right - screenRect.left;
    int screenHeight = screenRect.bottom - screenRect.top;

    // Calculate the new position for the console window to center it
    int x = (screenWidth / 2) - (consoleWidth / 2);
    int y = (screenHeight / 2) - (consoleHeight / 2);

    // Set the new position of the console window
    SetWindowPos(consoleWindow, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

int main() {

    // Center the console window
    centerConsoleWindow();

    SetConsoleTitleA("KRAFTCHEESE - SPOOFER");

    // Initialize KeyAuthApp and handle license
    KeyAuthApp.init();
    // Assuming these strings are defined in KeyAuthApp
    std::string name, ownerid, secret, version, url;
    name.clear(); ownerid.clear(); secret.clear(); version.clear(); url.clear();

    if (!KeyAuthApp.data.success) {
        Sleep(1500);
        return 1;
    }

    std::string key;
    std::cout << "\n License Key : ";
    std::cin >> key;
    KeyAuthApp.license(key);

    if (!KeyAuthApp.data.success) {
        Sleep(1500);
        return 1;
    }

    system("cls");

    for (int i = 0; i < KeyAuthApp.data.subscriptions.size(); i++) {
        auto sub = KeyAuthApp.data.subscriptions.at(i);
        std::cout << ("Key Expiry : ") << tm_to_readable_time(timet_to_tm(string_to_timet(sub.expiry)));
    }

    Sleep(2000);
    system("cls");

menu:
    int opt;

    std::cout << dye::grey("[") << dye::red("F1") << dye::grey("] ") << "- Loading Kraftspoofer " << std::endl;
    std::cout << dye::grey("[") << dye::red("F2") << dye::grey("] ") << "- Cleaning Registry " << std::endl;
    std::cout << dye::grey("[") << dye::red("F3") << dye::grey("] ") << "- Mac Address Changer " << std::endl;
    std::cout << dye::grey("[") << dye::red("F4") << dye::grey("] ") << "- Check My HWID " << std::endl;

    while (true) {
        if (GetAsyncKeyState(VK_F1)) {
            system(skCrypt("cls").decrypt());
            showLoadingBar(5000); // Call the loading bar with a total duration of 2000 ms

            loadspoofer();
            goto menu;
        }

        if (GetAsyncKeyState(VK_F2)) {
            system(skCrypt("cls").decrypt());
            registry_clean();
            goto menu;
        }

        if (GetAsyncKeyState(VK_F3)) {
            system(skCrypt("cls").decrypt());
            mac_changer();
            goto menu;
        }

        if (GetAsyncKeyState(VK_F4)) {
            system(skCrypt("cls").decrypt());
            check();
            goto menu;
        }

        Sleep(100);
    }
}

std::string tm_to_readable_time(tm ctx) {
    char buffer[80];

    strftime(buffer, sizeof(buffer), "%a %m/%d/%y %H:%M:%S %Z", &ctx);

    return std::string(buffer);
}

static std::time_t string_to_timet(std::string timestamp) {
    auto cv = strtol(timestamp.c_str(), NULL, 10); // long

    return (time_t)cv;
}

static std::tm timet_to_tm(time_t timestamp) {
    std::tm context;

    localtime_s(&context, &timestamp);

    return context;
}