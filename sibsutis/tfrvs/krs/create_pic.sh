#!/bin/bash

gnuplot << EOP

datafile = "o_1.0.data"

set terminal jpeg enhanced size 1080,710
set output "o_1.0.jpg"
set title "Зависимость от различных значений параметра lymbda"
set grid x y

set style line 1 lt 1 pt 9
set style line 2 lt 3 pt 7
set style line 3 lt 2 pt 5
set style line 4 lt 4 pt 3
set style line 5 lt 5 pt 2

set yrange [0:1.2]
set xrange [0:22]
set ytics
set ylabel "Среднее время восстановления живучей ВС, часы"
set xlabel "time"
plot datafile using 1:2 with linespoints linestyle 1 title "lyambda = 10^{-5} 1/ч",\
     datafile using 1:3 with linespoints linestyle 2 title "lyambda = 10^{-6} 1/ч",\
     datafile using 1:4 with linespoints linestyle 3 title "lyambda = 10^{-7} 1/ч",\
     datafile using 1:5 with linespoints linestyle 4 title "lyambda = 10^{-7} 1/ч"

EOP

gnuplot << EOP

datafile = "o_2.0.data"

set terminal jpeg enhanced size 1080,710
set output "o_2.0.jpg"
set title "Зависимость от различных значений параметра lymbda"
set grid x y

set style line 1 lt 1 pt 9
set style line 2 lt 3 pt 7
set style line 3 lt 2 pt 5
set style line 4 lt 4 pt 3
set style line 5 lt 5 pt 2

set yrange [0:1.2]
set xrange [0:22]
set ytics
set ylabel "Среднее время восстановления живучей ВС, часы"
set xlabel "time"
plot datafile using 1:2 with linespoints linestyle 1 title "lyambda = 10^{-5} 1/ч",\
     datafile using 1:3 with linespoints linestyle 2 title "lyambda = 10^{-6} 1/ч",\
     datafile using 1:4 with linespoints linestyle 3 title "lyambda = 10^{-7} 1/ч",\
     datafile using 1:5 with linespoints linestyle 4 title "lyambda = 10^{-7} 1/ч"

EOP

gnuplot << EOP

datafile = "o_3.0.data"

set terminal jpeg enhanced size 1080,710
set output "o_3.0.jpg"
set title "Зависимость от различных значений параметра lymbda"
set grid x y

set style line 1 lt 1 pt 9
set style line 2 lt 3 pt 7
set style line 3 lt 2 pt 5
set style line 4 lt 4 pt 3
set style line 5 lt 5 pt 2

set yrange [0:1.2]
set xrange [0:22]
set ytics
set ylabel "Среднее время восстановления живучей ВС, часы"
set xlabel "time"
plot datafile using 1:2 with linespoints linestyle 1 title "lyambda = 10^{-5} 1/ч",\
     datafile using 1:3 with linespoints linestyle 2 title "lyambda = 10^{-6} 1/ч",\
     datafile using 1:4 with linespoints linestyle 3 title "lyambda = 10^{-7} 1/ч",\
     datafile using 1:5 with linespoints linestyle 4 title "lyambda = 10^{-7} 1/ч"

EOP

gnuplot << EOP

datafile = "o_4.0.data"

set terminal jpeg enhanced size 1080,710
set output "o_4.0.jpg"
set title "Зависимость от различных значений параметра lymbda"
set grid x y

set style line 1 lt 1 pt 9
set style line 2 lt 3 pt 7
set style line 3 lt 2 pt 5
set style line 4 lt 4 pt 3
set style line 5 lt 5 pt 2

set yrange [0:1.2]
set xrange [0:22]
set ytics
set ylabel "Среднее время восстановления живучей ВС, часы"
set xlabel "time"
plot datafile using 1:2 with linespoints linestyle 1 title "lyambda = 10^{-5} 1/ч",\
     datafile using 1:3 with linespoints linestyle 2 title "lyambda = 10^{-6} 1/ч",\
     datafile using 1:4 with linespoints linestyle 3 title "lyambda = 10^{-7} 1/ч",\
     datafile using 1:5 with linespoints linestyle 4 title "lyambda = 10^{-7} 1/ч"

EOP

gnuplot << EOP

datafile = "o_1.1.data"

set terminal jpeg enhanced size 1080,710
set output "o_1.1.jpg"
set title "Зависимость от различных значений параметра lymbda"
set grid x y

set style line 1 lt 1 pt 9
set style line 2 lt 3 pt 7
set style line 3 lt 2 pt 5
set style line 4 lt 4 pt 3
set style line 5 lt 5 pt 2

set yrange [0:1.2]
set xrange [0:22]
set ytics
set ylabel "Среднее время восстановления живучей ВС, часы"
set xlabel "time"
plot datafile using 1:2 with linespoints linestyle 1 title "lyambda = 10^{-5} 1/ч",\
     datafile using 1:3 with linespoints linestyle 2 title "lyambda = 10^{-6} 1/ч",\
     datafile using 1:4 with linespoints linestyle 3 title "lyambda = 10^{-7} 1/ч",\
     datafile using 1:5 with linespoints linestyle 4 title "lyambda = 10^{-7} 1/ч"

EOP

gnuplot << EOP

datafile = "o_2.1.data"

set terminal jpeg enhanced size 1080,710
set output "o_2.1.jpg"
set title "Зависимость от различных значений параметра lymbda"
set grid x y

set style line 1 lt 1 pt 9
set style line 2 lt 3 pt 7
set style line 3 lt 2 pt 5
set style line 4 lt 4 pt 3
set style line 5 lt 5 pt 2

#set yrange [0:1.5]
#set xrange [0:22]
set ytics
set ylabel "Среднее время восстановления живучей ВС, часы"
set xlabel "time"
plot datafile using 1:2 with linespoints linestyle 1 title "lyambda = 10^{-5} 1/ч",\
     datafile using 1:3 with linespoints linestyle 2 title "lyambda = 10^{-6} 1/ч",\
     datafile using 1:4 with linespoints linestyle 3 title "lyambda = 10^{-7} 1/ч",\
     datafile using 1:5 with linespoints linestyle 4 title "lyambda = 10^{-7} 1/ч"

EOP

gnuplot << EOP

datafile = "o_3.1.data"

set terminal jpeg enhanced size 1080,710
set output "o_3.1.jpg"
set title "Зависимость от различных значений параметра lymbda"
set grid x y

set style line 1 lt 1 pt 9
set style line 2 lt 3 pt 7
set style line 3 lt 2 pt 5
set style line 4 lt 4 pt 3
set style line 5 lt 5 pt 2

set xrange [0:8]
set ytics
set ylabel "Среднее время восстановления живучей ВС, часы"
set xlabel "time"
plot datafile using 1:2 with linespoints linestyle 1 title "lyambda = 10^{-5} 1/ч",\
     datafile using 1:3 with linespoints linestyle 2 title "lyambda = 10^{-6} 1/ч",\
     datafile using 1:4 with linespoints linestyle 3 title "lyambda = 10^{-7} 1/ч",\
     datafile using 1:5 with linespoints linestyle 4 title "lyambda = 10^{-7} 1/ч"

EOP

gnuplot << EOP

datafile = "o_4.1.data"

set terminal jpeg enhanced size 1080,710
set output "o_4.1.jpg"
set title "Зависимость от различных значений параметра lymbda"
set grid x y

set style line 1 lt 1 pt 9
set style line 2 lt 3 pt 7
set style line 3 lt 2 pt 5
set style line 4 lt 4 pt 3
set style line 5 lt 5 pt 2

#set yrange [0:1.5]
set xrange [0:8]
set ytics
set ylabel "Среднее время восстановления живучей ВС, часы"
set xlabel "time"
plot datafile using 1:2 with linespoints linestyle 1 title "lyambda = 10^{-5} 1/ч",\
     datafile using 1:3 with linespoints linestyle 2 title "lyambda = 10^{-6} 1/ч",\
     datafile using 1:4 with linespoints linestyle 3 title "lyambda = 10^{-7} 1/ч",\
     datafile using 1:5 with linespoints linestyle 4 title "lyambda = 10^{-7} 1/ч"

EOP