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
  
  for(int relid = 0; relid < MAX_OPEN;relid++){
    RelCatEntry relCat;
    if(RelCacheTable::getRelCatEntry(relid, &relCat)!=SUCCESS){
      continue;
    }
    printf("Relation: %s\n", relCat.relName);

    for(int i=0;i<relCat.numAttrs;i++){
      AttrCatEntry attrCat;
      if(AttrCacheTable::getAttrCatEntry(relid,i,&attrCat)!=SUCCESS){
        continue;
      }
      std::string attrType=(attrCat.attrType==NUMBER)?"NUM":"STR";
      std::cout<<"  "<<attrCat.attrName<<": "<<attrType<<"\n";
    }
  }
  return 0;
  // StaticBuffer buffer;

  // OpenRelTable cache;

  // return FrontendInterface::handleFrontend(argc, argv);
}