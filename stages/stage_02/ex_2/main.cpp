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

  RecBuffer relCatBuffer(RELCAT_BLOCK);
  RecBuffer attrCatBuffer(ATTRCAT_BLOCK);

  HeadInfo relCatHeader;
  HeadInfo attrCatHeader;

  relCatBuffer.getHeader(&relCatHeader);
  // attrCatBuffer.getHeader(&attrCatHeader);

  for(int32_t i=0;i<relCatHeader.numEntries;i++){
    Attribute relCatRecord[RELCAT_NO_ATTRS];

    relCatBuffer.getRecord(relCatRecord, i);
    printf("Relation: %s\n", relCatRecord[RELCAT_REL_NAME_INDEX].sVal);

    RecBuffer attrCatBuffer(ATTRCAT_BLOCK);
    while(true){
     
      attrCatBuffer.getHeader(&attrCatHeader);

      for(int32_t j=0;j<attrCatHeader.numEntries;j++){
        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
  
        attrCatBuffer.getRecord(attrCatRecord,j);
  
        if(strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, relCatRecord[RELCAT_REL_NAME_INDEX].sVal) == 0){
          const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
          printf("  %s: %s\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
        }
        
        if(strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal, "Students") == 0 && strcmp(attrCatRecord[ATTRCAT_REL_NAME_INDEX].sVal,relCatRecord[RELCAT_REL_NAME_INDEX].sVal)==0) {
          if(strcmp(attrCatRecord[1].sVal,"Class")==0){
              strcpy(attrCatRecord[1].sVal,"Batch");
               unsigned char buffer[BLOCK_SIZE];
               Disk::readBlock(buffer, attrCatHeader.blockType);
               int start=32+attrCatHeader.numSlots+(16*attrCatHeader.numAttrs*j);
               memcpy(buffer+start,attrCatRecord,16*attrCatHeader.numAttrs);
               Disk::writeBlock(buffer,attrCatHeader.blockType);

               std::cout << "Updated attribute name from 'Class' to 'Batch' for relation 'Students'." << std::endl;
          const char *attrType = attrCatRecord[ATTRCAT_ATTR_TYPE_INDEX].nVal == NUMBER ? "NUM" : "STR";
          printf("  %s: %s\n\n", attrCatRecord[ATTRCAT_ATTR_NAME_INDEX].sVal, attrType);
          }
          
        }

      }
      
      if (attrCatHeader.rblock == -1){
        break;
      }
      attrCatBuffer = RecBuffer(attrCatHeader.rblock);
    }
    printf("\n");
  }
  return 0;
  // StaticBuffer buffer;

  // OpenRelTable cache;

  // return FrontendInterface::handleFrontend(argc, argv);
}