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

extern void tty_raw_mode(void);
extern void tty_reset(void);

// Buffer where line is stored
int line_length;
int cursor_pos;
char line_buffer[MAX_BUFFER_LINE];

// Simple history array
// This history does not change. 
// Yours have to be updated.
/*
int history_index = 0;
char * history [] = {
  "ls -al | grep x", 
  "ps -e",
  "cat read-line-example.c",
  "vi hello.c",
  "make",
  "ls -al | grep xxx | grep yyy"
};
int history_length = sizeof(history)/sizeof(char *);
*/

//forward declarations
void cursor_right(void);
void cursor_left(void);
void echo_ch(char ch);
void write_ch(char ch);

//=============================================================
//                   Line history
// Command history is stored in a linked list. The head of the
// list is the command that the user is currently inputting,
// and the last one in the list is the first command the user
// ever typed.
//=============================================================

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
bool history_initialized = false;

void init_history(void) {
  cur_line.next = NULL;
  cur_line.prev = NULL;
  cur_line.length = 0;
  
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

void copy_line_to_current(line_t* src) {
  cur_line.length = src->length;

  while(cursor_pos > 0) {
    cursor_left();
  }

  for(int i = 0; i < src->length; i++) {
    write_ch(src->text[i]);
  }
}

void select_next_history_entry(void) {
  if(cur_list_el == NULL) {
    cur_list_el = line_list_head;
  }
  else if(cur_list_el->next != NULL) {
    cur_list_el = cur_list_el->next;
  }
  else {
    return;
  }

  copy_line_to_current(cur_list_el);
}

void d_print_history_list(void)
{
  printf("History List\n  ");
  line_t* rover = line_list_head;
  while(rover != NULL) {
    for(int i = 0; i < rover->length; i++) {
      printf("%c", rover->text[i]);
    }

    rover = rover->next;
    printf("\n  ");
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
    " Backspace    Deletes last character\n"
    " up arrow     See last command in the history\n";

  write(1, usage, strlen(usage));
}

void write_ch(char ch) {
  write(1, & ch, 1);
  cur_line.text[cursor_pos] = ch;

  
  if(cursor_pos == cur_line.length)
    cur_line.length++;

  cursor_pos++;
}

void echo_ch(char ch) {
  write(1, &ch, 1);
}

void cursor_left(void) {
  if(cursor_pos > 0) {
    echo_ch(8);
    cursor_pos--;
  }
}

void cursor_right(void) {
  if(cursor_pos < cur_line.length) {
    echo_ch(cur_line.text[cursor_pos]);
    cursor_pos++;
  }
}

void shift_chars_left(void) {
  char buff[MAX_BUFFER_LINE];
  char* start = cur_line.text + cursor_pos;
  size_t len = cur_line.length - cursor_pos;

  strncpy(buff, start, cur_line.length);

  cursor_left();
  for(size_t i = 0; i < len; i++)
    write_ch(buff[i]);

  write_ch(' ');

  for(size_t i = 0; i <= len; i++)
    cursor_left();

  cur_line.length--;
}

void backspace(void) {
  if(cursor_pos == 0) return;

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

void delete(void) {
  if(cursor_pos != cur_line.length) {
    cursor_right();
    backspace();
  }
}

void home(void) {
  while(cursor_pos > 0) {
    cursor_left();
  }
}

void end(void) {
  while(cursor_pos < cur_line.length) {
    cursor_right();
  }
}

/* 
 * Input a line with some basic editing.
 */
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
      write_ch(ch);

      // Do echo
      //write(1,&ch,1);

      // If max number of character reached return.
      if (cur_line.length==MAX_BUFFER_LINE-2) break; 

      // add char to buffer.
      //line_buffer[line_length]=ch;
      //line_length++;
      //cursor_pos++;
    }
    else if (ch==10) {
      // <Enter> was typed. Return line
      
      // Print newline
      write(1,&ch,1);

      break;
    }
    else if (ch == 31) {
      // ctrl-?
      read_line_print_usage();
      cur_line.text[0]=0;
      break;
    }
    else if (ch == 8 || ch == 127) {
      // <backspace> was typed. Remove previous character read.
      backspace();
    }
    else if (ch == 4) {
      delete();
    }
    else if (ch == 1) {
      home();
    }
    else if (ch == 5) {
      end();
    }
    else if (ch==27) {
      // Escape sequence. Read two chars more
      //
      // HINT: Use the program "keyboard-example" to
      // see the ascii code for the different chars typed.
      //
      char ch1; 
      char ch2;
      read(0, &ch1, 1);
      read(0, &ch2, 1);
      if(ch1 == 91 && ch2==51) {
        delete();
        char ch3;
        read(0, &ch3, 1);
      }
      else if (ch1 == 91 && ch2 == 49) {
        home();
        char ch3;
        read(0, &ch3, 1);
      }
      else if (ch1 == 91 && ch2 == 52) {
        end();
        char ch3;
        read(0, &ch3, 1);
      }
      else if (ch1==91 && ch2==65) {
        // Up arrow. Print next line in history.
        select_next_history_entry();
      /*
        // Erase old line
        // Print backspaces
        int i = 0;
        for (i =0; i < line_length; i++) {
          ch = 8;
          write(1,&ch,1);
        }

        // Print spaces on top
        for (i =0; i < line_length; i++) {
          ch = ' ';
          write(1,&ch,1);
        }

        // Print backspaces
        for (i =0; i < line_length; i++) {
          ch = 8;
          write(1,&ch,1);
        }	

        // Copy line from history
        strcpy(line_buffer, history[history_index]);
        line_length = strlen(line_buffer);
        history_index=(history_index+1)%history_length;

        // echo line
        write(1, line_buffer, line_length);
        */
      }
      else if(ch1 == 91 && ch2 == 68) {
        //go left one char
        cursor_left();
      }
      else if (ch1 == 91 && ch2 == 67) {
        cursor_right();
      }
      
    }

  }

  // Add eol and null char at the end of string
  /*
  line_buffer[line_length]=10;
  line_length++;
  line_buffer[line_length]=0;
  */

  push_cur_history_line();

  cur_line.text[cur_line.length] = 10;
  cur_line.length++;
  cur_line.text[cur_line.length] = 0;

  tty_reset();

  return cur_line.text;
}

