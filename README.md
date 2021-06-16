# SimpleSheet

SimpleSheet - упрощенный аналог существующих таблиц(Microsoft Excel или Google Sheets). В ячейках таблицы могут быть текст или формулы. Формулы, как и в существующих решениях, могут содержать индексы ячеек. Формулы разбираются при помощи специальной программы ANTLR, которая генерирует код лексического и синтаксического анализаторов, а также код для обхода дерева разбора на С++.

## Требования
* C++17 и выше
* [Java SE Runtime Environment 8](https://www.oracle.com/java/technologies/javase-jre8-downloads.html)
* [ANTLR](https://www.antlr.org/) (ANother Tool for Language Recognition)

## Порядок сборки
1. Установить [Java SE Runtime Environment 8](https://www.oracle.com/java/technologies/javase-jre8-downloads.html).
2. Установить [ANTLR](https://www.antlr.org/) (ANother Tool for Language Recognition), выполнив все пункты в меню Quick Start.
  2.1. В случае установки на Windows может быть полезно данное [видеоъ(https://youtu.be/p2gIBPz69DM).
3. Проверить в файлах FindANTLR.cmake и CMakeLists.txt название файла antlr-X.X.X-complete.jar на корректность версии. Вместо "X.X.X" указать свою версию antlr.
4. Создайть папку с названием "antlr4_runtime" без кавычек и скачайть в неё [файлы](https://github.com/antlr/antlr4/tree/master/runtime/Cpp).
5. Запустить cmake build с CMakeLists.txt.
