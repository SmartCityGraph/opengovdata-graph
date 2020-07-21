#include <iostream>
#include <string>
#include <fstream>
#include <locale>
#include <Windows.h>
#include <vector>
#include <stdlib.h>
#include <map>
using namespace std;

// Объявляем файл основного вывода, карту  для того, чтобы иметь к нему доступ из любой точки кода.
ofstream OKTMO_ontology_object_database;
vector <vector <string>> capitals;

// Метод поиска позиции определённого вхождения определённого символа. Возвращает нулевой символ при поиске
// нулевого вхождения символа, а также последний символ, если вхождение символа больше чем количество
// искомых символов в строке. Этот метод игнорирует искомые символы, если они находятся в скобках или кавычках.
int char_pos(string line, char char_needed, int order)
{
    if (order == 0) {
        return -1;
    }
    int char_counter = 0;
    int quotes_count = 0;
    int brackets_count = 0;
    int i = 0;
    while (line[i] != '\0')
    {
        if (((quotes_count % 2) == 0) && ((brackets_count % 2) == 0)) 
        {
            if (line[i] == char_needed)
            {
                char_counter++;
            }
            if (char_counter == order)
            {
                return i;
            }
        }
        if (line[i] == '\"')
        {
            quotes_count++;
        }
        if ((line[i] == '(') || (line[i] == ')'))
        {
            brackets_count++;
        }
        i++;
    }
    return line.length();
}

// Функция экранирует все кавычки, помимо первой и последней
string clean_quotes(string line) 
{
    int quotes_count_now = 0;
    int quotes_count = 0;
    int line_length = line.length();
    for (int i = 0; i < line_length; i++) 
    {
        if (line[i] == '\"') 
        {
            quotes_count++;
        }
    }
    for (int i = 0; i < line_length; i++) 
    {
        if (line[i] == '\"') 
        {
            quotes_count_now++;
            if (quotes_count_now > 1 && quotes_count_now < quotes_count) 
            {
                line = line.substr(0, i) + '\\' + line.substr(i, line_length - i);
                line_length++;
                i++;
            }
        }
    }
    return line;
}

// Функция достаёт определённый атрибут из csv строки, в нашем случае атрибуты разделены ';'. 
// Если атрибут пустой, то функция возвращает "null".
string atribut(int atr_num, string line)
{
    int first_char = char_pos(line, ';', atr_num - 1) + 1;
    int second_char = char_pos(line, ';', atr_num);
    line = line.substr(first_char, (second_char - first_char));
    if (line.length() == 0) 
    {
        return "null";
    }
    else 
    {
        return line;
    }
}

// Функция возвращает класс объекта ОКТМО относящегося не к автономному округу или округу 
// (не возвращает подкласс региона, необходимо указывать вручную)
string get_class(map <string, string> object) 
{
    if (object["second_level_code"] == "000" && object["third_level_code"] == "000" && object["fourth_level_code"] == "000") 
    {
        return "Region";
    }
    if (object["fourth_level_code"] != "000") 
    {
        return "Locality";
    }
    if (object["third_level_code"] == "000" && object["fourth_level_code"] == "000")
    {
        if (object["second_level_code"].substr(0, 1) == "3" || object["second_level_code"].substr(0, 1) == "9") {
            return "IntracityAreas";
        }
        if (object["second_level_code"].substr(0, 1) == "5") {
            return "MunicipalOkrug";
        }
        if (object["second_level_code"].substr(0, 1) == "6") {
            return "MunicipalDistrict";
        }
        if (object["second_level_code"].substr(0, 1) == "7") {
            return "UrbanDistrict";
        }
    }
    if (object["third_level_code"].substr(0, 1) == "1") {
        return "UrbanSettlements";
    }
    if (object["third_level_code"].substr(0, 1) == "3") {
        return "IntracityMunicipality";
    }
    if (object["third_level_code"].substr(0, 1) == "4") {
        return "RuralSettlements";
    }
    if (object["third_level_code"].substr(0, 1) == "7") {
        return "InterSettlementTerritory";
    }
}
// Возвращает класс объекта ОКТМО относящегося к автономному округ или округу 
// (не возвращает класс "Округ" или подклассы класса "Регион" нужно прописывать вручную)
string get_autonomous_class(map <string, string> object)
{

    if (object["second_level_code"].substr(1,2) == "00" && object["third_level_code"] == "000" && object["fourth_level_code"] == "000")
    {
        return "Region";
    }
    if (object["fourth_level_code"] != "000")
    {
        return "Locality";
    }
    if (object["third_level_code"] == "000" && object["fourth_level_code"] == "000")
    {
        int second_level_value = atoi(object["second_level_code"].substr(1, 2).c_str());
        if (second_level_value > 10 && second_level_value < 30) {
            return "MunicipalDistrict";
        }
        if (second_level_value > 30 && second_level_value < 50) {
            return "MunicipalOkrug";
        }
        if (second_level_value > 50 && second_level_value < 99) {
            return "UrbanDistrict";
        }
    }
    if (object["third_level_code"].substr(0, 1) == "1") {
        return "UrbanSettlements";
    }
    if (object["third_level_code"].substr(0, 1) == "3") {
        return "IntracityMunicipality";
    }
    if (object["third_level_code"].substr(0, 1) == "4") {
        return "RuralSettlements";
    }
    if (object["third_level_code"].substr(0, 1) == "7") {
        return "InterSettlementTerritory";
    }
}

// Функция возвращает "1", если объект не является ОКТМО списком, и "0", если является 
// (для объектов не автономных округов и не округов)
boolean OKTMO_clean (map <string, string> object)
{
    if ((object["fourth_level_code"] == "000" && object["section"] == "2") 
        || (object["second_level_code"].substr(0, 1) != "0" && object["second_level_code"].substr(1, 2) == "00")
        || (object["third_level_code"].substr(0, 1) != "0" && object["third_level_code"].substr(1, 2) == "00")) 
    {
        return 0;
    }
    return 1;
}
// Функция возвращает "1", если объект не является ОКТМО списком, и "0", если является
// (для объектов автономных округов и округов).
boolean OKTMO_autonomous_clean(map <string, string> object)
{
    if ((object["fourth_level_code"] == "000" && object["section"] == "2")
        || (object["second_level_code"].substr(1, 1) == "1" && object["second_level_code"].substr(2, 1) == "0")
        || (object["second_level_code"].substr(1, 1) == "3" && object["second_level_code"].substr(2, 1) == "0")
        || (object["second_level_code"].substr(1, 1) == "5" && object["second_level_code"].substr(2, 1) == "0")
        || (object["third_level_code"].substr(0, 1) != "0" && object["third_level_code"].substr(1, 2) == "00"))
    {
        return 0;
    }
    return 1;
}

// Функция ищет административный центр объекта в дополнительном файле ввода.
string getCapital(map <string, string> object) 
{
    string potential_capital;
    vector <string> potential_capitals;
    string capital;
    ifstream OKTMO_capital_database("OKTMO_capital_database.csv");
    int region = atoi(object["first_level_code"].c_str());
    // Находим все административные центры в данном регионе, у которого название совпадает с административным центром объекта.
    for (int i = 0; i < capitals[region].size(); i++) 
    {
        potential_capital = capitals[region][i];
        capital = atribut(2, potential_capital);
        if (capital == object["capital"])
        {
                potential_capitals.push_back(potential_capital.substr(0, 11));
        }
    }
    // Осматриваем получившийся вектор на соответствие кодировке объекта, приоритетом идут те административные центры, чей ОКТМО 
    // код больше совпадает с ОКТМО кодом объекта
    for (int i = 0; i < potential_capitals.size(); i++) 
    {
        if (object["first_level_code"] + object["second_level_code"] + object["third_level_code"] ==
            potential_capitals[i].substr(0, 8)) 
        {
            OKTMO_capital_database.close();
            return potential_capitals[i];
        }
    }

    for (int i = 0; i < potential_capitals.size(); i++)
    {
        if (object["first_level_code"] + object["second_level_code"] == potential_capitals[i].substr(0, 5))
        {
            OKTMO_capital_database.close();
            return potential_capitals[i];
        }
    }

    for (int i = 0; i < potential_capitals.size(); i++)
    {
        if (object["first_level_code"] == potential_capitals[i].substr(0, 8))
        {
            OKTMO_capital_database.close();
            return potential_capitals[i];
        }
    }
    OKTMO_capital_database.close();
    return "null";
}

// Разбивает строчку на атрибуты и создаёт массив с ключами для удобной работы с кодом и его человеко-читаемости.
map <string, string> create_OKTMO_atributs_map (string object)
{
    map <string, string> OKTMO_atributs;
    OKTMO_atributs["first_level_code"] = (atribut(1, object)).substr(1, 3);
    OKTMO_atributs["second_level_code"] = (atribut(2, object)).substr(1, 3);
    OKTMO_atributs["third_level_code"] = (atribut(3, object)).substr(1, 3);
    OKTMO_atributs["fourth_level_code"] = (atribut(4, object)).substr(1,3);
    OKTMO_atributs["section"] = (atribut(6, object)).substr(1,1);
    OKTMO_atributs["name"] = atribut(7, object);
    OKTMO_atributs["capital"] = atribut(8, object);
    OKTMO_atributs["description"] = atribut(9, object);
    OKTMO_atributs["change_number"] = atribut(10, object);
    OKTMO_atributs["change_type"] = atribut(11, object);
    OKTMO_atributs["acceptance_date"] = atribut(12, object);
    OKTMO_atributs["initiation_date"] = atribut(13, object);
    return OKTMO_atributs;
}

void OKTMO_object_create (map <string, string> OKTMO_atributs, string OKTMO_class, string includedIn,
    string full_code, string value_code)
{
    // Начинаем создание TTL кода.
    OKTMO_ontology_object_database << "###  https://w3id.org/opengovdataOKTMO_" << full_code << endl
        << ":OKTMO_" << full_code << " rdf:type owl:NamedIndividual ," << endl
        << "OGD:" << OKTMO_class << " ;" << endl
        << "OGD:hasFullCode " << full_code << " ;" << endl
        << "OGD:hasValueCode " << OKTMO_atributs[value_code] << " ;" << endl;

    if (includedIn != "")
    {
        OKTMO_ontology_object_database << "OGD:includedIn :OKTMO_" << includedIn << " ;" << endl;
    }

    if (OKTMO_atributs["capital"] != "null" && getCapital(OKTMO_atributs) != "null")
    {
        OKTMO_ontology_object_database << "OGD:hasCapital :OKTMO_" << getCapital(OKTMO_atributs) << " ;" << endl;
    }

    if (OKTMO_atributs["description"] != "null")
    {
        OKTMO_ontology_object_database << "hasDescription: " << clean_quotes(OKTMO_atributs["description"]) << " ;" << endl;
    }

    if (OKTMO_atributs["change_number"] != "null" && OKTMO_atributs["change_number"] != "\"000\"")
    {
        OKTMO_ontology_object_database << "OGD:hasChangeNumber " << OKTMO_atributs["change_number"] << " ;" << endl;
        OKTMO_ontology_object_database << "OGD:hasChangeType " << OKTMO_atributs["change_type"] << " ;" << endl;
    }

    if (OKTMO_atributs["acceptance_date"] != "null" && OKTMO_atributs["acceptance_date"] != "14.06.2013")
    {
        OKTMO_ontology_object_database << "OGD:hasAcceptanceDate \"" << OKTMO_atributs["acceptance_date"] << "\" ;" << endl;
    }

    if (OKTMO_atributs["initiation_date"] != "null" && OKTMO_atributs["initiation_date"] != "01.01.2014")
    {
        OKTMO_ontology_object_database << "OGD:hasInitiationDate \"" << OKTMO_atributs["initiation_date"] << "\" ;" << endl;
    }

    OKTMO_ontology_object_database << "rdfs:label " << OKTMO_atributs["name"] << " ." << endl << endl << endl;
}

// Метод создаёт объекты не автономных округов и округов.
void OKTMO_non_autonomous_identification (string object) 
{
    map <string, string> OKTMO_atributs = create_OKTMO_atributs_map(object);
    if (OKTMO_clean(OKTMO_atributs))
    {   
        // Определяем полный и значимый код объекта, а также объект-родитель.
        string OKTMO_class = get_class(OKTMO_atributs);
        string full_code = OKTMO_atributs["first_level_code"] + OKTMO_atributs["second_level_code"]
            + OKTMO_atributs["third_level_code"];
        string included_in;
        string value_code = "first_level_code";
        if (OKTMO_atributs["second_level_code"] != "000") {
            value_code = "second_level_code";
            included_in = OKTMO_atributs["first_level_code"] + "000000";
        }
        if (OKTMO_atributs["third_level_code"] != "000") {
            value_code = "third_level_code";
            included_in = OKTMO_atributs["first_level_code"] + OKTMO_atributs["second_level_code"] + "000";
        }
        if (OKTMO_atributs["fourth_level_code"] != "000") {
            value_code = "fourth_level_code";
            included_in = full_code;
            full_code = full_code + OKTMO_atributs["fourth_level_code"];
        }
        OKTMO_object_create(OKTMO_atributs, OKTMO_class, included_in, full_code, value_code);
    }      
}
// Метод создаёт объекты автономных округов и округов.
void OKTMO_autonomous_identification(string object)
{
    map <string, string> OKTMO_atributs = create_OKTMO_atributs_map(object);
    if (OKTMO_autonomous_clean(OKTMO_atributs))
    {
        // Определяем полный и значимый код объекта, а также объект-родитель.
        string OKTMO_class = get_autonomous_class(OKTMO_atributs);
        string full_code = OKTMO_atributs["first_level_code"] + OKTMO_atributs["second_level_code"]
            + OKTMO_atributs["third_level_code"];
        string included_in = OKTMO_atributs["first_level_code"] + "000000";
        string value_code = "second_level_code";
        if (OKTMO_atributs["second_level_code"].substr(1,2) != "00") {
            included_in = OKTMO_atributs["first_level_code"] + OKTMO_atributs["second_level_code"].substr(0,1) + "00000";
        }
        if (OKTMO_atributs["third_level_code"] != "000") {
            value_code = "third_level_code";
            included_in = OKTMO_atributs["first_level_code"] + OKTMO_atributs["second_level_code"] + "000";
        }
        if (OKTMO_atributs["fourth_level_code"] != "000") {
            value_code = "fourth_level_code";
            included_in = full_code;
            full_code = full_code + OKTMO_atributs["fourth_level_code"];
        }
        OKTMO_object_create(OKTMO_atributs, OKTMO_class, included_in, full_code, value_code);
    }
}

// Создаёт список административных центров в виде массиве векторов, где каждая строка i-того номера - административные центры МО,
// региона с i-тым номером в ОКТМО.
void create_capitals_note()
{
    ifstream OKTMO_capital_database("OKTMO_capital_database.csv");
    int region_number = 1;
    string object;
    vector <string> a;
    getline(OKTMO_capital_database, object);
    for (int i = 0; i < 100; i++)
    {
        a.clear();
        while (region_number == i) 
        {
            a.push_back(object);
            getline(OKTMO_capital_database, object);
            if (atoi(object.substr(0, 2).c_str()) != i)
            {
                region_number++;
            }
        }
        capitals.push_back(a);
    }
}

int main()
{
    // Тут мы будем хранить csv-строчку ОКТМО
    string object;
    // Количество объектов ОКТМО помимо автономных округов/округов
    int OKTMO_amount;
    // Количество объектов ОКТМО автономных округов/округов
    int OKTMO_autonomous_amount;
    // Открываем базу данных, в которой должны сперва идти все объекты неавтономных округов/округов,
    // а затем все объекты автономных округов/округов. А также открываем файл записи наших объектов.
    ifstream OKTMO_database("OKTMO_database.csv");
    OKTMO_ontology_object_database.open("OKTMO_ontology_object_database.csv");
    // Создаём карту административных центров
    create_capitals_note();
    // Пропускаем служебную строчку
    getline(OKTMO_database, object);
    // Запрашиваем количество объектов
    cout << "Enter the number of non-autonomous okrug / non-okrug objects.";
    cin >> OKTMO_amount;
    cout << "Enter the number of autonomous okrug / okrug objects.";
    cin >> OKTMO_autonomous_amount;
    // Создаём объекты неавтономных округов/округов
    for (int i = 0; i < OKTMO_amount; i++) {      
        getline(OKTMO_database, object);
        OKTMO_non_autonomous_identification (object);
        if (i % 100 == 0) {
            cout << i;
        }
    }
    // Создаём объекты автономных округов/округов
    for (int i = 0; i < OKTMO_autonomous_amount; i++) {
        string object;
        getline(OKTMO_database, object);
        OKTMO_autonomous_identification(object);
    }
    // Закрываем файлы
    OKTMO_database.close();
    OKTMO_ontology_object_database.close();
}