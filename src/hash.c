#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

// Custom implementation of strdup()
char *my_strdup(const char *src)
{
    size_t len = strlen(src) + 1; // +1 for null terminator
    char *dest = malloc(len);
    if (dest == NULL)
    {
        fprintf(stderr, "Memory allocation failed for strdup()\n");
        exit(EXIT_FAILURE);
    }
    return strcpy(dest, src);
}

// Hash function using DJB2 algorithm
static unsigned long hashFunc(const char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash % MAX_TABLE_SIZE;
}

// Function to create a new hash table
HashTable *createHashTable()
{
    HashTable *hash = (HashTable *)malloc(sizeof(HashTable));
    if (hash == NULL)
    {
        fprintf(stderr, "Memory allocation failed for hash table\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < MAX_TABLE_SIZE; i++)
    {
        hash->table[i] = NULL;
    }
    return hash;
}

// Function to destroy a hash table
void destroyHashTable(HashTable *hash)
{
    if (hash == NULL)
        return;
    for (int i = 0; i < MAX_TABLE_SIZE; i++)
    {
        KeyValuePair *current = hash->table[i];
        while (current != NULL)
        {
            KeyValuePair *temp = current;
            current = current->next;
            free(temp->key);
            free(temp->value);
            free(temp);
        }
    }
    free(hash);
}

// Function to get the value associated with a key
char *hashTableGet(HashTable *hash, const char *key)
{
    unsigned long index = hashFunc(key);
    KeyValuePair *current = hash->table[index];
    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {
            return current->value;
        }
        current = current->next;
    }
    return NULL;
}

// Function to insert or update a key-value pair
void hashTablePut(HashTable *hash, const char *key, const char *value)
{
    unsigned long index = hashFunc(key);
    KeyValuePair *current = hash->table[index];
    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {
            // Update existing value
            free(current->value);
            current->value = my_strdup(value); // Use custom strdup() function
            if (current->value == NULL)
            {
                fprintf(stderr, "Memory allocation failed for value\n");
                exit(EXIT_FAILURE);
            }
            return;
        }
        current = current->next;
    }

    // Insert new key-value pair
    KeyValuePair *newPair = (KeyValuePair *)malloc(sizeof(KeyValuePair));
    if (newPair == NULL)
    {
        fprintf(stderr, "Memory allocation failed for key-value pair\n");
        exit(EXIT_FAILURE);
    }
    newPair->key = my_strdup(key);     // Use custom strdup() function
    newPair->value = my_strdup(value); // Use custom strdup() function
    if (newPair->key == NULL || newPair->value == NULL)
    {
        fprintf(stderr, "Memory allocation failed for key or value\n");
        exit(EXIT_FAILURE);
    }
    newPair->next = hash->table[index];
    hash->table[index] = newPair;
}

// Function to remove a key-value pair
void hashTableRemove(HashTable *hash, const char *key)
{
    unsigned long index = hashFunc(key);
    KeyValuePair *prev = NULL;
    KeyValuePair *current = hash->table[index];
    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {
            if (prev == NULL)
            {
                hash->table[index] = current->next;
            }
            else
            {
                prev->next = current->next;
            }
            free(current->key);
            free(current->value);
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

// Function to concatenate two strings
char *concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 2); // +2 for ':' and '\n'
    if (result == NULL)
    {
        fprintf(stderr, "Memory allocation failed for concatenation\n");
        exit(EXIT_FAILURE);
    }
    strcpy(result, s1);
    strcat(result, ":");
    strcat(result, s2);
    strcat(result, "\n");
    return result;
}

// get all key value pairs in the hash table
char *getAllKeyValuePairs(HashTable *hash)
{
    if (hash == NULL)
    {
        return NULL;
    }
    char *result = malloc(1); // Initialize an empty string
    if (result == NULL)
    {
        fprintf(stderr, "Memory allocation failed for result\n");
        exit(EXIT_FAILURE);
    }
    result[0] = '\0'; // Ensure the string is properly terminated

    for (int i = 0; i < MAX_TABLE_SIZE; i++)
    {
        KeyValuePair *current = hash->table[i];
        while (current != NULL)
        {
            char *pair = concat(current->key, current->value);
            size_t oldLen = strlen(result);
            result = realloc(result, oldLen + strlen(pair) + 1); // Resize result
            if (result == NULL)
            {
                fprintf(stderr, "Memory allocation failed for result\n");
                exit(EXIT_FAILURE);
            }
            strcat(result, pair);
            free(pair);
            current = current->next;
        }
    }

    return result;
}
