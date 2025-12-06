// AKuCU_lab2.cpp By Romanov Kirill 932424
// А закодировать было легко...(решил добавить ещё коментариев, чтобы самому вспомнить) отправляю спустя 3 недели...


#include <iostream>// стандарт
#include <fstream>// для файлов
#include <vector>//для формата записи
#include <map>// хранение основных данных
#include <string>//помощь в сравнении,построение кодов и передача названий файлов
#include <chrono>//для замера времени
#include <queue>// для декодера
using namespace std;

class ArithmeticCoder {// методы в классе т.к. может быть компилятор их сделает инлайн тогда скорость выполнения будет быстрее
private:
    uint32_t low, high;          // Границы текущего рабочего интервала
    map<char, uint64_t> frequency; // Таблица частот символов
    uint64_t total_freq;         // Общее количество символов в тексте
    vector<bool> output_bits;    // Выходной поток битов в который мы в процессе заполняем

    //для декодера
    queue<bool> input_bits;// входные биты
    uint32_t value;// текущее значение

    static constexpr uint32_t TOP_VALUE = 0xFFFFFFFFU;; // Максимальное значение интервала(наш верхний край)

    static constexpr uint32_t FIRST_QTR = 0x40000000U;//дополнительные ограничения это первая четверть
    static constexpr uint32_t HALF = 0x80000000U;//половина
    static constexpr uint32_t THIRD_QTR = 0xC0000000U;//  3/4
    int underflow_bits = 0;//сколько битов ещё надо обработать 

    void output_bit(bool bit) {// в выходной поток записываем бит который передаем
        output_bits.push_back(bit);
    }

    void finish_encoding() {// как раз работа с отложенными битами
        underflow_bits++;
        if (low < FIRST_QTR) {//если находимся в первой четверти
            output_bit(0);// финальный будет 0
            for (int i = 0; i < underflow_bits; i++) output_bit(1);// а наши отложенные - 1
        }
        else {// аналогично
            output_bit(1);
            for (int i = 0; i < underflow_bits; i++) output_bit(0);
        }
        output_bit(0);
        output_bit(0);
    }


    void encode_normalize() {//нормализация для более высокого сжатия(делаем более большой диапозон)
        while (true) {// расширяем интервал
            if (high < HALF) {//если находимся в нижней половине диапозоне
                output_bit(0);
                for (uint64_t i = 0; i < underflow_bits; i++) output_bit(1);
                underflow_bits = 0;
            }
            else if (low >= HALF) {// весь в верхней половине
                output_bit(1);
                for (uint64_t i = 0; i < underflow_bits; i++) output_bit(0);
                underflow_bits = 0;
                low -= HALF; high -= HALF;
            }
            else if (low >= FIRST_QTR && high < THIRD_QTR) {// где-то в середине, сдвигаем вниз, чтобы потом использовать первый if
                underflow_bits++;
                low -= FIRST_QTR; high -= FIRST_QTR;
            }
            else {// если не зашли в верхние то диапозон нормальный
                break;//останавливаем нормализацию
            }

            low <<= 1; high = (high << 1) | 1;
        }
    }


    void decode_normalize() {
        while (true) {
            if (high < HALF) {
                // Ничего не выводим, просто сдвигаем интервал вверх
            }
            else if (low >= HALF) {
                value -= HALF;
                low -= HALF;
                high -= HALF;
            }
            else if (low >= FIRST_QTR && high < THIRD_QTR) {
                value -= FIRST_QTR;
                low -= FIRST_QTR;
                high -= FIRST_QTR;
            }
            else {
                break; // Нормализация не требуется
            }

            // После всех преобразований — сдвигаем интервал и читаем бит
            low <<= 1;
            high = (high << 1) | 1;
            value = (value << 1) + (input_bits.empty() ? 0 : input_bits.front());
            if (!input_bits.empty()) {
                input_bits.pop();
            }
        }
    }

public: // AKuCU_lab2.cpp By Romanov Kirill 932424
    ArithmeticCoder() : low(0), high(TOP_VALUE), total_freq(0), value(0) {}// простой конструктор

    // Построение таблицы частот символов из входного текста
    void build_frequency_table(const string& text) {
        frequency.clear();
        for (char c : text) frequency[c]++; // Подсчитываем частоту каждого символа используя словарь
        total_freq = text.length();// берем сколько у нас всего символов
    }

    // Получение диапазона символа в таблице частот
    void get_symbol_range(char symbol, uint64_t& low_count, uint64_t& high_count) {
        low_count = 0;
        for (const auto& pair : frequency) {// Ищем символ в таблице и вычисляем его диапазон
            if (pair.first == symbol) {// нашли нужный
                high_count = low_count + pair.second;
                return;
            }
            low_count += pair.second;// если не тот то смещаемся
        }
        cerr << "Error: Symbol '" << symbol << "' not found in frequency table!" << endl;
    }

    // Кодирование одного символа
    void encode_symbol(char symbol) {
        uint64_t sym_low, sym_high;// границы символа нижняя и верхняя
        get_symbol_range(symbol, sym_low, sym_high);

        // Вычисление нового интервала на основе частот символа
        uint64_t range = high - low + 1ULL;//вычисление сколько текущая 
        uint64_t new_low = low + (range * sym_low) / total_freq;// новая верхняя
        uint64_t new_high = low + (range * sym_high) / total_freq - 1;// новая нижняя

        low = static_cast<uint64_t>(new_low);
        high = static_cast<uint64_t>(new_high);

        encode_normalize();// вызываем нашу нормализацию для обновления (главное стобы не вышли за границу )
    }

    // Конвертация вектора битов в байты для записи в файл
    vector<uint8_t> bits_to_bytes() {
        vector<uint8_t> bytes;// для хранения
        uint8_t current_byte = 0;//текущий байт
        int bit_count = 0;

        // Упаковываем биты в байты
        for (bool bit : output_bits) {// проходимся по всему из output_bits
            current_byte = (current_byte << 1) | (bit ? 1 : 0);// добавляем новый бит к текущему

            if (++bit_count == 8) {// набрали ли байт
                bytes.push_back(current_byte);// его в результат
                current_byte = 0;// сбрасываем 
                bit_count = 0;
            }
        }

        // Обработка неполного последнего байта
        if (bit_count > 0) {//то заполняем конец байта нулями а это переносим в начало 
            current_byte <<= (8 - bit_count);
            bytes.push_back(current_byte);
        }

        return bytes;
    }// AKuCU_lab2.cpp By Romanov Kirill 932424

    // Метод кодирования файла
    bool encode(const string& input_file, const string& output_file) {
        ifstream infile(input_file, ios::binary);
        if (!infile) {
            cerr << "Error: Cannot open input file" << endl;
            return false;
        }
        //читаю весь фаил в память(не думаю что будет более 10 gb) 
        string text((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
        infile.close();// он больше не нужен

        if (text.empty()) {// если он пустой
            cerr << "Error: Input file is empty" << endl;
            return false;
        }

        cout << "Input size: " << text.length() << " bytes" << endl;

        build_frequency_table(text); //делаем таблицу для кодирования 

        low = 0;//задаем начальные интервалы
        high = TOP_VALUE;
        output_bits.clear();// мало ли было что-то
        underflow_bits = 0;

        for (char c : text) { //кодируем каждый символ 
            encode_symbol(c);
        }
        finish_encoding();// и финал кодера

        auto encoded_data = bits_to_bytes();//вектор битов преобразуем в байты

        ofstream outfile(output_file, ios::binary);// создаем фаил для записи
        if (!outfile) {
            cerr << "Error: Cannot create output file" << endl;
            return false;
        }
        //сначала делаем заголовок
        uint64_t original_length = text.length();
        outfile.write(reinterpret_cast<const char*>(&original_length), sizeof(original_length));//длина - числом те 4 байта

        uint64_t symbol_count = frequency.size();// сколько у нас всего уникальных символов
        outfile.write(reinterpret_cast<const char*>(&symbol_count), sizeof(symbol_count));

        for (const auto& pair : frequency) {// записываем тблицу частот
            outfile.put(pair.first);//символ
            outfile.write(reinterpret_cast<const char*>(&pair.second), sizeof(pair.second)); //частоту
        }

        // закодированное добро
        outfile.write(reinterpret_cast<const char*>(encoded_data.data()), encoded_data.size());
        outfile.close();

        cout << "Compressed size: " << encoded_data.size() << " bytes" << endl;// в векторе, оказывается можно длинну взять
        double ratio = (1.0 - (double)encoded_data.size() / text.length()) * 100.0;// и нетрудной формулой можно посчитать степень сжатия
        cout << "Compression ratio: " << ratio << "%" << endl;

        return true;
    }// AKuCU_lab2.cpp By Romanov Kirill 932424

    bool decode(const string& input_file, const string& output_file) {
        ifstream infile(input_file, ios::binary);// открываем входной
        if (!infile) {// если не открылся
            cerr << "Error: Cannot open input file" << endl;
            return false;
        }


        uint64_t original_length;//читаю длинну 
        infile.read(reinterpret_cast<char*>(&original_length), sizeof(original_length));


        uint64_t symbol_count;// читаю количество символов
        infile.read(reinterpret_cast<char*>(&symbol_count), sizeof(symbol_count));

        // Читаем таблицу частот (не пропускаем!)
        frequency.clear();
        total_freq = 0;
        for (uint64_t i = 0; i < symbol_count; i++) {
            char symbol = infile.get();//сам символ
            uint64_t freq;// его частота
            infile.read(reinterpret_cast<char*>(&freq), sizeof(freq));
            frequency[symbol] = freq;//частоту в словарь
            total_freq += freq;
        }

        //читаем остаток
        vector<uint8_t> encoded_data;
        uint8_t byte;
        while (infile.read(reinterpret_cast<char*>(&byte), 1)) {
            encoded_data.push_back(byte);

        }
        infile.close();// закрываем, уже всё в памяти

        if (encoded_data.empty()) { // остаток нулевой, нечего декодировать
            cerr << "Error: No encoded data found" << endl;
            return false;
        }

        while (!input_bits.empty()) input_bits.pop();

        // байты в биты
        for (uint8_t b : encoded_data) {
            for (int i = 7; i >= 0; i--) {
                input_bits.push((b >> i) & 1);
            }
        }

        low = 0;
        high = TOP_VALUE;
        value = 0;


        for (int i = 0; i < 32; ++i) {
            value <<= 1;
            if (!input_bits.empty()) {
                value |= input_bits.front() ? 1U : 0U;
                input_bits.pop();
            }
            // Дочитываем нулями, если битов не хватает 
        }


        // создаем файл 
        ofstream outfile(output_file, ios::binary);
        if (!outfile) {
            cerr << "Error: Cannot create output file" << endl;
            return false;
        }
        cout << "Decoding " << original_length << " symbols..." << endl;


        // Основной декодер
        for (uint64_t i = 0; i < original_length; i++) {
            if (high <= low) { // Это катастрофа!
                cerr << "Error: Invalid interval [low=" << low << ", high=" << high << "]" << endl;
                return false;
            }

            uint64_t range = static_cast<uint64_t>(high) - low + 1ULL;;//ширина интервала
            if (range == 0) {
                cerr << "Error: Zero range!" << endl;
                return false;
            }
            uint64_t scale = ((static_cast<uint64_t>(value) - low) * total_freq) / range;
            if (scale >= total_freq) scale = total_freq - 1;  // защита от округления

            char symbol = 0; // для найденного символа 
            uint64_t cumulative = 0; //сумма частот предыдущих символов
            uint64_t sym_low = 0, sym_high = 0;

            // проходим по всем символам в таблице частот
            for (const auto& pair : frequency) {
                if (scale < cumulative + pair.second) {
                    symbol = pair.first;
                    sym_low = cumulative;
                    sym_high = cumulative + pair.second;
                    break;
                }
                cumulative += pair.second;
            }

            uint64_t new_low = low + (range * sym_low) / total_freq;
            uint64_t new_high = low + (range * sym_high) / total_freq - 1;
            if (new_high < new_low) {
                new_high = new_low;
            }
            if (new_low > 0xFFFFFFFFULL || new_high > 0xFFFFFFFFULL) {
                cerr << "Error: new_low/new_high overflow!" << endl;
                return false;
            }

            low = static_cast<uint32_t>(new_low);
            high = static_cast<uint32_t>(new_high);
            outfile.put(symbol); // записываем декодированный символ
            decode_normalize();// нормализация при каждой итерации
        }

        outfile.close();// всё, готово
        cout << "Decoding completed successfully!" << endl;
        cout << "Output size: " << original_length << " bytes" << endl;

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
                auto start = chrono::steady_clock::now();
                if (!coder.encode(inputFile, outputFile)) {
                    cout << "Encoding failed!" << endl;
                    return 1;
                }
                auto end = chrono::steady_clock::now();
                chrono::duration<double> elapsed = end - start;
                cout << "Time: " << elapsed.count() << " seconds\n";
                cout << "Encoding successful!" << endl;
            }
            break;// AKuCU_lab2.cpp By Romanov Kirill 932424

        case 2: // Декодирование
            cout << "Starting decoding" << endl;
            {
                auto start = chrono::steady_clock::now();
                if (!coder.decode(inputFile, outputFile)) {
                    cout << "Decoding failed!" << endl;
                    return 1;
                }
                auto end = chrono::steady_clock::now();
                chrono::duration<double> elapsed = end - start;
                cout << "Time: " << elapsed.count() << " seconds\n";
                cout << "Decoding successful!" << endl;
            }
            break;

        case 3: // Сравнение
            cout << "Comparing files..." << endl;
            {
                auto start = chrono::steady_clock::now();
                if (compareFiles(inputFile, outputFile)) {
                    cout << "Files are identical!" << endl;
                }
                else {
                    cout << "Files are different!" << endl;
                    return 1;
                }
                auto end = chrono::steady_clock::now();
                chrono::duration<double> elapsed = end - start;
                cout << "Time: " << elapsed.count() << " seconds\n";
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
                auto start = chrono::steady_clock::now();
                success = coder.encode(inputFile, outputFile);
                if (success) {
                    cout << "Encoding successful!" << endl;
                }
                else {
                    cout << "Encoding failed!" << endl;
                }
                auto end = chrono::steady_clock::now();
                chrono::duration<double> elapsed = end - start;
                cout << "Time: " << elapsed.count() << " seconds\n";
            }
            break;

        case 2: // Декодирование
            cout << "Starting decoding: " << inputFile << " -> " << outputFile << endl;
            {
                auto start = chrono::steady_clock::now();
                success = coder.decode(inputFile, outputFile);
                if (success) {
                    cout << "Decoding successful!" << endl;
                }
                else {
                    cout << "Decoding failed!" << endl;
                }
                auto end = chrono::steady_clock::now();
                chrono::duration<double> elapsed = end - start;
                cout << "Time: " << elapsed.count() << " seconds\n";
            }
            break;// AKuCU_lab2.cpp By Romanov Kirill 932424

        case 3: // Сравнение
            cout << "Comparing files: " << inputFile << " versus " << outputFile << endl;
            {
                auto start = chrono::steady_clock::now();
                if (compareFiles(inputFile, outputFile)) {
                    cout << "Files are identical!" << endl;
                }
                else {
                    cout << "Files are different!" << endl;
                }
                auto end = chrono::steady_clock::now();
                chrono::duration<double> elapsed = end - start;
                cout << "Time: " << elapsed.count() << " seconds\n";
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