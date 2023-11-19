#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>

#define BUFFER_SIZE 256

struct WordCount {
    char *word;
    int count;
    struct WordCount *next;
};

enum State {
    INITIAL_STATE,
    LETTERS_STATE,
    APOSTROPHE_STATE,
    HYPHEN_STATE,
    IGNORE_STATE
};

void addWord(struct WordCount **head, const char *word) {
    struct WordCount *current = *head;

    while (current != NULL) {
        if (strcmp(current->word, word) == 0) {
            current->count++;
            return;
        }
        current = current->next;
    }

    struct WordCount *newWord = (struct WordCount *)malloc(sizeof(struct WordCount));
    newWord->word = strdup(word);  // duplicate the word
    newWord->count = 1;
    newWord->next = *head;
    *head = newWord;
}


int compareWordCount(const struct WordCount *a, const struct WordCount *b) {
    if (a->count != b->count) {
        return b->count - a->count; //find the difference
    }

    // special characters come first
    if (!isalnum(a->word[0]) && isalnum(b->word[0])) {
        return -1;
    }
    if (isalnum(a->word[0]) && !isalnum(b->word[0])) {
        return 1;
    }

    // the special characters  sorted lexicographically
    if (!isalnum(a->word[0])) {
        return strcmp(a->word, b->word);
    }

    // capital letters come before lowercase letters
    if (isupper(a->word[0]) && islower(b->word[0])) {
        return -1;
    }
    if (islower(a->word[0]) && isupper(b->word[0])) {
        return 1;
    }

    // if there is the same character case, use lexicographic order
    return strcmp(a->word, b->word); //done with the string compare method
}

//adding the sorted onto a list (decremented order though)
void appendSorted(struct WordCount **head, struct WordCount *newWord) {
    struct WordCount *current = *head;
    struct WordCount *prev = NULL;

    while (current != NULL && compareWordCount(newWord, current) < 0) {
        prev = current;
        current = current->next;
    }

    if (prev == NULL) {
        newWord->next = *head;
        *head = newWord;
    } else {
        prev->next = newWord;
        newWord->next = current;
    }
}

//to help reverse the decremented order that is shown
void reverseList(struct WordCount **head) {
    struct WordCount *prev = NULL;
    struct WordCount *current = *head;
    struct WordCount *next = NULL;

    while (current != NULL) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }

    *head = prev;
}

//end of this part

int isDirectory(const char *path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

//its important to check for read permissions
int hasReadPermissions(const char *path) {
    // Check if the path is a directory
    if (isDirectory(path)) {
        // Check read permissions for the directory itself
        if (access(path, R_OK) != 0) {
            perror("No read permissions for directory");
            perror(path);
            return 0;
        }

        // Check read permissions for the parent directory
        char parentDir[PATH_MAX];
        snprintf(parentDir, sizeof(parentDir), "%s/..", path);
        if (access(parentDir, R_OK) != 0) {
            perror("No read permissions for parent directory");
            perror(parentDir);
            return 0;
        }
    } else {
        // Check read permissions for the file (or directory with files)
        if (access(path, R_OK) != 0) {
            perror("No read permissions for file or (directory with files)");
            perror(path);
            return 0;
        }
    }

    // Return 1 if read permissions are granted
    return 1;
}



int processFile(const char *filename, struct WordCount **wordList) {
    if (!hasReadPermissions(filename)) {
        // perror("No read permissions for file!");
        //already accounted for
        // perror(filename)
        return 1;
    }
    
    int input_file = open(filename, O_RDONLY);
    if (input_file == -1) {
        perror("Could not open the input file!!");
        return 1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    enum State state = INITIAL_STATE;
    char *word = NULL;
    size_t wordLength = 0;

    enum State nextBufferState = INITIAL_STATE;
    char nextBufferHyphen = '\0';

    int prevAlpha = 0; //prev char was alpha
    int prevApos = 0; //prev char was apostrophe


    while ((bytesRead = read(input_file, buffer, BUFFER_SIZE)) > 0) {
        for (int i = 0; i < bytesRead; i++) {
            char c = buffer[i];

            // (remaining code is unchanged)
            if (nextBufferState != INITIAL_STATE) {
                state = nextBufferState;
                if (state == HYPHEN_STATE) {
                    char *temp = (char *)realloc(word, (wordLength + 2) * sizeof(char));
                    if (temp == NULL) {
                        perror("Memory Allocation Error!");
                        return 1;
                    }
                    word = temp;
                    word[wordLength] = nextBufferHyphen;
                    word[wordLength + 1] = '\0';
                    wordLength++;
                }
                nextBufferState = INITIAL_STATE;
            }

            switch (state) {
                case INITIAL_STATE:
                    if (isalpha(c)) {
                        state = LETTERS_STATE;
                        word = (char *)malloc(2 * sizeof(char));
                        word[0] = c;
                        word[1] = '\0';
                        wordLength = 1;
                    } else if (c == '\'') {
                        state = APOSTROPHE_STATE;
                        word = (char *)malloc(2 * sizeof(char));
                        word[0] = c;
                        word[1] = '\0';
                        wordLength = 1;
                    } else {
                        state = IGNORE_STATE;
                    }
                    break;

                case LETTERS_STATE:
                    if (isalpha(c) || c == '\'') {
                        if (isalpha(c) || (c == '\'' && (i < bytesRead - 1 && isalpha(buffer[i + 1])))) {
                            state = LETTERS_STATE;
                        } else if (c == '\'') {
                            state = APOSTROPHE_STATE;
                        }
                        char *temp = (char *)realloc(word, (wordLength + 2) * sizeof(char));
                        if (temp == NULL) {
                            perror("Memory Allocation Error!");
                            return 1;
                        }
                        word = temp;
                        word[wordLength] = c;
                        word[wordLength + 1] = '\0';
                        wordLength++;
                    } else if (c == '-') {
                        state = HYPHEN_STATE;
                        prevAlpha = 1;
                    } else {
                        state = INITIAL_STATE;
                        addWord(wordList, word);
                        word = NULL;
                        wordLength = 0;
                    }
                    break;

                    case APOSTROPHE_STATE:
                        if (isalpha(c) || c == '\'') {
                            char *temp = (char *)realloc(word, (wordLength + 2) * sizeof(char));
                            if (temp == NULL) {
                                perror("Memory Allocation Error!");
                                return 1;
                            }
                            word = temp;
                            word[wordLength] = c;
                            word[wordLength + 1] = '\0';
                            wordLength++;

                            if (isalpha(c) || (c == '\'' && (i < bytesRead - 1 && isalpha(buffer[i + 1]))) || c == '-') {
                                state = LETTERS_STATE;
                            } else if (c == '\'') {
                                state = APOSTROPHE_STATE;
                            } else {
                                state = INITIAL_STATE;
                                addWord(wordList, word);
                                word = NULL;
                                wordLength = 0;
                            }
                        } else if (c == '-') {  // When it is apostrophe followed by a hyphen
                            state = HYPHEN_STATE;
                            prevApos = 1;
                        } else {
                            state = INITIAL_STATE;
                            addWord(wordList, word);
                            word = NULL;
                            wordLength = 0;
                        }
                        break;


                case HYPHEN_STATE:
                    if (c == '-') {
                        if (i == bytesRead - 1) {
                            nextBufferState = HYPHEN_STATE;
                            nextBufferHyphen = '-';
                        } else {
                            while (i < bytesRead - 1 && buffer[i + 1] == '-') {
                                i++;
                            }
                            if (i == bytesRead - 1) {
                                nextBufferState = HYPHEN_STATE;
                                nextBufferHyphen = '-';
                            } else {
                                state = INITIAL_STATE;
                                addWord(wordList, word);
                                word = NULL;
                                wordLength = 0;
                            }
                        }
                    } else if (isalpha(c) && prevAlpha == 1) {
                        state = LETTERS_STATE;
                        char *temp = (char *)realloc(word, (wordLength + 3) * sizeof(char));
                        if (temp == NULL) {
                            perror("Memory Allocation Error!");
                            return 1;
                        }
                        word = temp;
                        word[wordLength] = '-';
                        word[wordLength + 1] = c;
                        word[wordLength + 2] = '\0';
                        wordLength += 2;
                    }
                    else if (isalpha(c) && prevApos == 1){
                        state = INITIAL_STATE;
                        addWord(wordList, word);
                        word = NULL;
                        wordLength = 0;

                        word = (char *)malloc(2 * sizeof(char));
                        word[0] = c;
                        word[1] = '\0';
                        wordLength = 1;

                        state = LETTERS_STATE;
                        prevApos = 0;

                        // word[wordLength] = c;
                        // word[wordLength + 1] = '\0';
                        // wordLength++;
                    }
                    else if (c == '\'' && prevAlpha == 1){
                        state = INITIAL_STATE;
                        addWord(wordList, word);
                        word = NULL;
                        wordLength = 0;

                        word = (char *)malloc(2 * sizeof(char));
                        word[0] = c;
                        word[1] = '\0';
                        wordLength = 1;

                        state = APOSTROPHE_STATE;
                        prevAlpha = 0;
                    }
                    else if (c == '\'' && prevApos == 1){
                        state = INITIAL_STATE;
                        addWord(wordList, word);
                        word = NULL;
                        wordLength = 0;

                        word = (char *)malloc(2 * sizeof(char));
                        word[0] = c;
                        word[1] = '\0';
                        wordLength = 1;

                        state = APOSTROPHE_STATE;
                        prevApos = 0;
                    }
                    else {
                        state = INITIAL_STATE;
                        addWord(wordList, word);
                        word = NULL;
                        wordLength = 0;
                    }
                    break;

                case IGNORE_STATE:
                    if (isalpha(c) || c == '\'') {
                        if (isalpha(c)) {
                            state = LETTERS_STATE;
                        } else if (c == '\'') {
                            state = APOSTROPHE_STATE;
                            char *temp = (char *)realloc(word, (wordLength + 2) * sizeof(char));
                            if (temp == NULL) {
                                perror("Memory Allocation Error!");
                                return 1;
                            }
                            word = temp;
                            word[wordLength] = c;
                            word[wordLength + 1] = '\0';
                            wordLength++;
                        }
                        word = (char *)malloc(2 * sizeof(char));
                        word[0] = c;
                        word[1] = '\0';
                        wordLength = 1;
                    } else {
                        state = INITIAL_STATE;
                    }
                    break;
            }
        }
    }

    // Adding the last word if it exists
    if (word != NULL) {
        addWord(wordList, strdup(word));
    }

    close(input_file);

    if (*wordList == NULL) {
        printf("File '%s' has no words.\n", filename);
    }

    return 0;
}

void processDirectory(const char *dirname, struct WordCount **wordList) {
    if (!hasReadPermissions(dirname)) {
        // perror("No read permissions for directory!");
        // perror(dirname);
        //addressed in the above case by case situation
        return;
    }

    DIR *dir = opendir(dirname);
    if (dir == NULL) {
        // fprintf(stderr, "Error opening directory: %s\n", dirname);
        // perror("Error opening directory");
        // perror(dirname);  // Print the directory name
        return;
    }

    // int directoryHasWords = 0; //flag to check if directory has processed words 

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, ".txt") != NULL) {
            // It's a regular file with a '.txt' extension
            char filepath[PATH_MAX];
            snprintf(filepath, sizeof(filepath), "%s/%s", dirname, entry->d_name);
            if (!hasReadPermissions(filepath)) {
                // perror("No read permissions for file!");
            } else {
                processFile(filepath, wordList);
            }
        } else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            // It's a subdirectory (excluding "." and "..")
            char subdir[PATH_MAX];
            snprintf(subdir, sizeof(subdir), "%s/%s", dirname, entry->d_name);
            processDirectory(subdir, wordList);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        // fprintf(stderr, "Not Enough Arguments in %s <file or directory>\n", argv[0]);
        // perror("Not Enough Arguments");
        // perror(argv[0]);
        fprintf(stderr, "Not Enough Arguments: %s <file or directory>\n", argv[0]);
        return 1;
    }

    struct WordCount *wordList = NULL;

    for (int i = 1; i < argc; ++i){
        const char *input = argv[i];

        if (opendir(input) == NULL) {
            //file
            processFile(input, &wordList);
        } else {
            //directory
            processDirectory(input, &wordList);
        }
    }

    // (remaining code for sorting and printing remains unchanged)
    struct WordCount *sortedWordList = NULL;
    struct WordCount *current = wordList;

    while (current != NULL) {
        struct WordCount *next = current->next;
        current->next = NULL;  // unlink from the original list
        appendSorted(&sortedWordList, current);
        current = next;
    }

    // reverse the sorted word list
    reverseList(&sortedWordList);

    // printing the reversed linked list
    current = sortedWordList;
    while (current != NULL) {
        printf("%s: %d\n", current->word, current->count);
        struct WordCount *temp = current;
        current = current->next;
        free(temp->word);
        free(temp);
    }

    return 0;
}