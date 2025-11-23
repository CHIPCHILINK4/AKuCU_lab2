// AKuCU_lab2.cpp By Romanov Kirill 932424
// алгортим есть осталось дешифратор и работу с файлами


#include <iostream>// стандарт
#include <fstream>// для файлов
#include <vector>
#include <map>// хранение основных данных
#include <string>//помощь в сравнении,построение кодов и передача названий файлов
#include <chrono>//для замера времени
using namespace std;

class ArithmeticCoder {
private:
    uint32_t low, high;          // Границы текущего рабочего интервала
    map<char, uint32_t> frequency; // Таблица частот символов
    uint32_t total_freq;         // Общее количество символов в тексте
    vector<bool> output_bits;    // Выходной поток битов

    static constexpr uint32_t TOP_VALUE = 0xFFFFFFFF; // Максимальное значение интервала

public: // AKuCU_lab2.cpp By Romanov Kirill 932424
    ArithmeticCoder() : low(0), high(TOP_VALUE), total_freq(0) {}

    // Построение таблицы частот символов из входного текста
    void build_frequency_table(const string& text) {
        frequency.clear();
        for (char c : text) frequency[c]++; // Подсчитываем частоту каждого символа
        total_freq = text.length();
    }

    // Получение диапазона символа в таблице частот
    void get_symbol_range(char symbol, uint32_t& low_count, uint32_t& high_count) {
        low_count = 0;
        // Ищем символ в таблице и вычисляем его диапазон
        for (const auto& pair : frequency) {
            if (pair.first == symbol) {
                high_count = low_count + pair.second;
                return;
            }
            low_count += pair.second;
        }
        // Если символ не найден - добавляем с частотой 1
        high_count = low_count + 1;
    }

    // Кодирование одного символа
    void encode_symbol(char symbol) {
        uint32_t sym_low, sym_high;
        get_symbol_range(symbol, sym_low, sym_high);
        
        // Вычисление нового интервала на основе частот символа
        uint64_t range = static_cast<uint64_t>(high - low) + 1;
        high = low + static_cast<uint32_t>((range * sym_high) / total_freq) - 1;
        low = low + static_cast<uint32_t>((range * sym_low) / total_freq);
        
        cout << "Encoded '" << symbol << "': [" << low << ", " << high << "]" << endl;
    }

    // Конвертация вектора битов в байты для записи в файл
    vector<uint8_t> bits_to_bytes() {
        vector<uint8_t> bytes;
        uint8_t current_byte = 0;
        int bit_count = 0;
        
        // Упаковываем биты в байты
        for (bool bit : output_bits) {
            current_byte = (current_byte << 1) | (bit ? 1 : 0);
            if (++bit_count == 8) {
                bytes.push_back(current_byte);
                current_byte = 0; 
                bit_count = 0;
            }
        }
        
        // Обработка неполного последнего байта
        if (bit_count > 0) {
            current_byte <<= (8 - bit_count);
            bytes.push_back(current_byte);
        }
        
        return bytes;
    }// AKuCU_lab2.cpp By Romanov Kirill 932424

    // Метод кодирования файла
    bool encode(const string& input_file, const string& output_file) {
        
        // Чтение входного файла
        ifstream infile(input_file, ios::binary);
        if (!infile) return false;
        string text((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
        infile.close();
        
        cout << "Input size: " << text.length() << " bytes" << endl;
        
        build_frequency_table(text);
        
        // Сброс состояния кодера
        low = 0; 
        high = TOP_VALUE; 
        output_bits.clear();
        
        // Кодирование каждого символа
        for (char c : text) {
            encode_symbol(c);
        }
        
        // Временное решение: сохраняем исходный текст (будет заменено в следующих версиях)
        ofstream outfile(output_file, ios::binary);
        outfile.write(text.c_str(), text.length());
        outfile.close();
        

        
        return true;
    }// AKuCU_lab2.cpp By Romanov Kirill 932424

    bool decode(const string& input_file, const string& output_file) {
        cout << "Decoding not fully implemented" << endl;
        // Временное решение: просто копируем файл
        ifstream infile(input_file, ios::binary);
        ofstream outfile(output_file, ios::binary);
        outfile << infile.rdbuf();
        return true;
    }
};// AKuCU_lab2.cpp By Romanov Kirill 932424

bool compareFiles(const string& file1, const string& file2) {// сравниваем декодированный и исходный( взял из предыдущей программы)
    ifstream f1(file1, ios::binary);//пусть декодированный (ну не принципиально)
    ifstream f2(file2, ios::binary);//пусть исходный

    if (!f1 || !f2) {// файлы не открылись
        cout << "Error: Cannot open files for comparison" << endl;
        return false;
    }

    char c1, c2;//посимвольно сравниваем
    while (f1.get(c1) && f2.get(c2)) {
        if (c1 != c2) {
            f1.close();//если что-то пошло не так
            f2.close();
            return false;
        }
    }

    f1.close();//закрываем их
    f2.close();
    return true;
}// AKuCU_lab2.cpp By Romanov Kirill 932424


// Главная функция (взята на основе 1 лабы, с добавлением подсчета времени)
int main(int argc, char* argv[]) {
    ArithmeticCoder coder;

    if (argc == 4) {
        // Режим с аргументами 
        int mode = atoi(argv[1]);
        string inputFile = argv[2];
        string outputFile = argv[3];

        switch (mode) {
        case 1: // Кодирование
            cout << "Starting encoding" << endl;
            {
                auto start = std::chrono::steady_clock::now();
                if (!coder.encode(inputFile, outputFile)) {
                    cout << "Encoding failed!" << endl;
                    return 1;
                }
                auto end = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = end - start;
                cout << "Время выполнения: " << elapsed.count() << " секунд\n";
                cout << "Encoding successful!" << endl;
            }
            break;// AKuCU_lab2.cpp By Romanov Kirill 932424

        case 2: // Декодирование
            cout << "Starting decoding" << endl;
            {
                auto start = std::chrono::steady_clock::now();
                if (!coder.decode(inputFile, outputFile)) {
                    cout << "Decoding failed!" << endl;
                    return 1;
                }
                auto end = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = end - start;
                cout << "Время выполнения: " << elapsed.count() << " секунд\n";
                cout << "Decoding successful!" << endl;
            }
            break;

        case 3: // Сравнение
            cout << "Comparing files..." << endl;
            {
                auto start = std::chrono::steady_clock::now();
                if (compareFiles(inputFile, outputFile)) {
                    cout << "Files are identical!" << endl;
                }
                else {
                    cout << "Files are different!" << endl;
                    return 1;
                }
                auto end = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = end - start;
                cout << "Время выполнения: " << elapsed.count() << " секунд\n";
            }
            break;

        default:
            cout << "Unknown mode: " << mode << endl;
            cout << "Usage:" << endl;
            cout << "  Encode: " << argv[0] << " 1 input.txt output.bin" << endl;
            cout << "  Decode: " << argv[0] << " 2 input.bin output.txt" << endl;
            cout << "  Compare: " << argv[0] << " 3 file1.txt file2.txt" << endl;
            return 1;
        }
    }// AKuCU_lab2.cpp By Romanov Kirill 932424
    else if (argc == 1) {
        // Интерактивный режим
        cout << "Arithmetic compression" << endl;
        cout << "==================" << endl;
        cout << "1 - Encode file" << endl;
        cout << "2 - Decode file" << endl;
        cout << "3 - Compare files" << endl;
        cout << "0 - Exit" << endl;

        int choice;
        cout << "Select option: ";
        cin >> choice;

        if (choice == 0) {
            cout << "Goodbye!" << endl;
            return 0;
        }

        if (choice < 1 || choice > 3) {
            cout << "Invalid choice! Please select 0-3." << endl;
            cout << "Restart the programm" << endl;
            return 1;
        }

        string inputFile, outputFile;
        cout << "Input file: ";
        cin >> inputFile;
        cout << "Output file: ";
        cin >> outputFile;

        bool success = false;

        switch (choice) {
        case 1: // Кодирование
            cout << "Starting encoding: " << inputFile << " -> " << outputFile << endl;
            {
                auto start = std::chrono::steady_clock::now();
                success = coder.encode(inputFile, outputFile);
                if (success) {
                    cout << "Encoding successful!" << endl;
                }
                else {
                    cout << "Encoding failed!" << endl;
                }
                auto end = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = end - start;
                cout << "Время выполнения: " << elapsed.count() << " секунд\n";
            }
            break;

        case 2: // Декодирование
            cout << "Starting decoding: " << inputFile << " -> " << outputFile << endl;
            {
                auto start = std::chrono::steady_clock::now();
                success = coder.decode(inputFile, outputFile);
                if (success) {
                    cout << "Decoding successful!" << endl;
                }
                else {
                    cout << "Decoding failed!" << endl;
                }
                auto end = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = end - start;
                cout << "Время выполнения: " << elapsed.count() << " секунд\n";
            }
            break;// AKuCU_lab2.cpp By Romanov Kirill 932424

        case 3: // Сравнение
            cout << "Comparing files: " << inputFile << " versus " << outputFile << endl;
            {
                auto start = std::chrono::steady_clock::now();
                if (compareFiles(inputFile, outputFile)) {
                    cout << "Files are identical!" << endl;
                }
                else {
                    cout << "Files are different!" << endl;
                }
                auto end = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = end - start;
                cout << "Время выполнения: " << elapsed.count() << " секунд\n";
            }
            break;
        }

        cout << "Press Enter to exit!";
        cin.ignore();
        cin.get();
    }// AKuCU_lab2.cpp By Romanov Kirill 932424
    else {
        // Неправильное количество аргументов
        cout << "Usage:" << endl;
        cout << "  Encode: " << "AKuCU_lab2.exe" << " 1 input.txt output.bin" << endl;
        cout << "  Decode: " << "AKuCU_lab2.exe" << " 2 input.bin output.txt" << endl;
        cout << "  Compare: " << "AKuCU_lab2.exe" << " 3 file1.txt file2.txt" << endl;
        return 1;
    }

    return 0;
}// AKuCU_lab2.cpp By Romanov Kirill 932424