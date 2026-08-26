#include "OpenRelTable.h"
#include<malloc.h>
#include <cstring>

OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

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

/************ Setting up tableMetaInfo entries ************/
// in the tableMetaInfo array
  //   set free = false for RELCAT_RELID and ATTRCAT_RELID
  //   set relname for RELCAT_RELID and ATTRCAT_RELID
  tableMetaInfo[RELCAT_RELID].free = false;
  strcpy(tableMetaInfo[RELCAT_RELID].relName, RELCAT_RELNAME);

  tableMetaInfo[ATTRCAT_RELID].free = false;
  strcpy(tableMetaInfo[ATTRCAT_RELID].relName, ATTRCAT_RELNAME);

  // set free = true for all other entries
  for(int i=2; i<MAX_OPEN; i++){
    tableMetaInfo[i].free = true;
    tableMetaInfo[i].relName[0] = '\0'; // empty string
  }

}


OpenRelTable::~OpenRelTable() {
  // close all open relations (from rel-id = 2 onwards. Why?)
  for (int i = 2; i < MAX_OPEN; ++i) {
    if (!tableMetaInfo[i].free) {
      OpenRelTable::closeRel(i); // we will implement this function later
    }
  }
  // free all the memory that you allocated in the constructor
  for(int i=0;i<2;i++){
        free(RelCacheTable::relCache[i]);
        RelCacheTable::relCache[i]=nullptr;
    }

    for(int i=0;i<2;i++){
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

  for(int i = 0; i < MAX_OPEN; ++i) {
    if (!tableMetaInfo[i].free && strcmp(tableMetaInfo[i].relName, relName) == 0) {
      return i;
    }
  }
  return E_RELNOTOPEN;
}

int OpenRelTable::getFreeOpenRelTableEntry() {

  /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/

  // if found return the relation id, else return E_CACHEFULL.
  for(int i=0;i<MAX_OPEN;i++){
    if(tableMetaInfo[i].free){
        return i;
    }
  }
  return E_CACHEFULL;
}

int OpenRelTable::openRel(char relName[ATTR_SIZE]) {
  /* the relation `relName` already has an entry in the Open Relation Table */
  
  if(OpenRelTable::getRelId((char*)relName)!=E_RELNOTOPEN){
    // (checked using OpenRelTable::getRelId())

    // return that relation id;
    return OpenRelTable::getRelId((char*)relName);
  }

  // let relId be used to store the free slot.
  int relId = OpenRelTable::getFreeOpenRelTableEntry();

  if (relId == E_CACHEFULL) {
    return E_CACHEFULL;
  }


  /****** Setting up Relation Cache entry for the relation ******/

  /* search for the entry with relation name, relName, in the Relation Catalog using
      BlockAccess::linearSearch().
      Care should be taken to reset the searchIndex of the relation RELCAT_RELID
      before calling linearSearch().*/

  // relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
  RecId relcatRecId;
  RelCacheTable::resetSearchIndex(RELCAT_RELID);
  Attribute attrVal;
  strcpy(attrVal.sVal, (char*)relName);
  relcatRecId = BlockAccess::linearSearch(RELCAT_RELID,(char *)RELCAT_ATTR_RELNAME ,attrVal, EQ);


  if ( relcatRecId.block == -1 && relcatRecId.slot == -1) {
    // (the relation is not found in the Relation Catalog.)
    return E_RELNOTEXIST;
  }

  /* read the record entry corresponding to relcatRecId and create a relCacheEntry
      on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
      update the recId field of this Relation Cache entry to relcatRecId.    
  */
  RecBuffer relationBuffer(relcatRecId.block);
  Attribute relationRecord[RELCAT_NO_ATTRS];
  relationBuffer.getRecord(relationRecord, relcatRecId.slot);
  // NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
  RelCacheEntry *relCacheEntry = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  RelCacheTable::recordToRelCatEntry(relationRecord, &(relCacheEntry->relCatEntry));
  relCacheEntry->recId = relcatRecId;
  relCacheEntry->dirty = false;
  relCacheEntry->searchIndex = {-1, -1};
  // use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
  RelCacheTable::relCache[relId] = relCacheEntry;


  /****** Setting up Attribute Cache entry for the relation ******/

  Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
  // let listHead be used to hold the head of the linked list of attrCache entries.
  AttrCacheEntry* listHead=nullptr;
  AttrCacheEntry* prev = nullptr;

  int numAttrs = RelCacheTable::relCache[relId]->relCatEntry.numAttrs;
 
  /*iterate over all the entries in the Attribute Catalog corresponding to each
  attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
  care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
  corresponding to Attribute Catalog before the first call to linearSearch().*/
  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
  for(int i=0; i<numAttrs; i++){
    RecId attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, (char *)ATTRCAT_ATTR_RELNAME, attrVal, EQ);

    RecBuffer attrCatBlock(attrcatRecId.block);
    Attribute record[ATTRCAT_NO_ATTRS];
    attrCatBlock.getRecord(record , attrcatRecId.slot);
    AttrCacheEntry *entry = (AttrCacheEntry *)malloc(sizeof(AttrCacheEntry));
    AttrCacheTable::recordToAttrCatEntry(record, &entry->attrCatEntry);

    entry->recId.block = attrcatRecId.block;
    entry->recId.slot = attrcatRecId.slot;
    entry->dirty=false;
    entry->searchIndex = {-1, -1};
    entry->next = nullptr;

    if(listHead == nullptr){
        listHead = entry;
    } else {
        prev->next = entry;
    }
    prev = entry;
  }

  // set the relIdth entry of the AttrCacheTable to listHead.
  AttrCacheTable::attrCache[relId] = listHead;
  /****** Setting up metadata in the Open Relation Table for the relation******/

  // update the relIdth entry of the tableMetaInfo with free as false and
  // relName as the input.
  tableMetaInfo[relId].free = false;
  strcpy(tableMetaInfo[relId].relName, (char*)relName);

  return relId;
}


int OpenRelTable::closeRel(int relId) {
  if (relId == RELCAT_RELID || relId == ATTRCAT_RELID) {
    return E_NOTPERMITTED;
  }

  if (relId < 0 || relId >= MAX_OPEN) {
    return E_OUTOFBOUND;
  }

  if (tableMetaInfo[relId].free) {
    return E_RELNOTOPEN;
  }

  // free the memory allocated in the relation and attribute caches which was
  // allocated in the OpenRelTable::openRel() function
  free(RelCacheTable::relCache[relId]);
  free(AttrCacheTable::attrCache[relId]);
  // update `tableMetaInfo` to set `relId` as a free slot
  tableMetaInfo[relId].free = true;
  // update `relCache` and `attrCache` to set the entry at `relId` to nullptr
  RelCacheTable::relCache[relId] = nullptr;
  AttrCacheTable::attrCache[relId] = nullptr;
  
  return SUCCESS;
}
