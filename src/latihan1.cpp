//============================================================================
// Name        : latihan1.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <cstring>
#include <inttypes.h>

#include "cc1120.hpp"
#include "shape.hpp"

#include "cc112x_spi.h"
#include "cc1120Drv.h"
#include "cc112x_easy_link_reg_config.h"

using namespace std;

int main(void) {
   rectangle Rect;
   cc1120_T cc1120;

   cc1120.init(0, 0, 1);
   Rect.setWidth(5);
   Rect.setHeight(7);

   // Print the area of the object.
   cout << "Total area: " << Rect.getArea() << endl;

   cc112x_init(1, 0);

   return 0;
}

