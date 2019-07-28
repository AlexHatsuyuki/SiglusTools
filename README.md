# SiglusTools
A tool that can extract resources used by SiglusEngine and repack some of them for translation.

# SiglusTools
Набор утилит для распаковки ресурсов используемых в SiglusEngine и их перепаковки, в том числе и для перевода.

## VAScriptArchiver (vasa)
Утилита для перепаковки Scene.pck
Программа распаковывает Scene.pck в своего рода проект состоящий из:
1. файла проекта - info.xml
2. файлов с байт-кодом \*.slbc
3. файлов с игровым текстом \*.txt

Данный проект можно запаковать обратно в Scene.pck

Также поддерживается конвертация базы данных dbs в csv-файл.

## VAGConv (vagconv)
Конвертер графических файлов.

Конвертирует изображения из формата g00 в стандартные png, bmp, jpg и наоборот, создаёт из png/bmp фалов, файлы формата g00.

## SiglusDebugger
SiglusDebugger3.dll - библиотека активирующая режим отладки в SiglusEngine.
