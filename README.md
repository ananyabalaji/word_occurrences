# Word Occurrences 
# CS 214: Systems Programming Project 2
# Grade: 100%

## Author
- Name: Ananya Balaji
- NetID: ab2389

## Overview
This is a project that processes the words and its number of occurrences of files in a given directory and its subdirectories. 

A word is a sequence of letters and some punctuation. Specifically,
the apostrophe (single quote) may occur anywhere within a word, and a hyphen (dash) may occur in a word, but only if preceded and followed by a letter.

This project consider numbers, whitespace, and other punctuation to separate words. For simplicity, capitalization is significant.

The code takes one or more arguments, which may be text files or directories. It will open and process each named text file. For directories, words will recursively seek and process files in the directory whose names end with “.txt”.

I first drew out a Finite State Automata (to develop each case in a switch statement) before starting my code, and considered possible cases including:

## Test Cases
- Files and Directories with and without read() access
- Letter, Hyphen, Apostrophe and Random Characters in random orderings.
- I focused particularly on the orderings of:
    - Letter Hyphen Apostrophe
    - Letter Hyphen Letter
    - Apostrophe Hyphen Letter
    - Apostrophe Hyphen Apostrophe
- this is because of how apostrophes act similar to letters, except differently when next to hyphens
- to address them, I used variables called prevApos and prevAlpha to determine if the character prior to the Hyphen was a Apostrophe or Alphabetic Letter and made changes accordingly.

## Error Cases Looked Into

### Case 1: Text File or Directory Without Read Permissions
- **Description:** When attempting to process a text file or directory without read permissions.
  - Two-Line Answer:
    - State that there are no read permissions: "Permission denied."
    - Includes the name of the file or directory without read permissions.

### Case 2: Empty Directory
- **Description:** Handling an empty directory.
  - Treated the same way as having no `.txt` files.
  - Nothing is printed out.

### Case 3: ./words .
- **Description:** Processes the word counts of files in the current directory (and subdirectories).
  - Includes words that have no read permissions, if any.
  - Treats itself like a directory (so those no .txt files are not processed in output)

### Case 4: ./words ..
- **Description:** Processes the word counts of files in the parent directory (and subdirectories).
  - Includes words that have no read permissions, if any.

### Case 5: No Arguments Given
- **Description:** Handling when no arguments are provided.
  - Prints out an error stating "Not enough arguments" based on the file path.

### Case 6: Arguments Given but No Valid File to Be Read
- **Description:** Handling when arguments are given but no valid file or directory is present.
  - States if there is no such file or directory.
  - Provides the name of the non-existent file or directory.

### Case 7: Valid File With No Words
- **Description:** Handling a valid file with no words.
  - Indicates that the file has no words if considered as an argument.
  - Prints out if in a directory as well, except in large files like `./words .` or `./words ..` (due to space considerations).
