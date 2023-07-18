#include <shlobj.h>
#include <filesystem>
#include <fstream>
#include <set>

auto get_documents_folder_path() -> std::wstring {
    PWSTR path;
    SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT, NULL, &path);
    std::wstring strpath(path);
    CoTaskMemFree(path);
    return strpath;
}

auto get_whitelist(const std::wstring& path) -> std::vector<std::string> {
    std::vector<std::string> whitelist;
    whitelist.reserve(128);

    std::ifstream config_file(path);

    if (config_file.is_open()) {
        std::string line;
        while (std::getline(config_file, line)) {
            whitelist.push_back(line);
        }
        config_file.close();
    }

    return whitelist;
}

auto get_file_list(const std::filesystem::path& path) -> std::set<std::filesystem::path> {
    std::set<std::filesystem::path> file_list;

    for (auto const& dir_entry : std::filesystem::directory_iterator{path}) {
        file_list.insert(dir_entry.path());
    }

    return file_list;
}

auto delete_files(std::vector<std::string> whitelist, std::set<std::filesystem::path> files) -> bool {
    uint32_t deleted_files_count = 0;
    bool successful = true;
    for (auto & file : files) {
        bool delete_file = true;
        for (auto& ending : whitelist) {
            if (file.stem().string().ends_with(ending) and (file.stem().extension() != ".wav")) {
                delete_file = false;
                break;
            }
        }
        if (delete_file) {
            successful = std::filesystem::remove(file);
            if (not successful) {
                const std::wstring message = L"Failed to delete file : " + file.filename().wstring() + L".";
                MessageBox(nullptr, message.c_str(), TEXT("Message"), MB_OK);
            }
            ++deleted_files_count;
        }
    }
    if (deleted_files_count == 0) successful = false;
    return successful;
}





auto main() -> int {
    const auto documents_path = get_documents_folder_path();

    if (documents_path.empty()) {
        MessageBox(nullptr, TEXT("Could not find My Documents folder."), TEXT("Message"), MB_OK);
    }

    const auto config_file_path = documents_path + L"\\.flsc";
    
    if (not std::filesystem::exists(config_file_path)) {
        MessageBox(nullptr, TEXT("Configuration file \".flsc\" not found."), TEXT("Message"), MB_OK);
    }

    const auto whitelist = get_whitelist(config_file_path);

    const auto all_files = get_file_list(std::filesystem::current_path());
    if (delete_files(whitelist, all_files)) {
        MessageBox(nullptr, TEXT("FL Studio stems cleaned successfully."), TEXT("Message"), MB_OK);
    }
    else {
        MessageBox(nullptr, TEXT("No files deleted."), TEXT("Message"), MB_OK);
    }

    return 0;
}

