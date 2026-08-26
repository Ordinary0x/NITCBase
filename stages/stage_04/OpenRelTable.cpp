#include "OpenRelTable.h"
#include<malloc.h>
#include <cstring>

OpenRelTable::OpenRelTable() {

  // initialize relCache and attrCache with nullptr
  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
  }

  /************ Setting up Relation Cache entries ************/
  // (we need to populate relation cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Relation Cache Table****/
  RecBuffer relCatBlock(RELCAT_BLOCK);

  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);

  struct RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;

  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

  /**** setting up Attribute Catalog relation in the Relation Cache Table ****/
  

  // set up the relation cache entry for the attribute catalog similarly
  // from the record at RELCAT_SLOTNUM_FOR_ATTRCAT
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_ATTRCAT);

  struct RelCacheEntry attrCatCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord,&attrCatCacheEntry.relCatEntry);
  attrCatCacheEntry.recId.block = RELCAT_BLOCK;
  attrCatCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;

  // set the value at RelCacheTable::relCache[ATTRCAT_RELID]
  RelCacheTable::relCache[ATTRCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
    *(RelCacheTable::relCache[ATTRCAT_RELID]) = attrCatCacheEntry;


  /************ Setting up Attribute cache entries ************/
  // (we need to populate attribute cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Attribute Cache Table ****/
  RecBuffer attrCatBlock(ATTRCAT_BLOCK);

  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];

  // iterate through all the attributes of the relation catalog and create a linked
  // list of AttrCacheEntry (slots 0 to 5)
  // for each of the entries, set
  //    attrCacheEntry.recId.block = ATTRCAT_BLOCK;
  //    attrCacheEntry.recId.slot = i   (0 to 5)
  //    and attrCacheEntry.next appropriately
  // NOTE: allocate each entry dynamically using malloc
  AttrCacheEntry *head = nullptr;
  AttrCacheEntry *prev = nullptr;
  for(int no_attr=0; no_attr<RELCAT_NO_ATTRS; no_attr++){
    attrCatBlock.getRecord(attrCatRecord, no_attr);
    AttrCacheEntry *entry = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));

    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &entry->attrCatEntry);

    entry->recId.block = ATTRCAT_BLOCK;
    entry->recId.slot = no_attr;
    entry->next = nullptr;

    if(head == nullptr){
        head = entry;
    } else {
        prev->next = entry;
    }
    prev = entry;

  }

  // set the next field in the last entry to nullptr

  AttrCacheTable::attrCache[RELCAT_RELID] = head /* head of the linked list */;

  /**** setting up Attribute Catalog relation in the Attribute Cache Table ****/

  // set up the attributes of the attribute cache similarly.
  // read slots 6-11 from attrCatBlock and initialise recId appropriately
  head = nullptr;
  prev = nullptr;

  for(int no_attr=0; no_attr<ATTRCAT_NO_ATTRS; no_attr++){
    attrCatBlock.getRecord(attrCatRecord, no_attr + RELCAT_NO_ATTRS);
    AttrCacheEntry *entry = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
    AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &entry->attrCatEntry);

    entry->recId.block = ATTRCAT_BLOCK;
    entry->recId.slot = no_attr + RELCAT_NO_ATTRS;
    entry->next = nullptr;

    if(head == nullptr){
        head = entry;
    } else {
        prev->next = entry;
    }
    prev = entry;
  }
  // set the value at AttrCacheTable::attrCache[ATTRCAT_RELID]
  AttrCacheTable::attrCache[ATTRCAT_RELID] = head;

/************ Setting up Student relation in  the Relation Cache Table ************/
// find Students' record in the relation catalog
int studentsSlot=-1;
Attribute studentsRelRecord[RELCAT_NO_ATTRS];

  for(int slot=0;slot<20;slot++){
    if(relCatBlock.getRecord(studentsRelRecord,slot)!=SUCCESS){
        break;
    }
    if(strcmp(studentsRelRecord[0].sVal,"Students")==0){
        studentsSlot=slot;
        break;
    }
  }

  struct RelCacheEntry studCatCacheEntry;
  RelCacheTable::recordToRelCatEntry(studentsRelRecord,&studCatCacheEntry.relCatEntry);
  studCatCacheEntry.recId.block = RELCAT_BLOCK;
  studCatCacheEntry.recId.slot = studentsSlot;

  RelCacheTable::relCache[2] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[2]) = studCatCacheEntry;

  
/************ Setting up Student Attributes in the Attribute Cache Table ************/

int numAttrs = studCatCacheEntry.relCatEntry.numAttrs;
int found = 0;
int slot = 0;

struct AttrCacheEntry *head3 = nullptr;
struct AttrCacheEntry *tail3 = nullptr;

while(found < numAttrs){
    Attribute studentsAttrRecord[ATTRCAT_NO_ATTRS];
    if(attrCatBlock.getRecord(studentsAttrRecord, slot) != SUCCESS){
        break;
    }
    if(strcmp(studentsAttrRecord[0].sVal, "Students") == 0){
        struct AttrCacheEntry *entry = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
        AttrCacheTable::recordToAttrCatEntry(studentsAttrRecord, &entry->attrCatEntry);
        entry->next = nullptr;
        entry->recId.block = ATTRCAT_BLOCK;
        entry->recId.slot = slot;

        if(head3 == nullptr){
            head3 = entry;
        }
        else{
            tail3->next = entry;
        }
        tail3 = entry;
        found++;
    }
    slot++;
}

AttrCacheTable::attrCache[studentsSlot]=head3;
}


OpenRelTable::~OpenRelTable() {
  // free all the memory that you allocated in the constructor
  for(int i=0;i<=2;i++){
        free(RelCacheTable::relCache[i]);
        RelCacheTable::relCache[i]=nullptr;
    }

    for(int i=0;i<=2;i++){
        AttrCacheEntry *curr=AttrCacheTable::attrCache[i];
        while(curr!=nullptr){
            AttrCacheEntry *next=curr->next;
            free(curr);
            curr=next;
        }
        AttrCacheTable::attrCache[i]=nullptr;
    }

}

/* This function will open a relation having name `relName`.
Since we are currently only working with the relation and attribute catalog, we
will just hardcode it. In subsequent stages, we will loop through all the relations
and open the appropriate one.
*/
int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {

  // if relname is RELCAT_RELNAME, return RELCAT_RELID
  if (strcmp(relName, RELCAT_RELNAME) == 0) {
    return RELCAT_RELID;
  }

  // if relname is ATTRCAT_RELNAME, return ATTRCAT_RELID
  if (strcmp(relName, ATTRCAT_RELNAME) == 0) {
    return ATTRCAT_RELID;
  }

  return E_RELNOTOPEN;
}