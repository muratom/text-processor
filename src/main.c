#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>

#define SENT_LEN 100
#define TEXT_LEN 100
#define WORDS_NUM 50
#define PATH_LEN 102
#define USERS_LEN 12
#define GREEN "\033[0;32m"
#define NONE "\033[0m"
#define CASE_SENS towupper


typedef struct Word { // структура Word
    wchar_t* ptr; // указатель на слово
    wchar_t punc; // знак пункутации, стоящий после слова
} Word;

typedef struct Sentence { // структура Sentence
    Word* words; // указатель на массив структур Word
    wchar_t* ptr; // указатель на предложение
    int size; // количество символов
    int words_counter; // количество слов в предложении
    int have_newline; // флаг, указывающий на то, оканчивается ли предложения на L'\n'
    int is_empty; // флаг, указывающий на то, пустое ли предложение
    int only_newline; // флаг, указывающий на то, состоит ли предложение только из L'\n'
    int end_of_reading; // флаг, указывающий на то, закончилось ли считывание текста
} Sentence;

typedef struct Text { // структура Text
    Sentence* ptr; // указатель на начало массива предложении
    int size; // количество предложений
    int number_of_read_sent; // количество считанных предложении (ReadText)
    int number_of_splited_sent; // количество предложении, которые удалось разбить на слова (SplitText)
} Text;

// Интерфейс пользователя
void ChooseSource(); // прототип функции ChooseSource
void Menu(); // прототип функции Menu
int GetUserDecision(); // прототип функции GetUserDecision

// Основные части программы
int ChooseAndRead(Text* text);
void PerfomanceOfSubtasks(Text text);
void FreeMemory(Text* text);

// Ввод и вывод текста
Sentence ReadSentence(FILE* source); // прототип функции ReadSentence
Text ReadText(FILE* source); // прототиип функций ReadText
void WriteText(Text text); // прототип функции WriteText

// Первичная обработка текста
void DeleteSameSentences(Text* text_ptr); // прототип функции DeleteSameSentences
int SplitSentence(Sentence* ptr_sentence); // прототип функции SplitSentence
int SplitText(Text* text_ptr); // прототип функции SplitText

// Подзадача 1
void ReplaceWords(Text text); // прототип функции ReplaceWords

// Подзадача 2
int CompareSentences(const void* sent_1, const void* sent_2); // прототип функции CompareSentences
void SortSentences(Text text); // прототип функции SortSentences

// Подзадача 3
int HaveDigitsInMiddle(wchar_t* string); // прототип функции HaveDigitsInMiddle
void PaintWords(Text text); // прототип функции PaintWords

// Подзадача 4
void RepeatingLettersToSingle(wchar_t* word); // прототип функции RepeatingLettersToSingle
void RepeatingToSingle(Text text); // прототип функции RepeatingToSingle

int main() {
    setlocale(LC_ALL, "ru_RU.UTF-8"); // для вывода символов кириллицы
    Text my_text;

    if (!ChooseAndRead(&my_text)) { // процесс выбора источника и чтения данных
        // если невозможно выделить запрашиваемую память, ничего не было введено или
        // пользователь сам решил выйти из программы, то программа завершается
        return 0;
    }

    DeleteSameSentences(&my_text); // удаление повторяющихся предложении

    if (!SplitText(&my_text)) {
        wprintf(L"Запрашиваемая память не может быть выделена.\n");
        return 0;
    } // разбиение предложении в тексте на слова

    PerfomanceOfSubtasks(my_text); // выполнение подзадач

    FreeMemory(&my_text);

    return 0;
}

//------------------------------------------

void Menu() {
    wprintf(L"\nНабор доступных действий:\n");
    wprintf(L"1) В каждом предложении заменить первое слово на второе слово из предыдущего предложения;\n");
    wprintf(L"2) Отсортировать предложения по длине третьего слова;\n");
    wprintf(L"3) Вывести на экран все предложения, в которых в середине слова встречаются цифры и выделить эти слова зелёным цветом;\n");
    wprintf(L"4) В каждом предложении, в слове, все символы, которые встречаются несколько раз подряд заменить одним таким символом;\n");
    wprintf(L"5) Выход из программы.\n");
    wprintf(L"ПРИМЕЧАНИЕ: длина строки должна быть не более %d.\n", USERS_LEN-2);
    wprintf(L"Выполнить действие под номером: ");
} // Интерфейс пользователя при выборе подзадачи

//------------------------------------------

void ChooseSource() {
    wprintf(L"Каким образом будет подаваться текст в программу?\n");
    wprintf(L"1) Из стандартного потока;\n");
    wprintf(L"2) Из файла.\n");
    wprintf(L"3) Выход из программы.\n");
    wprintf(L"ПРИМЕЧАНИЕ: длина строки должна быть не более %d.\n", USERS_LEN-2);
    wprintf(L"Ваш выбор: ");
} // Интерфейс пользователя при выборе источника

//------------------------------------------

int GetUserDecision() {
    wchar_t buffer[USERS_LEN]; // буфер для строки
    fgetws(buffer, USERS_LEN, stdin); // считывание строки
    int user_decision; // переменная для хранения выбора пользователя

    for (unsigned long i=0; i < wcslen(buffer) - 1; i++) { // если в строке есть что-то помимо цифр, то она не корректна
        // (length - 1), так как не учитывается символ перевода строки
        if (!iswdigit(buffer[i]))
            return 0;
    }
    swscanf(buffer, L"%d", &user_decision);
    return user_decision;

} // Узнаем, что ввел пользователь

//------------------------------------------

int ChooseAndRead(Text* text) {
    int decision; // переменная для хранения выбора пользователя
    do {
        ChooseSource(); // интерфейс пользователя для выбора источника текста
        decision = GetUserDecision(); // узнаем у пользователя, откуда брать данные
        switch (decision) {
            case 1: { // если из конслои, то...
                *text = ReadText(stdin); // Не забудь освободить память!
                if (text->ptr == NULL) {
                    wprintf(L"Запрашиваемая память не может быть выделена. Выход из программы...\n");
                    return 0;
                }
                if (text->number_of_read_sent == 0) { // если текст оказался пустым
                    wprintf(L"Ничего не было введено. Выход из программы...\n");
                    return 0;
                }
                break; // предотвращение сквозного выполнения
            }
            case 2: { // если из файла, то...
                wprintf(L"ПРИМЕЧАНИЕ: Длина имени файла должна быть не более %d\n", PATH_LEN-2);
                wchar_t *file_name = (wchar_t*) calloc(PATH_LEN, sizeof(wchar_t)); // имя файла
                wprintf(L"Введите название файла, из которого нужно считать текст: ");
                fgetws(file_name, PATH_LEN, stdin); // считывание пути к файлу
                char *buffer = calloc(PATH_LEN * sizeof(wchar_t), sizeof(char)); // буфер для функции wcstombs
                *wcschr(file_name, L'\n') = '\0';// удаляется '\n'
                wcstombs(buffer, file_name, sizeof(wchar_t) * PATH_LEN); // преобразование пути к файлу
                // в многобайтовую строку (для функции fopen)
                FILE *file_ptr; // указатель на файл
                while ((file_ptr = fopen(buffer, "r")) == NULL) { // пока файл не найден
                    // fclose(file_ptr); // закрываем неопознанный файл
                    wprintf(L"Вы ввели некорректное название файла! Попробуйте еще раз.\n");
                    wprintf(L"ПРИМЕЧАНИЕ: Длина имени файла должна быть не более %d\n", PATH_LEN-2);
                    wprintf(L"Введите название файла, из которого нужно считать текст: ");
                    fgetws(file_name, PATH_LEN, stdin);
                    *wcschr(file_name, L'\n') = '\0';// удаляется '\n'
                    wcstombs(buffer, file_name, sizeof(wchar_t) * PATH_LEN);
                }
                *text = ReadText(file_ptr); // Не забудь освободить память!
                fclose(file_ptr); // закрытие файла
                free(file_name); // особождение памяти выделенной под имя файла и буфер
                free(buffer);
                if (text -> ptr == NULL) {
                    wprintf(L"Запрашиваемая память не может быть выделена.\n");
                    return 0;
                }
                if (text->number_of_read_sent == 0) { // если текст оказался пустым
                    wprintf(L"Ничего не было введено. Выход из программы...\n");
                    return 0;
                }
                break; // предотвращение сквозного выполнения
            }
            case 3: { // Выход из программы
                wprintf(L"Выход из программы...\n");
                return 0;
            }
            default: { // если пользователь ввёл некорректное значение
                wprintf(L"Введенные данные некорректны! Попробуйте еще раз.\n");
                break;
            }
        }
    } while((decision != 1) && (decision != 2));
    return 1;
} // Выбор источника текста и его считывание

//------------------------------------------

void PerfomanceOfSubtasks(Text text) {
    int decision; // переменная для хранения выбора пользователя
    do {
        Menu();
        decision = GetUserDecision(); // узнаем у пользователя, какую подзадачу необходимо выполнить
        switch (decision) {
            case 1: // выполнение первой подзадачи
                ReplaceWords(text);
                break;
            case 2: // выполнение второй подзадачи
                SortSentences(text);
                break;
            case 3: // выполнение третьей подзадачи
                PaintWords(text);
                break;
            case 4: // выполнение четвертой подзадачи
                RepeatingToSingle(text);
                break;
            case 5: // выход из программы
                wprintf(L"Выход из программы...\n");
                break;
            default: // если пользователь ввёл некорректное значение
                wprintf(L"Введенные данные некорректны! Попробуйте еще раз.\n");
        }
    }
    while (decision != 5); // пока пользователь не введет 5
} // Решение подзадач

//------------------------------------------

void FreeMemory(Text* text) {
    for (int i=0; i < (text -> size); i++) {
        free((text -> ptr)[i].words);
        free((text -> ptr)[i].ptr);
    }
    free(text -> ptr);
} // Освобождение выделенной памяти

//------------------------------------------

Sentence ReadSentence(FILE* source) {
    Sentence sentence = {NULL,
                         calloc(SENT_LEN, sizeof(wchar_t)),
                         0,
                         0,
                         0,
                         0,
                         0,
                         0}; // создание экземаляра структуры Sentence
    if (sentence.ptr == NULL) {
        return sentence; // если заправшиваемя память не может быть выделена, возвращается sentence с нулевым указателем
    }
    int index = 0; // индекс, по которому будет записан символ и счетчик считанных символов
    wchar_t *temp_ptr; // переменная для realloc
    wchar_t wc; // переменная для хранения очередного символа
    int mem_size = SENT_LEN; // размер выделенной памяти (в ячейках!)
    do {
        wc = fgetwc(source); // считывание очередного символа
        if (feof(source)) { // если считывание файла закончилось
            free(sentence.ptr); // освобождение выделенной памяти
            sentence.end_of_reading = 1; // считывание текста закончилось
            return sentence;
        }
        sentence.ptr[index++] = wc; // запись символа в выделенную память
        if (index == mem_size - 1) { // проверка на то, достаточно ли памяти
            mem_size += SENT_LEN; // увеличение размера буфера
            temp_ptr = realloc(sentence.ptr, sizeof(wchar_t) * mem_size); // динамическое выделение памяти
            if (temp_ptr == NULL) { // если память не была выделена
                free(sentence.ptr); // освобождение памяти
                sentence.ptr = NULL;
                return sentence; // возвращается sentence с нулевым указателем
            }
            sentence.ptr = temp_ptr; // полю sentence.ptr присваивается указатель на выделенную память
        }
    } while(!wcschr(L".!?\n", wc)); // пока не встречен один из разделителей
    sentence.ptr[index] = L'\0'; // добавление нулевого символа в конец строки
    if (wcschr(L".!?", sentence.ptr[0])) {
        free(sentence.ptr);
        fgetwc(source); // считывание разделителя после пустого предложения
        sentence.is_empty = 1;
    }
    else {
        if (sentence.ptr[0] == L'\n') { // если предложение состоит только из символа перевода строки
            free(sentence.ptr); // память выделяется, даже если введен только L'\n'; ее нужно освободить
            sentence.only_newline = 1; // предложение состоит только из символа перевода строки
        } else {
            sentence.size = index; // длина предложения с учетом знака препинания
            if (fgetwc(source) == L'\n') // проверяем, оканчивается ли предложения на L'\n'
                sentence.have_newline = 1;
        }
    }
    return sentence;
} // функция для считывания предложения

//------------------------------------------

// 1) Если был введен только символ перевода строки, то программа должна завершаться.
// 2) Если было введено пустое предложение (например, "."), то оно не учитывается.

Text ReadText(FILE* source) {
    Text text = {calloc(TEXT_LEN, sizeof(Sentence)),
                 0,
                 0,
                 0}; // создание экземпляра структуры Text
    if (text.ptr == NULL) {
        return text; // если память не была выделена, то возвращается text с нулевым указателем
    }
    int index = 0; // индекс, по которому будет записано предложение и счетчик считанных предложении
    int nc = 0; // счетчик подряд идущих L'\n'
    Sentence* temp_ptr; // переменная для realloc
    Sentence ws; // переменная для хранения очередного предложения
    int mem_size = TEXT_LEN; // размер выделенной памяти (в ячейках!)

    do {
        ws = ReadSentence(source); // считывание очередного предложения
        if (ws.ptr == NULL) { // если память не была выделена
            for (int j = 0; j < text.number_of_read_sent; j++) // особождение памяти от уже считанных предложении
                free(text.ptr[j].ptr);
            free(text.ptr);
            text.ptr = NULL;
            return text;
        }
        if (ws.only_newline) { // если предложение состоит только из L'\n'
            nc++; // увеличиваем счетчик подряд идущих символов перевода строки
            if (nc == 2)
                break; // прерываем выполение цикла
            continue; // чтобы L'\n' не записывался в текст
        }
        else {
            nc = 0; // учитывание только подряд идущих L'\n'
        }
        if (ws.end_of_reading) { // если считывание файла закончилось
            break;
        }
        if (ws.is_empty) { // если предложение пустое, то оно не считывается
            continue;
        }
        text.ptr[index++] = ws; // запись предложения в ячейку памяти
        if (index == mem_size - 1) { // если осталось мало выделенной памяти
            mem_size += TEXT_LEN;
            temp_ptr = realloc(text.ptr, sizeof(Sentence) * mem_size); // динамическое выделение памяти
            if (temp_ptr == NULL) {
                for (int j = 0; j < text.number_of_read_sent; j++) // особождение памяти от уже считанных предложении
                    free(text.ptr[j].ptr);
                free(text.ptr);
                text.ptr = NULL;
                return text;
            }
            text.ptr = temp_ptr; // полю text.ptr присваивается указатель на выделенную память
        }
        text.number_of_read_sent++; // увеличние кол-ва считанных предложении
    } while (1);
    if (text.number_of_read_sent == 0) { // если не было считано ни одного предложения
        free(text.ptr); // даже если ничего не было считано, память все же была выделена; ее нужно освободить
        return text; // количество считанных предложении используется как флаг
    }
    text.size = index;
    return text;
} // функция для считывания текста

//------------------------------------------

void DeleteSameSentences(Text* text_ptr) {
    int i = 0, j = 1; // индексы предложении, которые будут сравниваться
    while (i < text_ptr->size) { // индекс первого предложения меньше размера текста
        while (j < text_ptr->size) { // индекс второго предложения меньше размера текста
            if (!wcscasecmp((text_ptr->ptr)[i].ptr, (text_ptr->ptr)[j].ptr))  { // сравнение двух предложении
                free((text_ptr->ptr)[j].ptr); // освобождение памяти от повторяющегося предложения
                for (int k=j; k < text_ptr->size - 1; k++) // дополнительная переменная k для смещения
                    (text_ptr->ptr)[k] = (text_ptr->ptr)[k+1]; // удаление повторяющегося предложения путем
                // смещения следующих за ним предложении на одну ячейку влево
                (text_ptr->size)--; // уменьшение размера текста
                continue; // переход к следующей итерации для того, чтобы не увеличилось значение j
            }
            j++;
        }
        i++;
        j = i + 1; // чтобы не сраанивать предложение с самим собой
    }
} // функция для удаления повторяющихся предложении

//------------------------------------------

int SplitSentence(Sentence* ptr_sentence) {
    int size = WORDS_NUM; // начальный размер массива структур Word (в ячейка!)
    Word *words = (Word*) calloc(size, sizeof(Word)); // выделение памяти под массив структур Word
    if (words == NULL) {// если память не была выделена
        free(words);
        return 0;
    }
    int index = 0; // индекс, по которому будет записан указатель на слово, и счетчик слов
    wchar_t *state; // для функции wcstok (указатель на следующий токен)
    Word *temp_ptr; // в случае, если realloc вернет NULL
    wchar_t *word_ptr = wcstok(ptr_sentence->ptr, L" ", &state); // нахождение первого слова
    // предложение не пустые, это учитывается в ReadSentence и ReadText

    do {
        ptr_sentence->words_counter++; // счетчик слов увеличивается на единицу
        words[index].punc = 0; // изначально после слова ничего нет
        if (wcschr(L".!?,", word_ptr[wcslen(word_ptr) - 1])) { // если после слова стоит знак препинания
            words[index].punc = word_ptr[wcslen(word_ptr) - 1]; // сохранение знака препинания, который стоит после слова
            word_ptr[wcslen(word_ptr) - 1] = L'\0'; // удаление знака препинания
            }
        words[index++].ptr = word_ptr; // записываем в поле ptr структуры указатель на слово
        if (index == size - 1) { // если остается мало памяти
            size += WORDS_NUM;
            temp_ptr = (Word*) realloc(words, size * sizeof(Word)); // релокация выделенной памяти
            if (temp_ptr == NULL) { // если релокация памяти не удалась
                free(words);
                return 0;
            }
            words = temp_ptr; // сохранение указателя на выделенный блок памяти
        }
        word_ptr = wcstok(NULL, L" ", &state); // вновь находи слово
        if (word_ptr == NULL) // если все слова найдены
            break;
    } while (1);

    ptr_sentence -> words = words; // сохранение массива структур Word в поле структуры Sentence
    return 1;
} // функция для разбиения предложения на слова
// (для текста необходимо применить в цикле)

//------------------------------------------

int SplitText(Text* text_ptr) {
    for (int i=0; i < text_ptr -> size; i++) {
        if (SplitSentence((text_ptr -> ptr) + i)) {
            text_ptr -> number_of_splited_sent++; // считывание количества разделенных предложении
            continue;
        }
        else { // особождение памяти, если не удалось разбить на слова все предложения
            for (int j=0; j < text_ptr -> number_of_splited_sent; j++) // все предложения, которые удалось разбить на слова
                free((text_ptr -> ptr)[j].words);
            for (int j=0; j < text_ptr -> size; j++) // все считанные предложения
                free((text_ptr -> ptr)[j].ptr);
            free(text_ptr -> ptr);
            return 0;
        }
    }
    return 1;
} // функция для разбиения текста на слова

//------------------------------------------

int CompareSentences(const void* sent_1, const void* sent_2) {
    if (((((Sentence*)sent_1) -> words_counter) > 2) && ((((Sentence*)sent_2) -> words_counter) > 2)) { // если длина
        // обоих предложении больше 2
        if (wcslen((((Sentence*) sent_1) -> words)[2].ptr) > wcslen((((Sentence*) sent_2) -> words)[2].ptr))
            return 1;
        if (wcslen((((Sentence*) sent_1) -> words)[2].ptr) < wcslen((((Sentence*) sent_2) -> words)[2].ptr))
            return -1;
        if (wcslen((((Sentence*) sent_1) -> words)[2].ptr) == wcslen((((Sentence*) sent_2) -> words)[2].ptr))
            return 0;
        }
    if ((((Sentence*)sent_1)->words_counter) > 2) { // если только 1-ое предложение имеет длину больше, чем 2
        return 1;
    }
    if ((((Sentence*)sent_2)->words_counter) > 2) { // если только 2-ое предложение имеет длину больше, чем 2
        return -1;
    }
    return 0; // если оба предложения имеют длину меньше 3
} // функция-компаратор для предложении

//------------------------------------------

void ReplaceWords(Text text) {
    if (text.size > 1) {
        for (int i = 1; i < (text.size); i++) {
            if ((text.ptr)[i - 1].words_counter > 1) // если длина предыдущего предложения не менее 2
                (text.ptr)[i].words[0].ptr = (text.ptr)[i - 1].words[1].ptr; // первое слово из текущего предложения
                // заменяется на второе слово из предыдущего
            else // если длина предыдущего предложения равна 1
                (text.ptr)[i].words[0].ptr = (text.ptr)[i - 1].words[0].ptr;
        }
        if ((text.ptr)[(text.size) - 1].words_counter > 1) // если длина последнего предложения не менее 2
            (text.ptr)[0].words[0].ptr = (text.ptr)[(text.size) - 1].words[1].ptr; // первое слово из первого предложения
            // заменяется на второе слово из последнего
        else // если длина последнего предложения равна 1
            (text.ptr)[0].words[0].ptr = (text.ptr)[(text.size) - 1].words[0].ptr;

        WriteText(text); // вывод текста

    } else
        wprintf(L"Длина текста слишком мала.\n");
} // функция для перестановки слов (пункт 1); выводит измененный текст

//------------------------------------------

int HaveDigitsInMiddle(wchar_t* string) {
    if (wcslen(string) > 2) { // допустимая длина слова 2, в ином случае для него не опредлено понятие "середина"
        for (unsigned long i = 1; i < (wcslen(string) - 1); i++) { // проверяем все символы от первого до предпоследнего
            if (iswdigit(string[i])) // является ли символ цифрой
                return 1;
        }
    }
    return 0; // если длина предложения меньше 2 или если в строке не было найдено цифры
} // функция, проверяющая, есть ли в середине слова цифра

//------------------------------------------

void PaintWords(Text text) {
    wprintf(L"------------------------------\n");
    int have_digits = 0; // флаг, значение которого указывает, есть ли в предложении искомые слова
    for (int i = 0; i < text.size; i++) { // проходим через каждое предложение
        for (int j = 0; j < text.ptr[i].words_counter; j++) { // проверяем, есть ли в предложении искомые слова
            if (HaveDigitsInMiddle(text.ptr[i].words[j].ptr)) {
                have_digits = 1; // искомое слово найдено
                break;
            }
        }
        if (have_digits) {
            for (int j = 0; j < text.ptr[i].words_counter; j++) {
                if (HaveDigitsInMiddle(text.ptr[i].words[j].ptr))
                    wprintf(L"%s%ls%s", GREEN, text.ptr[i].words[j].ptr, NONE);
                else
                    wprintf(L"%ls", text.ptr[i].words[j].ptr);
                if (text.ptr[i].words[j].punc) // если после слова есть знак препинания, то он печатается
                    wprintf(L"%lc", text.ptr[i].words[j].punc);
                if (j == text.ptr[i].words_counter - 1)
                    wprintf(L"\n"); // переход на новую строку, когда предложение закончилось
                else
                    wprintf(L" "); // пробел между словами, если предложение еще не закончилось
            }
            have_digits = 0;
        }
    }
    wprintf(L"------------------------------\n");
} // функция, выделяющая зелёным цветом слова,
// у которых в середине есть хотя бы одна цифра (пункт 3); выводит текст

//------------------------------------------

void WriteText(Text text) {
    wprintf(L"------------------------------\n");
    for (int i=0; i < text.size; i++) { // проходим по каждому предложению
        // wprintf(L"Предложение номер %d: ", i);
        for (int j = 0; j < text.ptr[i].words_counter - 1; j++) {
            wprintf(L"%ls", text.ptr[i].words[j].ptr); // печтатается слово
            if (text.ptr[i].words[j].punc)
                wprintf(L"%lc", text.ptr[i].words[j].punc); // если есть, то знак препинания тоже печатается
            wprintf(L" "); // пробел между словами
        }
        wprintf(L"%ls", text.ptr[i].words[text.ptr[i].words_counter - 1].ptr); // печатается последнее слово
        if (text.ptr[i].words[text.ptr[i].words_counter - 1].punc) // и знак пунктуации
            wprintf(L"%lc", text.ptr[i].words[text.ptr[i].words_counter - 1].punc);
        if (text.ptr[i].have_newline) // если предложение оканчивается на L'\n'
            wprintf(L"\n");
        else
            wprintf(L" ");
    }
    wprintf(L"\n------------------------------\n");
} // функция для вывода текста

//------------------------------------------

void SortSentences(Text text) {
    qsort(text.ptr, text.size, sizeof(Sentence), CompareSentences);

    wprintf(L"------------------------------\n");
    for (int i=0; i < text.size; i++) { // вывод текста
        wprintf(L"Предложение номер %d: ", i);
        for (int j = 0; j < text.ptr[i].words_counter; j++) {
            wprintf(L"%ls", text.ptr[i].words[j].ptr); // печатается слово
            if (text.ptr[i].words[j].punc) // если есть знак препинания, то он печатается
                wprintf(L"%lc", text.ptr[i].words[j].punc);
            if (j < text.ptr[i].words_counter - 1)
                wprintf(L" ");
            else
                wprintf(L"\n");
        }
    }
    wprintf(L"------------------------------\n");
} // функция для сортировки предложении

//------------------------------------------

void RepeatingLettersToSingle(wchar_t* word) {
    int i = 0; // индекс текущего символа
    unsigned long word_len = wcslen(word); // длина слова
    while (i < word_len - 1) { // рассматриваем все символы до препоследнего
        if (CASE_SENS(word[i]) == CASE_SENS(word[i + 1])) { // если текущий и следующий за ним символы совпадают
            // (учитывать регистр символов или нет определяется макроопределением)
            for (unsigned long j = i + 1; j < word_len - 1; j++) // удаление следующего символа с помощью
                // смещения всех остальных (правее следующего) символов на одну ячейку влево
                word[j] = word[j + 1]; // замена текщуего символа следующим за ним
            word_len--; // уменьшение длины слова
            word[word_len] = L'\0'; // вместо последнего симовола L'\0'
            continue; // если символы совпали, то не увеличиваем индекс текущего элекмента i
        }
        i++; // инкрементация индекса
    }
}

//------------------------------------------

void RepeatingToSingle(Text text) {
    for (int i = 0; i < text.size; i++) { // проходим по каждому предложению
        for (int j = 0; j < text.ptr[i].words_counter; j++) { // проходим по каждому слову
            RepeatingLettersToSingle(text.ptr[i].words[j].ptr); // решаем подзадачу для каждого слова
        }
    }

    WriteText(text);
} // функция для замены подряд повторяющихся символов на один такой символ (пункт 4)
