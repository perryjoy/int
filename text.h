
#ifndef TEXT_H_INCLUDED
#define TEXT_H_INCLUDED
#include <windows.h>

enum scroll_direction // it says for itself
{
    FORWARD__ = 1,
    BACKWARD__ = -1
};

typedef struct text_to_display text_to_display;



/*Inits text_to_display pointer with structure with given text of fixed length
NB: ptr_to_init MUST BE adress of the valid ptr with value set to NULL
Return value is DONE constant or an error code*/
int InitTxt (char *raw_text, size_t text_len, text_to_display** ptr_to_init);

/*frees the memory allocated for the text_to_display* inner data and set ptr value to NULL*/
void DestroyText (text_to_display **t);

/*Gets a string of fixed length starting with the symbol with the number of first_symbol.
Sets *read_only_str_ptr value for the beginning of this string. If an error occurs, sets NULL instead
Returns number of symbols which are safe to read
NB: Value of the *read_only_str_ptr CANNOT BE CHANGED outside this module*/
int GetTextString (text_to_display *t, char** read_only_str_ptr, size_t str_num, size_t first_symbol);

/*Changes the position of the first symbol of the first string  (<str_num> and <first_symbol> params) to be shown
in order to execute shift of <step> symbols <times> times*/
size_t ShiftStrBeginPos (text_to_display *t, size_t *str_num, size_t *first_symbol, size_t step, size_t times, int direction);


size_t GetMaxLen(text_to_display* t); // returns the length of the longest string
size_t GetStrCount(text_to_display* t); // returns number of strings in text
size_t GetTextLen(text_to_display* t); // returns text length
size_t GetTextLenUpToStr(text_to_display* t, size_t str); // returns text length from the beginning to the start of the chosen string

#endif // TEXT_H_INCLUDED
