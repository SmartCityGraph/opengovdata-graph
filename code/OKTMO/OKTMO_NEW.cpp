﻿#include <iostream>
#include <string>
#include <fstream>
#include <Windows.h>
#include <vector>
#include <stdlib.h>
#include <map>
using namespace std;

// Объявляем файл основного вывода, карту  для того, чтобы иметь к нему доступ из любой точки кода.
ofstream OKTMO_ontology_object_database;
vector <vector <string>> capitals;
string locality_type_list[44][3];

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
    string potential_capital; // Объект, который мы получаем из карты административных центров
    string potential_capital_name; // Имя административного центра из карты
    vector <string> potential_capitals; // Список подходящих по названию и региону объектов в виде их ОКТМО кодов
    int region = atoi(object["first_level_code"].c_str()); // Код региона объекта
    // Находим все административные центры в данном регионе, у которого название совпадает с административным центром объекта.
    for (int i = 0; i < capitals[region].size(); i++) 
    {
        potential_capital = capitals[region][i];
        potential_capital_name = clean_quotes(atribut(2, potential_capital));
        if (potential_capital_name == object["capital"])
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
            return potential_capitals[i];
        }
    }

    for (int i = 0; i < potential_capitals.size(); i++)
    {
        if (object["first_level_code"] + object["second_level_code"] == potential_capitals[i].substr(0, 5))
        {
            return potential_capitals[i];
        }
    }

    for (int i = 0; i < potential_capitals.size(); i++)
    {
        if (object["first_level_code"] == potential_capitals[i].substr(0, 2))
        {
            return potential_capitals[i];
        }
    }
    return "null";
}

// Метод ищет по ключевым словам в названии объекта ОКТМО его тип и возвращает все типы (если их несколько)
vector <string> get_locality_type(map <string, string> OKTMO_atributs)
{
    vector <string> object_locality_type_list;
    for (int i = 0; i < 44; i++) 
    {
        if ((OKTMO_atributs["name"].find(" " + locality_type_list[i][0] +" ") != -1)
            || (OKTMO_atributs["name"].find("\"" + locality_type_list[i][0] + " ") != -1)
            || (OKTMO_atributs["name"].find(" " + locality_type_list[i][0] + "\"") != -1)
            || (OKTMO_atributs["name"].find(" " + locality_type_list[i][0] + ".") != -1)
            ) 
        {
            object_locality_type_list.push_back(locality_type_list[i][1]);
        }
    }
    return object_locality_type_list;
}

// Разбивает строчку на атрибуты и создаёт массив с ключами для удобной работы с кодом и его человеко-читаемости.
map <string, string> create_OKTMO_atributs_map (string object)
{
    map <string, string> OKTMO_atributs;
    OKTMO_atributs["first_level_code"] = (atribut(1, object)).substr(1, 2);
    OKTMO_atributs["second_level_code"] = (atribut(2, object)).substr(1, 3);
    OKTMO_atributs["third_level_code"] = (atribut(3, object)).substr(1, 3);
    OKTMO_atributs["fourth_level_code"] = (atribut(4, object)).substr(1,3);
    OKTMO_atributs["section"] = (atribut(6, object)).substr(1,1);
    OKTMO_atributs["name"] = clean_quotes(atribut(7, object));
    OKTMO_atributs["capital"] = clean_quotes(atribut(8, object));
    OKTMO_atributs["description"] = clean_quotes(atribut(9, object));
    OKTMO_atributs["change_number"] = atribut(10, object);
    OKTMO_atributs["change_type"] = atribut(11, object);
    OKTMO_atributs["acceptance_date"] = atribut(12, object);
    OKTMO_atributs["initiation_date"] = atribut(13, object);
    return OKTMO_atributs;
}

// Этот метод создаёт TTL объект на основе полученных данных и записывает его в файл вывода
void OKTMO_object_create (map <string, string> OKTMO_atributs, string OKTMO_class, string included_in,
    string full_code, string value_code)
{
    OKTMO_ontology_object_database << "###  https://w3id.org/opengovdata#OKTMO_" << full_code << endl
        << ":OKTMO_" << full_code << " rdf:type owl:NamedIndividual ," << endl
        << "OGD:" << OKTMO_class << " ;" << endl
        << "OGD:hasFullCode " << full_code << " ;" << endl
        << "OGD:hasValueCode " << OKTMO_atributs[value_code] << " ;" << endl;

    if (included_in != "")
    {
        OKTMO_ontology_object_database << "OGD:includedIn :OKTMO_" << included_in << " ;" << endl;
    }

    if (OKTMO_atributs["capital"] != "null") 
    {
        string OKTMO_capital = getCapital(OKTMO_atributs);
        if (OKTMO_capital != "null")
        {
            OKTMO_ontology_object_database << "OGD:hasCapital :OKTMO_" << OKTMO_capital << " ;" << endl;
        }
    }

    if (OKTMO_atributs["description"] != "null")
    {
        OKTMO_ontology_object_database << "OGD:hasDescription " << OKTMO_atributs["description"] << " ;" << endl;
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
    
    if (OKTMO_atributs["fourth_level_code"] != "000")
    {
        vector <string> object_locality_type_list = get_locality_type(OKTMO_atributs);
        for (int i = 0; i < object_locality_type_list.size(); i++)
        {
            OKTMO_ontology_object_database << "OGD:hasLocalityType :" << object_locality_type_list[i] << " ;" << endl;
        }
    }

    if ((OKTMO_atributs["description"].find("ЗАТО") != -1) || (OKTMO_atributs["name"].find("ЗАТО") != -1))
    {
        OKTMO_ontology_object_database << "OGD:isZATO : \"true\"^^xsd:boolean ;" << endl;
    }


    OKTMO_ontology_object_database << "rdfs:label " << OKTMO_atributs["name"] << " ." << endl << endl << endl;
}

// Метод определяет некоторые важные атрибуты объектов не автономных округов и округов и посылает команду создать объект.
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
// Метод определяет некоторые важные атрибуты объектов автономных округов и округов и посылает команду создать объект.
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
    string object_capital;
    vector <string> a;
    getline(OKTMO_capital_database, object_capital);
    for (int i = 0; i < 100; i++)
    {
        a.clear();
        while (region_number == i) 
        {
            a.push_back(object_capital);
            getline(OKTMO_capital_database, object_capital);
            if (atoi(object_capital.substr(0, 2).c_str()) != i)
            {
                region_number++;
            }
        }
        capitals.push_back(a);
    }
    OKTMO_capital_database.close();
}

// Создание массива типов населённых пунктов
void create_locality_type_list()
{
    string locality_type;
    ifstream OKTMO_loacality_type_database("OKTMO_locality_type_base.csv");
    for (int i = 0; i < 44; i++) {
        getline(OKTMO_loacality_type_database, locality_type);
        locality_type_list[i][0] = atribut(1, locality_type);
        locality_type_list[i][1] = atribut(2, locality_type);
    }
    OKTMO_loacality_type_database.close();
}

int main()
{
    // Тут мы будем хранить csv-строчку ОКТМО
    string object;

    // Открываем базу данных и файл записи наших объектов.
    ifstream OKTMO_database("OKTMO_database.csv");
    OKTMO_ontology_object_database.open("OKTMO_ontology_object_database.ttl");
    // Создаём карту административных центров и массив типов населённых пунктов
    create_capitals_note();
    create_locality_type_list();
    // Пропускаем служебную строчку и получаем первую
    getline(OKTMO_database, object);
    getline(OKTMO_database, object);
    int k = 0;
    // Создаём объекты, пока не встретим пустую строчку
    while (object != "")
    {
        if ((object[6] == '8') || (object[6] == '9' && object.substr(1, 2) == "71"))
        {
            OKTMO_autonomous_identification(object);
        }
        else 
        {
            OKTMO_non_autonomous_identification(object);
        }
        getline(OKTMO_database, object);
        k++;
        if (k % 1000 == 0)
        {
            cout << k;
        }
    }
    // Закрываем файлы
    OKTMO_database.close();
    OKTMO_ontology_object_database.close();
}