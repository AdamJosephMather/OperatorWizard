#pragma comment(lib, "Shell32.lib")

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <string>
#include <shlobj.h>
#include <vector>

template <typename F>
struct ScopeGuard {
	F func;
	~ScopeGuard() { func(); }
};

template <typename F>
ScopeGuard(F) -> ScopeGuard<F>;

namespace fs = std::filesystem;

using Data = std::vector<std::pair<std::string, std::string>>;

fs::path filePath;

std::vector<std::string> split(const std::string& s, char delim) {
	std::vector<std::string> tokens;
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		tokens.push_back(item);
	}
	return tokens;
}

fs::path GetLocalAppDataPath() {
	char path[MAX_PATH];
	// CSIDL_LOCAL_APPDATA gets the "AppData\Local" directory for the current user
	if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path))) {
		return fs::path(path);
	}
	return "";
}

Data getData() {
	Data out = {};
	
	std::ifstream inFile(filePath);
	if (inFile.is_open()) {
		std::string line;
		while (std::getline(inFile, line)) {
			auto parts = split(line, '|'); // the '|' is illegal in paths
			if (parts.size() != 2) {
				continue;
			}
			out.push_back({parts[0], parts[1]});
		}
		inFile.close();
	} else {
		std::cerr << "Failed to open file for reading.\n";
	}
	
	return out;
}

struct Color {
	int r;
	int g;
	int b;
};

Color errorColor        { 255, 100, 100 };
Color brightErrorColor  { 255, 150, 150 };
Color greyColor         { 100, 100, 100 };
Color blueColor         {  89, 153, 255 };
Color yellowColor       { 255, 164,  89 };
Color greenColor        { 100, 255, 100 };

void setColor(Color c) {
	std::cout << "\033[38;2;" << c.r << ";" << c.g << ";" << c.b << "m";
}

void reset() {
	std::cout << "\x1b[0m"; // reset to terminal defaults
}

bool writeData(Data dta) {
	std::ofstream outFile(filePath, std::ios::out);
	if (outFile.is_open()) {
		for (auto i  : dta) {
			outFile << i.first << "|" << i.second << "\n";
		}
		outFile.close();
	} else {
		setColor(errorColor);
		std::cout << "Couldn't write data.";
		reset();
		return false;
	}
	return true;
}

std::string input() { // returns the lowered input
	std::string out;
	std::getline(std::cin, out);
	std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return std::tolower(c); });
	return out;
}

int main() {
	ScopeGuard onExit = {reset};
	
	fs::path localAppData = GetLocalAppDataPath();
	if (localAppData.empty()) {
		std::cerr << "Failed to locate AppData\\Local directory.\n";
		return 1;
	}
	
	fs::path dirPath = localAppData / "OperatorWizard";
	filePath = dirPath / "storedPaths.txt";
	
	// ensure it's directory exists
	try {
		if (!fs::exists(dirPath)) {
			fs::create_directories(dirPath);
			std::cout << "Created directory: " << dirPath << "\n";
		}
	} catch (const fs::filesystem_error& e) {
		std::cerr << "Filesystem error: " << e.what() << "\n";
		return 1;
	}
	
	try {
		if (!fs::exists(filePath)) {
			std::cout << "Initializing database...\n";
			bool works = writeData({});
			if (!works) {
				return 1;
			}
		}
	} catch (const fs::filesystem_error& e) {
		std::cerr << "Filesystem error: " << e.what() << "\n";
		return 1;
	}
	
	Data data = getData();
	
	setColor(greyColor);
	std::cout << "Options: 'alias', 'manage'\n";
	setColor(blueColor);
	std::cout << "Operator, how may I direct your call? ";
	reset();
	
	std::string number = input(); // number as in the number the operator would dial... Very funny I know
	
	
	if (number == "alias") {
		setColor(greyColor);
		std::cout << "Leave blank to exit\n";
		setColor(yellowColor);
		std::cout << "Alias for current directory? ";
		reset();
		
		std::string alias = input();
		
		if (alias == "") {
			setColor(errorColor);
			std::cout << "Alias blank, exiting.\n";
			return 0;
		}else if (alias == "manage" || alias == "alias") {
			setColor(errorColor);
			std::cout << "Alias conflicts with keywords.\n";
			return 0;
		}
		
		for (auto d : data) {
			if (d.first == alias) {
				setColor(errorColor);
				std::cout << "Alias already in use by: ";
				setColor(brightErrorColor);
				std::cout << d.second << "\n";
				setColor(errorColor);
				std::cout << "Exiting.\n";
				return 0;
			}
		}
		
		std::string cwd;
		try {
			fs::path cwdpth = fs::current_path();
			cwd = cwdpth.string();
		} catch (const fs::filesystem_error& e) {
			setColor(errorColor);
			std::cerr << "Error: " << e.what() << "\nCouldn't get CWD\n";
			return 0;
		}
		
		data.push_back({alias, cwd});
		writeData(data);
		
		setColor(greenColor);
		std::cout << "Alias accepted, added alias \"" << alias << "\" -> \"" << cwd << "\"\n";
		return 0;
	}else if (number == "manage") {
		while (true) {
			setColor(greyColor);
			std::cout << "\nOptions: 'list', 'delete', 'quit'\n";
			setColor(yellowColor);
			std::cout << "Action: ";
			reset();
			std::string act = input();
			
			if (act == "list" || act == "l") {
				setColor(yellowColor);
				std::cout << "\nAlias\n--Path\n\n";
				
				for (auto d : data) {
					setColor(blueColor);
					std::cout << d.first;
					setColor(greyColor);
					std::cout << "\n->";
					setColor(yellowColor);
					std::cout << d.second << "\n";
				}
			} else if (act == "quit" || act == "q" || act == "exit") {
				setColor(greenColor);
				std::cout << "\nAll changes saved.\n";
				return 0;
			}else if (act == "delete" || act == "del") {
				setColor(blueColor);
				std::cout << "\nAlias to delete? ";
				reset();
				std::string al = input();
				
				bool foundIt = false;
				for (int i = 0; i < data.size(); i++) {
					if (data[i].first == al) {
						data.erase(data.begin()+i);
						foundIt = true;
						break;
					}
				}
				
				if (foundIt) {
					if (writeData(data)) {
						setColor(greenColor);
						std::cout << "Successfully deleted alias.\n";
						reset();
					}else{
						return 1;
					}
				}else {
					setColor(errorColor);
					std::cout << "Could not find alias.\n";
					reset();
				}
			}else{
				setColor(errorColor);
				std::cout << "\nUnknown action.\n";
				reset();
			}
		}
	}else {
		std::string new_path = "";
		for (auto a : data) {
			if (a.first == number) {
				new_path = a.second;
				break;
			}
		}
		
		if (new_path == "") {
			setColor(errorColor);
			std::cout << "Couldn't find alias.\n";
			return 0;
		}
		
		char* temp_env = std::getenv("TEMP");
		if (temp_env != nullptr) {
			std::string temp_file_path = std::string(temp_env) + "\\temp_cd_path.txt";
			
			std::ofstream cd_file(temp_file_path);
			if (cd_file.is_open()) {
				cd_file << new_path;
				cd_file.close();
			}
		} else {
			std::cerr << "Could not find TEMP environment variable.\n";
		}
	}
	
	return 0;
}