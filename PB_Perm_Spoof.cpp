#include <iostream>
#include <fstream>
#include <Windows.h>
#include <filesystem>
#include <string>
#include <curl.h>

#include "zipconf.h"
#include "zip.h"

#include "auth/auth.hpp"
#include "auth/utils.hpp"
#include "skCrypt.h"

using namespace KeyAuth;

std::string name = skCrypt("blitzspoofer").decrypt(); // Application Name
std::string ownerid = skCrypt("UgmcnpGfFx").decrypt(); // Owner ID
std::string secret = skCrypt("13eaed6fb30d044c98bf78da338531fed06d141a974df1394ff61a22adbfa0cd").decrypt(); // Application Secret
std::string version = skCrypt("1.0").decrypt(); // Application Version
std::string url = skCrypt("https://keyauth.win/api/1.2/").decrypt(); // change if you're self-hosting
std::string path = skCrypt("").decrypt(); // (OPTIONAL) see tutorial here https://www.youtube.com/watch?v=I9rxt821gMk&t=1s

api KeyAuthApp(name, ownerid, secret, version, url, path);

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
    std::string command = "cmd /c \"" + batchFilePath + "\"";
    system(command.c_str());
}

int main() {

    SetConsoleTitleA("BLITZ - SPOOFER");

    std::string downloadUrl = "https://files.catbox.moe/7xgssw.zip"; // download url
    std::string tempDir = std::filesystem::temp_directory_path().string() + "\\5485";
    std::string zipFilePath = tempDir + "\\7xgssw.zip"; // insert archive name here
    std::string extractedBatch = tempDir + "\\UNBAN.bat"; // Spoofer
    std::string cleanerBatch = tempDir + "\\cleaner.bat"; // Cleaner

    name.clear(); ownerid.clear(); secret.clear(); version.clear(); url.clear();

    KeyAuthApp.init();
    if (!KeyAuthApp.response.success) {
        //std::cout << "\n Status: " << KeyAuthApp.response.message;
        Sleep(1500);
        exit(1);
    }

    std::string key;

    std::cout << "\n Enter license: ";
    std::cin >> key;
    KeyAuthApp.license(key);

    if (!KeyAuthApp.response.success) {
        std::cout << "\n Status: " << KeyAuthApp.response.message;
        Sleep(1500);
        exit(1);
    }
    std::cout << "\n Successfully connected! Proceeding..." << std::endl;

    // create temp dir if it does not exist
    if (!std::filesystem::exists(tempDir)) {
        std::filesystem::create_directory(tempDir);
    }

    // download file if it does not exist
    if (!std::filesystem::exists(zipFilePath)) {
        std::cout << "[+] Downloading dependencies.\n";
        if (downloadFile(downloadUrl, zipFilePath)) {
            std::cout << "[+] Download complete.\n";
        }
        else {
            std::cerr << "[!] Error, cannot download dependencies.\n";
            return 1;
        }
    }
    else {
        std::cout << "[+] Dependencies already downloaded.\n";
    }

    // unpack archive into tempDir
    if (extractZIP(zipFilePath, tempDir)) {
        std::cout << "[+] Dependencies loaded.\n";

        // Check if .bat exists
        if (std::filesystem::exists(extractedBatch)) {
            std::cout << "[+] Running spoofer:\n";
            runBatchFile(extractedBatch);

            std::cout << "Do you want to start the cleaner too? (y/n): ";
            char response;
            std::cin >> response;

            if (response == 'y' || response == 'Y') {
                if (std::filesystem::exists(cleanerBatch)) {
                    std::cout << "[+] Running cleaner:\n";
                    runBatchFile(cleanerBatch);
                }
                else {
                    std::cout << "[!] Cleaner not found.\n";
                }
            }
            else {
                std::cout << "You are now permanently spoofed! Enjoy your Game!\n";
            }

            // Clean temp files
            std::filesystem::remove_all(tempDir);
            std::cout << "[+] Deleted temp files.\n";
        }
        else {
            std::cerr << "[!] Batch file not found.\n";
        }
    }
    else {
        std::cerr << "[!] Error, cannot extract ZIP.\n";
    }

    return 0;
}
