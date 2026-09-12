void main() {
    char *video_memory = (char *) 0xb8000;
    // Write 'X' further into the screen (row 2, column 5) instead of (0,0)
    // so it's not hidden behind window chrome.
    // int offset = 2 * (2 * 80 + 5); // row=2, col=5
    int offset = 0;
    // video_memory[offset] = 'X';

    // char* second = (char*) 0xb8010;
    // second[offset+2] = 'O';

    char* message = "HELLO WORLD!\0";
    char i = 0;
    while(*(message + i) != '\0'){
        video_memory[offset+2*i] = *(message + i);
        i++;
    }
}


