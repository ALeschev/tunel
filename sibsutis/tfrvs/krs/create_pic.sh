#!/bin/bash

data_dir="odata"
pic_dir="pic"

if [ -e "$data_dir" ]
then
     echo "intput data dir: '$data_dir'"
else
     echo "ERROR: dir with data not found!"
     exit
fi

if [ -e "$pic_dir" ]
then
     echo "dir for graphics: '$pic_dir'"
else
     echo "dir for graphics not found.. create it!"
     mkdir $pic_dir
fi

gnuplot << EOP

datafile = "odata/o_1.0.data"

set terminal jpeg enhanced size 1080,710
set output "$pic_dir/o_1.0.jpg"
set title "Зависимость функции потенциальной живучести (N) от различных значений состояния системы (i)"
set grid x y

set style line 1 lt 1 pt 9
set style line 2 lt 3 pt 7
set style line 3 lt 2 pt 5
set style line 4 lt 4 pt 3
set style line 5 lt 5 pt 2

set yrange [0.99995:1.00001]
set xrange [0:8]
set ytics
set ylabel "Функция потенциальной живучести"
set xlabel "Время наработки системы"
plot datafile using 1:2 with linespoints linestyle 1 title "i = 65533",\
     datafile using 1:3 with linespoints linestyle 2 title "i = 65534",\
     datafile using 1:4 with linespoints linestyle 3 title "i = 65535",\
     datafile using 1:5 with linespoints linestyle 4 title "i = 65536"

EOP

gnuplot << EOP

datafile = "odata/o_2.0.data"

set terminal jpeg enhanced size 1080,710
set output "$pic_dir/o_2.0.jpg"
set title "Зависимость потенциальной живучести (N) от различных значений интенсивности отказов (λ)"
set grid x y

set style line 1 lt 1 pt 9
set style line 2 lt 3 pt 7
set style line 3 lt 2 pt 5
set style line 4 lt 4 pt 3
set style line 5 lt 5 pt 2

set yrange [0:1.2]
set xrange [0:22]
set ytics
set ylabel "Функция потенциальной живучести"
set xlabel "Время наработки системы"
plot datafile using 1:2 with linespoints linestyle 1 title "λ = 10^{-3} 1/ч",\
     datafile using 1:3 with linespoints linestyle 2 title "λ = 10^{-4} 1/ч",\
     datafile using 1:4 with linespoints linestyle 3 title "λ = 10^{-5} 1/ч",\
     datafile using 1:5 with linespoints linestyle 4 title "λ = 10^{-6} 1/ч"

EOP

gnuplot << EOP

datafile = "odata/o_3.0.data"

set terminal jpeg enhanced size 1080,710
set output "$pic_dir/o_3.0.jpg"
set title "Зависимость потенциальной живучести (N) от различных значений интенсивности восстановления (μ)"
set grid x y

set style line 1 lt 1 pt 9
set style line 2 lt 3 pt 7
set style line 3 lt 2 pt 5
set style line 4 lt 4 pt 3
set style line 5 lt 5 pt 2

set yrange [0:1.2]
set xrange [0:10]
set ytics
set ylabel "Функция потенциальной живучести"
set xlabel "Время наработки системы"
plot datafile using 1:2 with linespoints linestyle 1 title "μ = 1",\
     datafile using 1:3 with linespoints linestyle 2 title "μ = 10",\
     datafile using 1:4 with linespoints linestyle 3 title "μ = 100",\
     datafile using 1:5 with linespoints linestyle 4 title "μ = 1000"

EOP

gnuplot << EOP

datafile = "odata/o_4.0.data"

set terminal jpeg enhanced size 1080,710
set output "$pic_dir/o_4.0.jpg"
set title "Зависимость потенциальной живучести (N) от различных значений колличества восстанавливающих устройств (m)"
set grid x y

set style line 1 lt 1 pt 9
set style line 2 lt 3 pt 7
set style line 3 lt 2 pt 5
set style line 4 lt 4 pt 3
set style line 5 lt 5 pt 2

set yrange [0:1.2]
set xrange [0:22]
set ytics
set ylabel "Функция потенциальной живучести"
set xlabel "Время наработки системы"
plot datafile using 1:2 with linespoints linestyle 1 title "m = 1",\
     datafile using 1:3 with linespoints linestyle 2 title "m = 2",\
     datafile using 1:4 with linespoints linestyle 3 title "m = 3",\
     datafile using 1:5 with linespoints linestyle 4 title "m = 4"

EOP

gnuplot << EOP

datafile = "odata/o_1.1.data"

set terminal jpeg enhanced size 1080,710
set output "$pic_dir/o_1.1.jpg"
set title "Зависимость функции занятости (М) от различных значений состояний системы(i)"
set grid x y

set style line 1 lt 1 pt 9
set style line 2 lt 3 pt 7
set style line 3 lt 2 pt 5
set style line 4 lt 4 pt 3
set style line 5 lt 5 pt 2

set yrange [0:1.2]
set xrange [0:6]
set ytics
set ylabel "Функция занятости ВС"
set xlabel "Время наработки системы"
plot datafile using 1:2 with linespoints linestyle 1 title "i = 65533",\
     datafile using 1:3 with linespoints linestyle 2 title "i = 65534",\
     datafile using 1:4 with linespoints linestyle 3 title "i = 65535",\
     datafile using 1:5 with linespoints linestyle 4 title "i = 65536"

EOP

gnuplot << EOP

datafile = "odata/o_2.1.data"

set terminal jpeg enhanced size 1080,710
set output "$pic_dir/o_2.1.jpg"
set title "Зависимость функции занятости (М) от различных значений интенсивности отказов (λ)"
set grid x y

set style line 1 lt 1 pt 9
set style line 2 lt 3 pt 7
set style line 3 lt 2 pt 5
set style line 4 lt 4 pt 3
set style line 5 lt 5 pt 2

#set yrange [0:1.5]
#set xrange [0:22]
set ytics
set ylabel "Функция занятости ВС"
set xlabel "Время наработки системы"
plot datafile using 1:2 with linespoints linestyle 1 title "λ = 10^(-3)",\
     datafile using 1:3 with linespoints linestyle 2 title "λ = 10^(-4)",\
     datafile using 1:4 with linespoints linestyle 3 title "λ = 10^(-5)",\
     datafile using 1:5 with linespoints linestyle 4 title "λ = 10^(-6)"

EOP

gnuplot << EOP

datafile = "odata/o_3.1.data"

set terminal jpeg enhanced size 1080,710
set output "$pic_dir/o_3.1.jpg"
set title "Зависимость функции занятости (М) от различных значений интенсивности восстановления (μ)"
set grid x y

set style line 1 lt 1 pt 9
set style line 2 lt 3 pt 7
set style line 3 lt 2 pt 5
set style line 4 lt 4 pt 3
set style line 5 lt 5 pt 2

set xrange [0:8]
set ytics
set ylabel "Функция занятости ВС"
set xlabel "Время наработки системы"
plot datafile using 1:2 with linespoints linestyle 1 title "μ = 1",\
     datafile using 1:3 with linespoints linestyle 2 title "μ = 10",\
     datafile using 1:4 with linespoints linestyle 3 title "μ = 100",\
     datafile using 1:5 with linespoints linestyle 4 title "μ = 1000"

EOP

gnuplot << EOP

datafile = "odata/o_4.1.data"

set terminal jpeg enhanced size 1080,710
set output "$pic_dir/o_4.1.jpg"
set title "Зависимость функции занятости системы (M) от различных значений колличества востанавливающих устройств (m)"
set grid x y

set style line 1 lt 1 pt 9
set style line 2 lt 3 pt 7
set style line 3 lt 2 pt 5
set style line 4 lt 4 pt 3
set style line 5 lt 5 pt 2

#set yrange [0:1.5]
set xrange [0:8]
set ytics
set ylabel "Функция занятости ВС"
set xlabel "Время наработки системы"
plot datafile using 1:2 with linespoints linestyle 1 title "m = 1",\
     datafile using 1:3 with linespoints linestyle 2 title "m = 2",\
     datafile using 1:4 with linespoints linestyle 3 title "m = 3",\
     datafile using 1:5 with linespoints linestyle 4 title "m = 4"

EOP

echo "graphics was created"