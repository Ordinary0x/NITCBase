#include "Buffer/StaticBuffer.h"
#include "Buffer/BlockBuffer.h"
#include "Cache/OpenRelTable.h"
#include "Disk_Class/Disk.h"
#include "FrontendInterface/FrontendInterface.h"
#include <cstring>   // for memcpy
#include <iostream>  // for std::cout

int main(int argc, char *argv[]) {
  /* Initialize the Run Copy of Disk */
  Disk disk_run;
  StaticBuffer buffer;
  OpenRelTable cache;
  
  return FrontendInterface::handleFrontend(argc, argv);
}

