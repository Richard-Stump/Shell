/*
 * CS252: Systems Programming
 * Purdue University
 * Example that shows how to read one line with simple editing
 * using raw terminal.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_BUFFER_LINE 2048

//forward declarations from tty-raw-mode.c
extern void tty_raw_mode(void);
extern void tty_reset(void);

// Position of the cursor in the buffer
int cursor_pos;

//forward declarations
void cursor_right(void);
void cursor_left(void);
void echo_ch(char ch);
void write_ch(char ch);

//=============================================================
//                   Line history
// Command history is stored in a linked list. The head of the
// list is the most recent line entered. The tail is the first
// line entered. When history items are selected, they are
// copied into the current line buffer so that the history
// doesn't get thrashed when the user edits a history line.
//=============================================================

// linked list node for the history_list
typedef struct line_s {
  struct line_s*  next;
  struct line_s*  prev;
  int             length;
  char            text[MAX_BUFFER_LINE];
} line_t;

line_t* line_list_head = NULL;  //Head of the history list
line_t* cur_list_el = NULL;     //The current element the user is looking at
line_t cur_line;                //The current line that is being edited
                                //cur_list_el gets copied into this so that
                                //history is not overwritten
line_t backup;
bool history_initialized = false;

//initializes the current line buffer and backup buffer
void init_history(void) {
  cur_line.next = NULL;
  cur_line.prev = NULL;
  cur_line.length = 0;

  backup.next = NULL;
  backup.prev = NULL;
  backup.length = 0;
  
  history_initialized = true;
}

void push_cur_history_line(void) {
  line_t* new_line = malloc(sizeof(line_t));
  new_line->next = line_list_head;
  new_line->prev = NULL;
  new_line->length = cur_line.length;

  strncpy(new_line->text, cur_line.text, cur_line.length);

  if(line_list_head == NULL) {
    line_list_head = new_line;
  }
  else {
    line_list_head->prev = new_line;
    line_list_head = new_line;
  }
}

/* copies the src line into the current editing buffer, and writes the output
 * to the terminal for the user to see.
 */
void copy_line_to_current(line_t* src) {
  int old_len = cur_line.length; //save the old length

  cur_line.length = src->length;

  //go to the beginning of the line
  while(cursor_pos > 0) {
    cursor_left();
  }

  //write the new line to the terminal and buffer
  for(int i = 0; i < src->length; i++) {
    write_ch(src->text[i]);
  }

  //if the old line is longer than the new one, overwrite those characters
  //with spaces, and return the cursor to the end of the line.
  for(int i = src->length; i < old_len; i++) {
    echo_ch(' ');
  }
  for(int i = src->length; i < old_len; i++) {
    echo_ch(8);
  }
}

/* creates a backup of the current line the user is editing */
void backup_cur_line(void) {
  strncpy(backup.text, cur_line.text, cur_line.length);
  backup.length = cur_line.length;
}

/* selects the next line in the history list. This new line is copied to the
 * editing buffer, and the new line is displayed
 */
void select_next_history_entry(void) {
  //if the user is not viewing any history
  if(line_list_head == NULL) {
    return;
  }
  if(cur_list_el == NULL) {
    backup_cur_line();  
    cur_list_el = line_list_head;
  }
  else if(cur_list_el->next != NULL) { //if there is more history
    cur_list_el = cur_list_el->next;
  }
  else {
    return;
  }

  copy_line_to_current(cur_list_el);
}

/* Selects the previous line in the history list. This line is copied to the 
 * editing buffer and is displayed for the user.
 */
void select_prev_history_entry(void) {
  if(cur_list_el == NULL) { //if the user is not looking at the history
    return;
  }
  else if (cur_list_el->prev == NULL) { //if the user is looking a the most 
    copy_line_to_current(&backup);      //recent item, restore the backup
    cur_list_el = NULL; 
  }
  else {                                //Select the next newest command
    cur_list_el = cur_list_el->prev;  
    copy_line_to_current(cur_list_el);
  }

}

//=============================================================
//                Line Editor Functions
// These functions handle the different commands for the line
// editor such as backspace, del, etc. 
//=============================================================

void read_line_print_usage()
{
  char * usage = "\n"
    " ctrl-?       Print usage\n"
    " backspace    Deletes the character to the left of the cursor\n"
    " delete       Deletes the character to the right of the cursor\n"
    " up arrow     See previous command in the history\n"
    " down arrow   See next command in the history\n"
    " home         Go to the begining of the line\n"
    " end          Go to the end of the line\n"
    ;

  write(1, usage, strlen(usage));
}

/* write a character to the terminal and the current line's buffer */
void write_ch(char ch) {
  write(1, & ch, 1);
  cur_line.text[cursor_pos] = ch;

  if(cursor_pos == cur_line.length)
    cur_line.length++;

  cursor_pos++;
}

/* insert a charcter between the characters to the left and the right of the
 * cursor position
 */
void insert_ch(char ch) {
  if(cursor_pos == cur_line.length) {
    write_ch(ch);
  }
  else {
    int old_len = cur_line.length;
    int start = cursor_pos;

    char buff[MAX_BUFFER_LINE];
    strncpy(buff, cur_line.text, cur_line.length);

    write_ch(ch);

    for(int i = start; i < old_len; i++) {
      write_ch(buff[i]);
    }
    for(int i = start; i < old_len; i++) {
      cursor_left();
    }
  }
}

/* write a character just to the terminal */
void echo_ch(char ch) {
  write(1, &ch, 1);
}

/* move the cursor to the left one character, if possible */
void cursor_left(void) {
  if(cursor_pos > 0) {
    echo_ch(8);
    cursor_pos--;
  }
}

/* move the cursor to the right one character, if possible */
void cursor_right(void) {
  if(cursor_pos < cur_line.length) {
    echo_ch(cur_line.text[cursor_pos]);
    cursor_pos++;
  }
}

/* shifts the characters after the cursor one character to the left */
void shift_chars_left(void) {
  char buff[MAX_BUFFER_LINE];
  char* start = cur_line.text + cursor_pos;
  size_t len = cur_line.length - cursor_pos;

  //backup the buffer
  strncpy(buff, start, cur_line.length);

  //rewrite the buffer backup one character to the left
  cursor_left();
  for(size_t i = 0; i < len; i++)
    write_ch(buff[i]);

  //overwrite the last character in old line
  write_ch(' ');

  //return the cursor the previous position
  for(size_t i = 0; i <= len; i++)
    cursor_left();

  cur_line.length--;
}

/* deletes the character before the cursor. */
void backspace(void) {
  //it is not possible to backspace the first character in the line
  if(cursor_pos == 0) return;

  //if the cursor is at the end of the line, overwrite the last character with
  //a space and decrement the cursor. Else, shift the characters after the
  //left by one character
  if(cursor_pos == cur_line.length) {
    cursor_left();
    write_ch(' ');
    cursor_left();
    cur_line.length--;
  }
  else if (cursor_pos > 0) {
    shift_chars_left();
  }
}

/* deletes the character to the right of the cursor, if possible */
void delete(void) {
  if(cursor_pos != cur_line.length) {
    cursor_right();
    backspace();
  }
}

/* returns the cursor to the beginning of the line */
void home(void) {
  while(cursor_pos > 0) {
    cursor_left();
  }
}

/* returns the cursor to the end of the line */
void end(void) {
  while(cursor_pos < cur_line.length) {
    cursor_right();
  }
}

//=============================================================
//                Line Editor Main Loop
// Loops until the user presses enter, or reach the maximum 
// line length. Special characters are read for the line
// editor functions.
//=============================================================
char * read_line() {
  if(!history_initialized)
    init_history();

  cur_list_el = NULL;

  // Set terminal in raw mode
  tty_raw_mode();

  cur_line.length = 0;
  cursor_pos = 0;

  // Read one line until enter is typed
  while (1) {

    // Read one character in raw mode.
    char ch;
    read(0, &ch, 1);

    if (ch>=32 && ch != 127) {
      // It is a printable character. 
      //write_ch(ch);
      insert_ch(ch);

      // If max number of character reached return.
      if (cur_line.length==MAX_BUFFER_LINE-2) break; 
    }
    else if (ch==10) {
      // <Enter> was typed. Return line and print a newline
      write(1,&ch,1);

      break;
    }
    else if (ch == 31) {
      // ctrl-?
      read_line_print_usage();
      cur_line.text[0]=0;
      break;
    }
    else if (ch == 8 || ch == 127) { // ctrl-H or backspace key
      // <backspace> was typed. Remove previous character read.
      backspace();
    }
    else if (ch == 4) { // ctrl-D was pressed
      delete();
    }
    else if (ch == 1) { // ctrl-A was pressed
      home();
    }
    else if (ch == 5) { // ctrl-E was pressed
      end();
    }
    else if (ch==27) {
      // Escape sequence. Read two chars more
      char ch1; 
      char ch2;
      read(0, &ch1, 1);
      read(0, &ch2, 1);

      if(ch1 == 91 && ch2==51) { //delete key was pressed
        delete();
        char ch3;         //eat the tilde that follows this key
        read(0, &ch3, 1);
      }
      else if (ch1 == 91 && ch2 == 49) { //home key was pressed
        home();
        char ch3;         //eat the tilde
        read(0, &ch3, 1);
      }
      else if (ch1 == 91 && ch2 == 52) { //end key was pressed
        end();
        char ch3;         //eat the tilde
        read(0, &ch3, 1);
      }
      else if (ch1==91 && ch2==65) { //up arrow
        select_next_history_entry();
      }
      else if (ch1 == 91 && ch2 == 66) { //down arrow
        select_prev_history_entry();
      }
      else if(ch1 == 91 && ch2 == 68) { //left arrow
        cursor_left();
      }
      else if (ch1 == 91 && ch2 == 67) { // right arrow
        cursor_right();
      }
    }

  }
  
  //push the line into the history, unless it is empty
  if(cur_line.length > 0)
    push_cur_history_line();

  // Add eol and null char at the end of string
  cur_line.text[cur_line.length] = 10;
  cur_line.length++;
  cur_line.text[cur_line.length] = 0;

  tty_reset();

  return cur_line.text;
}

