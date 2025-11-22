// AKuCU_lab2.cpp By Romanov Kirill 932424
// за сегодня успел сделать только скелет и вспомнить как работает алгоритм 

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <chrono>
using namespace std;

class ArithmeticCoder {
private:
    map<char, uint32_t> frequency;// Таблица частот символов - хранит сколько раз каждый символ встречается

    uint32_t total_freq;    // Общее количество символов в тексте

public:
    ArithmeticCoder() : total_freq(0) {}

    // Построение таблицы частот символов из входного текста
    void build_frequency_table(const string& text) {
        frequency.clear();
        for (char c : text) {//посимвольно подсчитываем
            frequency[c]++;
        }
        total_freq = text.length();
        cout << "Built frequency table with " << frequency.size() << " unique symbols" << endl;
    }

    // пока просто открытие файла
    bool encode(const string& input_file, const string& output_file) {
        cout << "Starting encoding: " << input_file << " -> " << output_file << endl;
        
        
        ifstream infile(input_file, ios::binary);// Чтение входного файла
        if (!infile) {
            cerr << "Error: Cannot open input file" << endl;
            return false;
        }
        
        string text((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());// Читаем весь файл в строку
        infile.close();
        
        cout << "Input size: " << text.length() << " bytes" << endl;
        
        // Строим таблицу частот
        build_frequency_table(text);
        
        // сделать сам алгоритм
        cout << "Encoding not implemented yet" << endl;
        
        return false;
    }

    // заглушка чтобы определиться с методами
    bool decode(const string& input_file, const string& output_file) {
        cout << "Starting decoding: " << input_file << " -> " << output_file << endl;
        cout << "Decoding not implemented yet" << endl;
        return false;
    }
};

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
}// AKuCU_lab1.cpp By Romanov Kirill 932424

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
            break;

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
            break;

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


//понять алгоритм 
//сделать реализацию