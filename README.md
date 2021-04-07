# Networkspellchecker by Carlos Chavez
## CIS 3207 Sect: 003 LAB 3
### Project Overview
Spell checkers are useful utility programs often bundled with text editors. The purpose of the assignment is to gain some exposure and practical experience with multi-threaded programming and the synchronization problems that go along with it, as well as with writing programs that communicate across networks. This spell check server creates a process that acquires (read) sequences of words. If a word is in its dictionary, it's considered to be spelled properly. If not, it's considered to be misspelled. The dictionary itself is nothing but a very long list of wordsstored in plain text form. Sophisticated matching is unnecessary for this project; for example, recognizing that perhaps the word "loud", which is not in the dictionary, might be the plural of "louds", which is in the dictionary.
### Instructions to Run
After you are finished forking this project on your local desktop, open the directory where the files are located and write make into the terminal. After the program is finished compiling, write ./test into the terminal. The server will start running as shown here:
![Image of make and test cmnd](https://github.com/almightmustang1998/networkspellchecker/blob/main/images/programcomp.png)

### Built With
1 C

