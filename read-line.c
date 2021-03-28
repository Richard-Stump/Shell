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

//=============================================================
//                   Line history
// Command history is stored in a linked list. The head of the
// list is the most recent command, and the tail is the first
// command entered. 
//=============================================================

typedef struct history_s {
  struct history_s* next;
  struct history_s* prev;
  size_t            line_length;
  char              line[MAX_BUFFER_LINE];
} history_t;

history_t* history_head = NULL; //head of the history list
history_t* history_tail = NULL; //tail of the history list
history_t* cur_history = NULL;  //the current history_t being showed, or NULL
                                //for the input of a new command

void push_current_line(void) {
  history_t* new_history = malloc(sizeof(history_t));
  strncpy(new_history->line, line_buffer, line_length);
  new_history->line_length = line_length;
  new_history->next = history_head;
  new_history->prev = NULL;

  if (history_head == NULL) {
    history_head = new_history;
    history_tail = new_history;
  }
  else {
    history_head->prev = new_history;
    history_head = new_history;
  }
}

void d_print_history_list(void)
{
  printf("History List\n  ");
  history_t* rover = history_head;
  while(rover != NULL) {
    for(int i = 0; i < rover->line_length; i++) {
      printf("%c", rover->line[i]);
    }

    rover = rover->next;
    printf("\n  ");
  }
}

void show_next_history(void)
{
  size_t old_length;

  if(cur_history == NULL) {
    cur_history = history_head;
    old_length = line_length;
  }
  else if(cur_history->next != NULL) {
    old_length = cur_history->line_length;
    cur_history = cur_history->next;
  }
  else {
    return;
  }

  //go to the start of the line
  for(int i = cursor_pos; i > 0; i--) {
    cursor_left();
  }
  //write the text for the new command to show
  for(int i = 0; i < cur_history->line_length; i++) {
    echo_ch(cur_history->line[i]);
  }
  //write spaces for the rest of the line
  for(int i = cur_history->line_length; i < old_length; i++) {
    echo_ch(' ');
  }
  for(int i = cur_history->line_length; i < old_length; i++) {
    echo_ch(8);
  }

  cursor_pos = cur_history->line_length;
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
  line_buffer[cursor_pos] = ch;

  if(cursor_pos == line_length)
    line_length++;
  
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
  if(cursor_pos < line_length) {
    echo_ch(line_buffer[cursor_pos]);
    cursor_pos++;
  }
}

void shift_chars_left(void)
{
  char buff[MAX_BUFFER_LINE];
  char* start = line_buffer + cursor_pos;
  size_t len = line_length - cursor_pos;

  strncpy(buff, start, line_length);

  cursor_left();
  for(size_t i = 0; i < len; i++)
    write_ch(buff[i]);

  write_ch(' ');

  for(size_t i = 0; i <= len; i++) {
    cursor_left();
  }

  line_length--;
}

void backspace(void) {
  if(cursor_pos == 0) 
    return;

  if(cursor_pos == line_length) {
    cursor_left();
    write_ch(' ');
    cursor_left();
    line_length--;
  } 
  else if (cursor_pos > 0) {
    shift_chars_left();
  }
}

void delete(void) {
  if(cursor_pos != line_length) {
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
  while(cursor_pos < line_length) {
    cursor_right();
  }
}

/* 
 * Input a line with some basic editing.
 */
char * read_line() {

  // Set terminal in raw mode
  tty_raw_mode();

  line_length = 0;
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
      if (line_length==MAX_BUFFER_LINE-2) break; 

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
      line_buffer[0]=0;
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
        show_next_history();
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

  push_current_line();

  // Add eol and null char at the end of string
  line_buffer[line_length]=10;
  line_length++;
  line_buffer[line_length]=0;

  tty_reset();

  return line_buffer;
}

