
    #include "text.h"
    #include "error.h"
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #define LEN_BUF_ALLOC_STEP 50


    #pragma warning(disable:4996)

    struct text_to_display
    {
        char *strings;  // text itself
        size_t *str_end_pos; // string ending indexes array
        size_t str_count; // num of strings in text
        size_t str_len_max; // max string length
    };

    size_t GetStrCount(text_to_display* t)
    {
      if (t == NULL)
        return 0;
      return t->str_count;
    }


    size_t GetMaxLen(text_to_display* t)
    {
      if (t == NULL)
        return 0;
      return t->str_len_max;
    }

    size_t GetTextLen(text_to_display* t)
    {
      if (t == NULL || t->str_count == 0)
        return 0;
      return t->str_end_pos[t->str_count-1];
    }

    size_t GetTextLenUpToStr(text_to_display* t, size_t str)
    {
      if (t == NULL || str > t->str_count || str < 0)
        return 0;
      return str == 0 ? 0 : t->str_end_pos[str-1];
    }



    int InitTxt (char *raw_text, size_t text_len, text_to_display** ptr_to_init)
    {
        size_t i;
        size_t max_len = 0;
        size_t curr_str = 0;

        //checking for input data
        if (ptr_to_init == NULL || raw_text == NULL)
            return MEM_ERR_NULLPTR_DEREFERENCING;

        if (*ptr_to_init != NULL)
            return MEM_ERR_HANGING_POINTER_OCCURED;

        text_to_display res = {NULL, NULL, 0, 0};

        //allocating memory for the text data
        res.strings = malloc (text_len);
        res.str_end_pos = malloc (sizeof(size_t) * LEN_BUF_ALLOC_STEP);

        if (res.str_end_pos == NULL || res.strings == NULL)
        {
            return MEM_ERR_FAILED_TO_ALLOCATE;
        }

        //copying the text
        //memcpy_s(res.strings, text_len, raw_text, text_len);
        memcpy(res.strings, raw_text, text_len);


        //mapping string endings array
        for (i=0; i < text_len - curr_str; i++)
        {
            // we are only looking for the strings endings
            if (raw_text[i] == '\n')
            {

                //refreshing max_len value
                if (curr_str == 0)
                    {
                        max_len=i;
                        }
                else
                    {
                        if(i-res.str_end_pos[curr_str-1] > max_len)
                        {
                            max_len=i-res.str_end_pos[curr_str-1];
                        }
                    }

               //setting end for the current string and swapping to the next
               res.str_end_pos[curr_str] = i;

               //making sure to reallocate memory for str_end array if there is any text left
               if(i != text_len-1)
               {
                    curr_str++;
                    if (curr_str % LEN_BUF_ALLOC_STEP == 0)
                    {
                        size_t * rlc_strend = realloc (res.str_end_pos, sizeof(size_t)*(LEN_BUF_ALLOC_STEP + curr_str));
                        if (rlc_strend == NULL)
                        {
                            free(res.str_end_pos);
                            free(res.strings);
                            res.strings = NULL;
                            res.str_end_pos = NULL;
                            return MEM_ERR_FAILED_TO_ALLOCATE;
                        }
                        else
                        {
                            res.str_end_pos=rlc_strend;
                        }
                    }
               }
            }
        }

        //we know where the last string ends whether it ends with \n or not
        res.str_end_pos[curr_str] = text_len-curr_str-1;
        res.str_count = curr_str+1;

        // setting max_len parameter according to the last string length
        if (curr_str != 0 && max_len < i - res.str_end_pos[curr_str-1])
        {
            max_len = i-res.str_end_pos[curr_str-1];
        }
        res.str_len_max = max_len;

        *ptr_to_init = malloc(sizeof(text_to_display));
        **ptr_to_init=res;
        return DONE;
    }

    void DestroyText (text_to_display **t)
    {
        if (t != NULL && *t != NULL)
        {
            if ((*t)->strings != NULL)
            {
                free((*t)->strings);
            }
            free((*t)->str_end_pos);
            (*t)->str_end_pos = NULL;
            (*t)->strings = NULL;
            (*t)->str_count=0;
            (*t)->str_len_max=0;
            (*t) = NULL;
        }
    }

    size_t TextStrLen(text_to_display *t, size_t str_num)
    {
        if (t == NULL)
          return 0;
        return t->str_end_pos[str_num] - (str_num == 0 ? 0 : t->str_end_pos[str_num-1]);
    }

    int GetTextString (text_to_display *t, char** read_only_str_ptr, size_t str_num, size_t first_symbol)
    {
        if (t==NULL || t->str_count<=str_num || TextStrLen(t, str_num)<=first_symbol)
        {
            *read_only_str_ptr = NULL;
            return 0;
        }

        *read_only_str_ptr = &(t->strings[(str_num == 0 ? 0 : t->str_end_pos[str_num-1]+1) + first_symbol]);
        return TextStrLen(t, str_num)-first_symbol;
    }

    size_t ShiftStrBeginPos (text_to_display *t, size_t *str_num, size_t *first_symbol, size_t step, size_t times, int direction)
    {
        if (t == NULL)
          return 0;

        size_t i = 0;
        size_t curr_str = *str_num, curr_pos = *first_symbol + (curr_str == 0 ? 0: t->str_end_pos[curr_str-1] + 1);
        if (direction == FORWARD__)
        {
          for (i = 0; i < times; i++)
          {
            curr_pos += step;
            if (curr_pos >= t->str_end_pos[curr_str])
            {
              if (curr_str == t->str_count - 1)
              {
                *str_num = t->str_count - 1;
                *first_symbol = (step > TextStrLen(t, *str_num) ? 0 : TextStrLen(t, *str_num) - step);
                return i;
              }
              else
              {
                curr_pos = t->str_end_pos[curr_str] + 1;
                curr_str++;
              }
            }
          }
        }
        if (direction == BACKWARD__)
        {
          for (i = 0; i < times; i++)
          {
            if (curr_pos < step || (curr_str > 0 && curr_pos < step + t->str_end_pos[curr_str - 1]))
            {
              if (curr_str == 0)
              {
                *str_num = 0;
                *first_symbol = 0;
                return i;
              }
              else
              {
                curr_str--;
                if (step >= TextStrLen(t, curr_str))
                  curr_pos = (curr_str == 0 ? 0 : t->str_end_pos[curr_str - 1] + 1);
                else
                  curr_pos = (curr_str == 0 ? 0 : t->str_end_pos[curr_str - 1] + 1) + step * (TextStrLen(t, curr_str) / step);
              }
            }
            else
            {
              curr_pos -= step;
            }
          }
        }
        *str_num = curr_str;
        *first_symbol = curr_pos - (curr_str == 0 ? 0: t->str_end_pos[curr_str-1] + 1);
        return i;
    }
