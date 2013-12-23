#!/bin/bash
#TODO make a makefile
CXX=clang++
$CXX -std=c++11 calculator.cpp -I/usr/include/qt4/{,QtCore,QtGui} -lQtCore -lQtGui

