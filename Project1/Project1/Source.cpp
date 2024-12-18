#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <direct.h>

std::vector<char> output_data;

struct DataBlock {
    std::vector<char> data;
    DataBlock* next;
    DataBlock(size_t size) : data(size), next(nullptr) {}
};

struct FileEntry {
    std::string name;
    DataBlock* head_block;
    int size;
    bool is_open;
    int current_position;

    FileEntry(const std::string& file_name)
        : name(file_name), head_block(nullptr), size(0), is_open(false), current_position(0) {}
};

struct DirectoryEntry {
    std::string name;
    std::vector<FileEntry> files;
    std::vector<DirectoryEntry> subdirs;

    DirectoryEntry(const std::string& dir_name) : name(dir_name) {}
};

struct FileSystem {
    DirectoryEntry root;
    DirectoryEntry* current_dir;

    FileSystem() : root("root"), current_dir(&root) {}
};

FileSystem fs;

int get_file_size(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        return file.tellg();
    }
    else {
        std::cout << "������: �� ������� ������� ���� " << filename << std::endl;
        return -1;
    }
}

void create_file(const std::string& file_name) {
    std::string file_with_extension = file_name;
    if (file_name.find('.') == std::string::npos) {
        file_with_extension += ".txt";
    }

    for (const auto& file : fs.current_dir->files) {
        if (file.name == file_with_extension) {
            std::cout << "������: ���� � ����� ������ ��� ����������.\n";
            return;
        }
    }

    fs.current_dir->files.emplace_back(file_with_extension);
    std::cout << "���� \"" << file_with_extension << "\" ������.\n";
}
void open_file(const std::string& filename) {
    for (auto& file : fs.current_dir->files) {
        if (file.name == filename) {
            if (file.is_open) {
                std::cout << "���� \"" << filename << "\" ��� ������.\n";
                return;
            }
            file.is_open = true;
            file.current_position = 0;
            std::cout << "���� \"" << filename << "\" ������� ������.\n";
            return;
        }
    }
    std::cout << "������: ���� \"" << filename << "\" �� ������.\n";
}

void close_file(const std::string& filename) {
    for (auto& file : fs.current_dir->files) {
        if (file.name == filename) {
            if (!file.is_open) {
                std::cout << "���� \"" << filename << "\" �� ������.\n";
                return;
            }
            file.is_open = false;
            std::cout << "���� \"" << filename << "\" ������� ������.\n";
            return;
        }
    }
    std::cout << "������: ���� \"" << filename << "\" �� ������.\n";
}

void change_position(const std::string& file_name, int new_position) {
    for (auto& file : fs.current_dir->files) {
        if (file.name == file_name) {
            if (!file.is_open) {
                std::cout << "������: ���� �� ������.\n";
                return;
            }
            if (new_position < 0 || new_position >= file.size) {
                std::cout << "������: ����� ������� ������� �� ������� �����.\n";
                return;
            }
            file.current_position = new_position;
            std::cout << "������� ������� � ����� \"" << file_name << "\": " << new_position << "\n";
            return;
        }
    }
    std::cout << "������: ���� \"" << file_name << "\" �� ������.\n";
}

void read_file(const std::string& filename, size_t block_size) {
    for (auto& file : fs.current_dir->files) {
        if (file.name == filename && file.is_open) {
            if (file.current_position + block_size > file.size) {
                std::cout << "������������� ���� ������� �� ������� �����.\n";
                return;
            }

            std::vector<char> data;
            DataBlock* current_block = file.head_block;
            int bytes_read = 0;
            while (current_block != nullptr && bytes_read < block_size) {
                int bytes_to_read = std::min(block_size - bytes_read, static_cast<size_t>(current_block->data.size()));
                data.insert(data.end(), current_block->data.begin(), current_block->data.begin() + bytes_to_read);
                bytes_read += bytes_to_read;
                current_block = current_block->next;
            }

            std::cout << "��������� " << bytes_read << " ���� �� ����� \"" << filename << "\": ";
            for (char byte : data) {
                std::cout << byte;
            }
            std::cout << std::endl;
            file.current_position += bytes_read;
            return;
        }
    }
    std::cout << "���� \"" << filename << "\" �� ������ ��� �� ������.\n";
}

void write_file(const std::string& filename, size_t block_size) {
    for (auto& file : fs.current_dir->files) {
        if (file.name == filename && file.is_open) {
            DataBlock* new_block = new DataBlock(block_size);
            file.size += block_size;
            std::cout << "������� ������ ��� ������ (�������� " << block_size << " ��������): ";
            std::cin.ignore();
            std::string user_input;
            std::getline(std::cin, user_input);
            for (size_t i = 0; i < block_size; ++i) {
                if (i < user_input.size()) {
                    new_block->data[i] = user_input[i];
                }
                else {
                    new_block->data[i] = '\0';
                }
            }
            if (file.head_block == nullptr) {
                file.head_block = new_block;
            }
            else {
                DataBlock* current = file.head_block;
                while (current->next != nullptr) {
                    current = current->next;
                }
                current->next = new_block;
            }
            std::cout << "�������� " << block_size << " ���� � ���� \"" << filename << "\".\n";
            return;
        }
    }
    std::cout << "������: ���� \"" << filename << "\" �� ������ ��� �� ������.\n";
}

void delete_file(const std::string& file_name) {
    auto& files = fs.current_dir->files;
    auto it = std::remove_if(files.begin(), files.end(),
        [&file_name](const FileEntry& file) { return file.name == file_name; });

    if (it != files.end()) {
        for (auto file_it = it; file_it != files.end(); ++file_it) {
            DataBlock* current = file_it->head_block;
            while (current != nullptr) {
                DataBlock* to_delete = current;
                current = current->next;
                delete to_delete;
            }
        }
        files.erase(it, files.end());
        std::cout << "���� \"" << file_name << "\" �����.\n";
    }
    else {
        std::cout << "������: ���� \"" << file_name << "\" �� ������.\n";
    }
}

void search_files(const std::string& mask) {
    std::cout << "����� ������ � ������ \"" << mask << "\":\n";
    for (const auto& file : fs.current_dir->files) {
        if (file.name.find(mask) != std::string::npos) {
            std::cout << file.name << "\n";
        }
    }
}

void create_directory(const std::string& dir_name) {
    for (const auto& dir : fs.current_dir->subdirs) {
        if (dir.name == dir_name) {
            std::cout << "������: ������� � ����� ������ ��� ����������.\n";
            return;
        }
    }

    fs.current_dir->subdirs.emplace_back(dir_name);
    std::cout << "������� \"" << dir_name << "\" ������.\n";
}

void delete_directory(const std::string& dir_name) {
    auto& subdirs = fs.current_dir->subdirs;
    auto it = std::remove_if(subdirs.begin(), subdirs.end(),
        [&dir_name](const DirectoryEntry& dir) { return dir.name == dir_name; });

    if (it != subdirs.end()) {
        subdirs.erase(it, subdirs.end());
        std::cout << "������� \"" << dir_name << "\" �����.\n";
    }
    else {
        std::cout << "������: ������� \"" << dir_name << "\" �� ������.\n";
    }
}
void change_directory(const std::string& dir_name) {
    for (auto& dir : fs.current_dir->subdirs) {
        if (dir.name == dir_name) {
            fs.current_dir = &dir;
            std::cout << "������� ������� ������ �� \"" << dir_name << "\".\n";
            return;
        }
    }
    std::cout << "������: ������� \"" << dir_name << "\" �� ������.\n";
}

void get_current_directory_info() {
    std::cout << "������� �������: " << fs.current_dir->name << "\n";
    std::cout << "���������� ������: " << fs.current_dir->files.size() << "\n";
    std::cout << "���������� ������������: " << fs.current_dir->subdirs.size() << "\n";
}

void import_file(const std::string& real_file, const std::string& virtual_file) {
    std::ifstream real_file_stream(real_file, std::ios::binary);
    if (!real_file_stream) {
        std::cout << "������: �� ������� ������� ���� �� �������� �������� �������.\n";
        return;
    }

    int file_size = get_file_size(real_file);
    if (file_size < 0) {
        return;
    }
    create_file(virtual_file);

    auto& virtual_file_entry = fs.current_dir->files.back();
    virtual_file_entry.size = file_size;

    while (file_size > 0) {
        size_t block_size = std::min(static_cast<size_t>(file_size), static_cast<size_t>(1024));
        DataBlock* new_block = new DataBlock(block_size);
        real_file_stream.read(new_block->data.data(), block_size);

        if (virtual_file_entry.head_block == nullptr) {
            virtual_file_entry.head_block = new_block;
        }
        else {
            DataBlock* current = virtual_file_entry.head_block;
            while (current->next != nullptr) {
                current = current->next;
            }
            current->next = new_block;
        }

        file_size -= block_size;
    }

    real_file_stream.close();
    std::cout << "���� \"" << real_file << "\" ������������ � ����������� �������� ������� ��� \"" << virtual_file << "\".\n";
}
void show_menu() {
    std::cout << "��������� ����������� �������� �������:\n";
    std::cout << "1. ������� ����� ����\n";
    std::cout << "2. ������� ����\n";
    std::cout << "3. ��������� ������ �� �����\n";
    std::cout << "4. �������� ������ � ����\n";
    std::cout << "5. �������� ������� � �����\n";
    std::cout << "6. ������� ����\n";
    std::cout << "7. ������� ����\n";
    std::cout << "8. ����� ������ �� �����\n";
    std::cout << "9. ������� �������\n";
    std::cout << "10. ������� �������\n";
    std::cout << "11. �������� ������� �������\n";
    std::cout << "12. �������� ���������� � �������� �������\n";
    std::cout << "13. ������������� ���� �� �������� �������� �������\n";
    std::cout << "0. �����\n";
}

void list_files() {
    std::cout << "����� � ������� ��������:\n";
    for (const auto& file : fs.current_dir->files) {
        std::cout << "���: " << file.name
            << ", ������: " << (file.is_open ? "��" : "���")
            << ", ������: " << file.size
            << ", ������� �������: " << file.current_position << "\n";
    }
}

int main() {
    int choice = 0;
    std::string filename = "", mask = "", dir_name = "", real_file = "", virtual_file = "";
    setlocale(LC_ALL, "RUS");
    while (true) {
        show_menu();
        std::cout << "�������� �����: ";
        std::cin >> choice;
        std::cin.ignore();
        switch (choice) {
        case 1:
            std::cout << "������� ��� �����: ";
            std::getline(std::cin, filename);
            create_file(filename);
            break;
        case 2:
            std::cout << "������� ��� ����� ��� ��������: ";
            std::getline(std::cin, filename);
            open_file(filename);
            break;
        case 3:
            std::cout << "������� ��� �����: ";
            std::getline(std::cin, filename);
            size_t size;
            std::cout << "������� ����� ����� ��� ������: ";
            std::cin >> size;
            read_file(filename, size);
            break;
        case 4:
            std::cout << "������� ��� �����: ";
            std::getline(std::cin, filename);
            std::cout << "������� ����� ����� ��� ������ � ������: ";
            std::cin >> size;
            write_file(filename, size);
            break;
        case 5:
            std::cout << "������� ��� �����: ";
            std::getline(std::cin, filename);
            int new_position;
            std::cout << "������� ����� �������: ";
            std::cin >> new_position;
            change_position(filename, new_position);
            break;
        case 6:
            std::cout << "������� ��� ����� ��� ��������: ";
            std::getline(std::cin, filename);
            close_file(filename);
            break;
        case 7:
            std::cout << "������� ��� ����� ��� ��������: ";
            std::getline(std::cin, filename);
            delete_file(filename);
            break;
        case 8:
            std::cout << "������� ��� ����� ��� ������: ";
            std::getline(std::cin, mask);
            search_files(mask);
            break;
        case 9:
            std::cout << "������� ��� �������� ��� ��������: ";
            std::getline(std::cin, dir_name);
            create_directory(dir_name);
            break;
        case 10:
            std::cout << "������� ��� �������� ��� ��������: ";
            std::getline(std::cin, dir_name);
            delete_directory(dir_name);
            break;
        case 11:
            std::cout << "������� ��� �������� ��� ��������� �������� ��������: ";
            std::getline(std::cin, dir_name);
            change_directory(dir_name);
            break;
        case 12:
            get_current_directory_info();
            break;
        case 13:
            std::cout << "������� ��� ��������� ����� ��� �������: ";
            std::getline(std::cin, real_file);
            std::cout << "������� ��� ������������ �����: ";
            std::getline(std::cin, virtual_file);
            import_file(real_file, virtual_file);
            break;
        case 0:
            std::cout << "����� �� ���������.\n";
            return 0;

        default:
            std::cout << "�������� �����. ���������� �����.\n";
        }
    }
}
