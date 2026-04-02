![Скриншот_1](1.png)
![Скриншот_2](2.png)
![Скриншот_3](3.png)
# 1 шаг  
Зависимости:  
`sudo apt install pkg-config`  
`sudo apt install libgtk-3-dev`  
# 2 шаг
Собрать всё (консоль + GUI + тесты + бенчмарки)  
`make all`  
Собрать только консольную версию  
`make console`  
Собрать только GUI  
`make gui`  
Собрать только edge-тесты  
`make edge-tests`  
Собрать только бенчмарки  
`make benchmark`  
Очистить сборку  
`make clean`  
# 3 шаг
Консольные тесты  
`make run` или `./bin/matrix_app `  
Графический интерфейс  
`make run-gui` или `./bin/matrix_gui_app`  
Edge-тесты (граничные случаи)  
`make run-edge` или `./bin/matrix_edge_tests`  
Бенчмарки (сравнение методов СЛАУ)  
`make run-bench` или `./bin/matrix_benchmark`  
Сохранить результаты в файл для отчёта  
`./bin/matrix_benchmark > results.txt`
