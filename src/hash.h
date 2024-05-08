#ifndef CUSTOM_HASH_TABLE_H
#define CUSTOM_HASH_TABLE_H

// Define the maximum size of the hash table
#define MAX_TABLE_SIZE 100

// Define the structure for a key-value pair node
typedef struct KeyValuePair
{
    char *key;
    char *value;
    struct KeyValuePair *next;
} KeyValuePair;

// Define the structure for the hash table
typedef struct
{
    KeyValuePair *table[MAX_TABLE_SIZE];
} HashTable;

// Function prototypes
HashTable *createHashTable();
void destroyHashTable(HashTable *hash);
char *hashTableGet(HashTable *hash, const char *key);
void hashTablePut(HashTable *hash, const char *key, const char *value);
void hashTableRemove(HashTable *hash, const char *key);
char *getAllKeyValuePairs(HashTable *hash);
char *concat(const char *s1, const char *s2);

#endif /* CUSTOM_HASH_TABLE_H */