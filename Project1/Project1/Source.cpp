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
        std::cout << "Ошибка: не удалось открыть файл " << filename << std::endl;
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
            std::cout << "Ошибка: файл с таким именем уже существует.\n";
            return;
        }
    }

    fs.current_dir->files.emplace_back(file_with_extension);
    std::cout << "Файл \"" << file_with_extension << "\" создан.\n";
}
void open_file(const std::string& filename) {
    for (auto& file : fs.current_dir->files) {
        if (file.name == filename) {
            if (file.is_open) {
                std::cout << "Файл \"" << filename << "\" уже открыт.\n";
                return;
            }
            file.is_open = true;
            file.current_position = 0;
            std::cout << "Файл \"" << filename << "\" успешно открыт.\n";
            return;
        }
    }
    std::cout << "Ошибка: файл \"" << filename << "\" не найден.\n";
}

void close_file(const std::string& filename) {
    for (auto& file : fs.current_dir->files) {
        if (file.name == filename) {
            if (!file.is_open) {
                std::cout << "Файл \"" << filename << "\" не открыт.\n";
                return;
            }
            file.is_open = false;
            std::cout << "Файл \"" << filename << "\" успешно закрыт.\n";
            return;
        }
    }
    std::cout << "Ошибка: файл \"" << filename << "\" не найден.\n";
}

void change_position(const std::string& file_name, int new_position) {
    for (auto& file : fs.current_dir->files) {
        if (file.name == file_name) {
            if (!file.is_open) {
                std::cout << "Ошибка: файл не открыт.\n";
                return;
            }
            if (new_position < 0 || new_position >= file.size) {
                std::cout << "Ошибка: новая позиция выходит за пределы файла.\n";
                return;
            }
            file.current_position = new_position;
            std::cout << "Текущая позиция в файле \"" << file_name << "\": " << new_position << "\n";
            return;
        }
    }
    std::cout << "Ошибка: файл \"" << file_name << "\" не найден.\n";
}

void read_file(const std::string& filename, size_t block_size) {
    for (auto& file : fs.current_dir->files) {
        if (file.name == filename && file.is_open) {
            if (file.current_position + block_size > file.size) {
                std::cout << "Запрашиваемый блок выходит за пределы файла.\n";
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

            std::cout << "Прочитано " << bytes_read << " байт из файла \"" << filename << "\": ";
            for (char byte : data) {
                std::cout << byte;
            }
            std::cout << std::endl;
            file.current_position += bytes_read;
            return;
        }
    }
    std::cout << "Файл \"" << filename << "\" не найден или не открыт.\n";
}

void write_file(const std::string& filename, size_t block_size) {
    for (auto& file : fs.current_dir->files) {
        if (file.name == filename && file.is_open) {
            DataBlock* new_block = new DataBlock(block_size);
            file.size += block_size;
            std::cout << "Введите данные для записи (максимум " << block_size << " символов): ";
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
            std::cout << "Записано " << block_size << " байт в файл \"" << filename << "\".\n";
            return;
        }
    }
    std::cout << "Ошибка: файл \"" << filename << "\" не найден или не открыт.\n";
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
        std::cout << "Файл \"" << file_name << "\" удалён.\n";
    }
    else {
        std::cout << "Ошибка: файл \"" << file_name << "\" не найден.\n";
    }
}

void search_files(const std::string& mask) {
    std::cout << "Поиск файлов с маской \"" << mask << "\":\n";
    for (const auto& file : fs.current_dir->files) {
        if (file.name.find(mask) != std::string::npos) {
            std::cout << file.name << "\n";
        }
    }
}

void create_directory(const std::string& dir_name) {
    for (const auto& dir : fs.current_dir->subdirs) {
        if (dir.name == dir_name) {
            std::cout << "Ошибка: каталог с таким именем уже существует.\n";
            return;
        }
    }

    fs.current_dir->subdirs.emplace_back(dir_name);
    std::cout << "Каталог \"" << dir_name << "\" создан.\n";
}

void delete_directory(const std::string& dir_name) {
    auto& subdirs = fs.current_dir->subdirs;
    auto it = std::remove_if(subdirs.begin(), subdirs.end(),
        [&dir_name](const DirectoryEntry& dir) { return dir.name == dir_name; });

    if (it != subdirs.end()) {
        subdirs.erase(it, subdirs.end());
        std::cout << "Каталог \"" << dir_name << "\" удалён.\n";
    }
    else {
        std::cout << "Ошибка: каталог \"" << dir_name << "\" не найден.\n";
    }
}
void change_directory(const std::string& dir_name) {
    for (auto& dir : fs.current_dir->subdirs) {
        if (dir.name == dir_name) {
            fs.current_dir = &dir;
            std::cout << "Текущий каталог изменён на \"" << dir_name << "\".\n";
            return;
        }
    }
    std::cout << "Ошибка: каталог \"" << dir_name << "\" не найден.\n";
}

void get_current_directory_info() {
    std::cout << "Текущий каталог: " << fs.current_dir->name << "\n";
    std::cout << "Количество файлов: " << fs.current_dir->files.size() << "\n";
    std::cout << "Количество подкаталогов: " << fs.current_dir->subdirs.size() << "\n";
}

void import_file(const std::string& real_file, const std::string& virtual_file) {
    std::ifstream real_file_stream(real_file, std::ios::binary);
    if (!real_file_stream) {
        std::cout << "Ошибка: не удается открыть файл из реальной файловой системы.\n";
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
    std::cout << "Файл \"" << real_file << "\" импортирован в виртуальную файловую систему как \"" << virtual_file << "\".\n";
}
void show_menu() {
    std::cout << "Программа виртуальная файловая система:\n";
    std::cout << "1. Создать новый файл\n";
    std::cout << "2. Открыть файл\n";
    std::cout << "3. Прочитать данные из файла\n";
    std::cout << "4. Записать данные в файл\n";
    std::cout << "5. Изменить позицию в файле\n";
    std::cout << "6. Закрыть файл\n";
    std::cout << "7. Удалить файл\n";
    std::cout << "8. Поиск файлов по маске\n";
    std::cout << "9. Создать каталог\n";
    std::cout << "10. Удалить каталог\n";
    std::cout << "11. Изменить текущий каталог\n";
    std::cout << "12. Получить информацию о файловой системе\n";
    std::cout << "13. Импортировать файл из реальной файловой системы\n";
    std::cout << "0. Выход\n";
}

void list_files() {
    std::cout << "Файлы в текущем каталоге:\n";
    for (const auto& file : fs.current_dir->files) {
        std::cout << "Имя: " << file.name
            << ", Открыт: " << (file.is_open ? "Да" : "Нет")
            << ", Размер: " << file.size
            << ", Текущая позиция: " << file.current_position << "\n";
    }
}

int main() {
    int choice = 0;
    std::string filename = "", mask = "", dir_name = "", real_file = "", virtual_file = "";
    setlocale(LC_ALL, "RUS");
    while (true) {
        show_menu();
        std::cout << "Выберите опцию: ";
        std::cin >> choice;
        std::cin.ignore();
        switch (choice) {
        case 1:
            std::cout << "Введите имя файла: ";
            std::getline(std::cin, filename);
            create_file(filename);
            break;
        case 2:
            std::cout << "Введите имя файла для открытия: ";
            std::getline(std::cin, filename);
            open_file(filename);
            break;
        case 3:
            std::cout << "Введите имя файла: ";
            std::getline(std::cin, filename);
            size_t size;
            std::cout << "Введите длину блока для чтения: ";
            std::cin >> size;
            read_file(filename, size);
            break;
        case 4:
            std::cout << "Введите имя файла: ";
            std::getline(std::cin, filename);
            std::cout << "Введите длину блока для записи в байтах: ";
            std::cin >> size;
            write_file(filename, size);
            break;
        case 5:
            std::cout << "Введите имя файла: ";
            std::getline(std::cin, filename);
            int new_position;
            std::cout << "Введите новую позицию: ";
            std::cin >> new_position;
            change_position(filename, new_position);
            break;
        case 6:
            std::cout << "Введите имя файла для закрытия: ";
            std::getline(std::cin, filename);
            close_file(filename);
            break;
        case 7:
            std::cout << "Введите имя файла для удаления: ";
            std::getline(std::cin, filename);
            delete_file(filename);
            break;
        case 8:
            std::cout << "Введите имя файла для поиска: ";
            std::getline(std::cin, mask);
            search_files(mask);
            break;
        case 9:
            std::cout << "Введите имя каталога для создания: ";
            std::getline(std::cin, dir_name);
            create_directory(dir_name);
            break;
        case 10:
            std::cout << "Введите имя каталога для удаления: ";
            std::getline(std::cin, dir_name);
            delete_directory(dir_name);
            break;
        case 11:
            std::cout << "Введите имя каталога для изменения текущего каталога: ";
            std::getline(std::cin, dir_name);
            change_directory(dir_name);
            break;
        case 12:
            get_current_directory_info();
            break;
        case 13:
            std::cout << "Введите имя реального файла для импорта: ";
            std::getline(std::cin, real_file);
            std::cout << "Введите имя виртуального файла: ";
            std::getline(std::cin, virtual_file);
            import_file(real_file, virtual_file);
            break;
        case 0:
            std::cout << "Выход из программы.\n";
            return 0;

        default:
            std::cout << "Неверный выбор. Попробуйте снова.\n";
        }
    }
}
